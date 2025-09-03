#include "UI/CControlBoxQuestAlarm.h"
#include "global.h"

CControlBoxQuestAlarm::CControlBoxQuestAlarm()
    : CControlBoxBase()
    , m_rootBtn()
    , m_rootText()
    , m_childBtns{}
    , m_childTexts{}
{
    // vftable 由編譯器處理；依反編譯流程呼叫：
    Init();
    CreateChildren();
    m_expandedFlag = 0;
    m_reserved = 0;
}

CControlBoxQuestAlarm::~CControlBoxQuestAlarm()
{
    // 全部用 RAII，自動解構即可
}

void CControlBoxQuestAlarm::Init()
{
    CControlBoxBase::Init();
    // 反編譯：*((_DWORD*)this + 13) = 0;
    // 該欄位用於 BoxBase 內部旗標。這裡語意等同「初始為收起且不自動顯示子項」。
    // 以本類別邏輯以 m_expandedFlag 控制，無需額外處理。
}

void CControlBoxQuestAlarm::Hide()
{
    CControlBase::Hide();
    m_rootBtn.Hide();
    m_rootText.Hide();
    for (int i = 0; i < 5; ++i) {
        m_childBtns[i].Hide();
        m_childTexts[i].Hide();
    }
}

void CControlBoxQuestAlarm::Show()
{
    CControlBase::Show();

    // 若目前是展開狀態，沒有文字的項目要隱藏（比照反編譯）
    if (m_expandedFlag == 1) {
        for (int i = 0; i < 5; ++i) {
            const char* t = m_childTexts[i].GetText();
            if (t && std::strcmp(t, "") == 0) {
                m_childBtns[i].Hide();
                m_childTexts[i].Hide();
            }
        }
    }
}

void CControlBoxQuestAlarm::HideChild()
{
    for (int i = 0; i < 5; ++i) {
        m_childBtns[i].Hide();
        m_childTexts[i].Hide();
    }
}

void CControlBoxQuestAlarm::CreateChildren()
{
    CControlBoxBase::CreateChildren();

    // Root Button (+312)
    m_rootBtn.Create(this);
    m_rootBtn.SetPos(10, 10);
    // 設定四態同圖：資源 335544327、索引 23
    m_rootBtn.SetImage(335544327u, 23, 335544327u, 23, 335544327u, 23, 335544327u, 23);
    // 若你的 CControlButton 有設定文字的 API，可設成空字串
    // m_rootBtn.SetText("");

    // Root Text (+1036)
    m_rootText.Create(33, 10, this); // 反編譯：SetPos(23, 10)；注意：原碼是 (23,10)
    m_rootText.SetPos(23, 10);
    m_rootText.SetControlSetFont("MeritoriousElement");
    m_rootText.SetTextColor(-1);

    // 子項（按鈕 + 文字）× 5
    int y = 25;
    for (int i = 0; i < 5; ++i) {
        // Button
        m_childBtns[i].Create(this);
        m_childBtns[i].SetPos(20, y + 1);
        m_childBtns[i].SetImage(335544327u, 29, 335544327u, 29, 335544327u, 29, 335544327u, 29);
        // 若支援文字可清空
        // m_childBtns[i].SetText("");

        // Text
        m_childTexts[i].Create(33, y, this);
        m_childTexts[i].SetControlSetFont("MeritoriousElement");
        m_childTexts[i].SetTextColor(-1);

        y += 15;
    }
}

CControlButton* CControlBoxQuestAlarm::GetButtonRootTree()
{
    return &m_rootBtn;
}

CControlText* CControlBoxQuestAlarm::GetTextRoot()
{
    return &m_rootText;
}

CControlButton* CControlBoxQuestAlarm::GetButtonChild(int idx)
{
    if (idx < 0 || idx >= 5) return nullptr;
    return &m_childBtns[idx];
}

