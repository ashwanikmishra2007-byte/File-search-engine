#define _WIN32_WINNT 0x0602
#include <iostream>
#include <string>
#include <cstdio>
#include <windows.h>

int main()
{
    std::wstring path;                                  // wstring is wide strings
    std::wcout << L"Enter directory path" << std::endl; // L prefix makes it a literal
    std::getline(std::wcin, path);

    HANDLE handle = CreateFileW(                                                        /*The function returns a HANDLE,
                                                             which is a Windows object reference used to work with the opened directory.*/
                                path.c_str(),                                           // c_str() provides a pointer to the null-terminated wide-character sequence representing the path.
                                0,                                                      /*his specifies the desired access rights. Zero requests no specific data-access rights.
                                                                                    We only need to retrieve metadata about the directory, rather than read or modify its contents*/
                                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, /*These flags allow other processes to read, write, or delete the directory
                                or its associated entry while our handle is open, subject to Windows' normal sharing rules.*/
                                nullptr,                                                // This represents the optional security attributes. We do not need to supply custom security attributes, so we use nullptr.
                                OPEN_EXISTING,                                          // This tells Windows to open an existing directory rather than create a new object.
                                FILE_FLAG_BACKUP_SEMANTICS,                             /*This flag is essential here.
                                                            It allows CreateFileW() to open a directory handle. Without it, opening a directory this way generally fails.*/
                                nullptr                                                 // Optional tempelate handle which is not needed
    );

    if (handle == INVALID_HANDLE_VALUE)
    {
        std::wcerr << L"Failed to open directory. Error: "
                   << GetLastError() << L'\n';
        return 1;
    }

    FILE_ID_INFO info{}; /*ILE_ID_INFO is a Windows-defined structure that holds identity information about a file or directory.
The structure contains two important fields:
- VolumeSerialNumber — identifies the volume associated with the object.
- FileId — contains the file ID, which is 128 bits (16 bytes).
The {} initializes the structure to zero-initialized values before Windows fills it.*/

    if (!GetFileInformationByHandleEx(
            handle, FileIdInfo, &info, sizeof(info)))
    {
        /*handle — the directory we opened earlier.
        FileIdInfo — tells Windows which category of information we want: the volume serial number and file ID.
        &info — the memory address of our structure, where Windows will write the result. The & operator obtains the address of info.
        sizeof(info) — tells Windows the size of the structure in bytes.*/
        std::cerr << "Failed to retrieve identity. Error: "
                  << GetLastError() << '\n';
        CloseHandle(handle);
        return 1;
    }

    std::cout << "Volume serial number: "
              << info.VolumeSerialNumber << '\n';

    std::cout << "FileId";

    for (int i = 0; i < 16; i++)
    {
        printf("%02X", static_cast<unsigned char>(info.FileId.Identifier[i])); // Converts the byte to an unsigned character type so it can be printed as a nonnegative numeric value.
    }
    std::cout << std::endl;

    CloseHandle(handle);
    return 0;
}