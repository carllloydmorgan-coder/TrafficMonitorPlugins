#include "pch.h"
#include "BatteryItem.h"
#include "DataManager.h"
#include "DrawCommon.h"
#include "GdiPlusTool.h"

static double battery_percent = 0.0;

namespace
{
    //电池图标的基准尺寸（放大前的尺寸，单位：像素）
    constexpr float BATTERY_ICON_BASE_SIZE{ 16.0f };
    //电池图标左右两侧的边距（基准尺寸下，单位：像素）
    constexpr float BATTERY_ICON_BASE_PADDING{ 2.0f };
    //电池图标的放大倍数。图标内部的电量指示、数值的位置都会按此倍数一起缩放
    constexpr float BATTERY_ICON_SCALE{ 3.0f };
    //向右侧间隙借用的像素数（DPI缩放前）。任务栏各项目之间的间隙由主程序的“项目间距”设置决定，
    //图标会向右伸入间隙这么多像素，使电池图标与相邻项目看起来更近。设为0恢复正常间隙。
    constexpr int BATTERY_GAP_BORROW_PIXELS{ 0 };

    //计算图标的实际绘制尺寸：按放大倍数计算，但不超过任务栏窗口中项目的实际高度
    inline int GetEffectiveIconSize()
    {
        int icon_size{ static_cast<int>(g_data.DPIF(BATTERY_ICON_BASE_SIZE * BATTERY_ICON_SCALE)) };
        if (g_data.m_taskbar_item_height > 0 && icon_size > g_data.m_taskbar_item_height)
            icon_size = g_data.m_taskbar_item_height;
        if (icon_size < 1)
            icon_size = 1;
        return icon_size;
    }

    //电池图标区域的宽度：左侧边距 + 图标实际尺寸 - 向右侧间隙借用的像素数。
    //使用图标的实际绘制尺寸（而不是按放大倍数直接计算），这样图标被窗口高度限制时不会在右侧留下空白
    inline int GetBatteryIconAreaWidth()
    {
        const int icon_size{ GetEffectiveIconSize() };
        const float unit{ icon_size / BATTERY_ICON_BASE_SIZE };
        int width{ static_cast<int>(BATTERY_ICON_BASE_PADDING * unit) + icon_size - g_data.DPI(BATTERY_GAP_BORROW_PIXELS) };
        if (width < icon_size / 2)
            width = icon_size / 2;
        return width;
    }
}

CBatteryItem::CBatteryItem()
{
    //设置一个定时器，让battery_percent的值每200毫秒加4，如果超100，则变为0
    SetTimer(NULL, 0, 200, [](HWND, UINT, UINT_PTR, DWORD)
        {
            battery_percent += 4;
            if (battery_percent > 100)
                battery_percent = 0;
        });
}

const wchar_t* CBatteryItem::GetItemName() const
{
    return g_data.StringRes(IDS_BATTERY);
}

const wchar_t* CBatteryItem::GetItemId() const
{
    return L"b5R30ITQ";
}

const wchar_t* CBatteryItem::GetItemLableText() const
{
    return L"";
}

const wchar_t* CBatteryItem::GetItemValueText() const
{
    static std::wstring battery_str;
    battery_str = g_data.GetBatteryString();
    return battery_str.c_str();
}

const wchar_t* CBatteryItem::GetItemValueSampleText() const
{
    if (g_data.m_setting_data.show_percent)
        return L"100%";
    else
        return L"100";
}

bool CBatteryItem::IsCustomDraw() const
{
    return g_data.m_setting_data.battery_type != BatteryType::NUMBER;
}

//int CBatteryItem::GetItemWidth() const
//{
//    return g_data.RDPI(m_item_width) + 2;
//}

int CBatteryItem::GetItemWidthEx(void * hDC) const
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);

    CString sample_str;
    if (g_data.m_setting_data.show_percent)
        sample_str = _T("100%");
    else
        sample_str = _T("100");

    const int icon_area_width{ GetBatteryIconAreaWidth() };

    switch (g_data.m_setting_data.battery_type)
    {
    case BatteryType::NUMBER:
        return pDC->GetTextExtent(sample_str).cx;
    case BatteryType::ICON:
        return icon_area_width;
    case BatteryType::NUMBER_BESIDE_ICON:
        return icon_area_width + pDC->GetTextExtent(sample_str).cx;
    }
    return icon_area_width + pDC->GetTextExtent(sample_str).cx;
}

void CBatteryItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    //绘图句柄
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    //矩形区域
    CRect rect(CPoint(x, y), CSize(w, h));
    // The icon is enlarged in the taskbar window and in the main window alike.
    // It can never overflow either, because it is clamped to the height of the
    // rectangle the caller gives it a few lines below: in the taskbar that is
    // the taskbar window height, and in the main window it is the skin's
    // text_height. A skin with a small text_height simply gets a small icon.
    const float scale{ BATTERY_ICON_SCALE };
    //记录任务栏窗口中本项目的实际高度，供计算宽度时使用
    if (g_data.m_draw_taskbar_wnd && rect.Height() > 0)
        g_data.m_taskbar_item_height = rect.Height();
    //计算电池图标的尺寸。图标放大后不能超出窗口的高度，否则会被裁剪
    int icon_size{ static_cast<int>(g_data.DPIF(BATTERY_ICON_BASE_SIZE * scale)) };
    if (rect.Height() > 0 && icon_size > rect.Height())
        icon_size = rect.Height();
    if (icon_size < 1)
        icon_size = 1;
    //1个基准像素对应的实际像素数。图标内部的各个尺寸都以此为基准计算，保持原来的比例
    const float unit{ icon_size / BATTERY_ICON_BASE_SIZE };
    //绘制电池图标
    HICON hIcon;
    if (g_data.IsAcOnline())
        hIcon = (dark_mode ? g_data.GetIcon(IDI_BATTERY_LIGHT_CHARGE, icon_size) : g_data.GetIcon(IDI_BATTERY_DARK_CHARGE, icon_size));
    else
        hIcon = (dark_mode ? g_data.GetIcon(IDI_BATTERY_LIGHT, icon_size) : g_data.GetIcon(IDI_BATTERY_DARK, icon_size));
    CPoint icon_point{ rect.TopLeft() };
    icon_point.x = rect.left + static_cast<int>(BATTERY_ICON_BASE_PADDING * unit);
    icon_point.y = rect.top + (rect.Height() - icon_size) / 2;
    ::DrawIconEx(pDC->GetSafeHdc(), icon_point.x, icon_point.y, hIcon, icon_size, icon_size, 0, NULL, DI_NORMAL);
    //填充电量指示
    if (g_data.m_sysPowerStatus.BatteryFlag != 128 && g_data.m_sysPowerStatus.BatteryLifePercent > 0 && g_data.m_sysPowerStatus.BatteryLifePercent <= 100)
    {
        //计算电量指示矩形区域
        Gdiplus::RectF rc_indicater;
        rc_indicater.X = icon_point.x + 1.0f * unit;
        rc_indicater.Y = icon_point.y + 6.0f * unit;
        double percent = g_data.m_sysPowerStatus.BatteryLifePercent;
        //显示充电动画
        if (g_data.m_setting_data.show_charging_animation && g_data.IsAcOnline() && g_data.m_sysPowerStatus.BatteryLifePercent < 100)
        {
            percent = g_data.m_sysPowerStatus.BatteryLifePercent + (battery_percent / 100 * (100 - g_data.m_sysPowerStatus.BatteryLifePercent));
        }
        float indicater_width = static_cast<float>(11.7f * unit * percent / 100);
        rc_indicater.Width = indicater_width;
        rc_indicater.Height = 3.7f * unit;
        //充电状态下的电量指示使用图标
        if (g_data.IsAcOnline())
        {
            HICON hFill;
            if (g_data.m_sysPowerStatus.BatteryLifePercent < 20)
                hFill = g_data.GetIcon(IDI_FILL_CRITICAL, icon_size);
            else if (g_data.m_sysPowerStatus.BatteryLifePercent < 60)
                hFill = g_data.GetIcon(IDI_FILL_LOW, icon_size);
            else
                hFill = g_data.GetIcon(IDI_FILL_HIGH, icon_size);
            //设置剪辑区域
            if (g_data.m_sysPowerStatus.BatteryLifePercent < 100)
            {
                CRgn rgn;
                rgn.CreateRectRgn(static_cast<int>(rc_indicater.GetLeft()), static_cast<int>(rc_indicater.GetTop())
                    , static_cast<int>(rc_indicater.GetRight()), static_cast<int>(rc_indicater.GetBottom()));
                pDC->SelectClipRgn(&rgn);
            }
            //绘制电量指示
            ::DrawIconEx(pDC->GetSafeHdc(), icon_point.x, icon_point.y, hFill, icon_size, icon_size, 0, NULL, DI_NORMAL);
            //恢复剪辑区域
            if (g_data.m_sysPowerStatus.BatteryLifePercent < 100)
            {
                pDC->SelectClipRgn(NULL);
            }
        }
        //非充电状态下电量指示使用原来的方式（矩形填充）
        else
        {
            CDrawCommon drawer;
            drawer.Create(pDC);
            int corner_radius{ static_cast<int>(1.0f * unit) };
            if (corner_radius < 1)
                corner_radius = 1;
            drawer.DrawRoundRect(rc_indicater, CGdiPlusTool::COLORREFToGdiplusColor(g_data.GetBatteryColor()), corner_radius);
        }
    }
    //绘制电池数值
    if (g_data.m_setting_data.battery_type == BatteryType::NUMBER_BESIDE_ICON)
    {
        CRect rc_text{ rect };
        rc_text.left = rect.left + icon_size + static_cast<int>(4.0f * unit);
        std::wstring battery_str{ g_data.GetBatteryString() };
        pDC->DrawText(battery_str.c_str(), rc_text, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }
}
