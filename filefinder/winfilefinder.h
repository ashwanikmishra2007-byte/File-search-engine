#pragma once
#include <unordered_map>
#include <filesystem>
#include <windows.h>
#include "filefinder.h"

namespace fs= std::filesystem;

struct DirectoryInfo
{
    DWORD volumenumber;
    FILE_ID_128 fileId;
};

class WinFileFinder : public FileFinder
{
    protected:
    //std::unordered_map<DirectoryInfo, fs::path> directories;

public:
    WinFileFinder(const std::string &filename);
    std::vector<fs::path> getSearchRoots() override;

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
