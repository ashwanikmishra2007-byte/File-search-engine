#pragma once
#include <unordered_map>
#include <windows.h>
#include <winioctl.h>
#include <string>
#include "filefinder.h"
#include "ntfsfilerecord.h"

class WinFilefinder : public FileFinder
{
protected:
    std::wstring volumename;
    HANDLE volumehandle;
    std::unordered_map<uint64_t, Filerecord> fileindex;

public:
    Extractstatus extract(const USN_RECORD*record, Filerecord &result);
};