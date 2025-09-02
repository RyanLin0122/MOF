#include "UI/CControlEditBox.h"
#include <algorithm>
#include "global.h"

extern "C" unsigned long __stdcall timeGetTime(void); // 與你專案一致（已在 CControlImage.h 有用過） :contentReference[oaicite:9]{index=9}

using std::min;
using std::max;

// 一致色彩：-7590900 = 0xFF8C2C0C（ARGB）
static constexpr DWORD kTextColor = 0xFF8C2C0C;

// 半透明選取色（反編譯調色）
static constexpr float kSelR = 0.54901963f;
static constexpr float kSelG = 0.17254902f;
static constexpr float kSelB = 0.047058828f;
static constexpr float kSelA = 0.3137255f;

// -----------------------------------------------------------------------------
// 建構/解構
// -----------------------------------------------------------------------------
CControlEditBox::CControlEditBox()
{
    // 反編譯：CControlBase::CControlBase(this)
    // 其餘欄位已在宣告處初始化
}

CControlEditBox::~CControlEditBox()
{
    // 對齊反編譯：釋放 IME slot
    if (m_imeIndex != 0xFFFF) {
        g_IMMList.DeleteIMMInfo(m_imeIndex); // :contentReference[oaicite:10]{index=10}
        m_imeIndex = 0xFFFF;
    }

    // 子控制用各自解構
}

// -----------------------------------------------------------------------------
// 建立
// -----------------------------------------------------------------------------
void CControlEditBox::Create(CControlBase* pParent)
{
    CControlBase::Create(pParent);                     // 基底掛載
    m_focus = 0;
    CreateChildren();                                  // 建立子控制
}

void CControlEditBox::Create(int x, int y, int eboxWidth, CControlBase* pParent, int align)
{
    CControlBase::Create(x, y, pParent);
    SetEBoxSize(eboxWidth, 0, align);
    SetWritable(0, 0, 0, 1);
    m_password = 0;
}

void CControlEditBox::Create(int x, int y, int eboxWidth, uint16_t maxLen, int mode,
    CControlBase* pParent, int align, int multiLine, int /*reserved*/, uint8_t visibleLines)
{
    CControlBase::Create(x, y, pParent);
    SetEBoxSize(eboxWidth, multiLine ? max<int>(int(GetHeight()), 0) : 0, align);
    SetWritable(1, maxLen, mode, (uint8_t)visibleLines);
    m_password = 0;
}

// -----------------------------------------------------------------------------
// 子控制
// -----------------------------------------------------------------------------
void CControlEditBox::CreateChildren()
{
    // 背景九宮格
    m_Back.Create(this); // 對齊反編譯：(*((this+1208)->vf+12))(this+1208, this)  :contentReference[oaicite:11]{index=11}

    // 主要文字
    m_Text.Create(this);
    m_Text.SetTextColor(kTextColor);                   // *((this)+801) = -7590900  :contentReference[oaicite:12]{index=12}

    // 選取區 Alpha 盒
    for (auto& box : m_Block) {
        box.Create(this);
        box.SetColor(kSelR, kSelG, kSelB, kSelA);      // 反編譯：SetColor(..., 0.3137)  :contentReference[oaicite:13]{index=13}
        box.Hide();
    }

    // 游標（以字元 'I' 粗體顯示/隱藏達到閃爍效果）
    m_Caret.Create(this);
    m_Caret.SetFontWeight(700);
    m_Caret.SetPos(5, 5);
    m_Caret.SetText("I");
    m_Caret.SetTextColor(kTextColor);                  // *((this)+909) = -7590900  :contentReference[oaicite:14]{index=14}

    // 密碼遮罩文字（位置微調：相對主文字 +5,+3）
    m_Mask.Create(this);
    m_Mask.SetPos(m_Text.GetX() + 5, m_Text.GetY() + 3);
    m_Mask.SetTextColor(kTextColor);
}

