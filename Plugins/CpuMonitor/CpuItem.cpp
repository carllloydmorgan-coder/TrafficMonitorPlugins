#include "pch.h"
#include "CpuItem.h"
#include "CpuData.h"
#include <Gdiplus.h>
#pragma comment(lib, "GdiPlus.lib")

namespace
{
    //---------------------------------------------------------------------
    //CPU图标的比例。所有数值都是图标边长的比例，因此图标在任何尺寸下都保持相同的外观。
    //---------------------------------------------------------------------
    constexpr float CPU_PIN_WIDTH{ 0.085f };    //引脚的宽度
    constexpr float CPU_BODY_INSET{ 0.115f };   //芯片主体距图标边缘的距离
    constexpr float CPU_DIE_INSET{ 0.315f };    //内部方块距图标边缘的距离
    constexpr float CPU_BODY_RADIUS{ 0.085f };  //芯片主体的圆角半径
    constexpr float CPU_DIE_RADIUS{ 0.045f };   //内部方块的圆角半径
    constexpr float CPU_STROKE{ 0.075f };       //芯片主体的线条宽度
    constexpr float CPU_PIN_SPAN{ 0.62f };      //引脚分布的范围（居中）
    constexpr int CPU_PIN_COUNT{ 3 };           //每一边的引脚数量

    //图标左侧的边距，与电池插件保持一致（基准16像素图标下为2像素）
    constexpr float CPU_ICON_PADDING_RATIO{ 2.0f / 16.0f };
    //图标与文字之间的间隔
    constexpr float CPU_TEXT_GAP_RATIO{ 4.0f / 16.0f };
    //没有绘制过时假定的项目高度，第一次绘制后会使用实际高度
    constexpr int CPU_DEFAULT_ITEM_HEIGHT{ 40 };

    //延迟初始化GDI+。在DllMain执行期间调用GdiplusStartup是不安全的，
    //因此在第一次绘制时才初始化。插件的生存期与主程序相同，不需要关闭。
    void EnsureGdiplus()
    {
        static ULONG_PTR s_token{ 0 };
        if (s_token == 0)
        {
            Gdiplus::GdiplusStartupInput input;
            Gdiplus::GdiplusStartup(&s_token, &input, NULL);
        }
    }

    //向路径中添加一个圆角矩形
    void AddRoundRect(Gdiplus::GraphicsPath& path, float left, float top,
        float right, float bottom, float radius)
    {
        float diameter{ radius * 2 };
        if (diameter > right - left)
            diameter = right - left;
        if (diameter > bottom - top)
            diameter = bottom - top;
        if (diameter <= 0)
        {
            path.AddRectangle(Gdiplus::RectF(left, top, right - left, bottom - top));
            return;
        }
        path.AddArc(left, top, diameter, diameter, 180.0f, 90.0f);
        path.AddArc(right - diameter, top, diameter, diameter, 270.0f, 90.0f);
        path.AddArc(right - diameter, bottom - diameter, diameter, diameter, 0.0f, 90.0f);
        path.AddArc(left, bottom - diameter, diameter, diameter, 90.0f, 90.0f);
        path.CloseFigure();
    }

