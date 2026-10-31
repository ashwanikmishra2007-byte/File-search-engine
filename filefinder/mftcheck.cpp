
#define _WIN32_WINNT 0x0602

#include <Windows.h>
#include <winioctl.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

// Compatible layout of the MFT enumeration request.
// Avoids depending on MFT_ENUM_DATA_V0 being declared.
struct MftEnumDataCompat
{
    DWORDLONG StartFileReferenceNumber;
    LONGLONG LowUsn;
    LONGLONG HighUsn;
};

// Header shared by USN records.
struct RecordHeader
{
    DWORD RecordLength;
    WORD MajorVersion;
    WORD MinorVersion;
};

// Compatibility definition for a Version 2 USN record.
struct UsnRecordV2Compat
{
    DWORD RecordLength;
    WORD MajorVersion;
    WORD MinorVersion;

    DWORDLONG FileReferenceNumber;
    DWORDLONG ParentFileReferenceNumber;

    LONGLONG Usn;
    LARGE_INTEGER TimeStamp;

    DWORD Reason;
    DWORD SourceInfo;
    DWORD SecurityId;
    DWORD FileAttributes;

    WORD FileNameLength;
    WORD FileNameOffset;
    WCHAR FileName[1];
};

// A 128-bit file identifier used by Version 3 records.
struct FileId128Compat
{
    BYTE Identifier[16];
};

// Compatibility definition for a Version 3 USN record.
struct UsnRecordV3Compat
{
    DWORD RecordLength;
    WORD MajorVersion;
    WORD MinorVersion;

    FileId128Compat FileReferenceNumber;
    FileId128Compat ParentFileReferenceNumber;

    LONGLONG Usn;
    LARGE_INTEGER TimeStamp;

    DWORD Reason;
    DWORD SourceInfo;
    DWORD SecurityId;
    DWORD FileAttributes;

    WORD FileNameLength;
    WORD FileNameOffset;
    WCHAR FileName[1];
};

// Read a 128-bit identifier without assuming it is an integer.
void printFileId128(const FileId128Compat& id)
{
    std::wcout << L"0x";

    for (int i = 15; i >= 0; --i)
    {
        std::wcout << std::hex
                   << std::setw(2)
                   << std::setfill(L'0')
                   << static_cast<unsigned int>(id.Identifier[i]);
    }

    std::wcout << std::dec << std::setfill(L' ') << L'\n';
}

// Validate the variable-length filename before accessing it.
bool isValidFileName(
    DWORD recordLength,
    WORD fileNameOffset,
    WORD fileNameLength,
    std::size_t fixedHeaderSize)
{
    if (fileNameOffset < fixedHeaderSize)
        return false;

    if (fileNameOffset > recordLength)
        return false;

    if (fileNameLength > recordLength - fileNameOffset)
        return false;

    if (fileNameLength % sizeof(WCHAR) != 0)
        return false;

    return true;
}