// -----------------------------------------------------------------------------
// 外觀/尺寸
// -----------------------------------------------------------------------------
void CControlEditBox::SetEBoxSize(int width, int height, int align)
{
    // 反編譯：*((WORD*)this+16)=width，並呼叫背景的 SetSize
    SetSize((uint16_t)width, GetHeight());
    m_Back.SetSize((unsigned short)width, (unsigned short)height);   // :contentReference[oaicite:15]{index=15}

    // 文字基準 Y 固定 +5
    m_Text.SetY(5);

    // X 對齊
    if (align == 2) {            // 右
        m_Text.SetX((uint16_t)width - 5);
    }
    else if (align == 1) {     // 中
        m_Text.SetX((int)((double)(uint16_t)width * 0.5));
    }
    else {                     // 左
        m_Text.SetX(5);
    }
    m_align = align;

    // 多行高度/預設多行寬
    if (height) {
        m_prefHeight = (uint16_t)height;
        m_Text.SetMultiLineSize((uint16_t)width, height);            // :contentReference[oaicite:16]{index=16}
        m_Text.SetWantSpaceFirstByte(1);
    }
    else {
        // 若未指定，以背景高度為準
        m_prefHeight = m_Back.GetHeight();
    }
}

// -----------------------------------------------------------------------------
// 可輸入/IME
// -----------------------------------------------------------------------------
void CControlEditBox::SetWritable(int writable, uint16_t maxLen, int mode, uint8_t visibleLines)
{
    m_writable = writable;
    m_maxLen = maxLen;
    m_visibleLines = visibleLines;

    if (m_writable) {
        if (m_imeIndex == 0xFFFF)
            SetupIME(1, maxLen, mode, (uint8_t)(visibleLines > 1 ? 1 : 0));
        TextClear();
    }
}

void CControlEditBox::SetupIME(int enable, uint16_t maxLen, int mode, uint8_t multiLineFlag)
{
    if (!enable) return;

    int idx = g_IMMList.GetUsableIMEIndex();          // 反編譯：GetUsableIMEIndex  :contentReference[oaicite:17]{index=17}
    m_imeIndex = (idx < 0) ? 0xFFFF : (uint16_t)idx;
    if (m_imeIndex != 0xFFFF) {
        RECT rc{ 0,0,0,0 };
        // SetIMMInfo(index, a3=0, a4=maxLen, a5=mode, a6=&rc, a7=multiLine, a8=0(lineSpace), a9=17(align))
        g_IMMList.SetIMMInfo(m_imeIndex, 0, maxLen, mode, &rc, (char)multiLineFlag, 0, 17);   // :contentReference[oaicite:18]{index=18}
    }
}

void CControlEditBox::SetAlphaDigit(int flag)
{
    if (m_imeIndex != 0xFFFF)
        g_IMMList.SetAlphaDigit(m_imeIndex, flag);    // 對應 DCTIMM +98（是否密碼/數字） :contentReference[oaicite:19]{index=19}
}

// -----------------------------------------------------------------------------
// 顯示/焦點
// -----------------------------------------------------------------------------
void CControlEditBox::Show()
{
    CControlBase::Show();
    SetFocus(0);
}

void CControlEditBox::SetFocus(int on)
{
    m_focus = on;
    if (m_writable) {
        if (on) {
            m_Caret.Show();
            g_IMMList.SetActive(m_imeIndex, 1, g_hWnd);
            g_IMMList.ChangeLanguage(m_imeIndex, 1, g_hWnd); // 對齊反編譯「ChangeLanguage(1,hWnd)」 :contentReference[oaicite:20]{index=20}
        }
        else {
            m_Caret.Hide();
            g_IMMList.SetActive(m_imeIndex, 0, g_hWnd);
        }
    }
    else {
        m_Caret.Hide();
    }
}

// -----------------------------------------------------------------------------
// 文字
// -----------------------------------------------------------------------------
void CControlEditBox::SetText(int stringId)
{
    const char* s = g_DCTTextManager.GetText(stringId); // :contentReference[oaicite:21]{index=21}
    SetText(s ? s : "");
}

void CControlEditBox::SetText(const char* s)
{
    if (m_imeIndex != 0xFFFF)
        g_IMMList.SetIMMText(m_imeIndex, const_cast<char*>(s)); // 反編譯先丟給 IME buffer  :contentReference[oaicite:22]{index=22}
    m_Text.SetText(s);
}

void CControlEditBox::SetTextItoa(int v)
{
    char buf[256]{};
    _snprintf(buf, sizeof(buf), "%d", v);
    m_Text.SetText(buf);
}