void CControlBoxQuestAlarm::SetRootName(char* name)
{
    CControlBase::Show(); // 反編譯先 Show 自己
    m_rootText.ClearText();
    m_rootText.SetText(name ? name : (char*)"");
    m_rootText.Show();
    m_rootBtn.Show();
    HideChild();
}

void CControlBoxQuestAlarm::SetChildText(int idx, char* text)
{
    if (idx < 0 || idx >= 5) return;
    m_childTexts[idx].ClearText();
    m_childTexts[idx].SetText(text ? text : (char*)"");
}

int CControlBoxQuestAlarm::SetChildTextColor(int idx, float r, float g, float b, float a)
{
    if (idx < 0 || idx >= 5) return 0;

    auto to8 = [](float v)->int {
        if (v >= 1.0f) return 255;
        if (v <= 0.0f) return 0;
        return static_cast<int>(v * 255.0f + 0.5f);
        };

    int R = to8(r);
    int G = to8(g);
    int B = to8(b);
    int A = to8(a);

    // 反編譯位運算：v8 | ((v7 | ((v6 | (alpha<<8)) << 8)) << 8)
    // 對應 0xAARRGGBB
    int color = (A << 24) | (R << 16) | (G << 8) | (B & 0xFF);
    m_childTexts[idx].SetTextColor(color);
    return color;
}

void CControlBoxQuestAlarm::ClearTextData(int idx)
{
    if (idx < 0 || idx >= 5) return;
    m_childTexts[idx].ClearText();
    m_childTexts[idx].Hide();
    // 若有需要也可清空子按鈕文字（若你的 Button 有文字）
    // m_childBtns[idx].SetText("");
    // m_childBtns[idx].Hide();
}

void CControlBoxQuestAlarm::ClearTextData()
{
    // Root
    // 若你的 Button 也有文字可一併清空
    // m_rootBtn.SetText("");
    m_rootText.ClearText();

    // Children
    for (int i = 0; i < 5; ++i) {
        m_childTexts[i].ClearText();
        // m_childBtns[i].SetText(""); // 若有
    }
}

int CControlBoxQuestAlarm::GetMaxLength()
{
    auto textLen = [](CControlText& t)->int {
        const char* s = t.GetText();
        return s ? static_cast<int>(std::strlen(s)) : 0;
        };

    int maxLen = textLen(m_rootText);

    for (int i = 0; i < 5; ++i) {
        const char* s = m_childTexts[i].GetText();
        if (s && std::strcmp(s, "") != 0 /* 非空 */) {
            // 反編譯還檢查了一個可見/有效旗標；就語意而言：只計算非空字串
            maxLen = max(maxLen, textLen(m_childTexts[i]));
        }
    }
    return maxLen;
}

int CControlBoxQuestAlarm::GetMaxHeight()
{
    // 收起：20；展開：20 + 每個非空子項 15
    int h = 20;
    if (!m_expandedFlag) return h;

    for (int i = 0; i < 5; ++i) {
        const char* s = m_childTexts[i].GetText();
        if (s && std::strcmp(s, "") != 0)
            h += 15;
    }
    return h;
}

void CControlBoxQuestAlarm::OnClickedButtonRoot()
{
    if (m_expandedFlag) {
        // 由展開 → 收起
        m_expandedFlag = 0;
        for (int i = 0; i < 5; ++i) {
            m_childTexts[i].SetText("");
            m_childBtns[i].Hide();
            m_childTexts[i].Hide();
        }
        // Root 按鈕四態設為 23
        m_rootBtn.SetImage(335544327u, 23, 335544327u, 23, 335544327u, 23, 335544327u, 23);
    }
    else {
        // 由收起 → 展開
        m_expandedFlag = 1;
        for (int i = 0; i < 5; ++i) {
            const char* s = m_childTexts[i].GetText();
            if (s && std::strcmp(s, "") != 0) {
                m_childBtns[i].Show();
                m_childTexts[i].Show();
            }
        }
        // Root 按鈕四態設為 25
        m_rootBtn.SetImage(335544327u, 25, 335544327u, 25, 335544327u, 25, 335544327u, 25);
    }
}
