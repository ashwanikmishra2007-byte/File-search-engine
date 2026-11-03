#pragma once
#include <unordered_map>
#include <cstdint>
#include <windows.h>
#include <winioctl.h>
#include <string>
#include "filefinder.h"
#include "ntfsfilerecord.h"

class WinFilefinder : public FileFinder
{
protected:
    std::wstring volumename;
    HANDLE volumehandle=INVALID_HANDLE_VALUE;
    std::unordered_map<std::uint64_t, Filerecord> fileindex;

public:
    WinFilefinder(const std::wstring &volumename);
    ~WinFilefinder();
    Extractstatus extract(const USN_RECORD*record, Filerecord &result);
    void enumerate();
};