void CControlEditBox::TextClear()
{
    if (m_writable && m_imeIndex != 0xFFFF)
        g_IMMList.SetIMMText(m_imeIndex, const_cast<char*>(""));     // 清 IME 文字 :contentReference[oaicite:23]{index=23}
    m_Text.ClearText();
    SetFocus(0);
    if (m_password)
        m_Mask.ClearText();
}

// -----------------------------------------------------------------------------
// 量測
// -----------------------------------------------------------------------------
uint16_t CControlEditBox::GetMaxTextSize() const
{
    return m_maxLen;
}

unsigned int CControlEditBox::GetCurTextSize() const
{
    const char* s = m_Text.GetText();
    return s ? (unsigned int)strlen(s) : 0;
}

BOOL CControlEditBox::IsMultiLine() const
{
    // 對齊語意：使用目前文字寬度換行數是否大於 1（單行=0/1）
    return (m_visibleLines > 1);
}

// -----------------------------------------------------------------------------
// 事件
// -----------------------------------------------------------------------------
int* CControlEditBox::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    if (m_writable) {
        if (msg == 3) // 反編譯：a2==3 時強制聚焦
            SetFocus(1);
        return CControlBase::ControlKeyInputProcess(msg, key, x, y, a6, a7);
    }
    return nullptr;
}

// -----------------------------------------------------------------------------
// PrepareDrawing（核心邏輯：閃爍、文字同步、遮罩、選取反白、游標位置）
// -----------------------------------------------------------------------------
void CControlEditBox::PrepareDrawing()
{
    if (!m_focus) {
        CControlBase::PrepareDrawing();
        return;
    }

    // 密碼模式：強制語系 0（對齊反編譯在密碼時呼叫 ChangeLanguage(0)）
    if (m_password)
        g_IMMList.ChangeLanguage(m_imeIndex, 0, g_hWnd);

    if (m_writable) {
        // 初始化閃爍時間
        if (!sBlinkInit) {
            sBlinkInit = true;
            sBlinkTick = timeGetTime();
        }

        // 每 ~300ms 切換 caret 顯示
        DWORD now = timeGetTime();
        if (now - sBlinkTick > 300) {
            sBlinkTick = now;
            if (m_Caret.IsVisible())
                m_Caret.Hide();
            else
                m_Caret.Show();
        }

        // 把 IME 文字拉回主文字
        char src[1024]{};           // Source
        char bak[1024]{};           // v32（備份原主文字）
        if (m_imeIndex != 0xFFFF)
            g_IMMList.GetIMMText(m_imeIndex, src, (int)m_maxLen);    // :contentReference[oaicite:24]{index=24}
        const char* cur = m_Text.GetText();
        if (cur) std::strncpy(bak, cur, sizeof(bak) - 1);
        m_Text.SetText(src);

        // 密碼遮罩
        if (m_password) {
            // 計算字元數（非位元組數）：DBCS lead byte 視為 1 字元
            int nChars = 0;
            for (int i = 0; src[i]; ++i) {
                if (IsDBCSLeadByte((BYTE)src[i]) && src[i + 1]) ++i;
                ++nChars;
            }

            std::string mask(nChars, '*');
            m_Mask.SetText(mask.c_str());
            m_Text.Hide();

            // caret X = 遮罩長度像素 + 2；Y 由 GetCaretPos 計（+5）
            int w = 0, h = 0;
            g_MoFFont.GetTextLength(&w, &h, "CharacterName", mask.c_str()); // :contentReference[oaicite:25]{index=25}
            int xy[2]{};
            GetCaretPos(xy, bak, mask.c_str(), m_imeIndex, 0);
            m_Caret.SetX(w + 2);
            m_Caret.SetY(xy[1] + 5);
        }
        else {
            // 非密碼：以實字串量測 caret
            int xy[2]{};
            GetCaretPos(xy, bak, src, m_imeIndex, 0);
            m_Caret.SetPos(xy[0] + 5, xy[1] + 5);
            m_Text.Show();
            m_Mask.Hide();
        }

        // 單行反白區塊
        if (m_writable && !IsMultiLine() && m_imeIndex != 0xFFFF) {
            unsigned int selLen = g_IMMList.GetSelectedBlockLen(m_imeIndex); // <=0 表示無選取 :contentReference[oaicite:26]{index=26}
            if ((int)selLen <= 0) {
                SetBlockShow(0, 0);
            }
            else {
                int start = g_IMMList.GetBlockStartPos(m_imeIndex);
                const char* full = m_Text.GetText();
                if (!full) full = "";

                // 計算選取前綴寬 / 選取段寬（以 "CharacterName" 字型）
                std::string prefix(full, full + min<int>(start, (int)strlen(full)));
                std::string mid(full + min<int>(start, (int)strlen(full)),
                    full + min<int>(start + (int)selLen, (int)strlen(full)));

                int w1 = 0, h1 = 0;
                int w2 = 0, h2 = 0;
                g_MoFFont.GetTextLength(&w1, &h1, "CharacterName", prefix.c_str()); // :contentReference[oaicite:27]{index=27}
                g_MoFFont.GetTextLength(&w2, &h2, "CharacterName", mid.c_str());

                // 反白盒（單段）
                SetBlockShow(1, 0);
                SetBlockBox(nullptr,
                    m_Text.GetAbsX() + w1,
                    m_Text.GetAbsY(),
                    (uint16_t)w2,
                    (uint16_t)m_Text.GetFontHeight(),
                    0);
                // 其餘隱藏
                for (int i = 1; i < 5; ++i) SetBlockShow(0, i);
            }
        }
    }

    CControlBase::PrepareDrawing();
}