    //绘制CPU图标。使用文字的颜色，因此会自动跟随深色/浅色模式和用户设置的颜色。
    void DrawCpuIcon(HDC hdc, int x, int y, int size, COLORREF color)
    {
        if (size < 4)
            return;

        EnsureGdiplus();

        Gdiplus::Graphics graphics(hdc);
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        const Gdiplus::Color gdi_color(255, GetRValue(color), GetGValue(color), GetBValue(color));
        Gdiplus::SolidBrush brush(gdi_color);

        const float s{ static_cast<float>(size) };
        const float left{ static_cast<float>(x) };
        const float top{ static_cast<float>(y) };

        float stroke{ s * CPU_STROKE };
        if (stroke < 1.0f)
            stroke = 1.0f;

        //芯片主体的矩形。线条以路径为中心向两侧扩展，因此向内缩进半个线宽
        const float body_left{ left + s * CPU_BODY_INSET + stroke / 2 };
        const float body_top{ top + s * CPU_BODY_INSET + stroke / 2 };
        const float body_right{ left + s * (1.0f - CPU_BODY_INSET) - stroke / 2 };
        const float body_bottom{ top + s * (1.0f - CPU_BODY_INSET) - stroke / 2 };

        //绘制引脚
        const float pin_width{ s * CPU_PIN_WIDTH };
        const float span{ s * CPU_PIN_SPAN };
        const float start{ (s - span) / 2 };
        const float step{ CPU_PIN_COUNT > 1 ? span / (CPU_PIN_COUNT - 1) : 0.0f };
        const float pin_end_top{ top + s * CPU_BODY_INSET };
        const float pin_end_bottom{ top + s * (1.0f - CPU_BODY_INSET) };
        const float pin_end_left{ left + s * CPU_BODY_INSET };
        const float pin_end_right{ left + s * (1.0f - CPU_BODY_INSET) };
        for (int i = 0; i < CPU_PIN_COUNT; i++)
        {
            const float offset{ start + i * step };
            //上下的引脚
            const float px{ left + offset - pin_width / 2 };
            graphics.FillRectangle(&brush, Gdiplus::RectF(px, top,
                pin_width, pin_end_top - top));
            graphics.FillRectangle(&brush, Gdiplus::RectF(px, pin_end_bottom,
                pin_width, top + s - pin_end_bottom));
            //左右的引脚
            const float py{ top + offset - pin_width / 2 };
            graphics.FillRectangle(&brush, Gdiplus::RectF(left, py,
                pin_end_left - left, pin_width));
            graphics.FillRectangle(&brush, Gdiplus::RectF(pin_end_right, py,
                left + s - pin_end_right, pin_width));
        }

        //绘制芯片主体的轮廓
        {
            Gdiplus::GraphicsPath path;
            AddRoundRect(path, body_left, body_top, body_right, body_bottom,
                s * CPU_BODY_RADIUS);
            Gdiplus::Pen pen(gdi_color, stroke);
            graphics.DrawPath(&pen, &path);
        }

        //绘制内部的方块
        {
            Gdiplus::GraphicsPath path;
            AddRoundRect(path,
                left + s * CPU_DIE_INSET, top + s * CPU_DIE_INSET,
                left + s * (1.0f - CPU_DIE_INSET), top + s * (1.0f - CPU_DIE_INSET),
                s * CPU_DIE_RADIUS);
            graphics.FillPath(&brush, &path);
        }
    }

    //任务栏窗口中本项目的高度。第一次绘制之前还不知道实际高度，
    //此时按设备的DPI换算一个假定值，绘制一次之后就会使用实际的高度。
    int GetItemHeight(CDC* pDC)
    {
        int height{ g_cpu_data.taskbar_item_height };
        if (height <= 0)
        {
            const int dpi{ ::GetDeviceCaps(pDC->GetSafeHdc(), LOGPIXELSY) };
            height = CPU_DEFAULT_ITEM_HEIGHT * (dpi > 0 ? dpi : 96) / 96;
        }
        return height;
    }

    //根据当前DC的字体创建一个高度适合上下两行的字体。
    //这样字体会跟随用户在选项中设置的字体，但不会因为太大而两行放不下。
    bool CreateTwoLineFont(CDC* pDC, int line_height, CFont& font)
    {
        LOGFONT log_font{};
        HFONT current_font{ static_cast<HFONT>(::GetCurrentObject(pDC->GetSafeHdc(), OBJ_FONT)) };
        if (current_font == NULL)
            return false;
        if (::GetObject(current_font, sizeof(log_font), &log_font) == 0)
            return false;

        //负值表示字符高度（不含行距）。取行高的3/4，使一行文字连同行距刚好放得下。
        int char_height{ line_height * 3 / 4 };
        if (char_height < 7)
            char_height = 7;
        log_font.lfHeight = -char_height;
        log_font.lfWidth = 0;
        return font.CreateFontIndirect(&log_font) != FALSE;
    }
}

const wchar_t* CCpuItem::GetItemName() const
{
    return L"CPU";
}

const wchar_t* CCpuItem::GetItemId() const
{
    //插件显示项目的唯一标识，不能与其他插件重复
    return L"cpuTM8x2q";
}

const wchar_t* CCpuItem::GetItemLableText() const
{
    return L"";
}

const wchar_t* CCpuItem::GetItemValueText() const
{
    //用于鼠标提示和非自绘的场合
    static std::wstring value_text;
    value_text = g_cpu_data.GetUsageString() + L" " + g_cpu_data.GetTemperatureString();
    return value_text.c_str();
}

