#pragma once
#include <cstdint>
#include <cstring>
#include "UI/CControlImage.h"
#include "UI/CControlText.h"

class CUIBase;
class CUIManager;

class CMoveIcon : public CControlImage
{
public:
	CMoveIcon();
	virtual ~CMoveIcon();

	// 邏輯初始化
	void InitData();

	// 啟動（開始拖曳）
	// a3: 0=不自動終結, 1=會
	int Start(CUIBase* pDownUI, int autoTerminateFlag);

	// 以群組/GI/block + 置中座標啟動
	int Start(CUIBase* pDownUI,
		unsigned int giGroup, unsigned int giid, unsigned short block,
		int centerAbsX, int centerAbsY);

	// 以來源控制屬性啟動（從 CControlImage 讀取 GI + shade）
	int Start(CUIBase* pDownUI,
		CControlImage* pSrcCtrl,
		int centerAbsX, int centerAbsY);

	int IsStart();

	// Data 64 組
	void         AddData(uint8_t idx, unsigned int value);
	void         OverlapData(uint8_t idx, unsigned int value);
	unsigned int GetData(uint8_t idx);
	int          IsSetData(uint8_t idx);

	// 移動/放置
	void Move(int absX, int absY);
	int  Put(CUIBase* pTargetUI);
	int  Put(int uiId);

	// 條件判斷
	int IsDownUIType(int typeId);
	int IsDownUIType(CUIBase* pUI);

	// 自動終結
	void SetNotAutoTerminate();
	void Terminate();
	int  AutoTerminate();

	// 文字控制取得（ground truth: 獨立函式，非 inline）
	CControlText* GetTextControl();

private:
	struct Slot { unsigned int value; unsigned int isSet; };

	// 反編譯 +208 起的 64 組（value/isSet 各一個 DWORD）
	Slot         m_slots[64];

	// 內嵌的文字控制（反編譯 +724）
	CControlText m_Text;

	// 指向操作起點/放置目標的 UI
	CUIBase*     m_pDownUI = nullptr;   // +48
	CUIBase*     m_pPutUI = nullptr;    // +49

	// 狀態旗標
	int          m_bAutoTerminate = 1;  // +50（預設 1）
	int          m_bStarted = 0;        // +51（Start 後=1）

	// 統計：已設定的槽位數
	uint8_t      m_setCount = 0;        // +720
};
