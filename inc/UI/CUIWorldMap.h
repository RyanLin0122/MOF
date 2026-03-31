#pragma once
#include <cstdint>
#include "UI/CUIBase.h"

struct stMapInfo;

// CUIWorldMap — 世界地圖 UI（部分前置宣告，供 CToolTip::ProcessWorldMap 使用）
// ground truth: CUIManager::GetUIWindow(g_UIMgr, 19) 取得
class CUIWorldMap : public CUIBase
{
public:
    // ground truth: unsigned __int16 __thiscall CUIWorldMap::GetToolTipData(this, int mouseX, int mouseY)
    // 依滑鼠座標回推地圖 kind（回傳 mapID，0 表示無效）
    uint16_t GetToolTipData(int mouseX, int mouseY);

    // ground truth: struct stMapInfo * __thiscall CUIWorldMap::GetMyMapInfo(this)
    stMapInfo* GetMyMapInfo();
};
