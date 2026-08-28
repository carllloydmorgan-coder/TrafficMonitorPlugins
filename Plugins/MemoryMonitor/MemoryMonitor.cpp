#include "pch.h"
#include "MemoryMonitor.h"
#include "MemoryData.h"

CMemoryMonitor CMemoryMonitor::m_instance;

CMemoryMonitor::CMemoryMonitor()
{
}

CMemoryMonitor& CMemoryMonitor::Instance()
{
    return m_instance;
}

IPluginItem* CMemoryMonitor::GetItem(int index)
{
    // this plugin provides a single display item
    if (index == 0)
        return &m_item;
    return nullptr;
}

void CMemoryMonitor::DataRequired()
{
    // The monitoring values arrive through OnMonitorInfo, so there is
    // nothing to fetch here.
}

const wchar_t* CMemoryMonitor::GetInfo(PluginInfoIndex index)
{
    switch (index)
    {
    case TMI_NAME:
        return L"Memory Monitor";
    case TMI_DESCRIPTION:
        return L"Shows memory usage in the taskbar as a coloured memory "
               L"module icon with the percentage underneath it.";
    case TMI_AUTHOR:
        return L"Carl";
    case TMI_COPYRIGHT:
        return L"";
    case TMI_VERSION:
        return L"1.00";
    case TMI_URL:
        return L"";
    default:
        break;
    }
    return L"";
}

void CMemoryMonitor::OnMonitorInfo(const MonitorInfo& monitor_info)
{
    // The main program passes every value it has gathered; only memory
    // usage is needed here.
    g_memory_data.memory_usage = monitor_info.memory_usage;
}

void CMemoryMonitor::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case ITMPlugin::EI_DRAW_TASKBAR_WND:
        // remember whether the taskbar window or the main window is being drawn
        g_memory_data.draw_taskbar_wnd = (data != nullptr && data[0] == L'1');
        break;
    default:
        break;
    }
}

const wchar_t* CMemoryMonitor::GetTooltipInfo()
{
    m_tooltip_info = L"Memory: " + g_memory_data.GetUsageString();
    return m_tooltip_info.c_str();
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CMemoryMonitor::Instance();
}
