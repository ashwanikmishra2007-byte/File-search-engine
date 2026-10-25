#pragma once
#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

class FileFinder
{
protected:
    std::string filename;
    std::unordered_map<std::string, std::vector<fs::path>> filename;
public:
    std::unordered_set<fs::path> findfiles();
    virtual std::vector<fs::path> getSearchRoots() = 0; // Pure virtual: each OS-specific class must provide its own implementation
};

class LinuxFileFinder : public FileFinder
{
public:
    LinuxFileFinder(const std::string &filename);
    std::vector<fs::path> getSearchRoots() override;
};

class MacFileFinder : public FileFinder
{
public:
    MacFileFinder(const std::string &filename);
    std::vector<fs::path> getSearchRoots() override;
};