#pragma once
#include <cstdint>
#include <string>

struct Filerecord{
    uint64_t filereference;
    uint64_t parentreference;
    std::wstring filename;
    uint32_t attributes;
};

enum class Extractstatus{
    Success,
    Nullrecord,
    Unsupportedversion,
    Invalidrecordlength,
    Invalidfilenamebounds,
    Invalidfilenamelength
};
