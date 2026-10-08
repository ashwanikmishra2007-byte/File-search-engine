#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace fs=std::filesystem;

class FileFinder {
    protected:
        std::string filename;
    public:
        FileFinder(const std::string& filename) {
            this->filename = filename;
        }
        std::vector<fs::path> findfiles();
        virtual std::vector<fs::path> getSearchRoots()=0; // make it pureely virtual every os specific class should have its own implementation
};

class WinFileFinder: public FileFinder{
    public:
    WinFileFinder(const std:: string &filename);
    std::vector<fs::path> getSearchRoots() override;
    void display(std::vector<fs::path> found_files){
        if(found_files.empty()){
            std::cout << "File not found." << std::endl;
        } else {
            std::cout << "File found at: " << std::endl;
            for(const auto& file : found_files){
                std::cout << file << std::endl;
            }
        }
    }
};

class LinuxFileFinder: public FileFinder{
    public:
    LinuxFileFinder(const std:: string &filename);
    std::vector<fs::path> getSearchRoots() override;
};

class MacFileFinder: public FileFinder{
    public:
    MacFileFinder(const std:: string &filename);
    std::vector<fs::path> getSearchRoots() override;
};