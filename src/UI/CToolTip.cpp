#include "UI/CToolTip.h"
#include "UI/CUIWorldMap.h"
#include "UI/CUIManager.h"
#include "global.h"
#include "Info/cltCharKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "System/cltPetSystem.h"
#include "Info/cltLessonKindInfo.h"
#include "Info/cltNPCInfo.h"
#include "Info/cltQuestKindInfo.h"
#include "Info/cltRegenMonsterKindInfo.h"
#include "Logic/Map.h"
#include "System/cltQuestSystem.h"
#include "Character/ClientCharacterManager.h"
#include "Info/cltPKRankKindInfo.h"
#include "System/cltEquipmentSystem.h"
#include "System/cltClassSystem.h"
#include "Info/cltEmoticonKindInfo.h"
#include "Info/cltPetSkillKindInfo.h"
#include "Info/cltCoupleRingKindInfo.h"
#include "Info/cltClimateKindInfo.h"
#include "Info/cltSkillKindInfo.h"
#include "Info/cltEmblemKindInfo.h"
#include "Info/cltTransformKindInfo.h"
#include "Info/cltMapUseItemInfoKindInfo.h"
#include "Info/cltMapInfo.h"
#include "System/cltSexSystem.h"
#include "System/cltUsingItemSystem.h"
#include "System/cltPlayerAbility.h"
#include "System/cltUsingSkillSystem.h"
#include "System/cltSkillSystem.h"
#include "System/CSpiritSystem.h"
#include "System/cltMarriageSystem.h"
#include "System/cltSpecialtySystem.h"
#include "Other/cltBaseAbility.h"
#include "Logic/cltBaseInventory.h"
#include "System/cltTradeSystem.h"
#include "System/cltStorageSystem.h"
#include "System/cltExStorageSystem.h"
#include <cstdio>
#include <cstring>

// OutputCashShopTime 依賴的外部符號
extern unsigned short dword_21C9C54[8];
extern int dword_B3D72C[40];
extern cltStorageSystem* dword_21BB2AC;

// ========================================
// CToolTip::CToolTip  (對齊 00426D90)
// ========================================
CToolTip::CToolTip()
    // m_innerBox, m_borderBox[4], m_sectionBar[5] — 由各自的預設建構子初始化
    // m_tipData — stToolTipData 預設建構子
    // m_icon — CControlImage 預設建構子
    // m_textMain, m_textTitle, m_textDesc — CControlText 預設建構子
    // m_labelText[20], m_valueText[20], m_worldMapText[50] — CControlText 陣列預設建構子
{
    m_nIndexCount = 0;
    m_nAlpha = 190;
}

// ========================================
// CToolTip::~CToolTip  (對齊 00426F00)
// ========================================
CToolTip::~CToolTip()
{
    // C++ 編譯器自動按宣告逆序解構所有成員
    // m_worldMapText[50] → m_valueText[20] → m_labelText[20]
    // → m_textDesc → m_textTitle → m_textMain
    // → m_icon → m_tipData
    // → m_sectionBar[5] → m_borderBox[4] → m_innerBox
}

// ========================================
// CToolTip::InitialUpdate  (對齊 00427030)
// ========================================
void CToolTip::InitialUpdate()
{
    // 主背景初始化
    m_innerBox.Create(nullptr);
    m_innerBox.SetWidth(260);
    m_innerBox.SetColor(0.29019609f, 0.44313729f, 0.10588236f, 0.74509805f);

    // 4 條邊框
    for (int i = 0; i < 4; i++)
    {
        m_borderBox[i].Create(&m_innerBox);
        m_borderBox[i].SetColor(0.0f, 0.19607845f, 0.0f, 1.0f);
    }

    // 主文字 (offset +2420)
    m_textMain.Create(10, 10, &m_innerBox);
    m_textMain.m_TextColor  = static_cast<DWORD>(-1);          // 0xFFFFFFFF (白色)
    m_textMain.m_OutlineColor = static_cast<DWORD>(-12634624);

    // 圖標
    m_icon.Create(&m_innerBox);

    // 標題 (offset +2852)
    m_textTitle.Create(130, 10, &m_innerBox);
    m_textTitle.SetControlSetFont("ToolTipTitle");
    m_textTitle.m_TextColor = static_cast<DWORD>(-1);
    m_textTitle.m_isCentered = 1;
    m_textTitle.m_OutlineColor = static_cast<DWORD>(-12634624);

    // 20 行標籤 / 值
    int y = 38;
    for (int i = 0; i < 20; i++)
    {
        m_labelText[i].Create(10, y, &m_innerBox);
        m_labelText[i].m_TextColor  = static_cast<DWORD>(-1638656);
        m_labelText[i].m_OutlineColor = static_cast<DWORD>(-12634624);

        m_valueText[i].Create(150, y, &m_innerBox);
        m_valueText[i].m_TextColor  = static_cast<DWORD>(-16384);
        m_valueText[i].m_OutlineColor = static_cast<DWORD>(-12634624);

        y += 18;
    }

    // 50 行世界地圖文字
    int wy = 10;
    for (int i = 0; i < 50; i++)
    {
        m_worldMapText[i].Create(10, wy, &m_innerBox);
        m_worldMapText[i].m_TextColor  = static_cast<DWORD>(-1);  // 白色
        m_worldMapText[i].m_OutlineColor = static_cast<DWORD>(-12634624);
        wy += 18;
    }

    // 描述 (offset +3284)
    m_textDesc.Create(130, 10, &m_innerBox);
    m_textDesc.m_TextColor  = static_cast<DWORD>(-986896);
    m_textDesc.SetMultiLineSize(240, 100);
    m_textDesc.m_isCentered = 1;

    m_nWorldMapTextCount = 0;
}

// ========================================
// CToolTip::GetWidth  (對齊 00427210)
// ========================================
int CToolTip::GetWidth()
{
    return m_innerBox.GetWidth() + 2;
}

// ========================================
// CToolTip::GetHeight  (對齊 00427220)
// ========================================
int CToolTip::GetHeight()
{
    return m_innerBox.GetHeight() + 2;
}

// ========================================
// CToolTip::SetPos  (對齊 00427230)
// ========================================
void CToolTip::SetPos(int x, int y)
{
    m_innerBox.SetPos(x, y);
}

// ========================================
// CToolTip::Poll  (對齊 00427250)
// ========================================
void CToolTip::Poll()
{
    if (!IsShow())
        return;

    // 重置計數器
    m_nIndexCount = 0;
    m_nVoidCount = 0;
    m_nSectionBarCount = 0;

    // 清除所有文字
    m_textMain.ClearText();
    m_textTitle.ClearText();
    m_textDesc.ClearText();

    for (int i = 0; i < 20; i++)
    {
        m_labelText[i].ClearText();
        m_labelText[i].m_TextColor = static_cast<DWORD>(-16384);

        m_valueText[i].ClearText();
        m_valueText[i].m_TextColor = static_cast<DWORD>(-16384);
    }

    for (int i = 0; i < 50; i++)
    {
        m_worldMapText[i].ClearText();
        m_worldMapText[i].m_TextColor = static_cast<DWORD>(-1);  // 白色
    }

    // 重設標題色 & 背景色
    m_textTitle.m_TextColor = static_cast<DWORD>(-1);
    m_innerBox.SetColor(0.29019609f, 0.44313729f, 0.10588236f, 0.74509805f);

    // 依據 tooltip type 分派處理
    switch (m_nType)
    {
    case 0:
        ProcessStatic();
        break;
    case 1:
        ProcessOneLineText(const_cast<char*>(m_strData.c_str()));
        break;
    case 2:
        ProcessMakingItem();
        SetUpSizeIndexData();
        break;
    case 3:
        ProcessItem();
        SetUpSizeIndexData();
        break;
    case 4:
        ProcessHunt();
        SetUpSizeIndexData();
        break;
    case 5:
        ProcessFashion();
        SetUpSizeIndexData();
        break;
    case 6:
        ProcessSkill(0);
        SetUpSizeIndexData();
        break;
    case 7:
        ProcessDesc();
        SetUpSizeIndexData();
        break;
    case 8:
        ProcessWorldMap(0);
        break;
    case 9:
        ProcessLesson();
        SetUpSizeIndexData();
        break;
    case 0xA:
        ProcessEmblem();
        SetUpSizeIndexData();
        break;
    case 0xB:
        ProcessEmoticon();
        SetUpSizeIndexData();
        break;
    case 0xC:
        ProcessPetSkill(0);
        SetUpSizeIndexData();
        break;
    case 0xD:
        ProcessCharInfo(reinterpret_cast<char*>(static_cast<intptr_t>(m_nExtraData)));
        SetUpSizeIndexData();
        break;
    case 0xE:
        ProcessClimate();
        SetUpSizeIndexData();
        break;
    case 0xF:
        ProcessWorldMap(1);
        break;
    case 0x10:
        ProcessCoupleRing();
        SetUpSizeIndexData();
        break;
    case 0x11:
    {
        m_textDesc.SetPos(130, 10);
        char* parsedText = g_DCTTextManager.GetParsedText(
            static_cast<int>(m_usKindID), 0, nullptr);
        m_textDesc.SetText(parsedText);
        SetUpSizeIndexData();
        break;
    }
    default:
        return;
    }
}

// ========================================
// CToolTip::HideSectionBar  (對齊 00427510)
// ========================================
void CToolTip::HideSectionBar()
{
    for (int i = m_nSectionBarCount; i < 5; i++)
    {
        m_sectionBar[i].Hide();
    }
}

// ========================================
// CToolTip::SetUpSizeIndexData  (對齊 00427550)
// ========================================
void CToolTip::SetUpSizeIndexData()
{
    if (m_nIndexCount > 20)
        return;

    // 隱藏未使用的標籤/值行
    for (int i = m_nIndexCount; i < 20; i++)
    {
        m_labelText[i].Hide();
        m_valueText[i].Hide();
    }

    // 隱藏 void index 行（僅標籤）
    for (int i = 0; i < m_nVoidCount; i++)
    {
        m_labelText[m_voidIndices[i]].Hide();
    }

    // 計算高度
    int height;
    if (m_nIndexCount <= 0)
    {
        // 無索引資料：使用標題文字高度
        DWORD textSize[2];
        m_textTitle.GetTextLength(textSize);
        height = static_cast<int>(static_cast<uint16_t>(textSize[1])) + 20;
    }
    else
    {
        // 使用最後一個值行的位置 + 文字高度
        DWORD textSize[2];
        m_valueText[m_nIndexCount - 1].GetTextLength(textSize);
        int lastTextHeight = static_cast<int>(static_cast<uint16_t>(textSize[1]));
        int lastAbsY = m_valueText[m_nIndexCount - 1].GetAbsY();
        int titleAbsY = m_textTitle.GetAbsY();
        height = lastAbsY + lastTextHeight - titleAbsY + 20;
    }

    // 若有描述文字，追加描述高度
    if (m_textDesc.IsStringData())
    {
        if (m_nType == 17)
        {
            // type 17 直接加上描述高度
            height = m_textDesc.GetCalcedTextBoxHeight(0) + height;
        }
        else
        {
            // 其他類型：描述在索引下方，額外留 10px 間距
            uint16_t descY = static_cast<uint16_t>(height + 10);
            m_textDesc.SetY(descY);
            height = descY + m_textDesc.GetCalcedTextBoxHeight(0) + 10;
        }
    }

    // 設定 innerBox 尺寸
    m_innerBox.SetWidth(260);
    m_innerBox.SetHeight(static_cast<uint16_t>(height));

    // 計算定位
    CalcPos();
}

// ========================================
// CToolTip::CalcPos  (對齊 004276B0)
// ========================================
void CToolTip::CalcPos()
{
    uint16_t w = m_innerBox.GetWidth();
    uint16_t h = m_innerBox.GetHeight();

    int posX = m_nMouseX;
    int posY;

    // 確保不超出螢幕右邊
    if (g_Game_System_Info.ScreenWidth - w - posX - 10 < 0)
        posX = g_Game_System_Info.ScreenWidth - w - 10;

    int mouseY = m_nMouseY;

    // 決定 Y：在游標下方或上方
    if (h + mouseY + 42 <= g_Game_System_Info.ScreenHeight)
    {
        // 下方：mouseY + 32
        posY = mouseY + 32;
        if (posY <= 15)
            posY = 15;
    }
    else
    {
        // 上方：mouseY - height - 10
        posY = mouseY - h - 10;
        if (posY <= 15)
            posY = 15;
    }

    m_innerBox.SetAbsPos(posX, posY);
    SetUpOutLayer();
}

