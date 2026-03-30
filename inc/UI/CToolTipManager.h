#pragma once
#include "UI/CToolTip.h"

/**
 * CToolTipManager - 管理 5 個 CToolTip 面板
 *
 * 記憶體布局（對齊反編譯）：
 *   +0     int m_nMouseX
 *   +4     int m_nMouseY
 *   +8     CToolTip m_tips[5]   (每個 42608 bytes)
 *   sizeof(CToolTipManager) = 213048 bytes (0x34038)
 *
 * tips[0] = 主提示  (+8)
 * tips[1] = 裝備比較提示 1  (+42616)
 * tips[2] = 裝備比較提示 2  (+85224)
 * tips[3] = 標題欄 1  (+127832)
 * tips[4] = 標題欄 2  (+170440)
 */
class CToolTipManager
{
public:
	CToolTipManager() = default;
	~CToolTipManager();

	void InitialUpdate();

	void Show(int x, int y, char uiType, stToolTipData* pData);
	void Hide();
	int  IsShow();

	void Poll();
	void PrepareDrawing();
	void Draw();

private:
	int      m_nMouseX = 0;
	int      m_nMouseY = 0;
	CToolTip m_tips[5];
};
