#include "UI/CControlText.h"
#include <cstdio>
#include <algorithm>

// 將 64 位整數轉換為帶千分位逗號的金額字串
void Numeric2MoneyByComma(long long Value, char* outputBuffer, int bufferSize, int showPositiveSign)
{
    int maxSize = bufferSize;
    char tempBuffer[1024];

    // 清空輸出緩衝區和臨時緩衝區
    memset(outputBuffer, 0, bufferSize);
    memset(tempBuffer, 0, sizeof(tempBuffer));

    // 將數字轉換為字串（10進制）
    _i64toa(Value, tempBuffer, 10);

    // 如果需要顯示正號且數值為正數，則在前面加上 '+'
    if (showPositiveSign == 1 && Value > 0) {
        memmove(&tempBuffer[1], tempBuffer, strlen(tempBuffer));
        tempBuffer[0] = '+';
    }

    // 檢查字串長度是否小於緩衝區大小
    if ((int)strlen(tempBuffer) < bufferSize) {
        strcpy(outputBuffer, tempBuffer);

        // 從右邊數來第3位開始插入逗號
        int insertPos = strlen(outputBuffer) - 3;

        if (insertPos >= 1) {
            while ((int)(strlen(outputBuffer) + 1) < maxSize &&
                (insertPos != 1 || (outputBuffer[0] != '-' && outputBuffer[0] != '+'))) {

                char* insertPoint = &outputBuffer[insertPos];

                // 向右移動字串內容，為逗號騰出空間
                memmove(&outputBuffer[insertPos + 1], &outputBuffer[insertPos],
                    strlen(&outputBuffer[insertPos]));

                // 插入逗號
                *insertPoint = ',';

                // 移動到下一個插入位置（向左3位）
                insertPos -= 3;

                if (insertPos < 1) {
                    break;
                }

                maxSize = bufferSize;
            }
        }
    }
}

// ============================ 建構 / 解構 ============================
CControlText::CControlText()
{
    // 依反編譯預設：行數快取 = 1、需要重算 = 1、字高=12、粗細=400、行距=5、主色 -10483191
    // 字型名預設為空，第一次 SetText 時若為空，會以 DCTTextManager(3264) 補上。
}

CControlText::~CControlText()
{
    // std::string 自動釋放即可；其餘無擁有資源
}

// =============================== 建立 ================================
void CControlText::Create(CControlBase* pParent)
{
    CControlBase::Create(pParent);
}
void CControlText::Create(int x, int y, CControlBase* pParent)
{
    CControlBase::Create(x, y, pParent);
}
void CControlText::Create(int x, int y, uint16_t w, uint16_t h, CControlBase* pParent)
{
    CControlBase::Create(x, y, w, h, pParent);
}

// ============================== 文字存取 =============================
void CControlText::ClearData()
{
    // 先讓基底遞迴
    CControlBase::ClearData();

    // 清本物件文字：模擬反編譯中 string 清理的效果
    m_text.clear();
    m_isStringData = false;
    MarkLayoutDirty();
}

void CControlText::SetText(const char* s)
{
    if (!s)
    {
        // 反編譯：若傳入 null，就把字串設成空字串（利用內部空字串）
        m_text.clear();
        m_isStringData = false;
        MarkLayoutDirty();
        return;
    }

    // 若字型名尚未設，依反編譯：用 DCTTextManager id=3264 來填預設字型名
    if (m_fontFace.empty())
    {
        //const char* defFace = g_DCTTextManager.GetText(3264);
        //if (defFace && *defFace) m_fontFace = defFace;
    }

    if (m_text != s)
    {
        m_text = s;
        m_isStringData = !m_text.empty();
        m_needRecalcLineCount = 1; // [43] = 1
    }
}

void CControlText::SetText(int stringId)
{
    //if (stringId >= 0)
        //SetText(g_DCTTextManager.GetText(stringId));
}

void CControlText::SetParsedText(int stringId)
{
    //if (stringId >= 0)
        //SetText(g_DCTTextManager.GetParsedText(stringId, 0, 0));
}

void CControlText::ClearText()
{
    m_text.clear();
    m_isStringData = false;
    MarkLayoutDirty();
}

void CControlText::SetTextItoa(int value)
{
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%d", value);
    SetText(buf);
}

void CControlText::SetTextMoney(unsigned int value)
{
    char buf[256];
    Numeric2MoneyByComma(value, buf, 256, 0);
    SetText(buf);
}

void CControlText::SetTextMoney(int fmtId, unsigned int value)
{
    char money[256];
    char fmt[256];
    Numeric2MoneyByComma(value, money, 256, 0);
    const char* f = NULL;// g_DCTTextManager.GetText(fmtId);
    std::snprintf(fmt, sizeof(fmt), "%s", f ? f : "%s");
    char outBuf[256];
    std::snprintf(outBuf, sizeof(outBuf), fmt, money);
    SetText(outBuf);
}

