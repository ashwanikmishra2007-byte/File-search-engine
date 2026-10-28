/*#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "filefinder.h"
#include "winfilefinder.h"

namespace fs = std::filesystem;*/

/*namespace std
{
    template <>
    struct hash<DirectoryInfo>
    {
        size_t operator()(const DirectoryInfo &info) const noexcept
        {
            const auto *bytes = reinterpret_cast<const unsigned char *>(&info);
            size_t hash = 14695981039346656037ull;

            for (std::size_t i = 0; i < sizeof(DirectoryInfo); ++i)
            {
                hash ^= static_cast<size_t>(bytes[i]);
                hash *= 1099511628211ull;
            }

            return hash;
        }
    };
}

FileFinder::FileFinder(const std::string &filename)
    : filename(filename)
{
}

WinFileFinder::WinFileFinder(const std::string &filename)
    : FileFinder(filename)
{
}

std::unordered_set<fs::path> FileFinder::findfiles()
{
    std::unordered_set<fs::path> found_files;
    std::vector<fs::path> search_roots = getSearchRoots();
    std::error_code ec;

    for (const auto &root : search_roots)
    {
        fs::recursive_directory_iterator dir_iter(root, fs::directory_options::skip_permission_denied, ec);
        fs::recursive_directory_iterator end_iter;

        if (ec)
        {
            std::cerr << "Error accessing directory: " << root << " - " << ec.message() << std::endl;
            continue;
        }

        while (dir_iter != end_iter)
        {
            std::error_code type_ac;

            if (dir_iter->is_directory(type_ac) &&
                !type_ac &&
                dir_iter->path().filename() == "$Recycle.Bin")
            {
                dir_iter.disable_recursion_pending();
            }

            if (dir_iter->is_regular_file(type_ac) && !type_ac && dir_iter->path().filename().string() == filename)
            {
                found_files.insert(dir_iter->path());
            }

            std::string current_path = dir_iter->path().string();

            dir_iter.increment(ec);

            if (ec)
            {
                std::cerr << "\nFilesystem traversal error\n";
                std::cerr << "Entry: " << current_path << '\n';
                std::cerr << "Error value: " << ec.value() << '\n';
                std::cerr << "Error category: "
                          << ec.category().name() << '\n';
                std::cerr << "Error message: "
                          << ec.message() << '\n';

                break;
            }
        }
    }

    return found_files;
}*/

/*int main()
{
    std::string filename;
    std::cout << "Enter the filename to search: ";
    std::cin >> filename;

    WinFileFinder finder(filename);
    finder.display(finder.findfiles());
    return 0;
}*/

/*#ifdef _WIN32
    filefind = new WinFileFinder(filename);
#elif defined(__linux__)
    filefind = new LinuxFileFinder(filename);
#elif defined(__APPLE__)
    filefind = new MacFileFinder(filename);
#endif*/

/*std::string filename;
std::cout << "Enter the filename to search: ";
std::cin >> filename;
std::cin.ignore();
FileFinder finder(filename);
finder.findfiles();
return 0;*/
