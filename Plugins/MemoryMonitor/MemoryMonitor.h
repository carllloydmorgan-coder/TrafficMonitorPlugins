#pragma once
#include "PluginInterface.h"
#include "MemoryItem.h"
#include <string>

class CMemoryMonitor : public ITMPlugin
{
private:
    CMemoryMonitor();

public:
    static CMemoryMonitor& Instance();

    virtual IPluginItem* GetItem(int index) override;
    virtual void DataRequired() override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual void OnMonitorInfo(const MonitorInfo& monitor_info) override;
    virtual void OnExtenedInfo(ExtendedInfoIndex index,
        const wchar_t* data) override;
    virtual const wchar_t* GetTooltipInfo() override;

private:
    static CMemoryMonitor m_instance;
    CMemoryItem m_item;
    std::wstring m_tooltip_info;
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();

#ifdef __cplusplus
}
#endif
