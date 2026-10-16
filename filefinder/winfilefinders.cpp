#include<windows.h>
#include<vector>
#include<filesystem>
#include<iostream>
#include <unordered_set>
#include "filefinder.h"
#include "winfilefinder.h"


std::vector<fs::path> WinFileFinder::getSearchRoots(){
    std::vector<fs::path> root;    //Vector to store the search roots
    DWORD drives=GetLogicalDrives();   //Get all the logical drives on the system

    for(char drive='A'; drive<='Z'; drive++){
        if(drives&(1<<(drive-'A'))){
            std::string roots = std::string(1, drive) + ":\\";
            root.push_back(roots);
        }
    }
    return root;
}

/*int main(){
    WinFileFinder finder("example.txt"); // Replace "example.txt" with the filename you want to search for

    std::vector<fs::path> roots = finder.getSearchRoots();

    for(const fs::path& root : roots){
        std::cout << root << std::endl;
    }

}*/