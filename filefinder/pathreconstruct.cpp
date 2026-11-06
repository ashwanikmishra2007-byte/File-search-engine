#include <iostream>
#include "winfilefinder.h"
#include "ntfsfilerecord.h"
#include <system_error>
#include <cstring>
#include <stack>
#include <unordered_set>


std::wstring WinFilefinder::pathreconstruct(uint64_t filerefencenumber){
    std::stack<std::wstring> path_components;
    // Implementation for path reconstruction
    uint64_t current_reference = filerefencenumber;
    std::wstring full_path= L"C:\\";
    std::unordered_set<uint64_t> visited_references; // To detect cycles

    auto it = recordindex.find((current_reference));
    if(it == recordindex.end()){
        std::cout<<"Record dont exist"<<std::endl;
        return L"";
    }

    Recordlocation location= it->second;
    Filerecord current = fileindex.at(location.filename).at(location.index);
    const Filerecord *current_record = &current;

    while((current_record->filereference &  0x0000FFFFFFFFFFFFULL)!=5){  /*current_record->filereference gives the full 64-bit file reference.
        & 0x0000FFFFFFFFFFFFULL masks out the upper 16 bits (sequence number).*/
        if(visited_references.find(current_reference) != visited_references.end()){
            std::cout<<"Cycle detected in path reconstruction"<<std::endl;
            return L"";
        }
        visited_references.insert(current_record->filereference);
        path_components.push(current_record->filename);
        current_reference= current_record->parentreference;
        it= recordindex.find(current_reference);
        if(it==recordindex.end()){
            std::cout<<"Parent record dont exist"<<std::endl;
            return L"";
        }
        location= it->second;
        current= fileindex.at(location.filename).at(location.index);
        current_record= &current;
    }

    while(!path_components.empty()){
        full_path+= path_components.top();
        path_components.pop();
        if(!path_components.empty()){  //So the condition simply ensures that we append a separator only when another component follows.
            full_path+= L"\\";
        }
    }
    return full_path;
}

std::vector<std::wstring> WinFilefinder::search(const std::wstring&filename){

    std::vector<std::wstring> results;
    auto it = fileindex.find(filename);
    if(it != fileindex.end()){
        const std::vector<Filerecord> &records= it->second;
        for(const auto &record: records){
            std::wstring full_path= pathreconstruct(record.filereference);
            if(!full_path.empty()){
                results.push_back(full_path);
            }
        }
    }
    return results;
}