const wchar_t* CCpuItem::GetItemValueSampleText() const
{
    return L"100% 100\u00b0C";
}

bool CCpuItem::IsCustomDraw() const
{
    return true;
}

int CCpuItem::GetItemWidthEx(void* hDC) const
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);

    const int item_height{ GetItemHeight(pDC) };
    const int icon_size{ item_height };
    const float unit{ icon_size / 16.0f };

    //文字宽度按上下两行中较宽的一行计算，并且使用与绘制时相同的字体
    int text_width{ 0 };
    CFont font;
    CFont* old_font{ nullptr };
    const bool font_created{ CreateTwoLineFont(pDC, item_height / 2, font) };
    if (font_created)
        old_font = pDC->SelectObject(&font);

    const CString usage_sample{ L"100%" };
    const CString temperature_sample{ L"100\u00b0C" };
    const int usage_width{ pDC->GetTextExtent(usage_sample).cx };
    const int temperature_width{ pDC->GetTextExtent(temperature_sample).cx };
    text_width = (usage_width > temperature_width ? usage_width : temperature_width);

    if (font_created && old_font != nullptr)
        pDC->SelectObject(old_font);

    return static_cast<int>(CPU_ICON_PADDING_RATIO * unit) + icon_size
        + static_cast<int>(CPU_TEXT_GAP_RATIO * unit) + text_width;
}

void CCpuItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    CRect rect(CPoint(x, y), CSize(w, h));

    //记录任务栏窗口中本项目的实际高度，供计算宽度时使用
    if (g_cpu_data.draw_taskbar_wnd && rect.Height() > 0)
        g_cpu_data.taskbar_item_height = rect.Height();

    if (rect.Height() <= 0 || rect.Width() <= 0)
        return;

    //图标占满项目的高度
    int icon_size{ rect.Height() };
    if (icon_size < 1)
        icon_size = 1;
    const float unit{ icon_size / 16.0f };

    //文字颜色由主程序在绘制前设置。图标使用相同的颜色，
    //这样图标会自动跟随深色/浅色模式以及用户设置的颜色。
    COLORREF text_color{ pDC->GetTextColor() };
    if (text_color == CLR_INVALID)
        text_color = (dark_mode ? RGB(255, 255, 255) : RGB(0, 0, 0));

    const int icon_left{ rect.left + static_cast<int>(CPU_ICON_PADDING_RATIO * unit) };
    DrawCpuIcon(pDC->GetSafeHdc(), icon_left, rect.top, icon_size, text_color);

    //文字区域：上一行为使用率，下一行为温度
    CRect text_rect{ rect };
    text_rect.left = icon_left + icon_size + static_cast<int>(CPU_TEXT_GAP_RATIO * unit);
    if (text_rect.left >= text_rect.right)
        return;

    const int line_height{ rect.Height() / 2 };
    CFont font;
    CFont* old_font{ nullptr };
    if (CreateTwoLineFont(pDC, line_height, font))
        old_font = pDC->SelectObject(&font);

    const int old_bk_mode{ pDC->SetBkMode(TRANSPARENT) };

    CRect usage_rect{ text_rect };
    usage_rect.bottom = usage_rect.top + line_height;
    CRect temperature_rect{ text_rect };
    temperature_rect.top = usage_rect.bottom;

    const std::wstring usage_text{ g_cpu_data.GetUsageString() };
    const std::wstring temperature_text{ g_cpu_data.GetTemperatureString() };
    // In the taskbar the item is measured to fit its own text, so the two
    // lines are drawn from the left. In the main window the width comes from
    // the skin and is fixed at the widest reading, so a short reading would
    // leave a gap before the next item; right-aligning keeps the spacing
    // constant whatever the values.
    const UINT text_align = static_cast<UINT>(
        g_cpu_data.draw_taskbar_wnd ? DT_LEFT : DT_RIGHT);
    pDC->DrawText(usage_text.c_str(), usage_rect,
        text_align | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    pDC->DrawText(temperature_text.c_str(), temperature_rect,
        text_align | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    pDC->SetBkMode(old_bk_mode);
    if (old_font != nullptr)
        pDC->SelectObject(old_font);
}