int main()
{
    // Open the C: volume.
    HANDLE volume = CreateFileW(
        L"\\\\.\\C:",
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (volume == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();

        std::wcerr << L"Failed to open C: volume. Error: "
                   << error << L'\n';

        return 1;
    }

    std::wcout << L"Volume opened successfully.\n";

    // Prepare the enumeration request.
    MftEnumDataCompat request{};

    request.StartFileReferenceNumber = 0;
    request.LowUsn = 0;
    request.HighUsn = MAXLONGLONG;

    // Allocate a 1 MiB output buffer.
    std::vector<std::uint8_t> buffer(1024 * 1024);

    DWORD bytesReturned = 0;

    // Ask NTFS to enumerate file records.
    BOOL success = DeviceIoControl(
        volume,
        FSCTL_ENUM_USN_DATA,
        &request,
        sizeof(request),
        buffer.data(),
        static_cast<DWORD>(buffer.size()),
        &bytesReturned,
        nullptr
    );

    if (!success)
    {
        DWORD error = GetLastError();

        std::wcerr << L"MFT enumeration failed. Error: "
                   << error << L'\n';

        CloseHandle(volume);
        return 1;
    }

    std::wcout << L"Bytes returned: "
               << bytesReturned << L'\n';

    // The first eight bytes contain the continuation value.
    if (bytesReturned < sizeof(DWORDLONG))
    {
        std::wcerr << L"Output does not contain a continuation value.\n";

        CloseHandle(volume);
        return 1;
    }

    DWORDLONG nextReference = 0;

    std::memcpy(
        &nextReference,
        buffer.data(),
        sizeof(nextReference)
    );

    std::wcout << L"Next starting reference: "
               << nextReference << L'\n';

    // Skip the continuation value and begin at the first record.
    std::size_t offset = sizeof(DWORDLONG);

    int displayed = 0;
    int parsedRecords = 0;

    // Display at most 20 records for this diagnostic test.
    while (offset <= bytesReturned &&
           bytesReturned - offset >= sizeof(RecordHeader) &&
           displayed < 20)
    {
        RecordHeader header{};

        // Copy the fixed record header from the current position.
        std::memcpy(
            &header,
            buffer.data() + offset,
            sizeof(header)
        );

        // Reject invalid or truncated records.
        if (header.RecordLength < sizeof(RecordHeader) ||
            header.RecordLength > bytesReturned - offset)
        {
            std::wcerr << L"Invalid record length at offset "
                       << offset << L'\n';

            break;
        }

        std::wcout << L"\nRecord version: "
                   << header.MajorVersion << L'.'
                   << header.MinorVersion << L'\n';

        // Version 2: 64-bit file-reference numbers.
        if (header.MajorVersion == 2 &&
            header.RecordLength >=
                offsetof(UsnRecordV2Compat, FileName))
        {
            UsnRecordV2Compat record{};

            constexpr std::size_t fixedSize =
                offsetof(UsnRecordV2Compat, FileName);

            // Copy only the fixed-size portion.
            std::memcpy(
                &record,
                buffer.data() + offset,
                fixedSize
            );

            if (isValidFileName(
                    header.RecordLength,
                    record.FileNameOffset,
                    record.FileNameLength,
                    fixedSize))
            {
                const auto* name =
                    reinterpret_cast<const WCHAR*>(
                        buffer.data() + offset +
                        record.FileNameOffset
                    );

                std::wstring filename(
                    name,
                    record.FileNameLength / sizeof(WCHAR)
                );

                std::wcout << L"Filename: "
                           << filename << L'\n';

                std::wcout << L"File reference: "
                           << record.FileReferenceNumber << L'\n';

                std::wcout << L"Parent reference: "
                           << record.ParentFileReferenceNumber << L'\n';

                std::wcout << L"File attributes: 0x"
                           << std::hex << record.FileAttributes
                           << std::dec << L'\n';

                ++displayed;
            }
            else
            {
                std::wcout << L"Invalid filename fields.\n";
            }
        }

        // Version 3: 128-bit file-reference numbers.
        else if (header.MajorVersion == 3 &&
                 header.RecordLength >=
                     offsetof(UsnRecordV3Compat, FileName))
        {
            UsnRecordV3Compat record{};

            constexpr std::size_t fixedSize =
                offsetof(UsnRecordV3Compat, FileName);

            std::memcpy(
                &record,
                buffer.data() + offset,
                fixedSize
            );

            if (isValidFileName(
                    header.RecordLength,
                    record.FileNameOffset,
                    record.FileNameLength,
                    fixedSize))
            {
                const auto* name =
                    reinterpret_cast<const WCHAR*>(
                        buffer.data() + offset +
                        record.FileNameOffset
                    );

                std::wstring filename(
                    name,
                    record.FileNameLength / sizeof(WCHAR)
                );

                std::wcout << L"Filename: "
                           << filename << L'\n';

                std::wcout << L"128-bit file reference: ";
                printFileId128(record.FileReferenceNumber);

                std::wcout << L"128-bit parent reference: ";
                printFileId128(record.ParentFileReferenceNumber);

                ++displayed;
            }
            else
            {
                std::wcout << L"Invalid filename fields.\n";
            }
        }
        else
        {
            std::wcout << L"Record format not decoded by this test.\n";
        }

        ++parsedRecords;

        // Advance to the next record using this record's actual length.
        offset += header.RecordLength;
    }

    std::wcout << L"\nRecords parsed: " << parsedRecords << L'\n';
    std::wcout << L"Records displayed: " << displayed << L'\n';

    CloseHandle(volume);

    return 0;
}
