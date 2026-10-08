#include <iostream>
#include <filesystem>
#include <string>
#include "filefinder.h"

namespace fs = std::filesystem;

WinFileFinder::WinFileFinder(const std::string& filename)
    : FileFinder(filename)
{
    this->filename = filename;
}

std::vector<fs::path> FileFinder::findfiles()
{
    std::vector<fs::path> found_files;
    std::vector<fs::path> search_roots = getSearchRoots(); // Get the search roots from the derived class

    for(const auto& root : search_roots)
    {
        for (const auto &entry : fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied))  //skip the directories for which permission is denied
        { /*recursive search in the root directory and its subdirectories starting from the search root*/
            if (entry.is_regular_file() && entry.path().filename() == filename)
            {
                found_files.push_back(entry.path());
            }
            /*else
            {
                std::cout << "File not found in: " << entry.path().string() << std::endl;
            }*/
        }
    }
    return found_files;
}

int main()
{
    std::string filename;
    std::cout << "Enter the filename to search: ";
    std::cin >> filename;

    WinFileFinder finder(filename); // Create an instance of WinFileFinder with the provided filename
    finder.display(finder.findfiles());
    return 0;
}

/*#ifdef _WIN32 // To check which operating system is using it and create the object of that specific class
    filefind = new WinFileFinder(filename);
#elif defined(__linux__)
    filefind = new LinuxFileFinder(filename);
#elif defined(__APPLE__)
    filefind = new MacFileFinder(filename);
#endif*/

/*std::string filename;
std::cout << "Enter the filename to search: ";
std::cin >> filename;
std::cin.ignore();  // Ignore the newline character left in the input buffer
FileFinder finder(filename);
finder.findfiles();
return 0;*/
