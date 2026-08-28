#pragma once
#include <string>

// Data shared inside this plugin. The main program supplies the monitoring
// values through OnMonitorInfo; the drawing code reads them from here.
struct MemoryData
{
    int memory_usage{ 0 };          // memory usage, per cent
    bool draw_taskbar_wnd{ false }; // true while the taskbar window is drawn
    int taskbar_item_height{ 0 };   // this item's real height in the taskbar
                                    // window, recorded while drawing

    static MemoryData& Instance();

    // Usage text, for example "63%"
    std::wstring GetUsageString() const;
};

#define g_memory_data MemoryData::Instance()