// ========================================
// CToolTip::SetUpOutLayer  (對齊 00427730)
// ========================================
void CToolTip::SetUpOutLayer()
{
    int x = m_innerBox.GetAbsX();
    int y = m_innerBox.GetAbsY();
    int w = m_innerBox.GetWidth();
    int h = m_innerBox.GetHeight();

    // 左邊框（垂直）
    m_borderBox[0].SetAbsPos(x - 1, y - 1);
    m_borderBox[0].SetWidth(1);
    m_borderBox[0].SetHeight(static_cast<uint16_t>(h + 1));

    // 上邊框（水平）
    m_borderBox[1].SetAbsPos(x, y - 1);
    m_borderBox[1].SetWidth(static_cast<uint16_t>(w + 1));
    m_borderBox[1].SetHeight(1);

    // 右邊框（垂直）
    m_borderBox[2].SetAbsPos(x + w, y);
    m_borderBox[2].SetWidth(1);
    m_borderBox[2].SetHeight(static_cast<uint16_t>(h + 1));

    // 下邊框（水平）
    m_borderBox[3].SetAbsPos(x - 1, y + h);
    m_borderBox[3].SetWidth(static_cast<uint16_t>(w + 1));
    m_borderBox[3].SetHeight(1);
}

void CToolTip::SetIndex(uint16_t textCode)
{
    const char* label = g_DCTTextManager.GetText(textCode);
    char buffer[256] = {};
    std::snprintf(buffer, sizeof(buffer), "%s : ", (label ? label : ""));

    m_labelText[m_nIndexCount].SetText(buffer);

    DWORD textSize[2] = {};
    m_labelText[m_nIndexCount].GetTextLength(textSize);
    const int labelWidth = static_cast<int>(LOWORD(textSize[0]));
    m_valueText[m_nIndexCount].SetX(m_labelText[m_nIndexCount].GetX() + labelWidth);
}

void CToolTip::AddIndexData(uint16_t textCode, int value, unsigned int color)
{
    char buffer[256] = {};
    SetIndex(textCode);
    std::snprintf(buffer, sizeof(buffer), "%d", value);
    m_valueText[m_nIndexCount].SetText(buffer);
    m_valueText[m_nIndexCount++].m_TextColor = color;
}
void CToolTip::AddIndexData(uint16_t textCode, char* value, unsigned int color)
{
    char buffer[256] = {};
    SetIndex(textCode);
    std::snprintf(buffer, sizeof(buffer), "%s", (value ? value : ""));
    m_valueText[m_nIndexCount].SetText(buffer);
    m_valueText[m_nIndexCount++].m_TextColor = color;
}
void CToolTip::AddIndexData(uint16_t textCode, char* format, int arg)
{
    char buffer[256] = {};
    SetIndex(textCode);
    std::snprintf(buffer, sizeof(buffer), (format ? format : "%d"), arg);
    m_valueText[m_nIndexCount].SetText(buffer);
    ++m_nIndexCount;
}
void CToolTip::AddIndexData(uint16_t textCode, char* format, float arg)
{
    char buffer[256] = {};
    SetIndex(textCode);
    std::snprintf(buffer, sizeof(buffer), (format ? format : "%f"), arg);
    m_valueText[m_nIndexCount].SetText(buffer);
    ++m_nIndexCount;
}
void CToolTip::AddIndexData(uint16_t textCode, char* format, int arg1, int arg2)
{
    char buffer[256] = {};
    SetIndex(textCode);
    std::snprintf(buffer, sizeof(buffer), (format ? format : "%d"), arg1, arg2);
    m_valueText[m_nIndexCount].SetText(buffer);
    ++m_nIndexCount;
}

void CToolTip::AddNextLineData(char* text)
{
    m_valueText[m_nIndexCount].SetText(text);
    m_valueText[m_nIndexCount].SetX(20);
    ++m_nIndexCount;
}

void CToolTip::AddDesc(uint16_t textCode, int itemFlag)
{
    if (textCode == 0)
    {
        m_textDesc.SetText("");
        return;
    }

    if (itemFlag)
    {
        char buffer[1024] = {};
        char backup[1024] = {};
        stItemKindInfo* itemInfo = g_clItemKindInfo.GetItemKindInfo(m_usKindID);
        if (!itemInfo)
            return;

        std::strcpy(buffer, g_DCTTextManager.GetParsedText(textCode, 0, nullptr));
        if (itemInfo->m_wItemType == 27 || itemInfo->m_wItemType == 28)
        {
            std::strcpy(backup, buffer);
            const char* fmt = g_DCTTextManager.GetParsedText(4743, 0, nullptr);
            std::snprintf(buffer, sizeof(buffer), (fmt ? fmt : "%s"), backup);
        }
        if (itemInfo->m_byQuestCollect == 1)
        {
            std::strcpy(backup, buffer);
            const char* fmt = g_DCTTextManager.GetParsedText(4744, 0, nullptr);
            std::snprintf(buffer, sizeof(buffer), (fmt ? fmt : "%s"), backup);
        }
        m_textDesc.SetText(buffer);
        return;
    }

    m_textDesc.SetText(g_DCTTextManager.GetParsedText(textCode, 0, nullptr));
}
void CToolTip::AddDesc(char* text)
{
    m_textDesc.SetText(text);
}

void CToolTip::AddSectionBar()
{
    const int y = m_labelText[m_nIndexCount].GetY();
    m_sectionBar[m_nSectionBarCount].SetY(y + 5);
    ++m_nSectionBarCount;
}

