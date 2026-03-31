#include "UI/CToolTip.h"
#include "global.h"
#include "Info/cltItemKindInfo.h"
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
#include <cstdio>
#include <cstring>

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
    m_nTitleColor = -1;
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
    m_nTitleColor = -1;
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
        ProcessCharInfo(reinterpret_cast<char*>(m_nExtraData));
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

// ========================================
// Stub implementations（尚未實作的函數）
// ========================================

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
    const uint16_t height = static_cast<uint16_t>(HIWORD(textSize[0]) + 20);
    m_innerBox.SetWidth(width);
    m_innerBox.SetHeight(height);
    CalcPos();
}
void CToolTip::ProcessWorldMap(int mode)
{
    // Ground-truth 對齊重點：
    // 1) mode==0 時由 WorldMap UI 依滑鼠位置回推 map kind
    // 2) map kind == 0 則直接隱藏 tooltip
    // 3) 先輸出主標題/基礎資訊，再輸出 world-map 專用多行內容
    // 4) 最後掃描 50 行 m_worldMapText 計算寬高
    //
    // 目前專案尚未完整還原 CUIWorldMap 介面，因此 mode==0 取值路徑採保留：
    // 若無法由 UI 回推 map kind，沿用 m_usKindID（Show() 前已寫入）。

    if (mode == 0 && m_usKindID == 0)
    {
        // 對齊 ground truth：找不到有效 map kind 時隱藏
        m_innerBox.Hide();
        return;
    }

    if (m_usKindID == 0)
    {
        m_innerBox.Hide();
        return;
    }

    stMapInfo* mapInfo = g_Map.GetMapInfoByID(m_usKindID);
    if (!mapInfo)
        return;

    SetTextMainTitle(mapInfo);
    SetTextDungeonBasic(mapInfo);

    // ===== 對齊 00427EA0 的 world-map 行輸出區 =====
    // 在原始客戶端，m_worldMapText 共 50 行，並搭配不同來源設定顏色：
    // - 怪物：依善惡值紅/黃/白
    // - NPC / 目的地：偏黃
    // 這裡保留同樣資料流與顏色規則（資料來源不足時盡量降級）。
    m_nWorldMapTextCount = 0;

    auto pushWorldLine = [this](const char* txt, DWORD color)
    {
        if (!txt || !txt[0] || m_nWorldMapTextCount >= 50)
            return;
        m_worldMapText[m_nWorldMapTextCount].SetText(txt);
        m_worldMapText[m_nWorldMapTextCount].m_TextColor = color;
        ++m_nWorldMapTextCount;
    };

    // 1) map 對應重生怪物（對齊 decompile 的前半段）
    // 原碼每圖取最多 5 隻怪物 kind。
    uint16_t* regenMonKinds = g_cltRegenMonsterKindInfo.GetRegenMonsterKindByMapID(m_usKindID);
    if (regenMonKinds)
    {
        for (int i = 0; i < 5 && m_nWorldMapTextCount < 50; ++i)
        {
            const uint16_t monKind = regenMonKinds[i];
            if (!monKind)
                continue;

            auto* monRaw = reinterpret_cast<unsigned char*>(g_pcltCharKindInfo ? g_pcltCharKindInfo->GetCharKindInfo(monKind) : nullptr);
            if (!monRaw)
                continue;

            // 依 decompile 偏移：
            // +2: name text code (WORD), +146: level (BYTE), +212: alignment/tribe (DWORD)
            const uint16_t nameCode = *reinterpret_cast<uint16_t*>(monRaw + 2);
            const uint8_t level = *(monRaw + 146);
            const int align = *reinterpret_cast<int*>(monRaw + 212);

            const char* monName = g_DCTTextManager.GetText(nameCode);
            char line[256] = {};
            std::snprintf(line, sizeof(line), "%s(Lv%d)", monName ? monName : "", static_cast<int>(level));

            DWORD color = 0xFFFFFF00; // -256
            if (align > 0) color = 0xFFFF0000;   // -65536
            else if (align < 0) color = 0xFFFFFFFF; // -1
            pushWorldLine(line, color);
        }
    }

    // 2) running quest 導向資訊（對齊 decompile 中段的 quest switch）
    //    由於目前結構尚未完整還原，採「可解析就輸出」策略：
    //    - 優先顯示 quest 指向 NPC 名稱
    //    - 若可拿到 NPC map，再附帶 (mapName)
    const uint8_t runningCount = g_clQuestSystem.GetRunningQuestCount();
    for (uint8_t i = 0; i < runningCount && m_nWorldMapTextCount < 50; ++i)
    {
        stPlayingQuestInfo* q = g_clQuestSystem.GetRunningQuestInfoByIndex(i);
        if (!q)
            continue;

        stQuestKindInfo* qk = g_clQuestKindInfo.GetQuestKindInfo(q->wQuestID);
        if (!qk)
            continue;

        // 與原碼一致，使用 bPlayType 決定追蹤目標。
        // 這裡先落地通用 NPC 路徑（delivery/hunt/common）。
        uint16_t npcKind = 0;
        switch (qk->bPlayType)
        {
        case 1: // DELIVERY
        case 3: // ONEWAY
        case 4: // BUY EMBLEM-like
        case 5: // SPECIAL DELIVERY
        case 6:
        case 7:
            npcKind = qk->extra.tail[2] | (static_cast<uint16_t>(qk->extra.tail[3]) << 8); // offset 98
            break;
        case 2: // HUNT
            npcKind = qk->extra.tail[6] | (static_cast<uint16_t>(qk->extra.tail[7]) << 8); // offset 102
            break;
        default:
            break;
        }

        if (!npcKind)
            continue;

        stNPCInfo* npc = g_clNPCInfo.GetNPCInfoByID(npcKind);
        if (!npc)
            continue;

        auto* npcRaw = reinterpret_cast<unsigned char*>(npc);
        const uint16_t npcNameCode = *reinterpret_cast<uint16_t*>(npcRaw + 4);
        const uint16_t npcMapID = *reinterpret_cast<uint16_t*>(npcRaw + 12);

        const char* npcName = g_DCTTextManager.GetText(npcNameCode);
        stMapInfo* npcMap = g_Map.GetMapInfoByID(npcMapID);
        const char* npcMapName = npcMap ? g_DCTTextManager.GetText(npcMap->m_wFileName) : nullptr;

        char line[256] = {};
        if (npcMapName && npcMapName[0])
            std::snprintf(line, sizeof(line), "%s(%s)", npcName ? npcName : "", npcMapName);
        else
            std::snprintf(line, sizeof(line), "%s", npcName ? npcName : "");
        pushWorldLine(line, 0xFFFFFF00); // -256
    }

    // ===== 尺寸計算（對齊 decompile 收尾）=====
    uint16_t width = 0;
    uint16_t height = 0;
    for (int i = 0; i < 50; ++i)
    {
        DWORD textSize[2] = {};
        m_worldMapText[i].GetTextLength(textSize);
        const uint16_t lineW = static_cast<uint16_t>(LOWORD(textSize[0]));
        const uint16_t lineH = static_cast<uint16_t>(HIWORD(textSize[0]));
        if (width < static_cast<uint16_t>(lineW + 20))
            width = static_cast<uint16_t>(lineW + 20);
        if (m_worldMapText[i].IsStringData())
            height = static_cast<uint16_t>(height + lineH + 7);
    }

    m_innerBox.SetWidth(width);
    m_innerBox.SetHeight(static_cast<uint16_t>(height + 20));
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
    // 盡可能對齊 004287D0（mofclient.c）：
    // - 先拉 item kind 與名稱
    // - 再依 item class/type 輸出 index line
    // - 最後補描述
    //
    // 注意：專案中 stItemKindInfo 多 union/欄位仍在持續還原，本函數採
    //「可讀欄位先完整落地，未知欄位保留明確 TODO」策略。
    stItemKindInfo* item = g_clItemKindInfo.GetItemKindInfo(m_usKindID);
    if (!item)
        return;

    // Name
    m_textTitle.SetText(g_DCTTextManager.GetText(item->m_wTextCode));

    // 共通欄位（對齊 decompile 中大量 AddIndexData 的骨架）
    AddIndexData(0x0C28u, const_cast<char*>(g_DCTTextManager.GetText(item->m_wTextCode)), 0xFFFFC000);

    // 價格（decompile 會依 UI 場景/compare flag 決定顯示格式；先落地主要值）
    if (item->m_dwPrice)
        AddIndexData(0x0C29u, const_cast<char*>("%d"), static_cast<int>(item->m_dwPrice));
    if (item->m_dwPrice1)
        AddIndexData(0x0ECFu, static_cast<int>(item->m_dwPrice1), 0xFFFFC000);
    if (item->m_dwPrice2)
        AddIndexData(0x0ED0u, static_cast<int>(item->m_dwPrice2), 0xFFFFC000);
    if (item->m_dwPrice3)
        AddIndexData(0x0EDBu, static_cast<int>(item->m_dwPrice3), 0xFFFFC000);

    // 類型分流（對齊原始 itemClass 主 switch）
    switch (item->m_byItemClass)
    {
    case ITEM_CLASS_INSTANT:
    case ITEM_CLASS_EVENT_INSTANT:
    case ITEM_CLASS_PCBANG_INSTANT:
    {
        // Instant 共通屬性（HP/MP / stat buffs / sustain time）
        if (item->Instant.m_wAddHP) AddIndexData(0x0C2Du, item->Instant.m_wAddHP, 0xFFFFC000);
        if (item->Instant.m_wAddMP) AddIndexData(0x0C2Eu, item->Instant.m_wAddMP, 0xFFFFC000);
        if (item->Instant.m_wAddHP_Percent) AddIndexData(0x0C2Fu, item->Instant.m_wAddHP_Percent, 0xFFFFC000);
        if (item->Instant.m_wAddMP_Percent) AddIndexData(0x0C30u, item->Instant.m_wAddMP_Percent, 0xFFFFC000);
        if (item->Instant.m_wAddStr) AddIndexData(0x0C31u, item->Instant.m_wAddStr, 0xFFFFC000);
        if (item->Instant.m_wAddDex) AddIndexData(0x0C32u, item->Instant.m_wAddDex, 0xFFFFC000);
        if (item->Instant.m_wAddInt) AddIndexData(0x0C33u, item->Instant.m_wAddInt, 0xFFFFC000);
        if (item->Instant.m_wAddSta) AddIndexData(0x0C34u, item->Instant.m_wAddSta, 0xFFFFC000);
        if (item->Instant.m_dwSustainTime) AddIndexData(0x0EDCu, static_cast<int>(item->Instant.m_dwSustainTime), 0xFFFFC000);
        break;
    }
    case ITEM_CLASS_HUNT:
    case ITEM_CLASS_EVENT_HUNT:
    case ITEM_CLASS_CASH_HUNT:
    {
        // Hunt 裝備屬性（目前結構已還原欄位優先）
        if (item->Equip.Hunt.m_byLevel) AddIndexData(0x0C3Bu, item->Equip.Hunt.m_byLevel, 0xFFFFC000);
        if (item->Equip.Hunt.m_wNeedStr)   AddIndexData(0x0C3Eu, item->Equip.Hunt.m_wNeedStr, 0xFFFFC000);
        if (item->Equip.Hunt.m_wNeedDex)   AddIndexData(0x0C3Fu, item->Equip.Hunt.m_wNeedDex, 0xFFFFC000);
        if (item->Equip.Hunt.m_wMinAttack) AddIndexData(0x0C50u, item->Equip.Hunt.m_wMinAttack, 0xFFFFC000);
        if (item->Equip.Hunt.m_wMaxAttack) AddIndexData(0x0C51u, item->Equip.Hunt.m_wMaxAttack, 0xFFFFC000);
        if (item->Equip.Hunt.m_wAddInt) AddIndexData(0x0C33u, item->Equip.Hunt.m_wAddInt, 0xFFFFC000);
        if (item->Equip.Hunt.m_wAddSta) AddIndexData(0x0C34u, item->Equip.Hunt.m_wAddSta, 0xFFFFC000);
        if (item->Equip.Hunt.m_wAddAccuracy) AddIndexData(0x0C5Du, item->Equip.Hunt.m_wAddAccuracy, 0xFFFFC000);
        if (item->Equip.Hunt.m_wAddCritical) AddIndexData(0x0C5Eu, item->Equip.Hunt.m_wAddCritical, 0xFFFFC000);
        if (item->Equip.Hunt.m_wAddEvasion) AddIndexData(0x0C5Fu, item->Equip.Hunt.m_wAddEvasion, 0xFFFFC000);
        if (item->Equip.Hunt.m_wAddAccuracy) AddIndexData(0x0C60u, item->Equip.Hunt.m_wAddAccuracy, 0xFFFFC000);
        break;
    }
    case ITEM_CLASS_FASHION:
    case ITEM_CLASS_EVENT_FASHION:
    case ITEM_CLASS_CASH_FASHION:
    {
        if (item->Equip.Hunt.m_byLevel) AddIndexData(0x0C3Bu, item->Equip.Hunt.m_byLevel, 0xFFFFC000);
        if (item->Equip.Fashion.m_dwAddStr) AddIndexData(0x0C31u, item->Equip.Fashion.m_dwAddStr, 0xFFFFC000);
        if (item->Equip.Fashion.m_dwAddDex) AddIndexData(0x0C32u, item->Equip.Fashion.m_dwAddDex, 0xFFFFC000);
        if (item->Equip.Fashion.m_dwAddInt) AddIndexData(0x0C33u, item->Equip.Fashion.m_dwAddInt, 0xFFFFC000);
        if (item->Equip.Fashion.m_dwAddSta) AddIndexData(0x0C34u, item->Equip.Fashion.m_dwAddSta, 0xFFFFC000);
        break;
    }
    case ITEM_CLASS_TRAINING:
    case ITEM_CLASS_CASH_TRAINING:
    case ITEM_CLASS_EVENT_TRAINING:
        // TODO(ground-truth): 004287D0 後半包含訓練卡、課程、時間限制與等級差色彩判斷
        break;
    default:
        break;
    }

    // 使用期限（原碼在多分支都會輸出）
    if (item->m_wUseTerm)
        AddIndexData(0x0C46u, static_cast<int>(item->m_wUseTerm), 0xFFFFC000);

    // 描述（ground truth：多數 itemFlag=1）
    AddDesc(item->m_wDescCode, 1);
}
void CToolTip::ProcessMakingItem()
{
    stItemKindInfo* item = g_clItemKindInfo.GetItemKindInfo(m_usKindID);
    if (!item)
        return;

    m_textTitle.SetText(g_DCTTextManager.GetText(item->m_wTextCode));

    // 對齊 00428DE0：specialty line
    const uint16_t specialtyKind = *(reinterpret_cast<uint16_t*>(reinterpret_cast<unsigned char*>(item) + 138)); // word[69]
    if (specialtyKind)
    {
        strSpecialtyKindInfo* spec = g_clSpecialtyKindInfo.GetSpecialtyKindInfo(specialtyKind);
        if (spec && spec->wTextCode)
        {
            // 原版會依「是否已習得」決定顏色；此專案 specialty system 全域未完整接線，先保留可讀顏色。
            AddIndexData(0x0F09u, g_DCTTextManager.GetText(spec->wTextCode), 0xFFFFC000);
        }
    }

    // 對齊 00428DE0：recipe / ingredient section
    const uint16_t makingKind = *(reinterpret_cast<uint16_t*>(reinterpret_cast<unsigned char*>(item) + 140)); // word[70]
    if (makingKind)
    {
        strMakingItemKindInfo* making = g_clMakingItemKindInfo.GetMakingItemKindInfo(makingKind);
        if (making)
        {
            int materialLineCount = 0;
            AddIndexData(0x0F0Au, const_cast<char*>(""), 0xFFFFC000);

            for (int i = 0; i < 10; ++i)
            {
                const uint16_t matKind = making->Ingredient[i].Kind;
                const uint16_t matCnt = making->Ingredient[i].Count;
                if (!matKind)
                    continue;

                stItemKindInfo* matInfo = g_clItemKindInfo.GetItemKindInfo(matKind);
                if (!matInfo)
                    continue;

                char line[256] = {};
                const char* countSuffix = g_DCTTextManager.GetText(3109);
                std::snprintf(
                    line,
                    sizeof(line),
                    "%s %d%s",
                    g_DCTTextManager.GetText(matInfo->m_wTextCode),
                    static_cast<int>(matCnt),
                    countSuffix ? countSuffix : "");
                AddNextLineData(line);
                ++materialLineCount;
            }

            if (materialLineCount > 0)
                SetVoidIndex(m_nIndexCount - materialLineCount, m_nIndexCount);

            char successRate[64] = {};
            std::snprintf(successRate, sizeof(successRate), "%d%%", making->SuccessPermyriad / 100);
            AddIndexData(0x0F0Bu, successRate, 0xFFFFC000);
            AddIndexData(0x0F0Cu, making->ExpBonus, 0xFFFFC000);
        }
    }

    AddDesc(item->m_wDescCode, 1);
}
void CToolTip::ProcessHunt()
{
    stItemKindInfo* item = g_clItemKindInfo.GetItemKindInfo(m_usKindID);
    if (!item)
        return;

    m_textTitle.SetText(g_DCTTextManager.GetText(item->m_wTextCode));

    // 對齊 00428FD0：類型名稱 + 武器/防具主屬性
    AddIndexData(0x0C28u, g_DCTTextManager.GetText(item->m_wTextCode), 0xFFFFC000);

    if (item->Equip.Hunt.m_wWeaponType)
        AddIndexData(0x0C38u, item->Equip.Hunt.m_wWeaponType, 0xFFFFC000);

    if (item->Equip.Hunt.m_wMinAttack || item->Equip.Hunt.m_wMaxAttack)
    {
        char atk[64] = {};
        std::snprintf(atk, sizeof(atk), "%d ~ %d", item->Equip.Hunt.m_wMinAttack, item->Equip.Hunt.m_wMaxAttack);
        AddIndexData(0x0C29u, atk, 0xFFFFC000);
    }

    if (item->Equip.Hunt.m_wDef)
        AddIndexData(0x0ECFu, item->Equip.Hunt.m_wDef, 0xFFFFC000);
    if (item->Equip.Hunt.m_dwAttSpeed)
        AddIndexData(0x0ED0u, static_cast<int>(item->Equip.Hunt.m_dwAttSpeed), 0xFFFFC000);

    // 裝備加成（對齊 decompile 常見欄位）
    if (item->Equip.Hunt.m_wAddStr) AddIndexData(0x0C31u, item->Equip.Hunt.m_wAddStr, 0xFFFFC000);
    if (item->Equip.Hunt.m_wAddDex) AddIndexData(0x0C32u, item->Equip.Hunt.m_wAddDex, 0xFFFFC000);
    if (item->Equip.Hunt.m_wAddInt) AddIndexData(0x0C33u, item->Equip.Hunt.m_wAddInt, 0xFFFFC000);
    if (item->Equip.Hunt.m_wAddSta) AddIndexData(0x0C34u, item->Equip.Hunt.m_wAddSta, 0xFFFFC000);

    if (item->Equip.Hunt.m_wAddAccuracy) AddIndexData(0x0C5Du, item->Equip.Hunt.m_wAddAccuracy, 0xFFFFC000);
    if (item->Equip.Hunt.m_wAddCritical) AddIndexData(0x0C5Eu, item->Equip.Hunt.m_wAddCritical, 0xFFFFC000);
    if (item->Equip.Hunt.m_wAddEvasion) AddIndexData(0x0C5Fu, item->Equip.Hunt.m_wAddEvasion, 0xFFFFC000);
    if (item->Equip.Hunt.m_wMagicResist) AddIndexData(0x0C60u, item->Equip.Hunt.m_wMagicResist, 0xFFFFC000);

    // 要求條件（等級 / 四維）
    if (item->Equip.Hunt.m_byLevel)
        AddIndexData(0x0C2Au, item->Equip.Hunt.m_byLevel, 0xFFFFC000);
    else
        AddIndexData(0x0C2Au, g_DCTTextManager.GetText(3178), 0xFFFFC000);

    PrintReqClassForHuntItem();
    if (item->Equip.Hunt.m_wNeedStr) AddIndexData(0x0C7Du, item->Equip.Hunt.m_wNeedStr, 0xFFFFC000);
    if (item->Equip.Hunt.m_wNeedDex) AddIndexData(0x0C7Eu, item->Equip.Hunt.m_wNeedDex, 0xFFFFC000);
    if (item->Equip.Hunt.m_wNeedSta) AddIndexData(0x0C7Fu, item->Equip.Hunt.m_wNeedSta, 0xFFFFC000);
    if (item->Equip.Hunt.m_wNeedInt) AddIndexData(0x0C80u, item->Equip.Hunt.m_wNeedInt, 0xFFFFC000);

    // 稀有度背景色（對齊 00428FD0 尾段）
    if (item->Equip.Hunt.m_byRareType == 1)
        m_innerBox.SetColor(0.0f, 0.078431375f, 0.49019611f, 0.58823532f);
    else if (item->Equip.Hunt.m_byRareType == 2)
        m_innerBox.SetColor(0.18039216f, 0.070588239f, 0.34117648f, 0.82352948f);

    OutputCashShopTime(item);
    AddDesc(item->m_wDescCode, 1);
}
void CToolTip::ProcessFashion()
{
    stItemKindInfo* item = g_clItemKindInfo.GetItemKindInfo(m_usKindID);
    if (!item)
        return;

    m_textTitle.SetText(g_DCTTextManager.GetText(item->m_wTextCode));
    AddIndexData(0x0C28u, g_DCTTextManager.GetText(item->m_wTextCode), 0xFFFFC000);

    // 性別需求（對齊 00429A00：F/M/ALL 三態）
    unsigned int genderColor = 0xFFFFC000;
    int genderTextCode = 3178; // ALL
    if (item->Equip.Fashion.m_byGender == 'F')
        genderTextCode = 3181;
    else if (item->Equip.Fashion.m_byGender == 'M')
        genderTextCode = 3180;
    AddIndexData(0x0C3Bu, g_DCTTextManager.GetText(genderTextCode), genderColor);

    OutputCashShopTime(item);

    if (item->Equip.Hunt.m_wNeedStr) AddIndexData(0x0C3Eu, item->Equip.Hunt.m_wNeedStr, 0xFFFFC000);
    if (item->Equip.Hunt.m_wNeedDex) AddIndexData(0x0C3Fu, item->Equip.Hunt.m_wNeedDex, 0xFFFFC000);

    if (item->Equip.Fashion.m_dwAddStr) AddIndexData(0x0C2Du, item->Equip.Fashion.m_dwAddStr, 0xFFFFC000);
    if (item->Equip.Fashion.m_dwAddDex) AddIndexData(0x0C2Fu, item->Equip.Fashion.m_dwAddDex, 0xFFFFC000);

    if (item->Equip.Fashion.m_dwAccuracyThousand)
    {
        float v = item->Equip.Fashion.m_dwAccuracyThousand * 0.001f;
        AddIndexData(0x0C32u, const_cast<char*>("%.1f"), v);
    }
    if (item->Equip.Fashion.m_dwEvasionThousand)
    {
        float v = item->Equip.Fashion.m_dwEvasionThousand * 0.001f;
        AddIndexData(0x0C33u, const_cast<char*>("%.1f"), v);
    }

    if (item->Equip.Fashion.m_dwDamagePercent) AddIndexData(0x0C5Du, item->Equip.Fashion.m_dwDamagePercent, 0xFFFFC000);
    if (item->Equip.Fashion.m_dwDefPercent) AddIndexData(0x0C60u, item->Equip.Fashion.m_dwDefPercent, 0xFFFFC000);
    if (item->Equip.Fashion.m_dwAddInt) AddIndexData(0x0C5Eu, item->Equip.Fashion.m_dwAddInt, 0xFFFFC000);
    if (item->Equip.Fashion.m_dwAddSta) AddIndexData(0x0C5Fu, item->Equip.Fashion.m_dwAddSta, 0xFFFFC000);

    AddDesc(item->m_wDescCode, 1);
}
void CToolTip::ProcessSkill(uint16_t skillKind)
{
    uint16_t useKind = skillKind ? skillKind : m_usKindID;
    stSkillKindInfo* skill = g_clSkillKindInfo.GetSkillKindInfo(useKind);
    if (!skill)
        return;

    const bool fromBook = (skillKind != 0);
    const bool isActive = g_clSkillKindInfo.IsActiveSkill(useKind) != 0;

    const uint16_t nameCode = *reinterpret_cast<uint16_t*>(skill->raw + 4);
    const uint16_t descCode = *reinterpret_cast<uint16_t*>(skill->raw + 6);
    m_textTitle.SetText(g_DCTTextManager.GetText(nameCode));
    AddIndexData(0x0C41u, g_DCTTextManager.GetText(isActive ? 3173 : 3172), 0xFFFFC000);

    if (isActive)
    {
        const int ap = *reinterpret_cast<int32_t*>(skill->raw + 24);
        const int dp = *reinterpret_cast<int32_t*>(skill->raw + 28);
        const int coolMs = *reinterpret_cast<int32_t*>(skill->raw + 36);
        const int needHp = *reinterpret_cast<uint16_t*>(skill->raw + 132);
        const int needMp = *reinterpret_cast<uint16_t*>(skill->raw + 134);
        if (ap) AddIndexData(0x0C50u, g_DCTTextManager.GetText(3244), ap);
        if (dp) AddIndexData(0x0C51u, g_DCTTextManager.GetText(3244), dp);
        if (coolMs)
        {
            char coolBuf[64] = {};
            std::snprintf(coolBuf, sizeof(coolBuf), "%.2f%s", coolMs * 0.001f, g_DCTTextManager.GetText(3224));
            AddIndexData(0x0C47u, coolBuf, 0xFFFFC000);
        }
        if (needHp) AddIndexData(0x0CC9u, needHp, 0xFFFFC000);
        if (needMp) AddIndexData(0x0CCAu, needMp, 0xFFFFC000);
    }
    else
    {
        const int bonus1 = *reinterpret_cast<int32_t*>(skill->raw + 152);
        const int bonus2 = *reinterpret_cast<int32_t*>(skill->raw + 156);
        const int bonus3 = *reinterpret_cast<int32_t*>(skill->raw + 160);
        if (bonus1) AddIndexData(0x0C52u, g_DCTTextManager.GetText(3244), bonus1);
        if (bonus2) AddIndexData(0x0C53u, g_DCTTextManager.GetText(3244), bonus2);
        if (bonus3) AddIndexData(0x0E30u, g_DCTTextManager.GetText(3244), bonus3);
    }

    PrintReqWeaponForSkill(skill, !fromBook);

    const uint16_t reqLv = *reinterpret_cast<uint16_t*>(skill->raw + 68);
    if (reqLv)
        AddIndexData(0x0BDBu, reqLv, 0xFFFFC000);
    else
        AddIndexData(0x0BDBu, g_DCTTextManager.GetText(3178), 0xFFFFC000);

    m_nTitleColor = (reqLv && g_clLevelSystem.GetLevel() < reqLv) ? 0xFFFF0000 : 0xFFFFFFFF;
    PrintReqClassForSkill(skill);

    if (!skillKind)
        AddDesc(descCode, 0);
}
void CToolTip::ProcessEmblem()
{
    strEmblemKindInfo* e = g_clEmblemKindInfo.GetEmblemKindInfo(m_usKindID);
    if (!e)
        return;

    m_textTitle.SetText(g_DCTTextManager.GetText(e->wEmblemNameCode));
    AddIndexData(0x0C28u, g_DCTTextManager.GetText(4246), 0xFFFFC000);

    if (e->wEquipConditionLevelFrom)
    {
        char buf[128] = {};
        std::snprintf(buf, sizeof(buf), g_DCTTextManager.GetParsedText(4305, 0, nullptr), e->wEquipConditionLevelFrom);
        AddIndexData(0x0C2Au, buf, 0xFFFFC000);
    }
    if (e->wEquipConditionLevelTo)
    {
        char buf[128] = {};
        std::snprintf(buf, sizeof(buf), g_DCTTextManager.GetParsedText(4306, 0, nullptr), e->wEquipConditionLevelTo);
        AddIndexData(0x0C2Au, buf, 0xFFFFC000);
    }

    if (e->dwExtraExperienceRate) AddIndexData(0x0F0Cu, g_DCTTextManager.GetText(3244), static_cast<int>(e->dwExtraExperienceRate));
    if (e->dwAcquireConditionSwordClassCompletionRate) AddIndexData(0x0C2Du, g_DCTTextManager.GetText(3244), static_cast<int>(e->dwAcquireConditionSwordClassCompletionRate));
    if (e->dwAcquireConditionArcheryClassCompletionRate) AddIndexData(0x0C2Fu, g_DCTTextManager.GetText(3244), static_cast<int>(e->dwAcquireConditionArcheryClassCompletionRate));
    if (e->dwAcquireConditionMagicClassCompletionRate) AddIndexData(0x10A4u, g_DCTTextManager.GetText(3244), static_cast<int>(e->dwAcquireConditionMagicClassCompletionRate));
    if (e->dwAcquireConditionTheologyClassCompletionRate) AddIndexData(0x10A5u, g_DCTTextManager.GetText(3244), static_cast<int>(e->dwAcquireConditionTheologyClassCompletionRate));

    if (e->dwExtraHitRate) AddIndexData(0x0C32u, g_DCTTextManager.GetText(3636), e->dwExtraHitRate * 0.001f);
    if (e->dwExtraCriticalRate) AddIndexData(0x0C33u, g_DCTTextManager.GetText(3636), e->dwExtraCriticalRate * 0.001f);
    if (e->dwExtraEvasionRate) AddIndexData(0x0C34u, g_DCTTextManager.GetText(3636), e->dwExtraEvasionRate * 0.001f);
    if (e->dwAcquireConditionCritDealtRate) AddIndexData(0x0CCBu, g_DCTTextManager.GetText(3636), e->dwAcquireConditionCritDealtRate * 0.001f);

    if (e->dwAcquireConditionItemSale) AddIndexData(0x10A6u, static_cast<int>(e->dwAcquireConditionItemSale), 0xFFFFC000);
    if (e->dwAcquireConditionPublicQuestCompletionRate) AddIndexData(0x10A7u, g_DCTTextManager.GetText(3636), e->dwAcquireConditionPublicQuestCompletionRate * 0.001f);
    if (e->dwAcquireConditionJobChangeQuestCompletion) AddIndexData(0x10A8u, static_cast<int>(e->dwAcquireConditionJobChangeQuestCompletion), 0xFFFFC000);
    if (e->dwAcquireConditionCircleTaskCompletion) AddIndexData(0x10A9u, g_DCTTextManager.GetText(3636), e->dwAcquireConditionCircleTaskCompletion * 0.001f);
    if (e->dwAcquireConditionAllBossKill) AddIndexData(0x10AAu, static_cast<int>(e->dwAcquireConditionAllBossKill), 0xFFFFC000);

    if (e->wTrainingCard)
    {
        stItemKindInfo* tr = g_clItemKindInfo.GetItemKindInfo(e->wTrainingCard);
        if (tr) AddIndexData(0x1FE0u, g_DCTTextManager.GetText(tr->m_wTextCode), 0xFFFFC000);
    }

    AddDesc(e->wEmblemDescriptionCode, 0);
}
void CToolTip::OutputCashShopTime(stItemKindInfo* pItemInfo)
{
    if (!pItemInfo)
        return;
    if (!cltItemKindInfo::IsCashItem(static_cast<EItemClass>(pItemInfo->m_byItemClass)) ||
        !cltItemKindInfo::IsTimerItem(static_cast<EItemClass>(pItemInfo->m_byItemClass)))
        return;

    if (pItemInfo->m_wUseTerm == 0)
    {
        AddIndexData(0x0C3Cu, g_DCTTextManager.GetText(4024), 0xFFFFC000);
        return;
    }

    // Ground-truth 會依 UI type/slot 從不同系統讀剩餘秒數；在還原不足情況下以 useTerm 作為 fallback。
    unsigned int remainSec = pItemInfo->m_wUseTerm;
    if (remainSec <= 365)
        AddIndexData(0x0C3Cu, g_DCTTextManager.GetText(4011), static_cast<int>(remainSec));
    else
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
    // 原始碼從當前地圖取 climate；這裡優先以 tooltip kind 作 climate code，
    // 若無資料則回退把 tooltip kind 視為 map id，顯示基本氣候標題。
    static cltClimateKindInfo climateInfo;
    strClimateInfo* c = climateInfo.GetClimateKindInfo(m_usKindID);
    if (!c)
    {
        stMapInfo* mapInfo = g_Map.GetMapInfoByID(m_usKindID);
        if (!mapInfo)
            return;
        m_textTitle.SetText(g_DCTTextManager.GetText(mapInfo->m_wRegionNameCode));
        AddIndexData(0x0C28u, g_DCTTextManager.GetText(8165), 0xFFFFC000);
        AddDesc(0, 0);
        return;
    }

    m_textTitle.SetText(g_DCTTextManager.GetText(c->nameTextId));
    AddIndexData(0x0C28u, g_DCTTextManager.GetText(8165), 0xFFFFC000);
    AddDesc(0, 0);

    if (c->moveSpeedPermil != 1000)
        AddIndexData(0x1FE6u, g_DCTTextManager.GetText(3636), (1000 - c->moveSpeedPermil) * 0.001f);
    if (c->hpRegenIncPermil)
        AddIndexData(0x1FE7u, g_DCTTextManager.GetText(3636), c->hpRegenIncPermil * 0.001f);
    if (c->mpRegenDecPermil)
        AddIndexData(0x1FE8u, g_DCTTextManager.GetText(3636), c->mpRegenDecPermil * 0.001f);
    if (c->itemDropIncPermil)
        AddIndexData(0x1FE9u, g_DCTTextManager.GetText(3636), (c->itemDropIncPermil % 1000) * 0.001f);
    if (c->unitCount)
        AddIndexData(0x1FEAu, g_DCTTextManager.GetText(8175), c->unitCount * 0.001f + 1.0f);
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
void CToolTip::ProcessPetSkill(uint16_t petSkillKind)
{
    uint16_t skillKind = petSkillKind ? petSkillKind : m_usKindID;
    strPetSkillKindInfo* info = g_clPetSkillKindInfo.GetPetSkillKindInfo(skillKind);
    if (!info)
        return;

    m_textTitle.SetText(g_DCTTextManager.GetText(info->wSkillNameTextId));
    AddIndexData(0x0C41u, g_DCTTextManager.GetText(7216), 0xFFFFC000);

    if (info->dwAttackPowerIncreasePerThousand)
        AddIndexData(0x0DCAu, const_cast<char*>("%.2f"), info->dwAttackPowerIncreasePerThousand * 0.001f);
    if (info->dwDefensePowerIncreasePerThousand)
        AddIndexData(0x0DCBu, const_cast<char*>("%.2f"), info->dwDefensePowerIncreasePerThousand * 0.001f);
    if (info->dwHitRateIncreasePerThousand)
        AddIndexData(0x0DCCu, const_cast<char*>("%.2f"), info->dwHitRateIncreasePerThousand * 0.001f);

    if (info->dwSkillAttackPowerIncrease)
        AddIndexData(0x0C2Eu, const_cast<char*>("%d"), static_cast<int>(info->dwSkillAttackPowerIncrease));
    if (info->dwAttackSpeed)
        AddIndexData(0x0ECFu, const_cast<char*>("%d"), static_cast<int>(info->dwAttackSpeed));

    if (info->dwHpAutoRecoveryRateChangePerThousand)
        AddIndexData(0x10B1u, const_cast<char*>("%.2f"), info->dwHpAutoRecoveryRateChangePerThousand * 0.001f);
    if (info->dwMpAutoRecoveryRateChangePerThousand)
        AddIndexData(0x10B3u, const_cast<char*>("%.2f"), info->dwMpAutoRecoveryRateChangePerThousand * 0.001f);

    if (info->wPickup)
        AddIndexData(0x1C31u, g_DCTTextManager.GetText(7219), 0xFFFFC000);

    if (info->wRequiredLevel)
    {
        const unsigned int levelColor = (g_clPetSystem.GetPetLevel() < info->wRequiredLevel) ? 0xFFFF4600 : 0xFFFFC000;
        AddIndexData(0x1C32u, static_cast<int>(info->wRequiredLevel), levelColor);
    }

    AddDesc(info->wSkillDescriptionTextId, 0);
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

    // 原碼這段會組 marriage system 的剩餘召喚次數；該全域目前未完整還原，先用可等價的可召喚/不可召喚文字。
    if (info->canSummonSpouse)
        AddIndexData(0x1FCAu, g_DCTTextManager.GetText(8216), 0xFFFFC000);
    else
        AddIndexData(0x1FCAu, g_DCTTextManager.GetText(8217), 0xFFFFC000);

    if (info->expRatePercent)
        AddIndexData(0x0C3Du, g_DCTTextManager.GetText(3244), info->expRatePercent);

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
    if (!pData)
        return;

    const unsigned char* raw = reinterpret_cast<const unsigned char*>(pData);
    m_nMouseX = x;
    m_nMouseY = y;
    m_nType = *reinterpret_cast<const int*>(raw + 0);
    m_nSubType = *reinterpret_cast<const int*>(raw + 4);
    m_usKindID = *reinterpret_cast<const uint16_t*>(raw + 8);
    m_nCompareFlag = *reinterpret_cast<const int*>(raw + 12);
    m_strData = *reinterpret_cast<const std::string*>(raw + 16);
    m_byUIType = *(raw + 32);
    m_usSlotIndex = *reinterpret_cast<const uint16_t*>(raw + 34);
    m_nExtraData = *reinterpret_cast<const int*>(raw + 36);
    (void)compareFlag;

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
    for (int i = from; i < to && m_nVoidCount < 20; ++i)
        m_voidIndices[m_nVoidCount++] = i;
}

void CToolTip::PrintReqWeaponForSkill(stSkillKindInfo* pSkill, int checkEquipped)
{
    if (!pSkill)
        return;

    const int reqWeaponCount = *reinterpret_cast<uint32_t*>(pSkill->raw + 88);
    if (reqWeaponCount <= 0)
    {
        AddIndexData(0x0C79u, g_DCTTextManager.GetText(3178), 0xFFFFC000);
        return;
    }

    SetVoidIndex(m_nIndexCount + 1, m_nIndexCount + reqWeaponCount);
    bool satisfied = !checkEquipped;

    uint16_t equipMainType = 0;
    uint16_t equipSubType = 0;
    if (checkEquipped && dword_21BA32C)
    {
        const uint16_t equipMain = dword_21BA32C->GetEquipItem(1u, 4u);
        const uint16_t equipSub = dword_21BA32C->GetEquipItem(1u, 5u);
        if (equipMain)
        {
            stItemKindInfo* info = g_clItemKindInfo.GetItemKindInfo(equipMain);
            if (info) equipMainType = info->Equip.Hunt.m_wWeaponType;
        }
        if (equipSub)
        {
            stItemKindInfo* info = g_clItemKindInfo.GetItemKindInfo(equipSub);
            if (info) equipSubType = info->Equip.Hunt.m_wWeaponType;
        }
    }

    for (int i = 0; i < 15; ++i)
    {
        if (pSkill->raw[72 + i])
        {
            if (checkEquipped && (i == equipMainType || i == equipSubType))
                satisfied = true;
            AddIndexData(0x0C79u, GetWeaponTypeText(i), satisfied ? 0xFFFFC000 : 0xFFFF4600);
        }
    }
}
void CToolTip::PrintReqClassForSkill(stSkillKindInfo* pSkill)
{
    if (!pSkill)
        return;
    const uint16_t myClass = g_clClassSystem.GetClass();
    if (!myClass)
        return;

    uint16_t classes[64] = {};
    const uint64_t classMask = *reinterpret_cast<uint64_t*>(pSkill->raw + 40);
    const int count = g_clClassKindInfo.GetClassKindsByAtb(classMask, classes);
    if (count <= 0)
    {
        AddIndexData(0x0C7Au, g_DCTTextManager.GetText(3178), 0xFFFFC000);
        return;
    }

    SetVoidIndex(m_nIndexCount + 1, m_nIndexCount + count);
    bool canUse = false;
    for (int i = 0; i < count; ++i)
    {
        if (classes[i] == myClass)
            canUse = true;
    }
    for (int i = 0; i < count; ++i)
    {
        strClassKindInfo* info = g_clClassKindInfo.GetClassKindInfo(classes[i]);
        if (info)
            AddIndexData(0x0C7Au, g_DCTTextManager.GetText(info->name_code), canUse ? 0xFFFFC000 : 0xFFFF4600);
    }
}
void CToolTip::PrintReqClassForHuntItem()
{
    uint16_t classes[32] = {};
    int count = 0;
    if (!g_clItemKindInfo.GetReqClassKindsForEquip(m_usKindID, &count, classes))
        return;

    if (count <= 0)
    {
        AddIndexData(0x0C2Bu, g_DCTTextManager.GetText(3178), 0xFFFFC000);
        return;
    }

    SetVoidIndex(m_nIndexCount + 1, m_nIndexCount + count);
    const uint16_t myClass = g_clClassSystem.GetClass();
    bool canUse = false;
    for (int i = 0; i < count; ++i)
        if (classes[i] == myClass) canUse = true;

    for (int i = 0; i < count; ++i)
    {
        strClassKindInfo* info = g_clClassKindInfo.GetClassKindInfo(classes[i]);
        if (info)
            AddIndexData(0x0C2Bu, g_DCTTextManager.GetText(info->name_code), canUse ? 0xFFFFC000 : 0xFFFF4600);
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
    if (!pMapInfo || m_nWorldMapTextCount > 50)
        return;
    const int textCode = pMapInfo->m_wDungeonNameCode ? pMapInfo->m_wDungeonNameCode : pMapInfo->m_wFileName;
    m_worldMapText[m_nWorldMapTextCount].SetText(g_DCTTextManager.GetText(textCode));
    m_worldMapText[m_nWorldMapTextCount++].m_TextColor = 0xFF00FF00; // -16711936
}
void CToolTip::SetTextDungeonBasic(stMapInfo* pMapInfo)
{
    if (!pMapInfo || m_nWorldMapTextCount > 50)
        return;

    // 盡量對齊 ground-truth：輸出副本基礎文字（地區、怪物群、難度）
    if (pMapInfo->m_wRegionCode)
    {
        m_worldMapText[m_nWorldMapTextCount].SetText(g_DCTTextManager.GetText(pMapInfo->m_wRegionCode));
        m_worldMapText[m_nWorldMapTextCount++].m_TextColor = 0xFFFFFFFF;
    }

    uint16_t* regen = g_cltRegenMonsterKindInfo.GetRegenMonsterKindByMapID(pMapInfo->m_wID);
    if (regen)
    {
        int count = 0;
        for (int i = 0; i < 5; ++i) if (regen[i]) ++count;
        if (count > 0)
        {
            char line[128] = {};
            std::snprintf(line, sizeof(line), "%s : %d", g_DCTTextManager.GetText(8218), count);
            if (m_nWorldMapTextCount < 50)
            {
                m_worldMapText[m_nWorldMapTextCount].SetText(line);
                m_worldMapText[m_nWorldMapTextCount++].m_TextColor = 0xFFFFFFFF;
            }
        }
    }
}
