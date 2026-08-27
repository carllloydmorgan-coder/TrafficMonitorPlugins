#include "pch.h"
#include "CpuData.h"

CpuData& CpuData::Instance()
{
    static CpuData instance;
    return instance;
}

std::wstring CpuData::GetUsageString() const
{
    wchar_t buff[16]{};
    swprintf_s(buff, L"%d%%", cpu_usage);
    return std::wstring(buff);
}

std::wstring CpuData::GetTemperatureString() const
{
    //温度为0时视为不可用（Lite版没有温度监控功能）
    if (cpu_temperature <= 0)
        return std::wstring(L"--");

    wchar_t buff[16]{};
    //°为摄氏度符号，这里使用转义字符以避免源文件编码问题
    swprintf_s(buff, L"%d\u00b0C", cpu_temperature);
    return std::wstring(buff);
}
