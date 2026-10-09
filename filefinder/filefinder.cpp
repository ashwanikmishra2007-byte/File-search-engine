#include <iostream>
#include <filesystem>
#include <string>
#include "filefinder.h"

namespace fs = std::filesystem;

WinFileFinder::WinFileFinder(const std::string &filename)
    : FileFinder(filename)
{
    this->filename = filename;
}

std::vector<fs::path> FileFinder::findfiles()
{
    std::vector<fs::path> found_files;
    std::vector<fs::path> search_roots = getSearchRoots(); // Get the search roots from the derived class
    std::error_code ec;                                    // To handle any potential errors during filesystem operations
    for (const auto &root : search_roots)
    {
        fs::recursive_directory_iterator dir_iter(root, fs::directory_options::skip_permission_denied, ec);
        fs::recursive_directory_iterator end_iter;

        if (ec)
        {
            std::cerr << "Error accessing directory: " << root << " - " << ec.message() << std::endl;
            continue; // Skip this root and continue with the next one
        }

        while (dir_iter != end_iter)
        {
            std::error_code type_ac; // To handle any potential errors during accessing the file type

            if (dir_iter->is_directory(type_ac) &&
                !type_ac &&
                dir_iter->path().filename() == "$Recycle.Bin")
            {
                dir_iter.disable_recursion_pending();
            } //Disabling the recursion before it goes into the $Recycle.Bin folder to avoid permission denied errors

            if (dir_iter->is_regular_file(type_ac) && !type_ac && dir_iter->path().filename() == filename)
            {
                found_files.push_back(dir_iter->path());
            } //If it is a regular file and the filename matches the one we are searching for, we add it to the found_files vector

            std::string current_path = dir_iter->path().string();
            std::cout << "Checking: " << dir_iter->path().string() << '\n';

            dir_iter.increment(ec); //Incrementing the iterator to move to the next file/directory, while also checking for errors during this operation

            if (ec)
            {
                std::cerr << "Iterator advancement failed.\n";
                std::cerr << "Previous entry: "
                          << current_path << '\n'; // Checking the previous entry to see where the error occurred in advancing the iterator
                std::cerr << "Error value: "
                          << ec.value() << '\n';
                std::cerr << "Error message: ["
                          << ec.message() << "]\n";

                break;
            }
        }
        std::cout << "Total matching files: "
                  << found_files.size() << '\n';
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
