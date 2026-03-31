#include "UI/CToolTipManager.h"
#include "UI/CUIManager.h"
#include "Info/cltItemKindInfo.h"
#include "System/cltEquipmentSystem.h"
#include "global.h"

// 裝備系統指標（對齊反編譯 dword_21BA32C）
extern cltEquipmentSystem* dword_21BA32C;

// ------------------------------------------------------------
// 解構
// ------------------------------------------------------------
CToolTipManager::~CToolTipManager()
{
	// C++ 自動依反向順序解構 m_tips[4..0]
}

// ------------------------------------------------------------
// InitialUpdate
// ------------------------------------------------------------
void CToolTipManager::InitialUpdate()
{
	for (int i = 0; i < 5; ++i)
		m_tips[i].InitialUpdate();
}

// ------------------------------------------------------------
// Show - 顯示工具提示，可能同時顯示裝備比較
// ------------------------------------------------------------
// ground truth: 建立 v40 (localData) 完整複製 a5 的所有欄位（含 string），
// 將 count 清 0 後傳給 tips[0]；裝備比較時設 count=1 並重用 localData。
void CToolTipManager::Show(int x, int y, char uiType, stToolTipData* pData)
{
	stToolTipData localData;

	m_nMouseX = x;
	m_nMouseY = y;

	// 完整複製 pData 到 localData（對齊 ground truth 的 v40 = deep copy a5）
	const unsigned char* src = reinterpret_cast<const unsigned char*>(pData);
	unsigned char* dst = reinterpret_cast<unsigned char*>(&localData);

	// 複製非字串欄位
	*reinterpret_cast<int*>(dst + 0)  = *reinterpret_cast<const int*>(src + 0);   // type
	*reinterpret_cast<int*>(dst + 4)  = *reinterpret_cast<const int*>(src + 4);   // color
	*reinterpret_cast<short*>(dst + 8) = *reinterpret_cast<const short*>(src + 8); // itemId
	*reinterpret_cast<int*>(dst + 12) = *reinterpret_cast<const int*>(src + 12);  // count

	// 複製字串（offset 16，使用 std::string::assign）
	std::string& dstStr = *reinterpret_cast<std::string*>(dst + 16);
	const std::string& srcStr = *reinterpret_cast<const std::string*>(src + 16);
	dstStr = srcStr;

	// 複製剩餘欄位
	*(dst + 32)  = *(src + 32);                                                     // grade
	*reinterpret_cast<short*>(dst + 34) = *reinterpret_cast<const short*>(src + 34); // durability
	*reinterpret_cast<int*>(dst + 36) = *reinterpret_cast<const int*>(src + 36);    // extra

	// ground truth: v42 = 0（清除 count，使主 tooltip 不進入比較模式）
	int savedCount = *reinterpret_cast<int*>(dst + 12);
	*reinterpret_cast<int*>(dst + 12) = 0;

	// 顯示主提示（ground truth: tips[0].Show(x, y, v40, 0)，用 localData 而非 pData）
	m_tips[0].Show(x, y, &localData, 0);

	// 隱藏其他 4 個
	m_tips[1].Hide();
	m_tips[2].Hide();
	m_tips[3].Hide();
	m_tips[4].Hide();

	// 裝備比較邏輯（ground truth: 檢查原始 count）
	if (savedCount)
	{
		int dataType = *reinterpret_cast<int*>(dst + 0);  // localData.type
		if (dataType == 4) // 物品類型
		{
			uint16_t itemId = *reinterpret_cast<uint16_t*>(dst + 8); // localData.itemId
			if (itemId)
			{
				stItemKindInfo* pItemInfo = g_clItemKindInfo.GetItemKindInfo(itemId);
				if (pItemInfo)
				{
					unsigned int pos1 = 0, pos2 = 0, pos3 = 0;
					stItemKindInfo* equipCount = dword_21BA32C->GetEquipablePosByItemKind(
						itemId, &pos1, &pos2, &pos3);

					uint16_t equipItemKind = dword_21BA32C->GetEquipItem(pos1, pos2);

					if (equipItemKind)
					{
						// ground truth: v42 = 1（設 count=1 使比較 tooltip 進入比較模式）
						*reinterpret_cast<int*>(dst + 12) = 1;

						// 顯示 "目前裝備" 標題在 tips[3]
						*reinterpret_cast<int*>(dst + 0)  = 0;     // type = static
						*reinterpret_cast<uint16_t*>(dst + 8) = 3931; // text code
						int mainWidth = m_tips[0].GetWidth();
						int posX = x + mainWidth;
						m_tips[3].Show(posX, y, &localData, 1);

						// 顯示裝備物品提示在 tips[1]
						*reinterpret_cast<int*>(dst + 0)  = 4;     // type = item
						*reinterpret_cast<uint16_t*>(dst + 8) = equipItemKind;
						int titleHeight = m_tips[3].GetHeight();
						m_tips[1].Show(posX, y + titleHeight, &localData, 1);

						// 第三個裝備位（如果有）
						if (equipCount == (stItemKindInfo*)2)
						{
							uint16_t equipItem2 = dword_21BA32C->GetEquipItem(pos1, pos3);
							if (equipItem2)
							{
								int subWidth = m_tips[1].GetWidth();
								int posX2 = posX + subWidth;

								*reinterpret_cast<int*>(dst + 0)  = 0;
								*reinterpret_cast<uint16_t*>(dst + 8) = 3931;
								m_tips[4].Show(posX2, y, &localData, 1);

								*reinterpret_cast<int*>(dst + 0)  = 4;
								*reinterpret_cast<uint16_t*>(dst + 8) = equipItem2;
								int titleHeight2 = m_tips[4].GetHeight();
								m_tips[2].Show(posX2, y + titleHeight2, &localData, 1);
							}
						}
					}
				}
			}
		}
	}
}

