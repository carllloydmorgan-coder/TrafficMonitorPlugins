#pragma once
#include "PluginInterface.h"
#include "CpuItem.h"
#include <string>

class CCpuMonitor : public ITMPlugin
{
private:
    CCpuMonitor();

public:
    static CCpuMonitor& Instance();

    virtual IPluginItem* GetItem(int index) override;
    virtual void DataRequired() override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual void OnMonitorInfo(const MonitorInfo& monitor_info) override;
    virtual void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;
    virtual const wchar_t* GetTooltipInfo() override;

private:
    static CCpuMonitor m_instance;
    CCpuItem m_item;
    std::wstring m_tooltip_info;
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();

#ifdef __cplusplus
}
#endif