void CControlText::SetParsedTextMoney(int fmtId, unsigned int value)
{
    char money[256];
    char fmt[256];
    Numeric2MoneyByComma(value, money, 256, 0);
    const char* f = NULL; // g_DCTTextManager.GetParsedText(fmtId, 0, 0);
    std::snprintf(fmt, sizeof(fmt), "%s", f ? f : "%s");
    char outBuf[256];
    std::snprintf(outBuf, sizeof(outBuf), fmt, money);
    SetText(outBuf);
}

// ============================ 字型 / 版面 ===========================
void CControlText::SetFontHeight(int h)
{
    m_fontHeight = h;
    MarkLayoutDirty();
}
void CControlText::SetFontWeight(int w)
{
    m_fontWeight = w;
    MarkLayoutDirty();
}

void CControlText::SetControlSetFont(const char* fontName)
{
    stFontInfo* fi = NULL; // g_MoFFont.GetFontInfo(fontName);
    if (fi)
    {
        // 反編譯：*((_DWORD*)fi + 64) 與 +65；字串位於 fi + 128
        // 這裡假設 stFontInfo 提供能取得高度/粗細/字型名的介面（或等價欄位）
        // 若你們的 stFontInfo 不同，請在此替換為對應 getter。
        // 為清楚起見，以下使用「假設的」擷取方法（請替換）：
        //extern int      FontInfo_GetHeight(stFontInfo*); // 由你們工程提供
        //extern int      FontInfo_GetWeight(stFontInfo*);
        //extern const char* FontInfo_GetFace(stFontInfo*);

        //m_fontHeight = FontInfo_GetHeight(fi);
        //m_fontWeight = FontInfo_GetWeight(fi);
        //m_fontFace = FontInfo_GetFace(fi) ? FontInfo_GetFace(fi) : "";

        MarkLayoutDirty();
    }
}

void CControlText::SetMultiLineSpace(int px)
{
    m_lineSpace = px;
    MarkLayoutDirty();
}
void CControlText::SetMultiLineSize(uint16_t w, int h)
{
    m_multiWidth = w;
    // 反編譯存進 WORD；這裡保留 word 範圍
    m_multiCellH = static_cast<uint16_t>(std::max(0, std::min(h, 0xFFFF)));
    MarkLayoutDirty();
}

// ============================== 繪製 ================================
void CControlText::Draw()
{
    if (!IsVisible() || !m_isStringData) return;

    const char* s = m_text.c_str();
    const int   x = GetAbsX();
    const int   y = GetAbsY();

    // 設定字型（對齊反編譯）
    //g_MoFFont.SetFont(m_fontHeight, m_fontFace.c_str(), m_fontWeight);

    const uint16_t boxW = GetWidth();
    const uint16_t boxH = GetHeight();

    if (boxW != 0 && boxH != 0)
    {
        // 文字盒子模式（多行/換行）
        if (m_colorEdge)  // [38]：先畫一個 (x+1,y+1) 的邊色
        {
            RectI__ rc; SetRect__(&rc, x + 1, y + 1, x + boxW, y + boxH);
            //g_MoFFont.SetTextBoxA(&rc, m_colorEdge, s, m_lineSpace, m_drawFlag, m_wantSpaceFirstByte);
        }
        else if (m_colorShadow) // [39]：畫 8 向偏移的陰影框，再畫主色
        {
            RectI__ rc;

            // 左  (-1, 0)
            SetRect__(&rc, x - 1, y, x + boxW - 1, y + boxH);
            //g_MoFFont.SetTextBoxA(&rc, m_colorShadow, s, m_lineSpace, m_drawFlag, m_wantSpaceFirstByte);

            // 左上 (-1,-1)
            SetRect__(&rc, x - 1, y - 1, x + boxW - 1, y + boxH - 1);
            //g_MoFFont.SetTextBoxA(&rc, m_colorShadow, s, m_lineSpace, m_drawFlag, m_wantSpaceFirstByte);

            // 上   (0,-1)
            SetRect__(&rc, x, y - 1, x + boxW, y + boxH - 1);
            //g_MoFFont.SetTextBoxA(&rc, m_colorShadow, s, m_lineSpace, m_drawFlag, m_wantSpaceFirstByte);

            // 右上 (+1,-1)
            SetRect__(&rc, x + 1, y - 1, x + boxW + 1, y + boxH - 1);
            //g_MoFFont.SetTextBoxA(&rc, m_colorShadow, s, m_lineSpace, m_drawFlag, m_wantSpaceFirstByte);

            // 右  (+1, 0)
            SetRect__(&rc, x + 1, y, x + boxW + 1, y + boxH);
            //g_MoFFont.SetTextBoxA(&rc, m_colorShadow, s, m_lineSpace, m_drawFlag, m_wantSpaceFirstByte);

            // 右下(+1,+1)
            SetRect__(&rc, x + 1, y + 1, x + boxW + 1, y + boxH + 1);
            //g_MoFFont.SetTextBoxA(&rc, m_colorShadow, s, m_lineSpace, m_drawFlag, m_wantSpaceFirstByte);

            // 下   (0,+1)
            SetRect__(&rc, x, y + 1, x + boxW, y + boxH + 1);
            //g_MoFFont.SetTextBoxA(&rc, m_colorShadow, s, m_lineSpace, m_drawFlag, m_wantSpaceFirstByte);

            // 左下 (-1,+1)
            SetRect__(&rc, x - 1, y + 1, x + boxW - 1, y + boxH + 1);
            //g_MoFFont.SetTextBoxA(&rc, m_colorShadow, s, m_lineSpace, m_drawFlag, m_wantSpaceFirstByte);
        }

        // 主色盒子（一定會畫）
        {
            RectI__ rc; SetRect__(&rc, x, y, x + boxW, y + boxH);
            //g_MoFFont.SetTextBoxA(&rc, m_colorMain, s, m_lineSpace, m_drawFlag, m_wantSpaceFirstByte);
        }
    }
    else
    {
        // 單行模式
        if (m_colorEdge)
        {
            //g_MoFFont.SetTextLineA(x + 1, y + 1, m_colorEdge, s, m_drawFlag, -1, -1);
        }
        else if (m_colorShadow)
        {
            //g_MoFFont.SetTextLineShadow(x, y, m_colorShadow, s, m_drawFlag);
        }

        //g_MoFFont.SetTextLineA(x, y, m_colorMain, s, m_drawFlag, -1, -1);
    }

    // 子物件照常繪製
    CControlBase::Draw();
}