// -----------------------------------------------------------------------------
// 滑鼠定位 -> caret index
// -----------------------------------------------------------------------------
int CControlEditBox::RenewMousePos(int* ptAbs, int a3 /*imeIndex*/)
{
    char src[1024]{};
    if (m_imeIndex != 0xFFFF) {
        g_IMMList.GetIMMText(m_imeIndex, src, (int)m_maxLen);
    }

    const size_t n = strlen(src);
    if (n == 0) return -1;

    // 從 1 開始遞增地量測切片，直到點擊落在該切片範圍
    size_t curCount = 1;
    int    prevX = 0, prevY = 5;

    while (curCount <= n) {
        char seg[1024]{};
        std::strncpy(seg, src, curCount);
        int xy[2]{};
        GetCaretPos(xy, seg, seg, a3, curCount);
        const int segL = prevX + 5;
        const int segT = prevY + 5;
        const int segR = xy[0] + 5;
        const int segB = xy[1] + m_Text.GetFontHeight() + 5;

        // 對齊反編譯的 SearchTextPos 決策
        uint32_t* thisAlias = reinterpret_cast<uint32_t*>(this); // 僅為對齊呼叫樣式
        if (SearchTextPos(thisAlias, &curCount, reinterpret_cast<uint32_t*>(ptAbs),
            prevX, prevY, xy[0], xy[1], src)) {
            return (int)curCount;
        }

        prevX = xy[0];
        prevY = xy[1];
        ++curCount;
    }

    return -1;
}

// -----------------------------------------------------------------------------
// 內部輔助
// -----------------------------------------------------------------------------
void CControlEditBox::DeleteBlockBox()
{
    for (auto& box : m_Block) box.Hide();
}

void CControlEditBox::SetBlockShow(int on, int idx)
{
    if (idx < 0 || idx >= 5) return;
    if (on) m_Block[idx].Show(); else m_Block[idx].Hide();
}

void CControlEditBox::SetBlockBox(CControlBase* /*unused*/, int x, int y, uint16_t w, uint16_t h, int which)
{
    if (which < 0 || which >= 5) return;
    auto& box = m_Block[which];
    box.SetAttr(x, y, w, h, kSelR, kSelG, kSelB, 1.0f);          // 外框色
    box.SetColor(kSelR, kSelG, kSelB, kSelA);                    // 內部透明度 0.3137  :contentReference[oaicite:28]{index=28}
}

void CControlEditBox::DiscriminStairBlock(stBlockStair* stair)
{
    // 反編譯為多行選取拆段；此處僅保留接口（單行不使用）。
    // 若需實作多行選取，可參考：m_Text.GetCharByteByLine(..., m_lineBreakBytes, 10) 與 g_IMMList 的 BlockStart/Len。
    (void)stair;
}

