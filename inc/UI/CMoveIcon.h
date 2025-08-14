#pragma once
#include <cstdint>
#include <cstring>
#include <algorithm>
#include "UI/CControlImage.h"
#include "UI/CControlText.h"
#include "Character/ClientCharacterManager.h"

class CUIBase {
public:
	virtual ~CUIBase() = default;
	virtual int  GetType() const { return 0; }   // 對應 *(_DWORD*)(p+12)
	virtual void OnDragEnd() {}                  // 對應 vtbl+20 呼叫
};

class CUIManager {
public:
	CUIBase* GetUIWindow(int id);
};

extern CUIManager* g_UIMgr;

class CMoveIcon : public CControlImage
{
public:
	CMoveIcon();
	virtual ~CMoveIcon();

	// ---- 邏輯初始化 ----
	void InitData();

	// ---- 啟動（開始拖曳）----
	int Start(CUIBase* pDownUI, int autoTerminateFlag /* a3: 0=不自動終結, 1=會 */);
	int Start(CUIBase* pDownUI,
		unsigned int giGroup, unsigned int giid, unsigned short block,
		int centerAbsX, int centerAbsY);

	// 來源控制的 GI 與 Shade 值由呼叫端提供（等價於反編譯用 a3+120/124/128/112 取值）
	int Start(CUIBase* pDownUI,
		unsigned int srcGIGroup, unsigned int srcGIID, unsigned short srcBlock,
		int srcShadeAlpha,
		int centerAbsX, int centerAbsY);

	int  IsStart() const { return m_bStarted ? 1 : 0; }

	// ---- Data 64 組 ----
	void        AddData(uint8_t idx, unsigned int value);
	void        OverlapData(uint8_t idx, unsigned int value);
	unsigned int GetData(uint8_t idx);
	int         IsSetData(uint8_t idx) const;

	// ---- 移動/放置 ----
	void Move(int absX, int absY);             // 只有可見時才移動
	int  Put(CUIBase* pTargetUI);              // 對齊反編譯 Put(CUIBase*)
	int  Put(int uiId);                        // 對齊反編譯 Put(int)

	// ---- 條件判斷 ----
	int IsDownUIType(int typeId) const;        // 對齊 IsDownUIType(_DWORD*, int)
	int IsDownUIType(CUIBase* pUI) const;      // 對齊 IsDownUIType(CMoveIcon*, CUIBase*)

	// ---- 自動終結 ----
	void SetNotAutoTerminate();                // 對齊 0x426C70
	void Terminate();                          // 對齊 0x426C80
	int  AutoTerminate();                      // 對齊 0x426CD0

	// ---- 文字控制取得 ----
	CControlText* GetTextControl() { return &m_Text; } // 0x426D00

private:
	struct Slot { unsigned int value; unsigned int isSet; };

	// 反編譯 +208 起的 64 組（value/isSet 各一個 DWORD）
	Slot        m_slots[64];

	// 內嵌的文字控制（反編譯 +724）
	CControlText m_Text;

	// 指向操作起點/放置目標的 UI
	CUIBase* m_pDownUI = nullptr; // +48
	CUIBase* m_pPutUI = nullptr;  // +49

	// 狀態旗標
	int         m_bAutoTerminate = 1;  // +50（預設 1）
	int         m_bStarted = 0;        // +51（Start 後=1）

	// 統計：已設定的槽位數（對齊 +720 的 byte）
	uint8_t     m_setCount = 0;

	// 透明陰影（對齊 +28，在 Start(…, a3, …) 會設為 0 或來源值）
	int         m_nShadeAlpha = 0;
};