void CToolTip::ProcessStatic()
{
    char* text = g_DCTTextManager.GetText(m_usKindID);
    ProcessOneLineText(text);
}
void CToolTip::ProcessOneLineText(char* text)
{
    m_textMain.SetText(text);

    uint16_t width = 0;
    int textX = 10;
    if (m_nCompareFlag)
    {
        m_textMain.m_isCentered = 1;
        width = 260;
        textX = 130;
    }
    else
    {
        DWORD textSize[2] = {};
        m_textMain.GetTextLength(textSize);
        width = static_cast<uint16_t>(LOWORD(textSize[0]) + 20);
        m_textMain.m_isCentered = 0;
    }
    m_textMain.SetX(textX);

    DWORD textSize[2] = {};
    m_textMain.GetTextLength(textSize);
    const uint16_t height = static_cast<uint16_t>(static_cast<uint16_t>(textSize[1]) + 20);
    m_innerBox.SetWidth(width);
    m_innerBox.SetHeight(height);
    CalcPos();
}
void CToolTip::ProcessWorldMap(int mode)
{
    // 對齊 ground truth 00427EA0

    // mode==0: 從 CUIWorldMap 取得滑鼠位置對應的 map kind
    if (!mode)
    {
        CUIWorldMap* pWorldMap = reinterpret_cast<CUIWorldMap*>(g_UIMgr->GetUIWindow(19));
        if (!pWorldMap)
            return;
        m_usKindID = pWorldMap->GetToolTipData(m_nMouseX, m_nMouseY);
    }

    uint16_t mapKind = m_usKindID;
    if (!mapKind)
    {
        m_innerBox.Hide();
        return;
    }

    char Buffer[256];
    char Buffer2[256];
    Buffer[0] = '\0';
    Buffer2[0] = '\0';
    memset(&Buffer[1], 0, 0xFCu);
    *reinterpret_cast<uint16_t*>(&Buffer[253]) = 0;
    Buffer[255] = 0;
    memset(&Buffer2[1], 0, 252);
    *reinterpret_cast<uint16_t*>(&Buffer2[249]) = 0;
    Buffer2[251] = 0;

    stMapInfo* mapInfo = g_Map.GetMapInfoByID(mapKind);
    stMapInfo* savedMapInfo = mapInfo;
    if (!mapInfo)
        return;

    SetTextMainTitle(mapInfo);
    SetTextDungeonBasic(mapInfo);

    // 條件：field map (regionType==1) 且無 dungeon name code
    if (mapInfo->m_wRegionType == 1 && !mapInfo->m_wDungeonNameCode)
    {
        uint16_t* regenMonKinds = g_cltRegenMonsterKindInfo.GetRegenMonsterKindByMapID(m_usKindID);
        if (!regenMonKinds)
            return;

        for (int i = 0; i < 5; ++i)
        {
            if (m_nWorldMapTextCount > 50)
                break;
            if (!regenMonKinds[i])
                continue;

            memset(Buffer, 0, sizeof(Buffer));
            auto* charInfo = reinterpret_cast<unsigned char*>(g_pcltCharKindInfo->GetCharKindInfo(regenMonKinds[i]));
            if (!charInfo)
                continue;

            int level = *(charInfo + 146);
            const char* monName = g_DCTTextManager.GetText(*reinterpret_cast<uint16_t*>(charInfo + 2));
            sprintf(Buffer, "%s(Lv%d)", monName, level);

            int alignment = *reinterpret_cast<int*>(charInfo + 212);
            if (alignment < 1)
            {
                if (alignment)
                    m_worldMapText[m_nWorldMapTextCount].m_TextColor = 0xFFFFFFFF; // -1 (white)
                else
                    m_worldMapText[m_nWorldMapTextCount].m_TextColor = 0xFFFFFF00; // -256 (yellow)
            }
            else
            {
                m_worldMapText[m_nWorldMapTextCount].m_TextColor = 0xFFFF0000; // -65536 (red)
            }
            m_worldMapText[m_nWorldMapTextCount].SetText(Buffer);
            ++m_nWorldMapTextCount;
        }
    }

    // Quest 導向資訊
    uint8_t questIdx = 0;
    int questLoopIdx = 0;
    if (g_clQuestSystem.GetRunningQuestCount())
    {
        do
        {
            stPlayingQuestInfo* questInfo = g_clQuestSystem.GetRunningQuestInfoByIndex(questIdx);
            if (!questInfo)
                goto NEXT_QUEST;

            {
                stQuestKindInfo* qk = g_clQuestKindInfo.GetQuestKindInfo(questInfo->wQuestID);
                if (!qk)
                    goto NEXT_QUEST;

                uint8_t playType = qk->bPlayType;
                uint16_t monKind;

                if (playType == 2)
                {
                    // HUNT: monster kind at offset 96
                    monKind = *reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(qk) + 96);
                    goto REGEN_MAP_SEARCH;

                REGEN_MAP_SEARCH_ENTRY_TYPE1:
                    // COLLECTION: monster kind at offset 100
                    monKind = *reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(qk) + 100);

                REGEN_MAP_SEARCH:
                    {
                        uint16_t* regenMaps = g_cltRegenMonsterKindInfo.GetRegenMapIDByMonsterKind(monKind);
                        int foundCount = 0;
                        int mapLoopIdx = 0;
                        uint16_t* mapPtr = regenMaps;
                        do
                        {
                            if (m_nWorldMapTextCount > 50)
                                break;
                            if (*mapPtr)
                            {
                                ++foundCount;
                                stMapInfo* regenMapInfo = g_Map.GetMapInfoByID(*mapPtr);
                                if (regenMapInfo)
                                {
                                    // 條件：(非 field 或有 dungeon name) 且 dungeon name 匹配
                                    if ((regenMapInfo->m_wRegionType != 1 || regenMapInfo->m_wDungeonNameCode)
                                        && regenMapInfo->m_wDungeonNameCode == savedMapInfo->m_wDungeonNameCode)
                                    {
                                        char* mapName = const_cast<char*>(g_DCTTextManager.GetText(regenMapInfo->m_wFileName));
                                        m_worldMapText[m_nWorldMapTextCount].SetText(mapName);
                                        ++m_nWorldMapTextCount;

                                        // 掃描該地圖的 5 隻重生怪，找 quest 目標
                                        uint16_t* regenMons = g_cltRegenMonsterKindInfo.GetRegenMonsterKindByMapID(*mapPtr);
                                        int monLoop = 5;
                                        do
                                        {
                                            uint16_t curMon = *regenMons;
                                            // 檢查是否為 quest 目標（offset 96 或 100）
                                            uint16_t target1 = *reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(qk) + 96);
                                            uint16_t target2 = *reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(qk) + 100);
                                            if (curMon && (target1 == curMon || target2 == curMon))
                                            {
                                                memset(Buffer, 0, sizeof(Buffer));
                                                auto* charInfo = reinterpret_cast<unsigned char*>(g_pcltCharKindInfo->GetCharKindInfo(curMon));
                                                if (charInfo)
                                                {
                                                    int level = *(charInfo + 146);
                                                    const char* monName = g_DCTTextManager.GetText(*reinterpret_cast<uint16_t*>(charInfo + 2));
                                                    sprintf(Buffer, "%s(Lv%d)", monName, level);

                                                    int alignment = *reinterpret_cast<int*>(charInfo + 212);
                                                    if (alignment < 1)
                                                    {
                                                        if (alignment)
                                                            m_worldMapText[m_nWorldMapTextCount].m_TextColor = 0xFFFFFFFF;
                                                        else
                                                            m_worldMapText[m_nWorldMapTextCount].m_TextColor = 0xFFFFFF00;
                                                    }
                                                    else
                                                    {
                                                        m_worldMapText[m_nWorldMapTextCount].m_TextColor = 0xFFFF0000;
                                                    }
                                                    m_worldMapText[m_nWorldMapTextCount].SetText(Buffer);
                                                    ++m_nWorldMapTextCount;
                                                }
                                            }
                                            ++regenMons;
                                            --monLoop;
                                        } while (monLoop);
                                    }
                                }
                            }
                            ++mapLoopIdx;
                            ++mapPtr;
                        } while (mapLoopIdx < 1);

                        if (foundCount)
                            goto NEXT_QUEST;
                        if (m_nWorldMapTextCount > 50)
                            break;

                        // NPC fallback: offset 102
                        uint16_t fallbackNpcKind = *reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(qk) + 102);
                        stNPCInfo* fallbackNpc = g_clNPCInfo.GetNPCInfoByID(fallbackNpcKind);
                        if (!fallbackNpc)
                            goto NEXT_QUEST;
                        stMapInfo* npcMap = g_Map.GetMapInfoByID(fallbackNpc->m_wMapID);
                        if (npcMap->m_wDungeonNameCode != savedMapInfo->m_wDungeonNameCode)
                            goto NEXT_QUEST;

                        const char* mapName2 = g_DCTTextManager.GetText(npcMap->m_wFileName);
                        const char* npcName2 = g_DCTTextManager.GetText(fallbackNpc->m_wNameCode);
                        sprintf(Buffer, "%s(%s)", npcName2, mapName2);
                        m_worldMapText[m_nWorldMapTextCount].m_TextColor = 0xFFFFFF00;
                        m_worldMapText[m_nWorldMapTextCount].SetText(Buffer);
                        ++m_nWorldMapTextCount;
                    }
                }
                else if (playType == 1)
                {
                    goto REGEN_MAP_SEARCH_ENTRY_TYPE1;
                }
                else if (playType != 3 && playType != 6 && playType != 5 && playType != 4 && playType != 7)
                {
                    goto NEXT_QUEST;
                }
                else
                {
                    // Types 3,4,5,6,7: NPC-based quest tracking
                    // NPC from offset 98
                    stNPCInfo* npc = g_clNPCInfo.GetNPCInfoByID(
                        *reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(qk) + 98));

                    // Type-specific NPC override
                    switch (playType)
                    {
                    case 4:
                    {
                        uint16_t npcCode = cltNPCInfo::TranslateKindCode("N0015");
                        npc = g_clNPCInfo.GetNPCInfoByID(npcCode);
                        break;
                    }
                    case 7:
                    {
                        uint16_t npcCode = cltNPCInfo::TranslateKindCode("N0020");
                        npc = g_clNPCInfo.GetNPCInfoByID(npcCode);
                        break;
                    }
                    case 5:
                    {
                        // Special: 4 NPC loop (N0016~N0019)
                        uint16_t npcIds[4];
                        npcIds[0] = g_clNPCInfo.GetNPCInfoByID(cltNPCInfo::TranslateKindCode("N0016"))->m_wKind;
                        npcIds[1] = g_clNPCInfo.GetNPCInfoByID(cltNPCInfo::TranslateKindCode("N0017"))->m_wKind;
                        npcIds[2] = g_clNPCInfo.GetNPCInfoByID(cltNPCInfo::TranslateKindCode("N0018"))->m_wKind;
                        npcIds[3] = g_clNPCInfo.GetNPCInfoByID(cltNPCInfo::TranslateKindCode("N0019"))->m_wKind;

                        for (int n = 0; n < 4; ++n)
                        {
                            if (m_nWorldMapTextCount > 50)
                                break;
                            stNPCInfo* loopNpc = g_clNPCInfo.GetNPCInfoByID(npcIds[n]);
                            if (!loopNpc)
                                continue;
                            stMapInfo* loopMap = g_Map.GetMapInfoByID(loopNpc->m_wMapID);
                            if (!loopMap)
                                continue;
                            if (savedMapInfo->m_wDungeonNameCode != loopMap->m_wDungeonNameCode)
                                continue;

                            const char* loopMapName = g_DCTTextManager.GetText(loopMap->m_wFileName);
                            const char* loopNpcName = g_DCTTextManager.GetText(loopNpc->m_wNameCode);
                            sprintf(Buffer2, "%s(%s)", loopNpcName, loopMapName);
                            m_worldMapText[m_nWorldMapTextCount].m_TextColor = 0xFFFFFF00;
                            m_worldMapText[m_nWorldMapTextCount].SetText(Buffer2);
                            ++m_nWorldMapTextCount;
                        }
                        goto NEXT_QUEST;
                    }
                    default:
                        break;
                    }

                    // Common path for types 3,4,6,7
                    if (!npc)
                        goto NEXT_QUEST;

                    stMapInfo* npcMap = g_Map.GetMapInfoByID(npc->m_wMapID);
                    if (!npcMap)
                        goto NEXT_QUEST;

                    // field map check
                    if (savedMapInfo->m_wRegionType == 1)
                        goto NEXT_QUEST;

                    // dungeon name match check
                    uint16_t dungeonCode = savedMapInfo->m_wDungeonNameCode;
                    if (npcMap->m_wDungeonNameCode != dungeonCode
                        || (!dungeonCode && savedMapInfo->m_wID != npcMap->m_wID))
                        goto NEXT_QUEST;

                    {
                        const char* mapNameStr = g_DCTTextManager.GetText(npcMap->m_wFileName);
                        const char* npcNameStr = g_DCTTextManager.GetText(npc->m_wNameCode);
                        sprintf(Buffer2, "%s(%s)", npcNameStr, mapNameStr);
                        m_worldMapText[m_nWorldMapTextCount].m_TextColor = 0xFFFFFF00;
                        m_worldMapText[m_nWorldMapTextCount].SetText(Buffer2);
                        ++m_nWorldMapTextCount;
                    }
                }
            }

        NEXT_QUEST:
            questIdx = static_cast<uint8_t>(++questLoopIdx);
        } while (questLoopIdx < g_clQuestSystem.GetRunningQuestCount());
    }

    // 尺寸計算（對齊 ground truth 收尾）
    uint16_t totalWidth = 0;
    int16_t totalHeight = 0;
    CControlText* pText = &m_worldMapText[0];
    int remaining = 50;
    do
    {
        DWORD sizeOut[2] = {};
        pText->GetTextLength(sizeOut);
        uint16_t lineW = static_cast<uint16_t>(sizeOut[0] & 0xFFFF);
        if (totalWidth < static_cast<uint16_t>(lineW + 20))
        {
            DWORD sizeOut2[2] = {};
            pText->GetTextLength(sizeOut2);
            totalWidth = static_cast<uint16_t>((sizeOut2[0] & 0xFFFF) + 20);
        }
        if (pText->IsStringData())
        {
            DWORD sizeOut3[2] = {};
            pText->GetTextLength(sizeOut3);
            totalHeight += static_cast<int16_t>(static_cast<uint16_t>(sizeOut3[1] & 0xFFFF) + 7);
        }
        pText = reinterpret_cast<CControlText*>(reinterpret_cast<char*>(pText) + 432);
        --remaining;
    } while (remaining);

    m_innerBox.SetWidth(totalWidth);
    m_innerBox.SetHeight(static_cast<uint16_t>(totalHeight + 20));
    CalcPos();
    m_nWorldMapTextCount = 0;
}
void CToolTip::ProcessLesson()
{
    strLessonKindInfo* lessonInfo = g_clLessonKindInfo.GetLessonKindInfo(static_cast<uint8_t>(m_usKindID));
    if (!lessonInfo)
        return;

    m_textTitle.SetText(g_DCTTextManager.GetText(lessonInfo->wNameCode));
    AddDesc(lessonInfo->wDescriptionCode, 0);
}
void CToolTip::ProcessItem()
{
    // 對齊 004287D0：先判斷是否為裝備類物品
    if (g_clItemKindInfo.IsEquipItem(m_usKindID))
    {
        ProcessHunt();
        return;
    }

    // 非裝備物品路線
    stItemKindInfo* v2 = g_clItemKindInfo.GetItemKindInfo(m_usKindID);
    if (!v2)
        return;

    const auto W = reinterpret_cast<int16_t*>(v2);
    const auto UW = reinterpret_cast<uint16_t*>(v2);
    const auto DW = reinterpret_cast<int32_t*>(v2);
    const auto UDW = reinterpret_cast<uint32_t*>(v2);
    const auto B = reinterpret_cast<uint8_t*>(v2);

    // 標題
    m_textTitle.SetText(g_DCTTextManager.GetText(UW[1]));

    // 類型名稱
    int typeCode = ExGetTextCodeInstantItemType(B[80]);
    AddIndexData(0x0C28u, g_DCTTextManager.GetText(typeCode), 0xFFFFC000);

    // MapUseItem 分支
    uint16_t mapUseItemId = UW[105];
    if (mapUseItemId)
    {
        strMapUseItemInfoKindInfo* v7 = g_clMapUseItemInfoKindInfo.GetMapUseItemInfoKindInfo(mapUseItemId);
        if (v7)
        {
            const auto MDW = reinterpret_cast<int32_t*>(v7);

            // 重設標題
            m_textTitle.SetText(g_DCTTextManager.GetText(UW[1]));

            // extra data 字串
            char* extraStr = reinterpret_cast<char*>(static_cast<intptr_t>(m_nExtraData));
            if (extraStr)
                AddIndexData(0x1FD1u, extraStr, 0xFFFFC000);

            if (MDW[17])
                AddIndexData(0x0C3Eu, g_DCTTextManager.GetText(3244), MDW[17]);
            if (MDW[18])
                AddIndexData(0x0C3Fu, g_DCTTextManager.GetText(3244), MDW[18]);
            if (MDW[19])
                AddIndexData(0x0C33u, g_DCTTextManager.GetText(3636), SetRatePerThousand(MDW[19]));
            if (MDW[20])
                AddIndexData(0x0C54u, g_DCTTextManager.GetText(3636), SetRatePerThousand(MDW[20]));
            if (MDW[21])
                AddIndexData(0x0C34u, g_DCTTextManager.GetText(3636), SetRatePerThousand(MDW[21]));
            if (MDW[22])
                AddIndexData(0x1FF7u, g_DCTTextManager.GetText(3636), SetRatePerThousand(MDW[22]));

            AddDesc(reinterpret_cast<uint16_t*>(v7)[7], 1);
        }
    }
    else
    {
        // 非 MapUseItem：一般消耗品
        if (W[41])
            AddIndexData(0x0C4Eu, static_cast<int>(W[41]), 0xFFFFC000);
        if (W[42])
            AddIndexData(0x0C4Fu, static_cast<int>(W[42]), 0xFFFFC000);
        if (W[43])
            AddIndexData(0x0C59u, g_DCTTextManager.GetText(3244), static_cast<int>(W[43]));
        if (W[44])
            AddIndexData(0x0C5Au, g_DCTTextManager.GetText(3244), static_cast<int>(W[44]));
        if (W[54])
            AddIndexData(0x0C5Bu, g_DCTTextManager.GetText(3244), static_cast<int>(W[54]));
        if (W[55])
            AddIndexData(0x0C5Cu, g_DCTTextManager.GetText(3244), static_cast<int>(W[55]));
        if (W[50])
            AddIndexData(0x0C5Du, static_cast<int>(W[50]), 0xFFFFC000);
        if (W[51])
            AddIndexData(0x0C5Eu, static_cast<int>(W[51]), 0xFFFFC000);
        if (W[53])
            AddIndexData(0x0C60u, static_cast<int>(W[53]), 0xFFFFC000);
        if (W[52])
            AddIndexData(0x0C5Fu, static_cast<int>(W[52]), 0xFFFFC000);
        if (DW[65])
            AddIndexData(0x0C50u, g_DCTTextManager.GetText(3244), DW[65]);
        if (DW[66])
            AddIndexData(0x0C51u, g_DCTTextManager.GetText(3244), DW[66]);
        if (W[56])
            AddIndexData(0x0C54u, g_DCTTextManager.GetText(3636), SetRatePerThousand(W[56]));
        if (W[58])
            AddIndexData(0x0C34u, g_DCTTextManager.GetText(3636), SetRatePerThousand(W[58]));
        if (W[57])
            AddIndexData(0x0C33u, g_DCTTextManager.GetText(3636), SetRatePerThousand(W[57]));

        int useTerm = DW[23];
        if (useTerm)
            AddIndexData(0x0C46u, useTerm, 0xFFFFC000);

        // 時間格式化（word[136] != 0）
        if (UW[136])
        {
            int totalHours = DW[69];
            int days = totalHours / 24;
            int hours = totalHours % 24;

            char dayBuf[128] = {};
            char hourBuf[128] = {};
            char combinedBuf[256] = {};

            if (days)
                std::snprintf(dayBuf, sizeof(dayBuf), g_DCTTextManager.GetText(3179), days);
            if (hours)
                std::snprintf(hourBuf, sizeof(hourBuf), g_DCTTextManager.GetText(4863), hours);

            std::snprintf(combinedBuf, sizeof(combinedBuf), "%s%s", dayBuf, hourBuf);
            AddIndexData(0x0C46u, combinedBuf, 0xFFFFC000);
        }

        // 時裝性別判定
        if (g_clUsingItemSystem.IsCosmeticItem(UW[0]))
        {
            uint8_t mySex = g_clSexSystem.GetSex();
            char itemSex = 0;
            if (DW[44])
                itemSex = B[172];
            else if (DW[46])
                itemSex = B[181];

            unsigned int sexColor;
            int sexTextCode;
            if (itemSex == 'F')
            {
                sexColor = (mySex != 'F') ? 0xFFFF4600u : 0xFFFFC000u;
                sexTextCode = 3181;
            }
            else if (itemSex == 'M')
            {
                sexColor = (mySex != 'M') ? 0xFFFF4600u : 0xFFFFC000u;
                sexTextCode = 3180;
            }
            else
            {
                sexColor = 0xFFFFC000u;
                sexTextCode = 3178;
            }
            AddIndexData(0x0C3Bu, g_DCTTextManager.GetText(sexTextCode), sexColor);
        }

        OutputCashShopTime(v2);

        // ProcessSkill / ProcessPetSkill
        uint16_t skillId = UW[104];
        if (skillId)
            ProcessSkill(skillId);
        uint16_t petSkillId = UW[119];
        if (petSkillId)
            ProcessPetSkill(petSkillId);

        AddDesc(UW[2], 1);
    }
}
void CToolTip::ProcessMakingItem()
{
    // 對齊 00428DE0
    stItemKindInfo* item = g_clItemKindInfo.GetItemKindInfo(m_usKindID);
    if (!item)
        return;

    const auto UW = reinterpret_cast<uint16_t*>(item);

    m_textTitle.SetText(g_DCTTextManager.GetText(UW[1]));

    uint16_t specialtyKind = UW[69];
    if (!specialtyKind)
        return;

    strSpecialtyKindInfo* v4 = g_clSpecialtyKindInfo.GetSpecialtyKindInfo(specialtyKind);
    if (!v4)
        return;

    uint16_t specTextCode = reinterpret_cast<uint16_t*>(v4)[1];
    if (specTextCode)
    {
        // 顏色取決於是否已習得
        unsigned int specColor;
        if (g_clSpecialtySystem.IsAcquiredSpecialty(specialtyKind))
            specColor = 0xFFFFC000u;  // -16384
        else
            specColor = 0xFFFF4600u;  // -47616
        AddIndexData(0x0F09u, g_DCTTextManager.GetText(specTextCode), specColor);
    }

    uint16_t makingKind = UW[70];
    if (!makingKind)
        return;

    strMakingItemKindInfo* v8 = g_clMakingItemKindInfo.GetMakingItemKindInfo(makingKind);
    if (!v8)
        return;

    int v16 = 0; // materialLineCount
    AddIndexData(0x0F0Au, const_cast<char*>(""), 0xFFFFC000);

    // 材料循環：v9 指向 (char*)v8 + 22，每次 +=2 words（4 bytes）
    uint16_t* v9 = reinterpret_cast<uint16_t*>(reinterpret_cast<char*>(v8) + 22);
    for (int i = 0; i < 10; ++i, v9 += 2)
    {
        uint16_t matKind = *(v9 - 1);
        if (!matKind)
            continue;

        stItemKindInfo* matItem = g_clItemKindInfo.GetItemKindInfo(matKind);
        if (!matItem)
            continue;

        uint16_t matCount = *v9;
        char Buffer[256] = {};
        const char* suffix = g_DCTTextManager.GetText(3109);
        const char* matName = g_DCTTextManager.GetText(reinterpret_cast<uint16_t*>(matItem)[1]);
        std::snprintf(Buffer, sizeof(Buffer), "%s %d%s", matName, static_cast<int>(matCount), suffix);
        AddNextLineData(Buffer);
        ++v16;
    }

    // 無條件呼叫 SetVoidIndex（GT 不檢查 v16 > 0）
    SetVoidIndex(m_nIndexCount - v16, m_nIndexCount);

    // 成功率：FP 計算 (double)SuccessPermyriad * 0.01
    const auto MDW = reinterpret_cast<int32_t*>(v8);
    char Buffer[256] = {};
    std::snprintf(Buffer, sizeof(Buffer), "%d%%",
        static_cast<unsigned int>(static_cast<int64_t>(static_cast<double>(MDW[3]) * 0.0099999998)));
    AddIndexData(0x0F0Bu, Buffer, 0xFFFFC000);
    AddIndexData(0x0F0Cu, MDW[4], 0xFFFFC000);

    AddDesc(UW[2], 1);
}
void CToolTip::ProcessHunt()
{
    // 對齊 00428FD0
    stItemKindInfo* v2 = g_clItemKindInfo.GetItemKindInfo(m_usKindID);
    if (!v2)
        return;

    const auto W = reinterpret_cast<int16_t*>(v2);
    const auto UW = reinterpret_cast<uint16_t*>(v2);
    const auto DW = reinterpret_cast<int32_t*>(v2);
    const auto UDW = reinterpret_cast<uint32_t*>(v2);
    const auto B = reinterpret_cast<uint8_t*>(v2);

    // 精靈系統
    uint8_t myLevel = ExGetMyLevel();
    uint16_t spiritKind = g_clSpiritSystem.GetSpiritKind(myLevel);
    uint16_t* v5 = reinterpret_cast<uint16_t*>(g_clSpiritSystem.GetSpiritInfo(spiritKind));

    // 標題
    m_textTitle.SetText(g_DCTTextManager.GetText(UW[1]));

    unsigned int equipAtb = UDW[20]; // byte 80
    int huntTypeTextCode = ExGetTextCodeHuntItemType(equipAtb);

    char Buffer[256] = {};

    switch (equipAtb)
    {
    case 0x8000u:
    case 0x10000u:
    {
        // 武器
        const char* typeText = g_DCTTextManager.GetText(huntTypeTextCode);
        const char* weaponText = GetWeaponTypeText(UW[42]);
        std::snprintf(Buffer, sizeof(Buffer), "%s(%s)", weaponText, typeText);
        AddIndexData(0x0C28u, Buffer, 0xFFFFC000);
        // 攻擊範圍
        AddIndexData(0x0C29u, g_DCTTextManager.GetText(3176), static_cast<int>(W[43]), static_cast<int>(W[44]));
        // 防禦
        if (DW[46])
            AddIndexData(0x0ECFu, DW[46], 0xFFFFC000);
        // 攻速
        if (static_cast<uint16_t>(UW[105]))
            AddIndexData(0x0ED0u, static_cast<int>(UW[105]), 0xFFFFC000);
        // 屬性
        int attrCode = ExGetTextCodeHuntItemAttribute(v2);
        AddIndexData(0x0EDBu, g_DCTTextManager.GetText(attrCode), 0xFFFFC000);
        // 命中率 (permil)
        if (W[78])
            AddIndexData(0x0C34u, g_DCTTextManager.GetText(3636), SetRatePerThousand(W[78]));
        // HP%
        if (DW[35])
            AddIndexData(0x0C2Fu, g_DCTTextManager.GetText(3244), DW[35]);
        // MP Add
        if (UW[68])
            AddIndexData(0x0C2Eu, g_DCTTextManager.GetText(3244), static_cast<int>(UW[68]));
        // 攻增
        if (W[72])
            AddIndexData(0x0C30u, g_DCTTextManager.GetText(3244), static_cast<int>(W[72]));
        if (W[73])
            AddIndexData(0x0C31u, g_DCTTextManager.GetText(3244), static_cast<int>(W[73]));
        // accuracy/critical/magicresist/evasion
        if (W[61])
            AddIndexData(0x0C5Du, static_cast<int>(W[61]), 0xFFFFC000);
        if (W[63])
            AddIndexData(0x0C5Eu, static_cast<int>(W[63]), 0xFFFFC000);
        if (W[62])
            AddIndexData(0x0C60u, static_cast<int>(W[62]), 0xFFFFC000);
        if (W[64])
            AddIndexData(0x0C5Fu, static_cast<int>(W[64]), 0xFFFFC000);
        // 命中率/迴避率 (permil)
        if (W[76])
            AddIndexData(0x0C32u, g_DCTTextManager.GetText(3636), SetRatePerThousand(W[76]));
        if (W[77])
            AddIndexData(0x0C33u, g_DCTTextManager.GetText(3636), SetRatePerThousand(W[77]));
        break;
    }
    case 0x1000u:
    case 0x20000u:
    case 0x40000u:
    case 0x80000u:
    case 0x100000u:
    case 0x200000u:
    {
        // 防具
        AddIndexData(0x0C28u, g_DCTTextManager.GetText(huntTypeTextCode), 0xFFFFC000);
        AddIndexData(0x0C38u, static_cast<int>(UW[49]), 0xFFFFC000);
        if (W[72])
            AddIndexData(0x0C30u, g_DCTTextManager.GetText(3244), static_cast<int>(W[72]));
        if (W[73])
            AddIndexData(0x0C31u, g_DCTTextManager.GetText(3244), static_cast<int>(W[73]));
        if (DW[46])
            AddIndexData(0x0EDCu, DW[46], 0xFFFFC000);
        if (DW[35])
            AddIndexData(0x0C2Fu, g_DCTTextManager.GetText(3244), DW[35]);
        break;
    }
    case 0x2000u:
    case 0x4000u:
    {
        // 盾牌/飾品
        AddIndexData(0x0C28u, g_DCTTextManager.GetText(huntTypeTextCode), 0xFFFFC000);
        if (DW[33])
            AddIndexData(0x0C2Du, g_DCTTextManager.GetText(3244), DW[33]);
        if (DW[46])
            AddIndexData(0x0EDCu, DW[46], 0xFFFFC000);
        if (UW[68])
            AddIndexData(0x0C2Eu, g_DCTTextManager.GetText(3244), static_cast<int>(UW[68]));
        if (DW[35])
            AddIndexData(0x0C2Fu, g_DCTTextManager.GetText(3244), DW[35]);
        if (W[72])
            AddIndexData(0x0C5Bu, g_DCTTextManager.GetText(3244), static_cast<int>(W[72]));
        if (W[73])
            AddIndexData(0x0C5Cu, g_DCTTextManager.GetText(3244), static_cast<int>(W[73]));
        if (W[74])
            AddIndexData(0x0C3Eu, g_DCTTextManager.GetText(3636), SetRatePerThousand(W[74]));
        if (W[75])
            AddIndexData(0x0C3Fu, g_DCTTextManager.GetText(3636), SetRatePerThousand(W[75]));
        if (W[76])
            AddIndexData(0x0C32u, g_DCTTextManager.GetText(3636), SetRatePerThousand(W[76]));
        if (W[77])
            AddIndexData(0x0C33u, g_DCTTextManager.GetText(3636), SetRatePerThousand(W[77]));
        if (W[78])
            AddIndexData(0x0C34u, g_DCTTextManager.GetText(3636), SetRatePerThousand(W[78]));
        if (W[61])
            AddIndexData(0x0C5Du, static_cast<int>(W[61]), 0xFFFFC000);
        if (W[63])
            AddIndexData(0x0C5Eu, static_cast<int>(W[63]), 0xFFFFC000);
        if (W[62])
            AddIndexData(0x0C60u, static_cast<int>(W[62]), 0xFFFFC000);
        if (W[64])
            AddIndexData(0x0C5Fu, static_cast<int>(W[64]), 0xFFFFC000);
        if (W[79])
            AddIndexData(0x1066u, g_DCTTextManager.GetText(3244), static_cast<int>(W[79]));
        if (W[80])
            AddIndexData(0x1067u, g_DCTTextManager.GetText(3244), static_cast<int>(W[80]));
        if (W[81])
            AddIndexData(0x1068u, g_DCTTextManager.GetText(3244), static_cast<int>(W[81]));
        if (W[82])
            AddIndexData(0x1069u, g_DCTTextManager.GetText(3244), static_cast<int>(W[82]));
        if (W[83])
            AddIndexData(0x1325u, g_DCTTextManager.GetText(3244), static_cast<int>(W[83]));
        if (W[84])
            AddIndexData(0x1326u, g_DCTTextManager.GetText(3244), static_cast<int>(W[84]));
        if (W[85])
            AddIndexData(0x1327u, g_DCTTextManager.GetText(3244), static_cast<int>(W[85]));
        if (W[86])
            AddIndexData(0x1328u, g_DCTTextManager.GetText(3244), static_cast<int>(W[86]));
        break;
    }
    case 0x400000u:
    {
        // 精靈裝備
        if (v5)
        {
            int val;
            val = v5[7]; if (static_cast<uint16_t>(val)) AddIndexData(0x0C5Du, val, 0xFFFFC000);
            val = v5[9]; if (static_cast<uint16_t>(val)) AddIndexData(0x0C5Eu, val, 0xFFFFC000);
            val = v5[10]; if (static_cast<uint16_t>(val)) AddIndexData(0x0C60u, val, 0xFFFFC000);
            val = v5[8]; if (static_cast<uint16_t>(val)) AddIndexData(0x0C5Fu, val, 0xFFFFC000);
        }
        break;
    }
    default:
        break;
    }

    // 等級需求（色彩判斷）
    if (B[120])
    {
        int reqLv = B[120];
        if (g_clLevelSystem.GetLevel() >= reqLv)
            AddIndexData(0x0C2Au, reqLv, 0xFFFFC000);
        else
            AddIndexData(0x0C2Au, reqLv, 0xFFFF4600);
    }
    else
    {
        AddIndexData(0x0C2Au, g_DCTTextManager.GetText(3178), 0xFFFFC000);
    }

    PrintReqClassForHuntItem();

    // 四維需求（色彩判斷）
    int needStr = UW[50];
    if (static_cast<uint16_t>(needStr))
    {
        if (g_clPlayerAbility.GetStr(reinterpret_cast<int>(v5), 0) >= needStr)
            AddIndexData(0x0C7Du, needStr, 0xFFFFC000);
        else
            AddIndexData(0x0C7Du, needStr, 0xFFFF4600);
    }
    int needDex = UW[52];
    if (static_cast<uint16_t>(needDex))
    {
        if (g_clPlayerAbility.GetDex(reinterpret_cast<int>(v5), 0) >= needDex)
            AddIndexData(0x0C7Eu, needDex, 0xFFFFC000);
        else
            AddIndexData(0x0C7Eu, needDex, 0xFFFF4600);
    }
    int needVit = UW[51];
    if (static_cast<uint16_t>(needVit))
    {
        if (g_clPlayerAbility.GetVit(reinterpret_cast<int>(v5), 0) >= needVit)
            AddIndexData(0x0C7Fu, needVit, 0xFFFFC000);
        else
            AddIndexData(0x0C7Fu, needVit, 0xFFFF4600);
    }
    int needInt = UW[53];
    if (static_cast<uint16_t>(needInt))
    {
        if (g_clPlayerAbility.GetInt(reinterpret_cast<int>(v5), 0) >= needInt)
            AddIndexData(0x0C80u, needInt, 0xFFFFC000);
        else
            AddIndexData(0x0C80u, needInt, 0xFFFF4600);
    }

    // 稀有度背景色
    if (B[218] == 1)
        m_innerBox.SetColor(0.0f, 0.078431375f, 0.49019611f, 0.58823532f);
    else if (B[218] == 2)
        m_innerBox.SetColor(0.18039216f, 0.070588239f, 0.34117648f, 0.82352948f);

    // CanEquipItemByItemKind
    if (!dword_21BA32C->CanEquipItemByItemKind(reinterpret_cast<int>(v5), UW[0]))
        m_textTitle.m_TextColor = static_cast<DWORD>(-65536);

    OutputCashShopTime(v2);
    AddDesc(UW[2], 1);
}
void CToolTip::ProcessFashion()
{
    // 對齊 00429A00（__usercall，a2@<ebp> 用於 CanEquipItemByItemKind）
    stItemKindInfo* v3 = g_clItemKindInfo.GetItemKindInfo(m_usKindID);
    if (!v3)
        return;

    const auto W = reinterpret_cast<int16_t*>(v3);
    const auto UW = reinterpret_cast<uint16_t*>(v3);
    const auto DW = reinterpret_cast<int32_t*>(v3);
    const auto B = reinterpret_cast<uint8_t*>(v3);

    m_textTitle.SetText(g_DCTTextManager.GetText(UW[1]));

    // Fashion type text (not item name)
    int fashionTypeCode = ExGetTextCodeFashionItemType(reinterpret_cast<uint32_t*>(v3)[20]);
    AddIndexData(0x0C28u, g_DCTTextManager.GetText(fashionTypeCode), 0xFFFFC000);

    // stat at offset +46 (0xC3D)
    if (W[46])
        AddIndexData(0x0C3Du, g_DCTTextManager.GetText(3244), static_cast<int>(W[46]));

    // 性別色彩判定
    uint8_t mySex = g_clSexSystem.GetSex();
    char itemSex = B[84]; // byte offset 84 = Equip.Fashion.m_byGender
    unsigned int sexColor;
    int sexTextCode;
    if (itemSex == 'F')
    {
        sexColor = (mySex != 'F') ? 0xFFFF4600u : 0xFFFFC000u;
        sexTextCode = 3181;
    }
    else if (itemSex == 'M')
    {
        sexColor = (mySex != 'M') ? 0xFFFF4600u : 0xFFFFC000u;
        sexTextCode = 3180;
    }
    else
    {
        sexColor = 0xFFFFC000u;
        sexTextCode = 3178;
    }
    AddIndexData(0x0C3Bu, g_DCTTextManager.GetText(sexTextCode), sexColor);

    OutputCashShopTime(v3);

    if (W[49])
        AddIndexData(0x0C3Eu, static_cast<int>(W[49]), 0xFFFFC000);
    if (W[50])
        AddIndexData(0x0C3Fu, static_cast<int>(W[50]), 0xFFFFC000);
    if (DW[29])
        AddIndexData(0x0C2Du, g_DCTTextManager.GetText(3244), DW[29]);
    if (DW[30])
        AddIndexData(0x0C2Fu, g_DCTTextManager.GetText(3244), DW[30]);
    if (DW[31])
        AddIndexData(0x0C32u, g_DCTTextManager.GetText(3636), SetRatePerThousand(DW[31]));
    if (DW[32])
        AddIndexData(0x0C33u, g_DCTTextManager.GetText(3636), SetRatePerThousand(DW[32]));
    if (DW[33])
        AddIndexData(0x0C5Du, DW[33], 0xFFFFC000);
    if (DW[36])
        AddIndexData(0x0C60u, DW[36], 0xFFFFC000);
    if (DW[34])
        AddIndexData(0x0C5Eu, DW[34], 0xFFFFC000);
    if (DW[35])
        AddIndexData(0x0C5Fu, DW[35], 0xFFFFC000);

    AddDesc(UW[2], 1);

    // CanEquipItemByItemKind（GT 用 a2@<ebp>，此處用精靈系統指標近似）
    uint16_t spiritKind = g_clSpiritSystem.GetSpiritKind(ExGetMyLevel());
    uint16_t* spiritInfo = reinterpret_cast<uint16_t*>(g_clSpiritSystem.GetSpiritInfo(spiritKind));
    if (!dword_21BA32C->CanEquipItemByItemKind(reinterpret_cast<int>(spiritInfo), UW[0]))
        m_textTitle.m_TextColor = static_cast<DWORD>(-65536);

    if (UW[77])
        m_textTitle.m_TextColor = static_cast<DWORD>(-1);
}
void CToolTip::ProcessSkill(uint16_t a2)
{
    // 對齊 00429C80
    uint16_t v2 = a2;
    BOOL v119;
    if (a2)
    {
        v119 = 1;
    }
    else
    {
        v2 = m_usKindID;
        v119 = (m_byUIType == 14);
    }

    stSkillKindInfo* v4 = g_clSkillKindInfo.GetSkillKindInfo(v2);
    if (!v4)
        return;

    const auto SDW = reinterpret_cast<int32_t*>(v4);
    const auto SUW = reinterpret_cast<uint16_t*>(v4);
    const auto SW = reinterpret_cast<int16_t*>(v4);
    const auto SB = reinterpret_cast<uint8_t*>(v4);

    m_textTitle.SetText(g_DCTTextManager.GetText(SUW[2]));
    g_clSkillSystem.IsAcquiredSkill(v2, 0);
    BOOL v6 = cltSkillKindInfo::IsActiveSkill(v2);

    AddIndexData(0x0C41u, g_DCTTextManager.GetText(v6 ? 3173 : 3172), 0xFFFFC000);

    char Buffer[256] = {};

    if (v6)
    {
        // Active skill
        if (SDW[6] && !SDW[35])
            AddIndexData(0x0DCAu, g_DCTTextManager.GetText(3244), SDW[6] + 100);
        if (static_cast<uint16_t>(SUW[83]))
            AddIndexData(0x0ED0u, static_cast<int>(SUW[83]), 0xFFFFC000);
        if (SW[57])
            AddIndexData(0x0C4Eu, static_cast<int>(SW[57]), 0xFFFFC000);
        if (SW[58])
            AddIndexData(0x0C4Fu, static_cast<int>(SW[58]), 0xFFFFC000);
        if (SDW[6] && SDW[35])
            AddIndexData(0x0C50u, g_DCTTextManager.GetText(3244), SDW[6]);
        if (SDW[7])
            AddIndexData(0x0C51u, g_DCTTextManager.GetText(3244), SDW[7]);
        if (SDW[31])
            AddIndexData(0x0C54u, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[31]));
        if (SDW[32])
            AddIndexData(0x0C34u, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[32]));
        if (SDW[30])
            AddIndexData(0x0C33u, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[30]));
        if (SDW[78])
            AddIndexData(0x1B24u, g_DCTTextManager.GetText(3244), SDW[78]);
        if (SDW[79])
            AddIndexData(0x1B25u, g_DCTTextManager.GetText(3244), SDW[79]);
        if (SDW[80])
            AddIndexData(0x1B26u, g_DCTTextManager.GetText(6962), 0xFFFFC000);
        if (SDW[81])
            AddIndexData(0x1B27u, g_DCTTextManager.GetText(6963), 0xFFFFC000);
        if (SDW[44])
            AddIndexData(0x0CC9u, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[44]));
        if (SDW[48])
            AddIndexData(0x0CCAu, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[48]));
        if (SDW[46])
            AddIndexData(0x0CCBu, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[46]));

        // Cooltime from v4+35
        int v121 = SDW[35];
        if (v121)
        {
            std::snprintf(Buffer, sizeof(Buffer), "%.2f%s",
                static_cast<double>(v121) * 0.001, g_DCTTextManager.GetText(3224));
            AddIndexData(0x0C46u, Buffer, 0xFFFFC000);
        }
        if (SDW[66])
            AddIndexData(0x1272u, g_DCTTextManager.GetText(3244), SDW[66]);
        if (SDW[67])
            AddIndexData(0x1273u, g_DCTTextManager.GetText(3244), SDW[67]);
        if (SDW[57])
            AddIndexData(0x1274u, SDW[57], 0xFFFFC000);
        if (SDW[58])
            AddIndexData(0x1276u, SDW[58], 0xFFFFC000);
        if (SDW[59])
            AddIndexData(0x1275u, SDW[59], 0xFFFFC000);
        if (SDW[60])
            AddIndexData(0x1277u, SDW[60], 0xFFFFC000);

        // Transform
        uint16_t transformKind = SUW[126];
        if (transformKind)
        {
            strTransformKindInfo* v31 = g_clTransformKindInfo.GetTransfromKindInfo(transformKind);
            if (v31)
            {
                const auto TDW = reinterpret_cast<int32_t*>(v31);
                if (TDW[6])
                    AddIndexData(0x1278u, TDW[6], 0xFFFFC000);
                if (TDW[7])
                    AddIndexData(0x1279u, TDW[7], 0xFFFFC000);
            }
        }

        // Skill cool time (from system)
        float v122 = static_cast<float>(
            static_cast<double>(g_clUsingSkillSystem.GetSkillCoolTimeByBaseCoolTime(0, SDW[9])) * 0.001);
        std::snprintf(Buffer, sizeof(Buffer), "%.2f%s", v122, g_DCTTextManager.GetText(3224));
        AddIndexData(0x0C47u, Buffer, 0xFFFFC000);

        // Skill type switch
        switch (SB[112])
        {
        case 1: AddIndexData(0x0E15u, g_DCTTextManager.GetText(3606), 0xFFFFC000); break;
        case 2: AddIndexData(0x0E15u, g_DCTTextManager.GetText(3607), 0xFFFFC000); break;
        case 3: case 4: AddIndexData(0x0E15u, g_DCTTextManager.GetText(3608), 0xFFFFC000); break;
        case 5: case 6: case 7: case 9: AddIndexData(0x0E15u, g_DCTTextManager.GetText(3610), 0xFFFFC000); break;
        case 8: AddIndexData(0x0E15u, g_DCTTextManager.GetText(3609), 0xFFFFC000); break;
        default: break;
        }

        // HP need
        int v36 = SUW[66];
        if (static_cast<uint16_t>(v36))
        {
            uint16_t hpIdx = SUW[68] ? 3943 : 3277;
            if (v119 || g_clPlayerAbility.GetHP() > v36)
                AddIndexData(hpIdx, v36, 0xFFFFC000);
            else
                AddIndexData(hpIdx, v36, 0xFFFF4600);
        }

        // MP need
        int v38 = g_clPlayerAbility.GetNeedManaForUsingSkill(SUW[67]);
        uint16_t mpIdx = SUW[69] ? 3944 : 3141;
        if (v119 || g_clPlayerAbility.GetMP() > v38)
            AddIndexData(mpIdx, v38, 0xFFFFC000);
        else
            AddIndexData(mpIdx, v38, 0xFFFF4600);
    }
    else
    {
        // Passive skill
        if (SW[57])
            AddIndexData(0x0C52u, g_DCTTextManager.GetText(3244), static_cast<int>(SW[57]));
        if (SW[58])
            AddIndexData(0x0C53u, g_DCTTextManager.GetText(3244), static_cast<int>(SW[58]));
        if (SDW[6])
            AddIndexData(0x0C50u, g_DCTTextManager.GetText(3244), SDW[6]);
        if (SDW[7])
            AddIndexData(0x0C51u, g_DCTTextManager.GetText(3244), SDW[7]);
        if (SDW[30])
            AddIndexData(0x0C54u, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[30]));
        if (SDW[67])
            AddIndexData(0x0C33u, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[67]));
        if (SDW[31])
            AddIndexData(0x0C34u, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[31]));
        if (SDW[32])
            AddIndexData(0x0E30u, g_DCTTextManager.GetText(3244), SDW[32]);
        if (SDW[80])
            AddIndexData(0x118Fu, SDW[80], 0xFFFFC000);
        if (SDW[81])
            AddIndexData(0x119Au, SDW[81], 0xFFFFC000);
        if (SDW[38])
            AddIndexData(0x119Bu, g_DCTTextManager.GetText(3244), SDW[38]);
        if (SDW[39])
            AddIndexData(0x119Cu, g_DCTTextManager.GetText(3244), SDW[39]);
        if (SDW[41])
            AddIndexData(0x1190u, g_DCTTextManager.GetText(3244), SDW[41]);
        if (SDW[34])
            AddIndexData(0x1191u, g_DCTTextManager.GetText(3244), SDW[34]);
        if (SDW[35])
            AddIndexData(0x1192u, g_DCTTextManager.GetText(3244), SDW[35]);
        if (SDW[36])
            AddIndexData(0x1193u, g_DCTTextManager.GetText(3244), SDW[36]);
        if (SDW[45])
            AddIndexData(0x1194u, g_DCTTextManager.GetText(3244), SDW[45]);
        if (SDW[46])
            AddIndexData(0x1195u, g_DCTTextManager.GetText(3244), SDW[46]);
        if (SDW[47])
            AddIndexData(0x1196u, g_DCTTextManager.GetText(3244), SDW[47]);
        if (SDW[48])
            AddIndexData(0x1197u, g_DCTTextManager.GetText(3244), SDW[48]);
        if (SDW[65])
            AddIndexData(0x1B28u, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[65]));
        if (SDW[66])
            AddIndexData(0x1B29u, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[66]));
        if (SDW[88])
            AddIndexData(0x1B2Au, SDW[88], 0xFFFFC000);
        if (SDW[90])
            AddIndexData(0x1B2Bu, g_DCTTextManager.GetText(3244), SDW[90]);
        if (SDW[91])
            AddIndexData(0x1B2Cu, g_DCTTextManager.GetText(3244), SDW[91]);
        if (SDW[92])
            AddIndexData(0x1B2Du, g_DCTTextManager.GetText(3244), SDW[92]);
        if (SDW[93])
            AddIndexData(0x1B2Eu, g_DCTTextManager.GetText(3244), SDW[93]);
        if (SDW[94])
            AddIndexData(0x1B2Fu, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[94]));
        if (SDW[95])
            AddIndexData(0x1B30u, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[95]));
        if (SDW[96])
            AddIndexData(0x1B31u, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[96]));
        if (SDW[97])
            AddIndexData(0x1315u, g_DCTTextManager.GetText(3244), SDW[97]);

        // Switch on v4+62
        uint16_t v71 = 0;
        switch (SDW[62])
        {
        case 1: case 2: v71 = 4504; break;
        case 3: case 4: v71 = 4505; break;
        case 5: case 6: v71 = 6964; break;
        default: break;
        }

        if (v71 && SDW[49])
            AddIndexData(v71, g_DCTTextManager.GetText(3636), SetRatePerThousand(SDW[49]));

        if (SDW[85])
            AddIndexData(0x119Eu, const_cast<char*>("%dms"), SDW[85]);
        if (SDW[83])
            AddIndexData(0x119Fu, const_cast<char*>("%dms"), SDW[83]);
    }

    PrintReqWeaponForSkill(v4, !v119);

    int v73 = SUW[34];
    if (static_cast<uint16_t>(v73))
    {
        if (g_clLevelSystem.GetLevel() >= v73)
            AddIndexData(0x0BDBu, v73, 0xFFFFC000);
        else
            AddIndexData(0x0BDBu, v73, 0xFFFF4600);
    }
    else
    {
        AddIndexData(0x0BDBu, g_DCTTextManager.GetText(3178), 0xFFFFC000);
    }

    if (g_clLevelSystem.GetLevel() >= v73)
        m_textTitle.m_TextColor = static_cast<DWORD>(-1);
    else
        m_textTitle.m_TextColor = static_cast<DWORD>(-65536);

    PrintReqClassForSkill(v4);

    if (!a2)
        AddDesc(SUW[3], 0);
}
void CToolTip::ProcessEmblem()
{
    // 對齊 0042A910：使用 raw DWORD offset 存取所有欄位
    strEmblemKindInfo* v3 = g_clEmblemKindInfo.GetEmblemKindInfo(m_usKindID);
    if (!v3)
        return;

    const auto EDW = reinterpret_cast<int32_t*>(v3);
    const auto EUW = reinterpret_cast<uint16_t*>(v3);

    char Buffer[256] = {};

    m_textTitle.SetText(g_DCTTextManager.GetText(EUW[14]));
    AddIndexData(0x0C28u, g_DCTTextManager.GetText(4246), 0xFFFFC000);

    // Level from (word[2]) with color check
    int v6 = EUW[2];
    if (static_cast<uint16_t>(v6))
    {
        std::snprintf(Buffer, sizeof(Buffer), g_DCTTextManager.GetParsedText(4305, 0, 0), v6);
        if (ExGetMyLevel() <= v6)
            AddIndexData(0x0C2Au, Buffer, v6);
        else
            AddIndexData(0x0C2Au, Buffer, 0xFFFF0000);
    }
    // Level to (word[1]) with color check
    int v8 = EUW[1];
    if (static_cast<uint16_t>(v8))
    {
        std::snprintf(Buffer, sizeof(Buffer), g_DCTTextManager.GetParsedText(4306, 0, 0), v8);
        if (ExGetMyLevel() >= v8)
            AddIndexData(0x0C2Au, Buffer, v8);
        else
            AddIndexData(0x0C2Au, Buffer, 0xFFFF0000);
    }

    if (EDW[65]) AddIndexData(0x0F0Cu, g_DCTTextManager.GetText(3244), EDW[65]);
    if (EDW[78]) AddIndexData(0x0C2Du, g_DCTTextManager.GetText(3244), EDW[78]);
    if (EDW[79]) AddIndexData(0x0C2Fu, g_DCTTextManager.GetText(3244), EDW[79]);
    if (EDW[80]) AddIndexData(0x10A4u, g_DCTTextManager.GetText(3244), EDW[80]);
    if (EDW[81]) AddIndexData(0x10A5u, g_DCTTextManager.GetText(3244), EDW[81]);
    if (EDW[67]) AddIndexData(0x0C32u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[67]));
    if (EDW[66]) AddIndexData(0x0C33u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[66]));
    if (EDW[68]) AddIndexData(0x0C34u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[68]));
    if (EDW[84]) AddIndexData(0x0CCBu, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[84]));
    if (EDW[85]) AddIndexData(0x10A6u, EDW[85], 0xFFFFC000);
    if (EDW[82]) AddIndexData(0x10A7u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[82]));
    if (EDW[83]) AddIndexData(0x10A8u, EDW[83], 0xFFFFC000);
    if (EDW[86]) AddIndexData(0x10A9u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[86]));
    if (EDW[87]) AddIndexData(0x10AAu, EDW[87], 0xFFFFC000);
    // Missing 22 stats from GT:
    if (EDW[69]) AddIndexData(0x10ABu, g_DCTTextManager.GetText(3244), EDW[69]);
    if (EDW[70]) AddIndexData(0x10ACu, g_DCTTextManager.GetText(3244), EDW[70]);
    if (EDW[71]) AddIndexData(0x10ADu, g_DCTTextManager.GetText(3244), EDW[71]);
    if (EDW[72]) AddIndexData(0x10AEu, g_DCTTextManager.GetText(3244), EDW[72]);
    if (EDW[73]) AddIndexData(0x10AFu, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[73]));
    if (EDW[74]) AddIndexData(0x10B0u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[74]));
    if (EDW[75]) AddIndexData(0x10B1u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[75]));
    if (EDW[76]) AddIndexData(0x10B2u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[76]));
    if (EDW[77]) AddIndexData(0x10B3u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[77]));
    if (EDW[88]) AddIndexData(0x10B4u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[88]));
    if (EDW[89]) AddIndexData(0x10B5u, g_DCTTextManager.GetText(3244), EDW[89]);
    if (EDW[90]) AddIndexData(0x1FD9u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[90]));
    if (EDW[91]) AddIndexData(0x1FDAu, g_DCTTextManager.GetText(3244), EDW[91]);
    if (EDW[92]) AddIndexData(0x1FDBu, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[92]));
    if (EDW[93]) AddIndexData(0x1FDCu, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[93]));
    if (EDW[94]) AddIndexData(0x1FDDu, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[94]));
    if (EDW[95]) AddIndexData(0x1FDEu, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[95]));
    if (EDW[96]) AddIndexData(0x1FDFu, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[96]));
    if (EDW[99]) AddIndexData(0x1FE1u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[99]));
    if (EDW[100]) AddIndexData(0x1FE2u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[100]));
    if (EDW[97]) AddIndexData(0x1FE3u, g_DCTTextManager.GetText(3636), SetRatePerThousand(EDW[97]));
    if (EDW[101]) AddIndexData(0x1FE4u, g_DCTTextManager.GetText(3244), EDW[101]);

    // Training card
    uint16_t trainCard = EUW[196];
    if (trainCard)
    {
        stItemKindInfo* tr = g_clItemKindInfo.GetItemKindInfo(trainCard);
        if (tr)
            AddIndexData(0x1FE0u, g_DCTTextManager.GetText(reinterpret_cast<uint16_t*>(tr)[1]), 0xFFFFC000);
    }

    AddDesc(EUW[15], 0);
}
void CToolTip::OutputCashShopTime(stItemKindInfo* a2)
{
    // 對齊 0042B0A0
    if (!a2)
        return;

    const auto AB = reinterpret_cast<uint8_t*>(a2);
    const auto AW = reinterpret_cast<uint16_t*>(a2);

    if (!g_clItemKindInfo.IsCashItem(AB[34]) || !g_clItemKindInfo.IsTimerItem(AB[34]))
        return;

    uint16_t useTerm = AW[30]; // word offset 30 = byte 60
    if (!useTerm)
    {
        AddIndexData(0x0C3Cu, g_DCTTextManager.GetText(4024), 0xFFFFC000);
        return;
    }

    unsigned int v8 = 0;
    switch (m_byUIType)
    {
    case 5: // equipment
    {
        unsigned int slot;
        if (m_nSubType == 4)
            slot = 1;
        else if (m_nSubType == 5)
            slot = 0;
        else
            break;
        if (m_usSlotIndex != 0xFFFF)
        {
            v8 = dword_21BA32C->GetEquipItemTime(slot, m_usSlotIndex);
            goto HAVE_TIME;
        }
        break;
    }
    case 4: // inventory
        if (m_usSlotIndex != 0xFFFF)
        {
            strInventoryItem* pItem = g_clMyInventory.GetInventoryItem(m_usSlotIndex);
            v8 = pItem->value0; // time value (DWORD offset +1)
            goto HAVE_TIME;
        }
        break;
    case 16: // trade
    {
        unsigned int targetAccount = *reinterpret_cast<uint32_t*>(dword_21C9C54);
        if ((m_usSlotIndex & 0xFFF8) != 0)
            targetAccount = ExGetMyAccount();
        cltTradeBasket* pBasket = g_clTradeSystem.GetTradeBasket(targetAccount);
        if (pBasket)
        {
            v8 = pBasket->GetItemTime(m_usSlotIndex & 7);
            goto HAVE_TIME;
        }
        break;
    }
    case 43:
    case 45:
    case 47:
    case 12:
        AddIndexData(0x0C3Cu, g_DCTTextManager.GetText(4011), static_cast<int>(useTerm));
        return;
    case 48: // pet inventory / special UI
        if (m_usSlotIndex != 0xFFFF)
        {
            ClientCharacter* pMyChar = g_ClientCharMgr.GetMyCharacterPtr();
            if (pMyChar)
            {
                if (*(reinterpret_cast<uint8_t*>(pMyChar) + 11524) == 3)
                {
                    int uiWindow = reinterpret_cast<int>(g_UIMgr->GetUIWindow(48));
                    v8 = *reinterpret_cast<uint32_t*>(uiWindow + 1956 * static_cast<uint16_t>(m_usSlotIndex) + 7924);
                }
                else
                {
                    v8 = dword_B3D72C[4 * static_cast<uint16_t>(m_usSlotIndex)];
                }
                goto HAVE_TIME;
            }
        }
        break;
    default:
        if (m_byUIType == 29 && m_usSlotIndex != 0xFFFF && g_UIMgr->IsOpenUserInterface(29))
        {
            unsigned int v20 = 0;
            uint16_t outKind = 0;
            uint16_t outQty = 0;
            int uiWindow = reinterpret_cast<int>(g_UIMgr->GetUIWindow(29));
            if (*reinterpret_cast<int*>(uiWindow + 3300))
            {
                g_clExStorageSystem.GetStorageItem(
                    static_cast<uint8_t>(m_usSlotIndex),
                    &outKind, &outQty, &v20);
            }
            else
            {
                dword_21BB2AC->GetStorageItem(
                    static_cast<uint8_t>(m_usSlotIndex),
                    &outKind, &outQty, &v20);
            }
            v8 = v20;
            if (v20)
                goto HAVE_TIME;
        }
        break;
    }

    // 最終 fallback
    if (!a2 || !AW[77])
    {
        AddIndexData(0x0C3Cu, g_DCTTextManager.GetText(4480), 0xFFFFC000);
        return;
    }
    return;

HAVE_TIME:
    if (!a2)
    {
        AddIndexData(0x0C3Cu, g_DCTTextManager.GetText(4480), 0xFFFFC000);
        return;
    }
    {
        unsigned int remain = ExGetTimeOutItemRemindTime(v8, *reinterpret_cast<uint16_t*>(a2));
        if (remain <= 0x16D)
        {
            AddIndexData(0x0C3Cu, g_DCTTextManager.GetText(4011), static_cast<int>(remain));
            return;
        }
    }
    // fall through: check word[77]
    if (!a2 || !AW[77])
        AddIndexData(0x0C3Cu, g_DCTTextManager.GetText(4480), 0xFFFFC000);
}
void CToolTip::ProcessDesc()
{
    stItemKindInfo* item = g_clItemKindInfo.GetItemKindInfo(m_usKindID);
    if (!item)
        return;

    m_textTitle.SetText(g_DCTTextManager.GetText(item->m_wTextCode));
    AddDesc(item->m_wDescCode, 1);

    // 對齊 0042B370：若是製作武器(ITEM_TYPE_MAKING_WEAPON=27)，追加製作資訊
    if (item->m_wItemType == ITEM_TYPE_MAKING_WEAPON)
    {
        strMakingItemKindInfo* mk = g_clMakingItemKindInfo.GetMakingItemKindInfoByItemID(item->m_wKind);
        if (!mk)
            return;

        stItemKindInfo* resultItem = g_clItemKindInfo.GetItemKindInfo(mk->ResultItemID);
        AddIndexData(0x0C28u, g_DCTTextManager.GetText(8171), 0xFFFFC000);

        if (resultItem && resultItem->m_wTextCode)
            AddIndexData(0x1FECu, g_DCTTextManager.GetText(resultItem->m_wTextCode), 0xFFFFC000);

        if (mk->SuccessPermyriad)
        {
            const float rate = mk->SuccessPermyriad * 0.0001f;
            AddIndexData(0x0F0Bu, g_DCTTextManager.GetText(3636), rate);
        }
        if (mk->ExpBonus)
            AddIndexData(0x0F0Cu, mk->ExpBonus, 0xFFFFC000);
        if (resultItem && resultItem->Equip.Hunt.m_byLevel)
            AddIndexData(0x1FEDu, resultItem->Equip.Hunt.m_byLevel, 0xFFFFC000);
        if (resultItem && resultItem->Equip.Hunt.m_wWeaponType)
            AddIndexData(0x1FEEu, GetWeaponTypeText(resultItem->Equip.Hunt.m_wWeaponType), 0xFFFFC000);
    }
}
void CToolTip::ProcessClimate()
{
    // 對齊 0042B3D0：從當前地圖取氣候資訊
    strClimateInfo* v2 = g_Map.GetClimateKindByMapId(dword_21B8DF4);
    if (!v2)
        return;

    const auto CW = reinterpret_cast<uint16_t*>(v2);
    const auto CDW = reinterpret_cast<int32_t*>(v2);

    m_textTitle.SetText(g_DCTTextManager.GetText(CW[1]));
    AddIndexData(0x0C28u, g_DCTTextManager.GetText(8165), 0xFFFFC000);
    AddDesc(0, 0);

    uint16_t moveSpeed = CW[2];
    if (moveSpeed != 1000)
        AddIndexData(0x1FE6u, g_DCTTextManager.GetText(3636), SetRatePerThousand(1000 - static_cast<unsigned int>(moveSpeed)));
    if (CDW[2])
        AddIndexData(0x1FE7u, g_DCTTextManager.GetText(3636), SetRatePerThousand(CDW[2]));
    if (CDW[3])
        AddIndexData(0x1FE8u, g_DCTTextManager.GetText(3636), SetRatePerThousand(CDW[3]));
    if (CW[8])
        AddIndexData(0x1FE9u, g_DCTTextManager.GetText(3636), SetRatePerThousand(static_cast<uint16_t>(CW[8]) % 1000));
    uint16_t v10 = CW[14];
    if (v10)
        AddIndexData(0x1FEAu, g_DCTTextManager.GetText(8175), static_cast<float>(static_cast<double>(v10) * 0.001 + 1.0));
}
void CToolTip::ProcessEmoticon()
{
    // 對齊 0042B650：僅 UI type=52 且 subtype=1 時處理
    if (m_byUIType != 52 || m_usKindID != 1)
        return;

    stEmoticonWordInfo* info = g_clEmoticonKindInfo.GetEmoticonWordInfoByKind(m_usSlotIndex);
    if (!info)
        return;

    m_textTitle.SetText(g_DCTTextManager.GetText(info->nameId));
    AddDesc(info->tooltipId, 0);
}
void CToolTip::ProcessPetSkill(uint16_t a2)
{
    // 對齊 0042B6B0
    uint16_t v2 = a2 ? a2 : m_usKindID;
    strPetSkillKindInfo* v4 = g_clPetSkillKindInfo.GetPetSkillKindInfo(v2);
    if (!v4)
        return;

    const auto PDW = reinterpret_cast<int32_t*>(v4);
    const auto PUW = reinterpret_cast<uint16_t*>(v4);

    m_textTitle.SetText(g_DCTTextManager.GetText(PUW[1]));
    AddIndexData(0x0C41u, g_DCTTextManager.GetText(7216), 0xFFFFC000);

    if (PDW[5])
        AddIndexData(0x0DCAu, g_DCTTextManager.GetText(3636), SetRatePerThousand(PDW[5]));
    if (PDW[6])
        AddIndexData(0x0DCBu, g_DCTTextManager.GetText(3636), SetRatePerThousand(PDW[6]));
    if (PDW[7])
        AddIndexData(0x0DCCu, g_DCTTextManager.GetText(3636), SetRatePerThousand(PDW[7]));
    if (PDW[4])
        AddIndexData(0x0C2Eu, g_DCTTextManager.GetText(3244), PDW[4]);
    if (PDW[10])
        AddIndexData(0x0ECFu, g_DCTTextManager.GetText(3636), PDW[10]);
    if (PDW[8])
        AddIndexData(0x10B1u, g_DCTTextManager.GetText(3636), SetRatePerThousand(PDW[8]));
    if (PDW[9])
        AddIndexData(0x10B3u, g_DCTTextManager.GetText(3636), SetRatePerThousand(PDW[9]));
    if (PUW[22])
        AddIndexData(0x1C31u, g_DCTTextManager.GetText(7219), 0xFFFFC000);

    int v14 = PUW[7];
    if (static_cast<uint16_t>(v14))
    {
        if (g_clPetSystem.GetPetLevel() < v14)
            AddIndexData(0x1C32u, v14, 0xFFFF4600);
        else
            AddIndexData(0x1C32u, v14, 0xFFFFC000);
    }

    AddDesc(PUW[2], 0);
}
void CToolTip::ProcessCoupleRing()
{
    // 對齊 0042B8A0：僅 UI type=5 時處理
    if (m_byUIType != 5)
        return;

    strCoupleRingKindInfo* info = g_clCoupleRingKindInfo.GetCoupleRingKindInfo(m_usKindID);
    if (!info)
        return;

    m_textTitle.SetText(g_DCTTextManager.GetText(info->textId));
    AddIndexData(0x0C28u, g_DCTTextManager.GetText(8214), 0xFFFFC000);

    if (info->blockId)
        AddIndexData(0x0ECCu, g_DCTTextManager.GetText(8215), 0xFFFFC000);

    // 對齊 0042B8A0：marriage system recall quantity
    const auto RDW = reinterpret_cast<int32_t*>(info);
    char* recallText;
    if (RDW[2]) // canSummonSpouse
    {
        char myItemBuf[128] = {};
        char totalBuf[128] = {};
        if (g_clMarriageSystem->GetRemainedRecallQty_MyItem())
        {
            int myItemQty = g_clMarriageSystem->GetRemainedRecallQty_MyItem();
            int remainQty = g_clMarriageSystem->GetRemainedRecallQty();
            std::snprintf(myItemBuf, sizeof(myItemBuf), g_DCTTextManager.GetText(8375), remainQty, myItemQty);
        }
        int totalQty = g_clMarriageSystem->GetRemainedRecallQty_Total();
        std::snprintf(totalBuf, sizeof(totalBuf), g_DCTTextManager.GetText(8216), totalQty, myItemBuf);
        recallText = totalBuf;
        AddIndexData(0x1FCAu, recallText, 0xFFFFC000);
    }
    else
    {
        AddIndexData(0x1FCAu, g_DCTTextManager.GetText(8217), 0xFFFFC000);
    }

    int v8 = RDW[1]; // expRatePercent
    if (v8)
        AddIndexData(0x0C3Du, g_DCTTextManager.GetText(3244), v8);

    AddIndexData(0x2015u, g_DCTTextManager.GetText(8218), 0xFFFFC000);

    char desc[256] = {};
    std::snprintf(desc, sizeof(desc), g_DCTTextManager.GetText(8211), m_nExtraData);
    AddDesc(desc);
}

