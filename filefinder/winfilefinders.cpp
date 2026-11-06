#include <iostream>
#include "winfilefinder.h"
#include "ntfsfilerecord.h"
#include <system_error>
#include <cstring>

WinFilefinder::WinFilefinder(const std::wstring &volumename)
    : volumename(volumename)
{
    // Open the volume for reading.
    volumehandle = CreateFileW(
        volumename.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (volumehandle == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        throw std::system_error(error, std::system_category(), "Failed to open volume");
    }
}

void WinFilefinder::enumerate()
{
    MFT_ENUM_DATA mft = {};

    mft.StartFileReferenceNumber = 0; // Start from the beginning of the MFT
    mft.LowUsn = 0;                   // Start from the beginning of the USN journal
    mft.HighUsn = MAXLONGLONG;        // No upper limit on the USN journal

    constexpr DWORD bufferSize = 1024 * 1024; // 1 MiB buffer
    std::vector<BYTE> buffer(bufferSize);

    while (true)
    {
        DWORD bytesReturned = 0;
        BOOL success = DeviceIoControl(
            volumehandle,
            FSCTL_ENUM_USN_DATA,
            &mft,
            sizeof(mft),
            buffer.data(),
            buffer.size(),
            &bytesReturned,
            NULL);
        if (!success)
        {
            DWORD error = GetLastError();
            if (error == ERROR_HANDLE_EOF)
            {
                // Reached the end of the enumeration.
                break;
            }
            throw std::system_error(error, std::system_category(), "DeviceIoControl failed");
        }

        if (bytesReturned < sizeof(DWORDLONG))
        {
            throw std::runtime_error("Unexpected bytes returned from DeviceIoControl");
        }

        DWORDLONG nextreference = 0;

        std::memcpy(&nextreference, buffer.data(), sizeof(nextreference));

        mft.StartFileReferenceNumber = nextreference;

        size_t offset = sizeof(nextreference);

        while (offset < bytesReturned)
        {
            if (bytesReturned - offset < offsetof(USN_RECORD, FileName))
            { /*This line is a boundary check.
Its purpose is to ensure that enough bytes remain in our buffer to safely read the fixed portion of a USN_RECORD*/

                throw std::runtime_error("Buffer does not contain enough data for a complete USN_RECORD");
            }
            auto record = reinterpret_cast<const USN_RECORD *>(buffer.data() + offset); // Convert the buffer data to a USN_RECORD pointer
            if (record->RecordLength < offsetof(USN_RECORD, FileName) || record->RecordLength > bytesReturned - offset)
            { // Check if the RecordLength is valid
                throw std::runtime_error("Invalid RecordLength in USN_RECORD");
            }

            Filerecord result;
            Extractstatus status = extract(record, result);
            if (status == Extractstatus::Success)
            {
                insert(result); // Store the extracted Filerecord
            }
            offset += record->RecordLength; // Move the offset to the next record
        }
    }
}

Extractstatus WinFilefinder::extract(const USN_RECORD *record, Filerecord &result)
{
    if (record == nullptr)
    {
        return Extractstatus::Nullrecord;
    }

    constexpr std::size_t minimum_v2_size = offsetof(USN_RECORD, FileName); // gives you the exact position where the filename begins.

    if (record->MajorVersion != 2 || record->MinorVersion != 0)
    { /*Different versions can have different layouts. For example,
version 3 uses 128-bit file reference identifiers, whereas our current Filerecord uses 64-bit references.*/
        return Extractstatus::Unsupportedversion;
    }

    if (record->RecordLength < minimum_v2_size)
    {
        return Extractstatus::Invalidrecordlength;
    }

    result.filereference = record->FileReferenceNumber;
    result.parentreference = record->ParentFileReferenceNumber;
    result.attributes = record->FileAttributes;

    if (record->FileNameOffset > record->RecordLength ||
        record->FileNameLength > record->RecordLength - record->FileNameOffset)
    {
        return Extractstatus::Invalidfilenamebounds;
    }

    if (record->FileNameLength % sizeof(WCHAR) != 0)
    {
        return Extractstatus::Invalidfilenamelength;
    }

    if (record->FileNameLength % sizeof(WCHAR) != 0)
    {
        return Extractstatus::Invalidfilenamelength;
    }

    const WCHAR *filenameptr = reinterpret_cast<const WCHAR *>(
        reinterpret_cast<const BYTE *>(record) + record->FileNameOffset);
    size_t filenamelength = record->FileNameLength / sizeof(WCHAR);

    std::wstring filename(filenameptr, filenamelength);
    result.filename = filename;
    return Extractstatus::Success;
}

void WinFilefinder::insert(Filerecord &record)
{
    fileindex[record.filename].push_back(std::move(record));
}

const auto &WinFilefinder::getFileIndex()
{
    return fileindex;
}

void WinFilefinder::recordIndexInsert(){
    recordindex.clear(); // Clear the existing record index before populating it
    Recordlocation recordlocation;
    for(const auto &entry : fileindex){
        const std::wstring &filename = entry.first;
        const std::vector<Filerecord> &records = entry.second;
        for(size_t i=0; i<records.size(); i++){
            recordlocation.filename = filename;
            recordlocation.index = i;
            recordindex[records[i].filereference] = recordlocation;
        }
    }
}

/*int main()
{
    try
    {
        WinFilefinder finder(L"\\\\.\\C:");

        std::cout << "Starting NTFS enumeration...\n";

        finder.enumerate();

        const auto &index = finder.getFileIndex();

        std::cout << "\nEnumeration completed!\n";

        std::cout << "Unique filenames: "
                  << index.size() << '\n';

        size_t count = 0;

        for (const auto &entry : index)
        {
            std::wcout << L"\nFilename: " << entry.first;

            std::wcout << L"\nMatching records: "
                       << entry.second.size() << '\n';

            for (const auto &record : entry.second)
            {
                std::wcout << L"  File reference: "
                           << record.filereference << '\n';

                std::wcout << L"  Parent reference: "
                           << record.parentreference << '\n';
            }

            count++;

            if (count == 10)
                break;
        }

        std::wstring searchname;
        do
        {
            std::wcout << L"\nEnter filename to search: ";
            std::getline(std::wcin, searchname);

            std::vector<Filerecord> results = finder.search(searchname);

            if (results.empty())
            {
                std::wcout << L"No matching files found.\n";
            }
            else
            {
                std::wcout << L"\nFound "
                           << results.size()
                           << L" matching records.\n";

                for (const auto &record : results)
                {
                    std::wcout << L"\nFilename: " << record.filename
                               << L"\nFile reference: " << record.filereference
                               << L"\nParent reference: " << record.parentreference
                               << L"\n";
                }
            }
        } while (true);
    }
    catch (const std::system_error &e)
    {
        std::cerr << "System error: " << e.what()
                  << "\nError code: " << e.code() << '\n';
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }

    return 0;
}*/