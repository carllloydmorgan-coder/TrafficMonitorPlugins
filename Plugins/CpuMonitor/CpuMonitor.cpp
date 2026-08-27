#include "pch.h"
#include "CpuMonitor.h"
#include "CpuData.h"

CCpuMonitor CCpuMonitor::m_instance;

CCpuMonitor::CCpuMonitor()
{
}

CCpuMonitor& CCpuMonitor::Instance()
{
    return m_instance;
}

IPluginItem* CCpuMonitor::GetItem(int index)
{
    //本插件只提供一个显示项目
    if (index == 0)
        return &m_item;
    return nullptr;
}

void CCpuMonitor::DataRequired()
{
    //监控数据由主程序通过OnMonitorInfo传入，这里不需要做任何事
}

const wchar_t* CCpuMonitor::GetInfo(PluginInfoIndex index)
{
    switch (index)
    {
    case TMI_NAME:
        return L"CPU监视器";
    case TMI_DESCRIPTION:
        return L"在任务栏中以一个CPU图标显示CPU使用率和CPU温度，使用率显示在上方，温度显示在下方。";
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

void CCpuMonitor::OnMonitorInfo(const MonitorInfo& monitor_info)
{
    //主程序把获取到的所有监控数据传给插件，这里只取需要的两项。
    //温度需要标准版的主程序，Lite版没有温度监控功能，此时温度为0。
    g_cpu_data.cpu_usage = monitor_info.cpu_usage;
    g_cpu_data.cpu_temperature = monitor_info.cpu_temperature;
}

void CCpuMonitor::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case ITMPlugin::EI_DRAW_TASKBAR_WND:
        //记录当前绘制的是任务栏窗口还是主窗口
        g_cpu_data.draw_taskbar_wnd = (data != nullptr && data[0] == L'1');
        break;
    default:
        break;
    }
}

const wchar_t* CCpuMonitor::GetTooltipInfo()
{
    m_tooltip_info = L"CPU: " + g_cpu_data.GetUsageString()
        + L"    " + g_cpu_data.GetTemperatureString();
    return m_tooltip_info.c_str();
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CCpuMonitor::Instance();
}