// ------------------------------------------------------------
// Hide
// ------------------------------------------------------------
void CToolTipManager::Hide()
{
	for (int i = 0; i < 5; ++i)
		m_tips[i].Hide();
}

// ------------------------------------------------------------
// IsShow
// ------------------------------------------------------------
int CToolTipManager::IsShow()
{
	return m_tips[0].IsShow();
}

// ------------------------------------------------------------
// Poll
// ------------------------------------------------------------
void CToolTipManager::Poll()
{
	for (int i = 0; i < 5; ++i)
		m_tips[i].Poll();

	// 若比較提示可見，重新計算位置
	if (m_tips[1].IsShow())
	{
		uint16_t mainW = (uint16_t)m_tips[0].GetWidth();
		uint16_t mainH = (uint16_t)m_tips[0].GetHeight();
		uint16_t subW = (uint16_t)m_tips[1].GetWidth();
		uint16_t subH = (uint16_t)m_tips[1].GetHeight();
		uint16_t totalW = mainW + subW;

		if (mainH <= subH)
			mainH = subH;

		int hasThird = m_tips[2].IsShow();
		if (hasThird)
		{
			totalW += subW;
			uint16_t thirdH = (uint16_t)m_tips[2].GetHeight();
			if (mainH <= thirdH)
				mainH = thirdH;
		}

		// 水平位置調整
		int posX;
		int overflow = g_Game_System_Info.ScreenWidth - totalW - m_nMouseX - 10;
		if (overflow >= 0)
			posX = m_nMouseX;
		else
			posX = m_nMouseX + overflow;

		// 垂直位置調整
		int posY = m_nMouseY;
		if ((int)(mainH + posY + 42) <= g_Game_System_Info.ScreenHeight)
		{
			posY = posY + 32;
			if (posY <= 15)
				posY = 15;
		}
		else
		{
			posY = posY - mainH - 10;
			if (posY <= 15)
				posY = 15;
		}

		int rightX = posX + mainW;

		m_tips[0].SetPos(posX, posY);
		m_tips[3].SetPos(rightX, posY);
		uint16_t title1H = (uint16_t)m_tips[3].GetHeight();
		m_tips[1].SetPos(rightX, posY + title1H);

		if (hasThird)
		{
			int thirdX = rightX + subW;
			m_tips[4].SetPos(thirdX, posY);
			uint16_t title2H = (uint16_t)m_tips[4].GetHeight();
			m_tips[2].SetPos(thirdX, posY + title2H);
		}
	}
}

// ------------------------------------------------------------
// PrepareDrawing
// ------------------------------------------------------------
void CToolTipManager::PrepareDrawing()
{
	for (int i = 0; i < 5; ++i)
		m_tips[i].PrepareDrawing();
}

// ------------------------------------------------------------
// Draw
// ------------------------------------------------------------
// ground truth: Draw 先畫 tips[0]，再將 tips[0].alpha 同步到 tips[1] 和 tips[2]，
// 然後依序畫 tips[1]~[4]
void CToolTipManager::Draw()
{
	m_tips[0].Draw();

	// 對齊反編譯：*(this+15904) = *(this+5252); *(this+26556) = *(this+5252);
	// 即 tips[1].m_nAlpha = tips[0].m_nAlpha; tips[2].m_nAlpha = tips[0].m_nAlpha;
	int alpha = m_tips[0].GetAlpha();
	m_tips[1].SetAlpha(alpha);
	m_tips[2].SetAlpha(alpha);

	m_tips[1].Draw();
	m_tips[2].Draw();
	m_tips[3].Draw();
	m_tips[4].Draw();
}