// caret 位置量測（對齊反編譯參數順序與計算路徑）
void CControlEditBox::GetCaretPos(int outXY[2], const char* a3, const char* Source, int imeIndex, size_t Count)
{
    // a3 = 未遮罩原字串（若有）；Source = 顯示字串（可能是遮罩）
    // Count==0 表示使用 IME 內部 caret 位置
    const int isComposition = (imeIndex != 0xFFFF) ? g_IMMList.IsComposition(imeIndex) : 0; // :contentReference[oaicite:29]{index=29}
    const int fontH = m_Text.GetFontHeight();                                              // :contentReference[oaicite:30]{index=30}

    size_t caretPos = Count ? Count : (size_t)g_IMMList.GetEditPosition(imeIndex);         // :contentReference[oaicite:31]{index=31}
    const size_t srcLen = strlen(Source);

    // 多行處理（對齊語意）：依 m_visibleLines 與行寬切分
    int baseY = 0;
    int baseX = 0;

    if (IsMultiLine()) {
        // 使用 CControlText 的多行行距資訊來估計 Y
        const int multiSpace = m_Text.GetMultiLineSpace();                                  // :contentReference[oaicite:32]{index=32}

        // 以行分割資訊找 caret 所在行
        int lines = m_Text.GetCharByteByLine(m_lineBreakBytes, 10);                         // :contentReference[oaicite:33]{index=33}
        int lineIdx = 0;
        int acc = 0;
        for (int i = 0; i < lines; ++i) {
            int br = (i == 0) ? m_lineBreakBytes[0] : (m_lineBreakBytes[i] - m_lineBreakBytes[i - 1]);
            if ((int)caretPos <= (acc + br)) { lineIdx = i; break; }
            acc += br;
        }
        baseY = lineIdx * (fontH + multiSpace);
        baseX = 0;

        // 取該行起點到 caret 的子串
        int lineStart = (lineIdx == 0) ? 0 : m_lineBreakBytes[lineIdx - 1];
        int subLen = (int)caretPos - lineStart;
        int min = min(subLen, (int)srcLen);
        subLen = (0 > min) ? 0 : min;

        std::string sub(Source + lineStart, Source + lineStart + subLen);
        int w = 0, h = 0;
        g_MoFFont.GetTextLength(&w, &h, "CharacterName", sub.c_str());                      // :contentReference[oaicite:34]{index=34}
        // 若組字且 caret 現在在 DBCS lead 上，加一個字寬（以字高近似）
        if (isComposition && caretPos < srcLen && IsDBCSLeadByte((BYTE)Source[caretPos]))
            w += fontH;

        outXY[0] = w;
        outXY[1] = baseY;
    }
    else {
        // 單行：直接量測 [0..caretPos) 的寬度
        std::string left(Source, Source + min(srcLen, caretPos));
        int w = 0, h = 0;
        g_MoFFont.GetTextLength(&w, &h, "CharacterName", left.c_str());                     // :contentReference[oaicite:35]{index=35}
        if (isComposition && caretPos < srcLen && IsDBCSLeadByte((BYTE)Source[caretPos]))
            w += fontH;

        outXY[0] = w;
        outXY[1] = 0;
    }

    m_caretIndex = (int)caretPos;
}

BOOL CControlEditBox::SearchTextPos(uint32_t* pThisAlias, size_t* curCount, uint32_t* ptAbs,
    int segL, int segT, int segR, int segB, const char* fullText)
{
    // 對齊反編譯的決策（簡化版）：
    // 落在垂直區段內，靠近左/右邊界者決定 caret 在前/後。
    const size_t fullLen = strlen(fullText);
    const bool atLineBreak = (*curCount == fullLen) || (fullText[*curCount] == '\n');

    const int px = (int)ptAbs[0];
    const int py = (int)ptAbs[1];
    const int fontH = m_Text.GetFontHeight();

    // 垂直測試
    if (py < segT || py >(segT + fontH)) {
        return FALSE;
    }

    // 水平測試
    if (!atLineBreak) {
        if (px < segL || px > segR) return FALSE;
    }
    else {
        if (px < segL) return FALSE;
    }

    // 判斷靠左或靠右
    const int dL = std::abs(px - segL);
    const int dR = std::abs(px - segR);
    if (dL >= dR) {
        m_caretX = segR; m_caretY = segB;
    }
    else {
        m_caretX = segL; m_caretY = segT;
        if (*curCount > 0) --(*curCount);
    }
    return TRUE;
}

char* CControlEditBox::GetText()
{
    return (char *)m_Text.GetText();
}