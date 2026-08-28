#include "pch.h"
#include "MemoryItem.h"
#include "MemoryData.h"
#include <Gdiplus.h>
#pragma comment(lib, "GdiPlus.lib")

namespace
{
    //-----------------------------------------------------------------
    // Colours of the memory module icon. Unlike the CPU icon, this one is
    // deliberately coloured rather than following the text colour, so it
    // stays recognisable on a light or a dark taskbar alike. Change these
    // to restyle the icon.
    //-----------------------------------------------------------------
    constexpr BYTE MEMORY_BOARD_R{ 52 };    // circuit board, green
    constexpr BYTE MEMORY_BOARD_G{ 148 };
    constexpr BYTE MEMORY_BOARD_B{ 86 };
    constexpr BYTE MEMORY_CHIP_R{ 26 };     // the chips on the board
    constexpr BYTE MEMORY_CHIP_G{ 74 };
    constexpr BYTE MEMORY_CHIP_B{ 45 };
    constexpr BYTE MEMORY_PIN_R{ 214 };     // contact pins, gold
    constexpr BYTE MEMORY_PIN_G{ 168 };
    constexpr BYTE MEMORY_PIN_B{ 58 };

    //-----------------------------------------------------------------
    // Proportions of the icon. Values are fractions of the icon HEIGHT
    // unless the name says width, so the icon looks the same at any size.
    //-----------------------------------------------------------------
    constexpr float MEMORY_ICON_ASPECT{ 1.5f };   // width divided by height
    constexpr float MEMORY_BOARD_TOP{ 0.02f };    // top of the board
    constexpr float MEMORY_BOARD_BOTTOM{ 0.72f }; // bottom of the board
    constexpr float MEMORY_CHIP_TOP{ 0.20f };     // top of the chips
    constexpr float MEMORY_CHIP_BOTTOM{ 0.56f };  // bottom of the chips
    constexpr float MEMORY_PIN_TOP{ 0.74f };      // top of the contact pins
    constexpr float MEMORY_CORNER{ 0.10f };       // board corner radius

    constexpr float MEMORY_SIDE_W{ 0.04f };       // board inset, of width
    constexpr float MEMORY_CHIP_GAP_W{ 0.055f };  // chip margin, of width
    constexpr float MEMORY_PIN_W{ 0.045f };       // pin width, of width
    constexpr float MEMORY_PIN_GAP_W{ 0.035f };   // gap between pins, of width
    constexpr int MEMORY_CHIP_COUNT{ 3 };         // chips drawn on the board

    // The icon takes up this fraction of the item's height, leaving the rest
    // for the percentage underneath it.
    constexpr float MEMORY_ICON_HEIGHT_RATIO{ 0.5f };
    // Assumed item height before anything has been drawn; the real height is
    // used from the first draw onwards.
    constexpr int MEMORY_DEFAULT_ITEM_HEIGHT{ 40 };

    // Initialise GDI+ lazily. Calling GdiplusStartup while DllMain runs is
    // unsafe, so it happens on the first draw instead. The plugin lives as
    // long as the main program, so it never needs shutting down.
    void EnsureGdiplus()
    {
        static ULONG_PTR s_token{ 0 };
        if (s_token == 0)
        {
            Gdiplus::GdiplusStartupInput input;
            Gdiplus::GdiplusStartup(&s_token, &input, NULL);
        }
    }

