#pragma once
#include <cstdint>
#include <string>
#include "UI/CControlAlphaBox.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"

struct stItemKindInfo;
struct stSkillKindInfo;
struct stMapInfo;
class stToolTipData;

/**
 * CToolTip - 遊戲內工具提示面板
 *
 * 記憶體布局（對齊反編譯）：
 *   +0       vftable
 *   +4       CControlAlphaBox  m_innerBox          (主背景)
 *   +212     CControlAlphaBox  m_borderBox[4]      (邊框 4 條)
 *   +1044    CControlAlphaBox  m_sectionBar[5]     (分隔線)
 *   +2084    int               m_nSectionBarCount  (section bar 數量)
 *   +2088    int               m_nIndexCount       (索引行數)
 *   +2092    int               m_voidIndices[20]   (void index 陣列)
 *   +2172    int               m_nVoidCount        (void index 數量)
 *   +2180    int               m_nMouseX           (滑鼠 X)
 *   +2184    int               m_nMouseY           (滑鼠 Y)
 *   +2188    int               m_nType             (tooltip 類型，同 stToolTipData 佈局)
 *   +2192    int               m_nSubType          (顏色/子類型)
 *   +2196    uint16_t          m_usKindID          (物品/技能 kind)
 *   +2200    int               m_nCompareFlag      (比較旗標)
 *   +2204    std::string       m_strData           (字串數據)
 *   +2220    uint8_t           m_byUIType          (UI 類型)
 *   +2222    uint16_t          m_usSlotIndex       (slot 索引)
 *   +2224    int               m_nExtraData        (額外數據)
 *   +2228    CControlImage     m_icon              (圖標)
 *   +2420    CControlText      m_textMain          (主文字)
 *   +2852    CControlText      m_textTitle         (標題)
 *   +3284    CControlText      m_textDesc          (描述)
 *   +3716    CControlText      m_labelText[20]     (20 行索引標籤)
 *   +12356   CControlText      m_valueText[20]     (20 行索引值)
 *   +21000   int               m_nAlpha            (透明度)
 *   +21004   CControlText      m_worldMapText[50]  (世界地圖 50 行)
 *   +42604   int               m_nWorldMapTextCount(世界地圖文字數)
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

	// alpha 存取（供 CToolTipManager::Draw 同步用）
	int  GetAlpha() const { return m_nAlpha; }
	void SetAlpha(int alpha) { m_nAlpha = alpha; }

	char* GetWeaponTypeText(int weaponType);
	void SetVoidIndex(int from, int to);

	void PrintReqWeaponForSkill(stSkillKindInfo* pSkill, int checkEquipped);
	void PrintReqClassForSkill(stSkillKindInfo* pSkill);
	void PrintReqClassForHuntItem();

	void ProcessCharInfo(char* charName);
	void SetTextMainTitle(stMapInfo* pMapInfo);
	void SetTextDungeonBasic(stMapInfo* pMapInfo);

private:
	// ---- 控制物件（對齊反編譯偏移）----

	// 主背景 Box（+4）
	CControlAlphaBox m_innerBox;

	// 邊框 4 條（+212）
	CControlAlphaBox m_borderBox[4];

	// 分隔線 5 條（+1044）
	CControlAlphaBox m_sectionBar[5];

	// ---- 狀態數據（+2084，位於 sectionBar 和 tipData 之間）----

	// +2084: section bar 數量  [this+521]
	int m_nSectionBarCount = 0;

	// +2088: 當前索引行數  [this+522]
	int m_nIndexCount = 0;

	// +2092: void index 陣列  [this+523] ~ [this+542] (20 個)
	int m_voidIndices[20] = {};

	// +2172: void index 數量  [this+543]
	int m_nVoidCount = 0;

	// +2176: padding（對齊至 +2180）
	int m_nPadding2176 = 0;

	// +2180: 滑鼠 X  [this+545]
	int m_nMouseX = 0;
	// +2184: 滑鼠 Y  [this+546]
	int m_nMouseY = 0;

	// ---- tooltip 狀態（+2188，覆蓋 m_tipData 佈局）----
	// GT 中 Show() 直接寫入這些欄位，與 stToolTipData 佈局相同

	// tooltip type  [this+547]  (+2188)
	int m_nType = -1;
	// tooltip color/sub  [this+548]  (+2192)
	int m_nSubType = 0;

	// item/skill kind  [this+1098 as WORD]  (+2196)
	uint16_t m_usKindID = 0;
	// +2198: reserved
	uint16_t m_usReserved1 = 0;

	// m_nCount（compare flag）[this+550]  (+2200)
	int m_nCompareFlag = 0;

	// 字串數據  [this+551~555] → std::string at +2204
	std::string m_strData;

	// UI type byte  [this+2220 as BYTE]  (+2220)
	uint8_t m_byUIType = 0;
	// +2221: reserved
	uint8_t m_byReserved2 = 0;

	// slot index  [this+1111 as WORD]  (+2222)
	uint16_t m_usSlotIndex = 0xFFFF;

	// extra dword  [this+556]  (+2224)
	int m_nExtraData = 0;

	// ---- 控制物件（續）----

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

	// alpha  [this+5250]  (+21000)
	int m_nAlpha = 190;

	// 世界地圖 50 行（+21004）
	CControlText m_worldMapText[50];

	// world map text count  [this+10651]  (+42604)
	int m_nWorldMapTextCount = 0;
};
