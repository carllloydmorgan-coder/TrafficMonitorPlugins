#include "pch.h"
#include "MemoryData.h"

MemoryData& MemoryData::Instance()
{
    static MemoryData instance;
    return instance;
}

std::wstring MemoryData::GetUsageString() const
{
    wchar_t buff[16]{};
    swprintf_s(buff, L"%d%%", memory_usage);
    return std::wstring(buff);
}
