#include <vector>
#include <filesystem>
#include <iostream>
#include <unordered_set>
#include "winfilefinder.h"
#include "ntfsfilerecord.h"

Extractstatus WinFilefinder::extract(const USN_RECORD *record, Filerecord &result)
{
    if (record == nullptr)
    {
        return Extractstatus::Nullrecord;
    }

    if(record->MajorVersion!=2|| record->MinorVersion!=0){   /*Different versions can have different layouts. For example, 
        version 3 uses 128-bit file reference identifiers, whereas our current Filerecord uses 64-bit references.*/
        return Extractstatus::Unsupportedversion;
    }

    result.filereference = record->FileReferenceNumber;
    result.parentreference = record->ParentFileReferenceNumber;

    if (record->FileNameOffset > record->RecordLength ||
        record->FileNameLength > record->RecordLength - record->FileNameOffset)
    {
        return Extractstatus::Invalidfilenamebounds;
    }

    WCHAR *filenameptr = (WCHAR *)((BYTE *)record + record->FileNameOffset);
    size_t filenamelength = record->FileNameLength / sizeof(WCHAR);

    std::wstring filename(filenameptr, filenamelength);
    result.filename = filename;
    return Extractstatus::Success;
}