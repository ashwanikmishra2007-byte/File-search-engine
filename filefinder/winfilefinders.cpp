#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0602
#endif
#include <windows.h>
#include <vector>
#include <filesystem>
#include <iostream>
#include <unordered_set>
#include "winfilefinder.h"

std::vector<fs::path> WinFileFinder::getSearchRoots()
{
    std::vector<fs::path> root;        // Vector to store the search roots
    DWORD drives = GetLogicalDrives(); // Get all the logical drives on the system

    for (char drive = 'A'; drive <= 'Z'; drive++)
    {
        if (drives & (1 << (drive - 'A')))
        {
            std::string roots = std::string(1, drive) + ":\\";
            root.push_back(roots);
        }
    }
    return root;
}

bool WinFileFinder::getfilesysteminfo(const fs::path &path, Filesysteminfo &identity)
{
    HANDLE handle = CreateFileW(
        path.c_str(),
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS, // FILE_FLAG_BACKUP_SEMANTICS allows you to open directories as well as files
        nullptr);

    if (handle == INVALID_HANDLE_VALUE)
    { // If handle is invalid
        return false;
    }

    FILE_ID_INFO info{}; // Window defined structure for fileid and volume

    bool success = GetFileInformationByHandleEx(
        handle,
        FileIdInfo,
        &info,
        sizeof(info)); // Calling the function and storing the boolean value of true or false in success

    CloseHandle(handle);

    if (!success)
    {
        return false;
    }

    identity.volumenumber = info.VolumeSerialNumber;
    identity.fileId = info.FileId;

    return true;
}

void WinFileFinder::findfiles()
{

    auto drives = getSearchRoots(); // Retrieving all the logical drives present in the system
    Filesysteminfo identity;
    for (const auto &root : drives)
    {
        fs::recursive_directory_iterator dir_iter(root, fs::directory_options::skip_permission_denied);
        fs::recursive_directory_iterator end_iter;

        while (dir_iter != end_iter)
        {
            fs::path path = dir_iter->path();
            if (getfilesysteminfo(path, identity))
            {
                filemap.emplace(identity, path);
            }
            else
            {
                std::cout << "Could not get filesystem information";
            }
            std::error_code ec;

            dir_iter.increment(ec);

            if (ec)
            {
                std::cerr << "\nTraversal error at: "
                          << path.string()
                          << "\nError code: " << ec.value()
                          << "\nMessage: " << ec.message() << '\n';

                break; // Temporary: stop to isolate the failure.
            }
        }
    }
}

int main()
{
    WinFileFinder finder;
    finder.findfiles();
}