void CToolTip::PrepareDrawing()
{
    if (IsShow())
        m_innerBox.PrepareDrawing();
}
void CToolTip::Draw()
{
    if (IsShow())
        m_innerBox.Draw();
}

void CToolTip::Show(int x, int y, const stToolTipData* pData, int compareFlag)
{
    m_nMouseX = x;
    m_nMouseY = y;

    // ground truth: 逐欄位從 stToolTipData 複製到 CToolTip 內部狀態
    m_nType = pData->m_type;              // *((_DWORD *)this + 547) = *(_DWORD *)a4
    m_nSubType = pData->m_color;          // *((_DWORD *)this + 548) = *((_DWORD *)a4 + 1)
    m_usKindID = pData->m_itemId;         // *((_WORD *)this + 1098) = *((_WORD *)a4 + 4)
    m_nCompareFlag = pData->m_count;      // *((_DWORD *)this + 550) = *((_DWORD *)a4 + 3)
    m_strData.assign(pData->m_text);      // std::string::assign((char *)this + 2204, (char *)a4 + 16, ...)
    m_byUIType = pData->m_grade;          // *((_BYTE *)this + 2220) = *((_BYTE *)a4 + 32)
    m_usSlotIndex = pData->m_durability;  // *((_WORD *)this + 1111) = *((_WORD *)a4 + 17)
    m_nExtraData = pData->m_extra;        // *((_DWORD *)this + 556) = *((_DWORD *)a4 + 9)

    m_innerBox.Show();

    if (m_nType == 0 || m_nType == 1 || m_nType == 8)
    {
        m_textTitle.Hide();
        m_textDesc.Hide();
        for (int i = 0; i < 20; ++i)
        {
            m_labelText[i].Hide();
            m_valueText[i].Hide();
        }
    }
    else
    {
        m_textMain.Hide();
    }

    // ground truth: 色彩由 m_nCompareFlag（data.count）決定
    // compare mode: loc_5D1F38 → 0x5D1F39 → R=93(0x5D), G=31(0x1F), B=57(0x39)
    // normal mode:  loc_4A7119 → 0x4A711B → R=74(0x4A), G=113(0x71), B=27(0x1B)
    if (m_nCompareFlag)
    {
        // compare mode: 0x5D1F39 | (alpha << 24)
        m_innerBox.SetColor(93 / 255.0f, 31 / 255.0f, 57 / 255.0f, m_nAlpha / 255.0f);
        for (int i = 0; i < 4; ++i)
            m_borderBox[i].SetColor(93 / 255.0f, 31 / 255.0f, 57 / 255.0f, (m_nAlpha + 60) / 255.0f);
    }
    else
    {
        // normal mode: 0x4A711B | (alpha << 24)
        m_innerBox.SetColor(74 / 255.0f, 113 / 255.0f, 27 / 255.0f, m_nAlpha / 255.0f);
        for (int i = 0; i < 4; ++i)
            m_borderBox[i].SetColor(0.0f, 50 / 255.0f, 0.0f, (m_nAlpha + 60) / 255.0f);
    }
}
void CToolTip::Hide()
{
    m_innerBox.Hide();
    m_nAlpha = 190;
}
// ground truth: return *((_DWORD *)this + 13)
// this+13 DWORD = m_innerBox 的 m_bIsVisible 欄位
int  CToolTip::IsShow()
{
    return m_innerBox.IsVisible();
}