    // Add a rounded rectangle to a path
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
            path.AddRectangle(Gdiplus::RectF(left, top,
                right - left, bottom - top));
            return;
        }
        path.AddArc(left, top, diameter, diameter, 180.0f, 90.0f);
        path.AddArc(right - diameter, top, diameter, diameter, 270.0f, 90.0f);
        path.AddArc(right - diameter, bottom - diameter, diameter, diameter,
            0.0f, 90.0f);
        path.AddArc(left, bottom - diameter, diameter, diameter, 90.0f, 90.0f);
        path.CloseFigure();
    }

    // The icon's width for a given height
    int GetIconWidth(int icon_height)
    {
        int width{ static_cast<int>(icon_height * MEMORY_ICON_ASPECT) };
        if (width < 1)
            width = 1;
        return width;
    }

    // Draw the memory module: a green board carrying dark chips, with gold
    // contact pins along the bottom edge.
    void DrawMemoryIcon(HDC hdc, int x, int y, int icon_height)
    {
        if (icon_height < 4)
            return;

        EnsureGdiplus();

        Gdiplus::Graphics graphics(hdc);
        graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        const Gdiplus::Color board_color(255, MEMORY_BOARD_R,
            MEMORY_BOARD_G, MEMORY_BOARD_B);
        const Gdiplus::Color chip_color(255, MEMORY_CHIP_R,
            MEMORY_CHIP_G, MEMORY_CHIP_B);
        const Gdiplus::Color pin_color(255, MEMORY_PIN_R,
            MEMORY_PIN_G, MEMORY_PIN_B);
        Gdiplus::SolidBrush board_brush(board_color);
        Gdiplus::SolidBrush chip_brush(chip_color);
        Gdiplus::SolidBrush pin_brush(pin_color);

        const float h{ static_cast<float>(icon_height) };
        const float w{ static_cast<float>(GetIconWidth(icon_height)) };
        const float left{ static_cast<float>(x) };
        const float top{ static_cast<float>(y) };

        const float board_left{ left + w * MEMORY_SIDE_W };
        const float board_right{ left + w - w * MEMORY_SIDE_W };
        const float board_top{ top + h * MEMORY_BOARD_TOP };
        const float board_bottom{ top + h * MEMORY_BOARD_BOTTOM };

        // the circuit board
        {
            Gdiplus::GraphicsPath path;
            AddRoundRect(path, board_left, board_top, board_right,
                board_bottom, h * MEMORY_CORNER);
            graphics.FillPath(&board_brush, &path);
        }

        // the chips on the board
        {
            const float inner_left{ board_left + w * MEMORY_CHIP_GAP_W };
            const float inner_right{ board_right - w * MEMORY_CHIP_GAP_W };
            const float gap{ w * MEMORY_CHIP_GAP_W * 0.7f };
            const float total{ inner_right - inner_left };
            const float chip_w{ (total - gap * (MEMORY_CHIP_COUNT - 1))
                / MEMORY_CHIP_COUNT };
            if (chip_w > 0)
            {
                for (int i = 0; i < MEMORY_CHIP_COUNT; i++)
                {
                    const float chip_left{ inner_left + i * (chip_w + gap) };
                    graphics.FillRectangle(&chip_brush, Gdiplus::RectF(
                        chip_left, top + h * MEMORY_CHIP_TOP,
                        chip_w, h * (MEMORY_CHIP_BOTTOM - MEMORY_CHIP_TOP)));
                }
            }
        }

        // the contact pins along the bottom edge
        {
            const float pin_w{ w * MEMORY_PIN_W };
            const float step{ w * (MEMORY_PIN_W + MEMORY_PIN_GAP_W) };
            const float pin_top{ top + h * MEMORY_PIN_TOP };
            const float pin_height{ top + h - pin_top };
            if (pin_w > 0 && step > 0 && pin_height > 0)
            {
                for (float px = board_left + pin_w * 0.5f;
                    px + pin_w <= board_right; px += step)
                {
                    graphics.FillRectangle(&pin_brush,
                        Gdiplus::RectF(px, pin_top, pin_w, pin_height));
                }
            }
        }
    }

    // This item's height in the taskbar window. Before the first draw the
    // real height is unknown, so a value is derived from the device's DPI;
    // from the first draw onwards the real height is used.
    int GetItemHeight(CDC* pDC)
    {
        int height{ g_memory_data.taskbar_item_height };
        if (height <= 0)
        {
            const int dpi{ ::GetDeviceCaps(pDC->GetSafeHdc(), LOGPIXELSY) };
            height = MEMORY_DEFAULT_ITEM_HEIGHT * (dpi > 0 ? dpi : 96) / 96;
        }
        return height;
    }

    // Create a font, based on the one currently in the device context, whose
    // height suits the single line of text under the icon.
    bool CreateFittedFont(CDC* pDC, int line_height, CFont& font)
    {
        LOGFONT log_font{};
        HFONT current_font{ static_cast<HFONT>(
            ::GetCurrentObject(pDC->GetSafeHdc(), OBJ_FONT)) };
        if (current_font == NULL)
            return false;
        if (::GetObject(current_font, sizeof(log_font), &log_font) == 0)
            return false;

        // A negative height means character height excluding leading. Three
        // quarters of the line height leaves a line that fits with its spacing.
        int char_height{ line_height * 3 / 4 };
        if (char_height < 7)
            char_height = 7;
        log_font.lfHeight = -char_height;
        log_font.lfWidth = 0;
        return font.CreateFontIndirect(&log_font) != FALSE;
    }

    // Horizontal margin either side of the icon and text
    int GetSidePadding(int item_height)
    {
        int padding{ item_height / 20 };
        if (padding < 1)
            padding = 1;
        return padding;
    }
}

