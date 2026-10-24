#pragma once
#include <unordered_map>
#include <filesystem>
#include <windows.h>
#include <cstring>
#include <cstddef>
#include "filefinder.h"
#include <unordered_set>

namespace fs = std::filesystem;

struct Filesysteminfo
{
    DWORD volumenumber;
    FILE_ID_128 fileId;

    bool operator==(const Filesysteminfo &other) const
    { // Overloaded operator to compare volumenumber and 128 bit array of fileid
        return volumenumber == other.volumenumber &&
               std::memcmp( // Compare the 128 bit fileid identifier of two objects
                   /*memcmp() returns an integer indicating the comparison result:
   - 0 if all compared bytes are equal.
   - A negative integer if the first differing byte in the first memory region is smaller.
   - A positive integer if the first differing byte in the first memory region is larger.*/
                   fileId.Identifier,
                   other.fileId.Identifier,
                   sizeof(fileId.Identifier)) == 0;  //Compare only the array of 16 bytes not the entire structure because Windows defines FILE_ID_128 essentially as a structure containing a 16-byte array:
    }
};

//std::size_t is an unsigned integer type provided by C++ that is capable of representing the size of any object in bytes.
auto hashfunction = [] (const Filesysteminfo &info)-> std::size_t{
    std::size_t hashvalue= std::hash<DWORD>{}(info.volumenumber);  //CPP library tempelate takes a value and converts it into a number called a hash value.
    for(unsigned char byte: info.fileId.Identifier){
        //std::hash<unsigned char>{}(byte) takes the value stored in byte and calculates a hash value from it.
        hashvalue= hashvalue^ (std::hash<unsigned char> {}(byte)+
        //This is a hexadecimal integer constant used by a common hash-combining formula.

//It isn't part of your directory identity. It helps mix the different components together.
/*Those shift amounts come from a commonly used hash-combining formula. They are heuristic choices intended to mix bits, not mathematical guarantees of a collision-free hash.*/
        0x9e3779b9+(hashvalue<<6)+(hashvalue>>2));
    }
    return hashvalue;
};

class WinFileFinder : public FileFinder
{
protected:
    Filesysteminfo directory;
    /*decltype(hashfunction) is the type of the lambda object, used as the map's hash-function type.
      {0, hashfunction} constructs the unordered_map with zero buckets and a copy of the hash function. */
    inline static std::unordered_map<Filesysteminfo, fs::path, decltype(hashfunction)> filemap{0, hashfunction};

public:
    std::vector<fs::path> getSearchRoots() override;

    void findfiles();

    bool getfilesysteminfo(const fs::path &path, Filesysteminfo &identity);
    std::unordered_set<fs::path> getfiles();
    void buildhashmap();

    void display(const std::vector<fs::path> &found_files)
    {
        if (found_files.empty())
        {
            std::cout << "File not found." << std::endl;
        }
        else
        {
            std::cout << "File found at:" << std::endl;
            for (const auto &file : found_files)
            {
                std::cout << file << std::endl;
            }
        }
    }
};
