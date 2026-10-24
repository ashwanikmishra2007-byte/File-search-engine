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

std::unordered_set<fs::path> WinFileFinder::getfiles()
{
    std::unordered_set<fs::path> files;
    std::unordered_set<Filesysteminfo, decltype(hashfunction)> visiteddir{0, hashfunction};
    std::vector<fs::path> root = getSearchRoots();
    std::error_code typeError;
    std::error_code ec;
    std::error_code incrementerror;

    for (const auto &path : root)
    {
        Filesysteminfo identity;

        if (getfilesysteminfo(path, identity))
        {
            visiteddir.insert(identity);
        }
    }

    while (!root.empty())
    {
        fs::path current = root.back();
        root.pop_back();
        fs::directory_iterator it(current, ec);
        fs::directory_iterator end;

        if (ec)
        { // If there is an error if the directory cannot be opened skip it
            ec.clear();
            continue;
        }

        while (it != end)
        {
            const fs::directory_entry &entry = *it;
            fs::path entrypath = entry.path();

            fs::file_status status = entry.symlink_status(typeError); // To check if it is a symbolic link
            if (!typeError && fs::is_directory(status))
            { // Type error iterators type cannot be inspected
                Filesysteminfo identity;
                if (getfilesysteminfo(entrypath, identity))
                {
                    auto [it, inserted] = visiteddir.insert(identity);
                    /*emplace() returns a pair containing an iterator and a Boolean.
            Variable	Meaning
            it	Iterator to the existing or newly inserted entry
            inserted	true if insertion succeeded; otherwise false
            We need inserted when we want to decide whether to traverse the directory.*/
                    if (inserted)
                    {
                        root.push_back(entrypath);
                    }
                }
            }
            else if (!typeError && fs::is_regular_file(status))
            {
                files.insert(entrypath);
            }
            typeError.clear();

            it.increment(incrementerror);

            if (incrementerror)
            {
                incrementerror.clear();
                break;
            }
        }
    }

    return files;
}

void WinFileFinder::buildhashmap()
{
    std::unordered_set<fs::path> allfiles = getfiles();

    for (const auto &file : allfiles)
    {
        Filesysteminfo identity;
        if (getfilesysteminfo(file, identity))
        {
            filemap.emplace(identity, file);
        }
    }
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
    // std::ve<fs::path> files = finder.getfiles();
    // std::cout << "Files found: " << files.size() << '\n';

    // for (const auto &file : files)
    {
        //    std::cout << file.string() << '\n';
    }
}