char* CToolTip::GetWeaponTypeText(int weaponType)
{
    switch (weaponType)
    {
    case 1:  return g_DCTTextManager.GetText(56601);
    case 2:  return g_DCTTextManager.GetText(56602);
    case 3:  return g_DCTTextManager.GetText(56603);
    case 4:  return g_DCTTextManager.GetText(56604);
    case 5:  return g_DCTTextManager.GetText(56605);
    case 6:  return g_DCTTextManager.GetText(56606);
    case 7:  return g_DCTTextManager.GetText(56607);
    case 8:  return g_DCTTextManager.GetText(56608);
    case 9:  return g_DCTTextManager.GetText(56609);
    case 10: return g_DCTTextManager.GetText(3315);
    case 11: return g_DCTTextManager.GetText(4404);
    case 12: return g_DCTTextManager.GetText(4405);
    case 13: return g_DCTTextManager.GetText(4406);
    case 14: return g_DCTTextManager.GetText(3327);
    default: return g_DCTTextManager.GetText(0);
    }
}
void CToolTip::SetVoidIndex(int from, int to)
{
    for (int i = from; i < to; ++m_nVoidCount)
        m_voidIndices[m_nVoidCount] = i++;
}

void CToolTip::PrintReqWeaponForSkill(stSkillKindInfo* pSkill, int checkEquipped)
{
    uint16_t equipSubType = 0;
    const int reqWeaponCount = *reinterpret_cast<uint32_t*>(pSkill->raw + 88);
    if (reqWeaponCount)
    {
        SetVoidIndex(m_nIndexCount + 1, m_nIndexCount + reqWeaponCount);
        char satisfied = 0;

        if (checkEquipped)
        {
            uint16_t equipMain = dword_21BA32C->GetEquipItem(1u, 4u);
            uint16_t equipSub = dword_21BA32C->GetEquipItem(1u, 5u);
            uint16_t equipMainType = 0;
            if (equipMain)
                equipMainType = *reinterpret_cast<uint16_t*>(
                    reinterpret_cast<unsigned char*>(g_clItemKindInfo.GetItemKindInfo(equipMain)) + 84);
            if (equipSub)
                equipSubType = *reinterpret_cast<uint16_t*>(
                    reinterpret_cast<unsigned char*>(g_clItemKindInfo.GetItemKindInfo(equipSub)) + 84);

            // ground truth: 先掃描所有 15 種武器，判斷是否有任何一種匹配裝備
            for (int i = 0; i < 15; ++i)
            {
                if (pSkill->raw[72 + i] && (i == equipMainType || i == equipSubType))
                {
                    satisfied = 1;
                    break;
                }
            }
        }

        // ground truth: 再用 satisfied flag 決定所有武器項目的統一顏色
        for (int i = 0; i < 15; ++i)
        {
            if (pSkill->raw[72 + i])
            {
                unsigned int color;
                if (!checkEquipped || satisfied)
                    color = 0xFFFFC000;
                else
                    color = 0xFFFF4600;
                AddIndexData(0x0C79u, GetWeaponTypeText(i), color);
            }
        }
    }
    else
    {
        AddIndexData(0x0C79u, g_DCTTextManager.GetText(3178), 0xFFFFC000);
    }
}
void CToolTip::PrintReqClassForSkill(stSkillKindInfo* a2)
{
    // 對齊 0042C160：class hierarchy walk
    uint16_t v3 = g_clClassSystem.GetClass();
    if (!v3)
        return;

    uint16_t v15[64] = {};
    int v4 = g_clClassKindInfo.GetClassKindsByAtb(*reinterpret_cast<uint64_t*>(reinterpret_cast<char*>(a2) + 40), v15);
    if (v4 <= 0)
    {
        AddIndexData(0x0C7Au, g_DCTTextManager.GetText(3178), 0xFFFFC000);
        return;
    }

    SetVoidIndex(m_nIndexCount + 1, m_nIndexCount + v4);

    // 第一遍：走 class hierarchy 判斷是否可用
    int v6 = 0; // canUse flag
    for (int i = 0; i < v4; ++i)
    {
        if (!v15[i])
            continue;

        // 取玩家當前職業資訊，向上走 parent chain
        strClassKindInfo* v8 = g_clClassKindInfo.GetClassKindInfo(v3);
        v6 = 0;
        while (v8)
        {
            strClassKindInfo* target = g_clClassKindInfo.GetClassKindInfo(v15[i]);
            if (target)
            {
                // 比較 attribute mask 交集 (QWORD offset 1 = byte offset 8)
                uint64_t myAtb = *reinterpret_cast<uint64_t*>(reinterpret_cast<char*>(v8) + 8);
                uint64_t reqAtb = *reinterpret_cast<uint64_t*>(reinterpret_cast<char*>(target) + 8);
                if ((myAtb & reqAtb) != 0)
                {
                    v6 = 1;
                    i = v4; // break outer loop too
                    break;
                }
            }
            // Walk up to parent class: word offset 10 = byte offset 20
            uint16_t parentClass = reinterpret_cast<uint16_t*>(v8)[10];
            v8 = g_clClassKindInfo.GetClassKindInfo(parentClass);
        }
    }

    // 第二遍：輸出所有職業（色彩依 canUse）
    for (int i = 0; i < v4; ++i)
    {
        strClassKindInfo* v10 = g_clClassKindInfo.GetClassKindInfo(v15[i]);
        if (v10)
        {
            unsigned int color = v6 ? 0xFFFFC000u : 0xFFFF4600u;
            AddIndexData(0x0C7Au, g_DCTTextManager.GetText(reinterpret_cast<uint16_t*>(v10)[8]), color);
        }
    }
}
void CToolTip::PrintReqClassForHuntItem()
{
    // 對齊 0042C2B0：class hierarchy walk
    uint16_t v19[32] = {};
    int v17 = 0;
    if (!g_clItemKindInfo.GetReqClassKindsForEquip(m_usKindID, &v17, v19))
        return;

    if (v17 <= 0)
    {
        AddIndexData(0x0C2Bu, g_DCTTextManager.GetText(3178), 0xFFFFC000);
        return;
    }

    SetVoidIndex(m_nIndexCount + 1, m_nIndexCount + v17);

    // 第一遍：走 class hierarchy 判斷是否可用
    int v18 = 0; // canUse flag
    for (int i = 0; i < v17; ++i)
    {
        if (!v19[i])
            continue;

        strClassKindInfo* v7 = g_clClassKindInfo.GetClassKindInfo(v19[i]);
        if (!v7)
            continue;

        uint16_t myClassCode = g_clClassSystem.GetClass();
        strClassKindInfo* v9 = g_clClassKindInfo.GetClassKindInfo(myClassCode);

        while (v9)
        {
            uint64_t reqAtb = *reinterpret_cast<uint64_t*>(reinterpret_cast<char*>(v7) + 8);
            uint64_t myAtb = *reinterpret_cast<uint64_t*>(reinterpret_cast<char*>(v9) + 8);
            if ((reqAtb & myAtb) != 0)
            {
                v18 = 1;
                break;
            }
            uint16_t parentClass = reinterpret_cast<uint16_t*>(v9)[10];
            v9 = g_clClassKindInfo.GetClassKindInfo(parentClass);
        }
    }

    // 第二遍：輸出所有職業
    for (int i = 0; i < v17; ++i)
    {
        strClassKindInfo* v12 = g_clClassKindInfo.GetClassKindInfo(v19[i]);
        if (v12)
        {
            unsigned int color = v18 ? 0xFFFFC000u : 0xFFFF4600u;
            AddIndexData(0x0C2Bu, g_DCTTextManager.GetText(reinterpret_cast<uint16_t*>(v12)[8]), color);
        }
    }
}

