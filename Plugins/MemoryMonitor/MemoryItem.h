#pragma once
#include "PluginInterface.h"

// The memory display item: a coloured memory module icon in the upper half,
// with the memory usage percentage centred underneath it.
class CMemoryItem : public IPluginItem
{
public:
    virtual const wchar_t* GetItemName() const override;
    virtual const wchar_t* GetItemId() const override;
    virtual const wchar_t* GetItemLableText() const override;
    virtual const wchar_t* GetItemValueText() const override;
    virtual const wchar_t* GetItemValueSampleText() const override;
    virtual bool IsCustomDraw() const override;
    virtual int GetItemWidthEx(void* hDC) const override;
    virtual void DrawItem(void* hDC, int x, int y, int w, int h,
        bool dark_mode) override;
    // Always occupy a full-height column of its own in the taskbar window.
    // Other display items are then arranged in stacked pairs beside it.
    virtual int IsDoubleLineExclusive() const override { return 1; }
};
