#pragma once
#include <cstdint>
#include <string>
#include "UI/CControlAlphaBox.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"
#include "UI/stToolTipData.h"

struct stItemKindInfo;
struct stSkillKindInfo;
struct stMapInfo;

/**
 * CToolTip - 遊戲內工具提示面板
 *
 * 記憶體布局（對齊反編譯）：
 *   +0       vftable
 *   +4       CControlAlphaBox  m_innerBox          (主背景)
 *   +212     CControlAlphaBox  m_borderBox[4]      (邊框 4 條)
 *   +1044    CControlAlphaBox  m_sectionBar[5]     (分隔線)
 *   +2188    stToolTipData     m_tipData           (工具提示數據)
 *   +2228    CControlImage     m_icon              (圖標)
 *   +2420    CControlText      m_textMain          (主文字)
 *   +2852    CControlText      m_textTitle         (標題)
 *   +3284    CControlText      m_textDesc          (描述)
 *   +3716    CControlText      m_labelText[20]     (20 行索引標籤)
 *   +12356   CControlText      m_valueText[20]     (20 行索引值)
 *   +21004   CControlText      m_worldMapText[50]  (世界地圖 50 行)
 *   sizeof(CToolTip) = 42608 bytes (0xA670)
 */
class CToolTip
{
public:
	CToolTip();
	virtual ~CToolTip();

	void InitialUpdate();

	int  GetWidth();
	int  GetHeight();
	void SetPos(int x, int y);

	void Poll();

	void HideSectionBar();
	void SetUpSizeIndexData();
	void CalcPos();
	void SetUpOutLayer();

	void SetIndex(uint16_t textCode);

	// AddIndexData 多載
	void AddIndexData(uint16_t textCode, int value, unsigned int color);
	void AddIndexData(uint16_t textCode, char* value, unsigned int color);
	void AddIndexData(uint16_t textCode, char* format, int arg);
	void AddIndexData(uint16_t textCode, char* format, float arg);
	void AddIndexData(uint16_t textCode, char* format, int arg1, int arg2);

	void AddNextLineData(char* text);

	void AddDesc(uint16_t textCode, int itemFlag);
	void AddDesc(char* text);

	void AddSectionBar();

	// Process 系列
	void ProcessStatic();
	void ProcessOneLineText(char* text);
	void ProcessWorldMap(int mode);
	void ProcessLesson();
	void ProcessItem();
	void ProcessMakingItem();
	void ProcessHunt();
	void ProcessFashion();
	void ProcessSkill(uint16_t skillKind);
	void ProcessEmblem();
	void OutputCashShopTime(stItemKindInfo* pItemInfo);
	void ProcessDesc();
	void ProcessClimate();
	void ProcessEmoticon();
	void ProcessPetSkill(uint16_t petSkillKind);
	void ProcessCoupleRing();

	void PrepareDrawing();
	void Draw();

	void Show(int x, int y, const stToolTipData* pData, int compareFlag);
	void Hide();
	int  IsShow();

	char* GetWeaponTypeText(int weaponType);
	void SetVoidIndex(int from, int to);

	void PrintReqWeaponForSkill(stSkillKindInfo* pSkill, int checkEquipped);
	void PrintReqClassForSkill(stSkillKindInfo* pSkill);
	void PrintReqClassForHuntItem();

	void ProcessCharInfo(char* charName);
	void SetTextMainTitle(stMapInfo* pMapInfo);
	void SetTextDungeonBasic(stMapInfo* pMapInfo);

private:
	// 主背景 Box（+4）
	CControlAlphaBox m_innerBox;

	// 邊框 4 條（+212）
	CControlAlphaBox m_borderBox[4];

	// 分隔線 5 條（+1044）
	CControlAlphaBox m_sectionBar[5];

	// 工具提示數據（+2188）
	stToolTipData m_tipData;

	// 圖標（+2228）
	CControlImage m_icon;

	// 主文字（+2420）
	CControlText m_textMain;

	// 標題（+2852）
	CControlText m_textTitle;

	// 描述（+3284）
	CControlText m_textDesc;

	// 20 行索引標籤（+3716）
	CControlText m_labelText[20];

	// 20 行索引值（+12356）
	CControlText m_valueText[20];

	// 世界地圖 50 行（+21004）
	CControlText m_worldMapText[50];

	// ---- 狀態數據（對齊反編譯偏移）----

	// +2084: 當前索引行數  [this+522]
	int m_nIndexCount = 0;

	// +2088: void index 陣列  [this+523] ~ [this+542] (20 個)
	int m_voidIndices[20] = {};

	// +2168: void index 數量  [this+543]
	int m_nVoidCount = 0;

	// +2172: section bar 數量  [this+521] → 注意：實際偏移 2084 之前
	// 重新排列以匹配反編譯
	int m_nSectionBarCount = 0;

	// +2180: 滑鼠 X  [this+545]
	int m_nMouseX = 0;
	// +2184: 滑鼠 Y  [this+546]
	int m_nMouseY = 0;

	// +2188 已由 m_tipData 佔用
	// 以下欄位對齊到 [this+547] 起

	// tooltip type  [this+547]
	int m_nType = -1;
	// tooltip color/sub  [this+548]
	int m_nSubType = 0;

	// item/skill kind  [this+1098 as WORD]  → 偏移 2196
	uint16_t m_usKindID = 0;

	// m_nCount（compare flag）[this+550]
	int m_nCompareFlag = 0;

	// 字串數據  [this+551~555] → std::string at +2204
	std::string m_strData;

	// UI type byte  [this+2220 as BYTE]
	uint8_t m_byUIType = 0;

	// slot index  [this+1111 as WORD] → 偏移 2222
	uint16_t m_usSlotIndex = 0xFFFF;

	// extra dword  [this+556]
	int m_nExtraData = 0;

	// title color  [this+750]
	int m_nTitleColor = -1;

	// alpha  [this+5250]
	int m_nAlpha = 190;

	// world map text count  [this+10651]
	int m_nWorldMapTextCount = 0;
};
