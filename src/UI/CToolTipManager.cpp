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
void CToolTipManager::Show(int x, int y, char uiType, stToolTipData* pData)
{
	stToolTipData localData;

	m_nMouseX = x;
	m_nMouseY = y;

	// 複製傳入的數據到本地
	localData.SetKindType(
		*reinterpret_cast<int*>(pData),                       // type
		*reinterpret_cast<short*>((char*)pData + 8),          // itemId
		*reinterpret_cast<int*>((char*)pData + 12),           // count
		*reinterpret_cast<int*>((char*)pData + 4),            // color
		*reinterpret_cast<char*>((char*)pData + 32),          // grade
		*reinterpret_cast<short*>((char*)pData + 34),         // durability
		*reinterpret_cast<int*>((char*)pData + 36)            // extra
	);

	// 字串需要特殊處理：直接用 SetStringType 如果是字串類型
	// 先用 Init 方式拷貝所有欄位（簡化版本）
	// 重建一個乾淨的 stToolTipData 然後傳下去
	// 注意：count 欄位清零
	int savedCount = *reinterpret_cast<int*>((char*)pData + 12);
	*reinterpret_cast<int*>((char*)&localData + 12) = 0;

	// 顯示主提示
	m_tips[0].Show(x, y, pData, 0);

	// 隱藏其他 4 個
	m_tips[1].Hide();
	m_tips[2].Hide();
	m_tips[3].Hide();
	m_tips[4].Hide();

	// 裝備比較邏輯
	if (savedCount)
	{
		int dataType = *reinterpret_cast<int*>(pData);
		if (dataType == 4) // 物品類型
		{
			uint16_t itemId = *reinterpret_cast<uint16_t*>((char*)pData + 8);
			if (itemId)
			{
				stItemKindInfo* pItemInfo = cltItemKindInfo::GetItemKindInfo(
					(cltItemKindInfo*)&g_clItemKindInfo, itemId);
				if (pItemInfo)
				{
					unsigned int pos1 = 0, pos2 = 0, pos3 = 0;
					stItemKindInfo* equipCount = cltEquipmentSystem::GetEquipablePosByItemKind(
						dword_21BA32C, itemId, &pos1, &pos2, &pos3);

					uint16_t equipItemKind = cltEquipmentSystem::GetEquipItem(
						dword_21BA32C, pos1, pos2);

					if (equipItemKind)
					{
						// 裝備比較模式
						// 顯示 "目前裝備" 標題在 tips[3]
						stToolTipData titleData;
						*reinterpret_cast<int*>(&titleData) = 0;           // type = static
						*reinterpret_cast<uint16_t*>((char*)&titleData + 8) = 3931; // text code
						int mainWidth = m_tips[0].GetWidth();
						int posX = x + mainWidth;
						m_tips[3].Show(posX, y, &titleData, 1);

						// 顯示裝備物品提示在 tips[1]
						stToolTipData equipData;
						*reinterpret_cast<int*>(&equipData) = 4;          // type = item
						*reinterpret_cast<uint16_t*>((char*)&equipData + 8) = equipItemKind;
						int titleHeight = m_tips[3].GetHeight();
						m_tips[1].Show(posX, y + titleHeight, &equipData, 1);

						// 第三個裝備位（如果有）
						if (equipCount == (stItemKindInfo*)2)
						{
							uint16_t equipItem2 = cltEquipmentSystem::GetEquipItem(
								dword_21BA32C, pos1, pos3);
							if (equipItem2)
							{
								int subWidth = m_tips[1].GetWidth();
								int posX2 = posX + subWidth;

								stToolTipData titleData2;
								*reinterpret_cast<int*>(&titleData2) = 0;
								*reinterpret_cast<uint16_t*>((char*)&titleData2 + 8) = 3931;
								m_tips[4].Show(posX2, y, &titleData2, 1);

								stToolTipData equipData2;
								*reinterpret_cast<int*>(&equipData2) = 4;
								*reinterpret_cast<uint16_t*>((char*)&equipData2 + 8) = equipItem2;
								int titleHeight2 = m_tips[4].GetHeight();
								m_tips[2].Show(posX2, y + titleHeight2, &equipData2, 1);
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
void CToolTipManager::Draw()
{
	m_tips[0].Draw();

	// 對齊反編譯：將 tips[0] 的 alpha 傳播到 tips[1] 和 tips[2]
	// tips[0].m_nAlpha → tips[1].m_nAlpha, tips[2].m_nAlpha
	// （這裡通過直接存取私有成員的方式在原始碼中不可行，
	//   但反編譯顯示它直接寫偏移。用友元或公開方法替代。）
	// 簡化：Draw 順序保持一致
	m_tips[1].Draw();
	m_tips[2].Draw();
	m_tips[3].Draw();
	m_tips[4].Draw();
}
