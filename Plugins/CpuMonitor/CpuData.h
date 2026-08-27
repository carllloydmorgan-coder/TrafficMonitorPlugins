#pragma once
#include <string>

//本插件的共享数据。由主程序通过OnMonitorInfo传入监控数据，绘图时使用。
struct CpuData
{
    int cpu_usage{ 0 };             //CPU使用率（百分比）
    int cpu_temperature{ 0 };       //CPU温度（摄氏度），Lite版或无法获取时为0
    bool draw_taskbar_wnd{ false }; //当前是否正在绘制任务栏窗口
    int taskbar_item_height{ 0 };   //任务栏窗口中本项目的实际高度（绘制时记录）

    static CpuData& Instance();

    //使用率文本，例如“37%”
    std::wstring GetUsageString() const;
    //温度文本，例如“52°C”。温度不可用时返回“--”
    std::wstring GetTemperatureString() const;
};

#define g_cpu_data CpuData::Instance()