const wchar_t* CMemoryItem::GetItemName() const
{
    return L"Memory";
}

const wchar_t* CMemoryItem::GetItemId() const
{
    // unique identifier for this display item; must not clash with any
    // other plugin
    return L"memTMq7v3";
}

const wchar_t* CMemoryItem::GetItemLableText() const
{
    return L"";
}

const wchar_t* CMemoryItem::GetItemValueText() const
{
    // used for the mouse tooltip and wherever custom drawing is not used
    static std::wstring value_text;
    value_text = g_memory_data.GetUsageString();
    return value_text.c_str();
}

const wchar_t* CMemoryItem::GetItemValueSampleText() const
{
    return L"100%";
}

bool CMemoryItem::IsCustomDraw() const
{
    return true;
}

int CMemoryItem::GetItemWidthEx(void* hDC) const
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);

    const int item_height{ GetItemHeight(pDC) };
    const int icon_height{ static_cast<int>(
        item_height * MEMORY_ICON_HEIGHT_RATIO) };
    const int icon_width{ GetIconWidth(icon_height) };

    // The text is measured with the same font the drawing code uses.
    CFont font;
    CFont* old_font{ nullptr };
    const int line_height{ item_height - icon_height };
    const bool font_created{ CreateFittedFont(pDC, line_height, font) };
    if (font_created)
        old_font = pDC->SelectObject(&font);

    const CString sample{ L"100%" };
    const int text_width{ pDC->GetTextExtent(sample).cx };

    if (font_created && old_font != nullptr)
        pDC->SelectObject(old_font);

    // The item is as wide as the wider of the icon and the text
    const int content_width{ icon_width > text_width ? icon_width : text_width };
    return content_width + GetSidePadding(item_height) * 2;
}

void CMemoryItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    UNREFERENCED_PARAMETER(dark_mode);

    CDC* pDC = CDC::FromHandle((HDC)hDC);
    CRect rect(CPoint(x, y), CSize(w, h));

    // Record this item's real height in the taskbar window, for use by the
    // width calculation.
    if (g_memory_data.draw_taskbar_wnd && rect.Height() > 0)
        g_memory_data.taskbar_item_height = rect.Height();

    if (rect.Height() <= 0 || rect.Width() <= 0)
        return;

    // The icon occupies the upper part of the item, the text the rest.
    int icon_height{ static_cast<int>(
        rect.Height() * MEMORY_ICON_HEIGHT_RATIO) };
    if (icon_height < 1)
        icon_height = 1;
    const int icon_width{ GetIconWidth(icon_height) };
    const int line_height{ rect.Height() - icon_height };

    // In the taskbar the item is measured to fit its own content, so the icon
    // and the percentage are centred in it. In the main window the width comes
    // from the skin and is fixed at the widest reading, so centring a short
    // reading would push the content away from the item on its left;
    // left-aligning keeps the spacing constant whatever the value.
    const bool centre_content{ g_memory_data.draw_taskbar_wnd };
    const int side_padding{ GetSidePadding(rect.Height()) };

    // the icon
    const int icon_left{ centre_content
        ? rect.left + (rect.Width() - icon_width) / 2
        : rect.left + side_padding };
    DrawMemoryIcon(pDC->GetSafeHdc(), icon_left, rect.top, icon_height);

    if (line_height <= 0)
        return;

    // the percentage, centred underneath
    CFont font;
    CFont* old_font{ nullptr };
    if (CreateFittedFont(pDC, line_height, font))
        old_font = pDC->SelectObject(&font);

    const int old_bk_mode{ pDC->SetBkMode(TRANSPARENT) };

    CRect text_rect{ rect };
    text_rect.top = rect.top + icon_height;
    if (!centre_content)
        text_rect.left = rect.left + side_padding;

    const UINT text_align{ centre_content ? DT_CENTER : DT_LEFT };
    const std::wstring usage_text{ g_memory_data.GetUsageString() };
    pDC->DrawText(usage_text.c_str(), text_rect,
        text_align | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    pDC->SetBkMode(old_bk_mode);
    if (old_font != nullptr)
        pDC->SelectObject(old_font);
}