// ============================== 命中 ================================
// 追加或替換：CControlText::PtInCtrl
bool CControlText::PtInCtrl(stPoint pt)
{
    // 1) 與反編譯一致的前置檢查
    if (!m_bIsVisible)           return false;  // 等同 *((_DWORD*)this + 12)
    if (!IsStringData())         return false;  // 等同 *((_DWORD*)this + 32)

    // 2) 取得文字實際尺寸（你已有 GetTextLength 介面）
    int size[2] = { 0.0f, 0.0f };
    GetTextLength(size[0], size[1]);                 // size[0] = width, size[1] = height
    const int w = static_cast<int>(size[0]);
    const int h = static_cast<int>(size[1]);
    if (w == 0 || h == 0)        return false;

    // 3) 取得絕對位置
    int ax = 0, ay = 0;
    GetAbsPos(ax, ay);                    // 對齊反編譯 CControlBase::GetAbsPos

    // 4) 命中測試（右/下為開區間，等同 Win32 PtInRect）
    const int left = ax;
    const int top = ay;
    const int right = ax + w;
    const int bottom = ay + h;

    return (pt.x >= left) && (pt.x < right) &&
        (pt.y >= top) && (pt.y < bottom);
}

// ============================== 量測/資訊 ============================
void CControlText::GetTextLength(int& outW, int& outH)
{
    if (!m_isStringData) { outW = outH = 0; return; }
    int sizeArr[2] = { 0, 0 };
    //g_MoFFont.GetTextLength(sizeArr, m_fontFace.c_str(), m_text.c_str(), m_fontHeight, m_fontWeight);
    outW = sizeArr[0];
    outH = sizeArr[1];
}

int CControlText::GetCalcedTextBoxHeight(uint16_t width)
{
    if (!m_isStringData) return 0;
    uint16_t w = (width != 0) ? width : m_multiWidth;
    const uint8_t lines = GetMultiTextLineCount(w);
    return (m_fontHeight + m_lineSpace) * static_cast<int>(lines);
}

int CControlText::GetCharByteByLine(uint8_t* outBytesPerLine, int maxLines)
{
    if (!m_isStringData) return 0;
    return 0; // g_MoFFont.GetCharByteByLine(static_cast<uint16_t>(GetFontHeight()),
        //m_text.c_str(), outBytesPerLine, maxLines);
}

uint8_t CControlText::GetMultiTextLineCount(uint16_t width)
{
    if (!m_isStringData) return 0;

    if (!m_needRecalcLineCount)
        return m_cachedLineCount;

    uint16_t w = (width != 0) ? width : m_multiWidth;
    if (w == 0) w = static_cast<uint16_t>(GetWidth());

    uint8_t cnt = 0; // g_MoFFont.GetLineCountByWidth(w, m_text.c_str(),
        //m_fontFace.c_str(), m_fontHeight, m_fontWeight);
    m_cachedLineCount = cnt;
    m_needRecalcLineCount = 0;
    return cnt;
}

// =========================== 置中（Y-5） ============================
void CControlText::SetTextPosToParentCenter()
{
    SetCenterPos();                // 置中
    SetY(GetY() - 5);              // 反編譯：*((DWORD*)this + 7) = v2 - 5
    m_centeredYMinus5 = true;      // 僅作紀錄
}

// ============================ 內部工具 ==============================
void CControlText::MarkLayoutDirty()
{
    m_needRecalcLineCount = 1;
}