void CToolTip::ProcessCharInfo(char* charName)
{
    ClientCharacter* chr = g_ClientCharMgr.GetCharByName(charName);
    if (!chr || !charName || !std::strlen(charName))
    {
        Hide();
        return;
    }

    m_textDesc.Show();
    strClassKindInfo* cls = g_clClassKindInfo.GetClassKindInfo(*reinterpret_cast<uint16_t*>(reinterpret_cast<unsigned char*>(chr) + 9712));
    if (!cls)
        return;

    m_textTitle.SetText(charName);
    AddIndexData(0x1B55u, g_DCTTextManager.GetText(cls->name_code), 0xFFFFC000);
    AddIndexData(0x1B56u, static_cast<int>(*(reinterpret_cast<unsigned char*>(chr) + 536)), 0xFFFFC000);

    strPKRankKindInfo* pk = g_clPKRankKindInfo.GetPKRankKindInfo(*reinterpret_cast<uint16_t*>(reinterpret_cast<unsigned char*>(chr) + 14680));
    if (pk)
        AddIndexData(0x1B57u, g_DCTTextManager.GetText(pk->textId), 0xFFFFC000);
}
void CToolTip::SetTextMainTitle(stMapInfo* pMapInfo)
{
    // 對齊 0042C3D0：GT 使用 <= 50（允許 count == 50），即 > 50 才 return
    if (m_nWorldMapTextCount > 50 || !pMapInfo)
        return;
    const auto MW = reinterpret_cast<uint16_t*>(pMapInfo);
    const int textCode = MW[186] ? MW[186] : MW[2];
    m_worldMapText[m_nWorldMapTextCount].SetText(g_DCTTextManager.GetText(textCode));
    m_worldMapText[m_nWorldMapTextCount++].m_TextColor = 0xFF00FF00; // -16711936
}
void CToolTip::SetTextDungeonBasic(stMapInfo* pMapInfo)
{
    // 對齊 0042C5B0：GT 使用 <= 50（允許 count == 50），即 > 50 才 return
    if (m_nWorldMapTextCount > 50 || !pMapInfo)
        return;

    const auto MW = reinterpret_cast<uint16_t*>(pMapInfo);

    uint16_t dungeonNameCode = MW[186];

    if (dungeonNameCode)
    {
        // 副本路線：遍歷所有屬於同副本的地圖
        int mapCount = g_pcltMapInfo->GetMapKindCountByDungeonNameCode(dungeonNameCode);
        if (mapCount <= 0)
            return;

        // 取得地圖列表起始位址
        uint16_t* mapList = reinterpret_cast<uint16_t*>(reinterpret_cast<char*>(g_pcltMapInfo) + 20);
        if (!mapList)
            return;

        for (int m = 0; m < mapCount; ++m)
        {
            uint16_t* regenMonsters = g_cltRegenMonsterKindInfo.GetRegenMonsterKindByMapID(mapList[m]);
            if (!regenMonsters)
                continue;

            for (int i = 0; i < 5; ++i)
            {
                stCharKindInfo* charInfo = static_cast<stCharKindInfo*>(g_pcltCharKindInfo->GetCharKindInfo(regenMonsters[i]));
                if (!charInfo)
                    continue;
                const auto CDW = reinterpret_cast<int32_t*>(charInfo);
                const auto CUW = reinterpret_cast<uint16_t*>(charInfo);
                const auto CB = reinterpret_cast<uint8_t*>(charInfo);

                if (CDW[53] < 1)
                    continue;

                char Buffer[256] = {};
                std::snprintf(Buffer, sizeof(Buffer), "%s(Lv%d)",
                    g_DCTTextManager.GetText(CUW[1]), static_cast<int>(CB[146]));

                // 重複檢查 + 寫入
                bool found = false;
                for (int j = 1; j < 50; ++j)
                {
                    CControlText* line = reinterpret_cast<CControlText*>(
                        reinterpret_cast<char*>(this) + 21436 + 432 * (j - 1));

                    if (std::strcmp(line->GetText(), Buffer) == 0)
                    {
                        found = true;
                        break;
                    }
                    if (std::strcmp(line->GetText(), Buffer) == 0 || !line->IsStringData())
                    {
                        if (std::strcmp(line->GetText(), Buffer) != 0 && !line->IsStringData())
                        {
                            // 寫入新行
                            m_worldMapText[m_nWorldMapTextCount].SetText(Buffer);
                            m_worldMapText[m_nWorldMapTextCount].m_TextColor = -65536; // 紅色
                            ++m_nWorldMapTextCount;
                            found = true;
                            break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        // 非副本路線：單一地圖
        uint16_t* regenMonsters = g_cltRegenMonsterKindInfo.GetRegenMonsterKindByMapID(MW[0]);
        if (!regenMonsters)
            return;

        for (int i = 0; i < 5; ++i)
        {
            stCharKindInfo* charInfo = static_cast<stCharKindInfo*>(g_pcltCharKindInfo->GetCharKindInfo(regenMonsters[i]));
            if (!charInfo)
                continue;
            const auto CDW = reinterpret_cast<int32_t*>(charInfo);
            const auto CUW = reinterpret_cast<uint16_t*>(charInfo);
            const auto CB = reinterpret_cast<uint8_t*>(charInfo);

            if (CDW[53] < 1)
                continue;

            char Buffer[256] = {};
            std::snprintf(Buffer, sizeof(Buffer), "%s(Lv%d)",
                g_DCTTextManager.GetText(CUW[1]), static_cast<int>(CB[146]));

            // 重複檢查 + 寫入
            for (int j = 1; j < 50; ++j)
            {
                CControlText* line = &m_worldMapText[j];

                if (std::strcmp(line->GetText(), Buffer) == 0)
                    break;
                if (std::strcmp(line->GetText(), Buffer) == 0 || !line->IsStringData())
                {
                    if (std::strcmp(line->GetText(), Buffer) != 0 && !line->IsStringData())
                    {
                        m_worldMapText[m_nWorldMapTextCount].SetText(Buffer);
                        m_worldMapText[m_nWorldMapTextCount].m_TextColor = -65536;
                        ++m_nWorldMapTextCount;
                        break;
                    }
                }
            }
        }
    }
}
