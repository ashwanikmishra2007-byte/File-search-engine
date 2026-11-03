#include <vector>
#include <filesystem>
#include <iostream>
#include <unordered_set>
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
    std::vector<Filerecord> filerecords; // Vector to store extracted Filerecords

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

        if(bytesReturned < sizeof(DWORDLONG))
        {
            throw std::runtime_error("Unexpected bytes returned from DeviceIoControl");
        }

        DWORDLONG nextreference = 0;

        std::memcpy(&nextreference, buffer.data(), sizeof(nextreference));

        mft.StartFileReferenceNumber = nextreference;

        size_t offset = sizeof(nextreference);

        while (offset < bytesReturned)
        {
            if(bytesReturned - offset < offsetof(USN_RECORD, FileName)){   /*This line is a boundary check. 
                Its purpose is to ensure that enough bytes remain in our buffer to safely read the fixed portion of a USN_RECORD*/
                
                throw std::runtime_error("Buffer does not contain enough data for a complete USN_RECORD");
            }
            auto record = reinterpret_cast<const USN_RECORD *>(buffer.data() + offset); // Convert the buffer data to a USN_RECORD pointer
            if(record->RecordLength<offsetof(USN_RECORD, FileName) || record->RecordLength>bytesReturned-offset){  // Check if the RecordLength is valid
                throw std::runtime_error("Invalid RecordLength in USN_RECORD");
            }

            Filerecord result;
            Extractstatus status = extract(record, result);
            if (status == Extractstatus::Success){
                filerecords.push_back(std::move(result)); // Store the extracted Filerecord
            }
            offset += record->RecordLength;                                             // Move the offset to the next record
        }
    }
}

Extractstatus WinFilefinder::extract(const USN_RECORD *record, Filerecord &result)
{
    constexpr std::size_t minimum_v2_size = sizeof(*record);

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

    if (record->FileNameOffset > record->RecordLength ||
        record->FileNameLength > record->RecordLength - record->FileNameOffset)
    {
        return Extractstatus::Invalidfilenamebounds;
    }

    if (record->FileNameLength % alignof(WCHAR) != 0)
    {
        return Extractstatus::Invalidfilenamelength;
    }

    if (record->FileNameLength % sizeof(WCHAR) != 0)
    {
        return Extractstatus::Invalidfilenamelength;
    }

    WCHAR *filenameptr = (WCHAR *)((BYTE *)record + record->FileNameOffset);
    size_t filenamelength = record->FileNameLength / sizeof(WCHAR);

    std::wstring filename(filenameptr, filenamelength);
    result.filename = filename;
    return Extractstatus::Success;
}