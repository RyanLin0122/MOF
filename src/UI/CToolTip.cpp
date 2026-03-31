#include "UI/CToolTip.h"
#include "global.h"

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

void CToolTip::SetIndex(uint16_t textCode) { /* stub */ }

void CToolTip::AddIndexData(uint16_t textCode, int value, unsigned int color) { /* stub */ }
void CToolTip::AddIndexData(uint16_t textCode, char* value, unsigned int color) { /* stub */ }
void CToolTip::AddIndexData(uint16_t textCode, char* format, int arg) { /* stub */ }
void CToolTip::AddIndexData(uint16_t textCode, char* format, float arg) { /* stub */ }
void CToolTip::AddIndexData(uint16_t textCode, char* format, int arg1, int arg2) { /* stub */ }

void CToolTip::AddNextLineData(char* text) { /* stub */ }

void CToolTip::AddDesc(uint16_t textCode, int itemFlag) { /* stub */ }
void CToolTip::AddDesc(char* text) { /* stub */ }

void CToolTip::AddSectionBar() { /* stub */ }

void CToolTip::ProcessStatic() { /* stub */ }
void CToolTip::ProcessOneLineText(char* text) { /* stub */ }
void CToolTip::ProcessWorldMap(int mode) { /* stub */ }
void CToolTip::ProcessLesson() { /* stub */ }
void CToolTip::ProcessItem() { /* stub */ }
void CToolTip::ProcessMakingItem() { /* stub */ }
void CToolTip::ProcessHunt() { /* stub */ }
void CToolTip::ProcessFashion() { /* stub */ }
void CToolTip::ProcessSkill(uint16_t skillKind) { /* stub */ }
void CToolTip::ProcessEmblem() { /* stub */ }
void CToolTip::OutputCashShopTime(stItemKindInfo* pItemInfo) { /* stub */ }
void CToolTip::ProcessDesc() { /* stub */ }
void CToolTip::ProcessClimate() { /* stub */ }
void CToolTip::ProcessEmoticon() { /* stub */ }
void CToolTip::ProcessPetSkill(uint16_t petSkillKind) { /* stub */ }
void CToolTip::ProcessCoupleRing() { /* stub */ }

void CToolTip::PrepareDrawing() { /* stub */ }
void CToolTip::Draw() { /* stub */ }

void CToolTip::Show(int x, int y, const stToolTipData* pData, int compareFlag) { /* stub */ }
void CToolTip::Hide() { /* stub */ }
int  CToolTip::IsShow() { return 0; /* stub */ }

char* CToolTip::GetWeaponTypeText(int weaponType) { return nullptr; /* stub */ }
void CToolTip::SetVoidIndex(int from, int to) { /* stub */ }

void CToolTip::PrintReqWeaponForSkill(stSkillKindInfo* pSkill, int checkEquipped) { /* stub */ }
void CToolTip::PrintReqClassForSkill(stSkillKindInfo* pSkill) { /* stub */ }
void CToolTip::PrintReqClassForHuntItem() { /* stub */ }

void CToolTip::ProcessCharInfo(char* charName) { /* stub */ }
void CToolTip::SetTextMainTitle(stMapInfo* pMapInfo) { /* stub */ }
void CToolTip::SetTextDungeonBasic(stMapInfo* pMapInfo) { /* stub */ }
