#pragma once
#include "PluginInterface.h"
#include "DataManager.h"

class CBatteryItem : public IPluginItem
{
public:
    CBatteryItem();

    virtual const wchar_t* GetItemName() const override;
    virtual const wchar_t* GetItemId() const override;
    virtual const wchar_t* GetItemLableText() const override;
    virtual const wchar_t* GetItemValueText() const override;
    virtual const wchar_t* GetItemValueSampleText() const override;
    virtual bool IsCustomDraw() const override;
    virtual int GetItemWidthEx(void* hDC) const override;
    virtual void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;
    //在任务栏中始终独占双行（占满整个窗口高度），其他显示项目会两个一组排列在旁边
    virtual int IsDoubleLineExclusive() const override { return 1; }
};
