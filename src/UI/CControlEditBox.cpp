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
    // 對齊反編譯：建構子先呼叫 InitOpen 再 CreateChildren
    InitOpen();
    CreateChildren();
}

CControlEditBox::~CControlEditBox()
{
    // 對齊反編譯：釋放 IME slot（不設回 0xFFFF）
    if (m_imeIndex != 0xFFFF) {
        g_IMMList.DeleteIMMInfo(m_imeIndex);
    }

    // 子控制用各自解構
}

// -----------------------------------------------------------------------------
// InitOpen — 對齊反編譯：建構子中呼叫，在 CreateChildren 之前
// （原始函式體在反編譯中僅為 idb 宣告，無完整函式體）
// -----------------------------------------------------------------------------
void CControlEditBox::InitOpen()
{
    // 反編譯中此函式體未被 IDA 完整還原（標記為 idb），
    // 保留空實作以維持呼叫流程一致。
}

// -----------------------------------------------------------------------------
// 建立
// -----------------------------------------------------------------------------
void CControlEditBox::Create(CControlBase* pParent)
{
    CControlBase::Create(pParent);                     // 基底掛載
    m_password = 0;                                    // 對齊反編譯：*((_DWORD *)this + 41) = 0
}

void CControlEditBox::Create(int x, int y, int eboxWidth, CControlBase* pParent, int align)
{
    CControlBase::Create(x, y, pParent);
    SetEBoxSize(eboxWidth, 0, align);
    SetWritable(0, 0, 0, 1);
    m_password = 0;
}

void CControlEditBox::Create(int x, int y, int eboxWidth, uint16_t maxLen, int mode,
    CControlBase* pParent, int writable, int align, int height, uint8_t visibleLines)
{
    CControlBase::Create(x, y, pParent);
    SetEBoxSize(eboxWidth, height, align);             // 對齊反編譯：SetEBoxSize(a4, a10, a9)
    SetWritable(writable, maxLen, mode, visibleLines); // 對齊反編譯：SetWritable(a8, a5, a6, a11)
    m_password = 0;                                    // 對齊反編譯：*((_DWORD *)this + 41) = 0
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

    // 選取區 Alpha 盒（反編譯：建立並設色，但 CreateChildren 中不 Hide）
    for (auto& box : m_Block) {
        box.Create(this);
        box.SetColor(kSelR, kSelG, kSelB, kSelA);      // 反編譯：SetColor(..., 0.3137)  :contentReference[oaicite:13]{index=13}
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

    // 對齊反編譯：*((_DWORD *)this + 13) = 1;
    // CControlBase 區域的旗標，標記子控制已建立完成
    reinterpret_cast<int*>(this)[13] = 1;
}

// -----------------------------------------------------------------------------
// 外觀/尺寸
// -----------------------------------------------------------------------------
void CControlEditBox::SetEBoxSize(int width, int height, int align)
{
    // 對齊反編譯：*((_WORD *)this + 16) = a2 (m_usWidth)
    m_usWidth = static_cast<uint16_t>(width);
    m_Back.SetSize(static_cast<unsigned short>(width), static_cast<unsigned short>(height));

    // 文字基準 Y 固定 +5
    m_Text.SetY(5);

    // X 對齊
    if (align == 0) {            // 左
        m_Text.SetX(5);
    }
    else if (align == 2) {       // 右
        m_Text.SetX(static_cast<uint16_t>(width) - 5);
    }
    else if (align == 1) {       // 中
        m_Text.SetX(static_cast<int>(static_cast<double>(static_cast<uint16_t>(width)) * 0.5));
    }
    m_align = align;

    // 多行高度/預設多行寬
    if (static_cast<uint16_t>(height)) {
        m_usHeight = static_cast<uint16_t>(height);
        m_Text.SetMultiLineSize(static_cast<uint16_t>(width), height);
        m_Text.SetWantSpaceFirstByte(1);
    }
    else {
        // 對齊反編譯：若未指定，以背景高度為準
        m_usHeight = m_Back.GetHeight();
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
            SetupIME(writable, maxLen, mode, visibleLines); // 對齊反編譯：直接傳入原始參數
        TextClear();
    }
}

void CControlEditBox::SetupIME(int enable, uint16_t maxLen, int mode, uint8_t multiLineFlag)
{
    if (!enable) return;

    // 對齊反編譯：直接賦值 unsigned __int16 回傳值，無負值檢查
    m_imeIndex = static_cast<uint16_t>(g_IMMList.GetUsableIMEIndex());
    if (m_imeIndex != 0xFFFF) {
        RECT rc{ 0,0,0,0 };
        // SetIMMInfo(index, a3=0, a4=maxLen, a5=mode, a6=&rc, a7=multiLine, a8=0(lineSpace), a9=17(align))
        g_IMMList.SetIMMInfo(m_imeIndex, 0, maxLen, mode, &rc, (char)multiLineFlag, 0, 17);   // :contentReference[oaicite:18]{index=18}
    }
}

void CControlEditBox::SetAlphaDigit(int flag)
{
    // 對齊反編譯：無條件呼叫（不檢查 m_imeIndex）
    g_IMMList.SetAlphaDigit(m_imeIndex, flag);
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
            g_IMMList.ChangeLanguage(1, g_hWnd); // 對齊反編譯：2-param overload
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
    // 對齊反編譯：無 null guard，直接傳入 GetText 結果
    const char* s = g_DCTTextManager.GetText(stringId);
    SetText(s);
}

void CControlEditBox::SetText(const char* s)
{
    // 對齊反編譯：無條件呼叫（不檢查 m_imeIndex）
    g_IMMList.SetIMMText(m_imeIndex, const_cast<char*>(s));
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
    // 對齊反編譯：只檢查 m_writable，不檢查 m_imeIndex
    if (m_writable)
        g_IMMList.SetIMMText(m_imeIndex, const_cast<char*>(""));
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
    // 對齊反編譯：無 null guard
    return (unsigned int)strlen(m_Text.GetText());
}

BOOL CControlEditBox::IsMultiLine() const
{
    // 對齊反編譯：比較 EditBox 本身高度與背景高度，不同即為多行
    unsigned short backH = const_cast<CEditBoxBackgroundImage&>(m_Back).GetHeight();
    return const_cast<CControlEditBox*>(this)->GetHeight() != backH;
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

    // 密碼模式：強制語系 0（對齊反編譯：ChangeLanguage(0, hWnd)）
    if (m_password)
        g_IMMList.ChangeLanguage(0, g_hWnd);

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
        // 對齊反編譯：無條件呼叫（不檢查 m_imeIndex）
        g_IMMList.GetIMMText(m_imeIndex, src, (int)m_maxLen);
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

            // 對齊反編譯：strcpy(v32, Destination) → 用遮罩覆寫備份
            std::strncpy(bak, mask.c_str(), sizeof(bak) - 1);

            // caret X = 遮罩長度像素 + 2；Y 由 GetCaretPos 計（+5）
            int w = 0, h = 0;
            g_MoFFont.GetTextLength(&w, &h, "CharacterName", mask.c_str());
            int xy[2]{};
            GetCaretPos(xy, bak, mask.c_str(), m_imeIndex, 0);
            m_Caret.SetX(w + 2);
            m_Caret.SetY(xy[1] + 5);
        }
        else {
            // 非密碼：以實字串量測 caret（對齊反編譯：不呼叫 Show/Hide）
            int xy[2]{};
            GetCaretPos(xy, bak, src, m_imeIndex, 0);
            m_Caret.SetPos(xy[0] + 5, xy[1] + 5);
        }

        // 對齊反編譯：反白區塊（含 DiscriminStairBlock）
        if (m_writable && !IsMultiLine() && m_imeIndex != 0xFFFF) {
            if ((int)g_IMMList.GetSelectedBlockLen(m_imeIndex) <= 0) {
                SetBlockShow(0, 0);
            }
            else {
                int v25 = g_IMMList.GetBlockStartPos(m_imeIndex);
                int absX = m_Text.GetAbsX();
                int absY = m_Text.GetAbsY();

                // 對齊反編譯：零化內部欄位 this[36], this[37], WORD this[76]
                reinterpret_cast<int*>(this)[36] = 0;
                reinterpret_cast<int*>(this)[37] = 0;
                reinterpret_cast<short*>(this)[76] = 0;

                int v9 = m_Text.GetCharByteByLine(m_lineBreakBytes, 10);
                stBlockStair stair{};
                DiscriminStairBlock(&stair);

                if (v9 > 0) {
                    size_t* v10 = stair.len;
                    int remaining = v9;
                    const char* Source = m_Text.GetText();
                    if (!Source) Source = "";
                    uint16_t fontH = static_cast<uint16_t>(m_Text.GetFontHeight());
                    do {
                        if (*v10) {
                            char Dest[1024]{};
                            std::strncpy(Dest, Source, v10[1]);
                            int w1 = 0, h1 = 0;
                            g_MoFFont.GetTextLength(&w1, &h1, "CharacterName", Dest);

                            std::memset(Dest, 0, sizeof(Dest));
                            std::strncpy(Dest, &Source[v25], *v10);
                            int w2 = 0, h2 = 0;
                            g_MoFFont.GetTextLength(&w2, &h2, "CharacterName", Dest);

                            SetBlockShow(1, 0);
                            SetBlockBox(nullptr, absX + w1, absY,
                                static_cast<uint16_t>(w2), fontH, 0);
                        }
                        v10 += 2;
                    } while (--remaining);
                }
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
    // 對齊反編譯：無條件呼叫（不檢查 m_imeIndex）
    g_IMMList.GetIMMText(m_imeIndex, src, (int)m_maxLen);

    const size_t n = strlen(src);
    if (n == 0) return 0;  // 對齊反編譯：回傳 0，不是 -1

    // 從 1 開始遞增地量測切片，直到點擊落在該切片範圍
    size_t curCount = 1;
    int    prevX = 0, prevY = 5;

    while (curCount <= n) {
        char seg[1024]{};
        std::strncpy(seg, src, curCount);
        int xy[2]{};
        GetCaretPos(xy, seg, seg, a3, curCount);
        // 對齊反編譯：v9 = *ptr + 5, v10 = *(ptr+1) + 5
        int v9 = xy[0] + 5;
        int v10 = xy[1] + 5;

        // 對齊反編譯的 SearchTextPos 決策
        uint32_t* thisAlias = reinterpret_cast<uint32_t*>(this);
        if (SearchTextPos(thisAlias, &curCount, reinterpret_cast<uint32_t*>(ptAbs),
            prevX, prevY, v9, v10, src)) {
            return (int)curCount;
        }

        prevX = v9;   // 對齊反編譯：v7 = v9（已 +5）
        prevY = v10;  // 對齊反編譯：v14 = v10（已 +5）
        ++curCount;
    }

    return -1;
}

// -----------------------------------------------------------------------------
// 內部輔助
// -----------------------------------------------------------------------------
void CControlEditBox::DeleteBlockBox()
{
    // 對齊反編譯：呼叫 vtable+36 (ClearData)，非 Hide
    for (auto& box : m_Block) box.ClearData();
}

void CControlEditBox::SetBlockShow(int on, int idx)
{
    // 對齊反編譯：ground truth 檢查 a2 == 1，不是 truthy
    if (on == 1) m_Block[idx].Show(); else m_Block[idx].Hide();
}

void CControlEditBox::SetBlockBox(CControlBase* /*unused*/, int x, int y, uint16_t w, uint16_t h, int which)
{
    // 對齊反編譯：無 bounds check
    auto& box = m_Block[which];
    box.SetAttr(x, y, w, h, kSelR, kSelG, kSelB, 1.0f);          // 外框色
    box.SetColor(kSelR, kSelG, kSelB, kSelA);                    // 內部透明度 0.3137  :contentReference[oaicite:28]{index=28}
}

void CControlEditBox::DiscriminStairBlock(stBlockStair* a2)
{
    // 對齊反編譯 0041E750：完整多行階梯選取拆段
    int v3 = -1;              // endLine
    int v14 = -1;             // startLine
    int v12 = -1;             // endLine copy
    int v4 = m_Text.GetCharByteByLine(m_lineBreakBytes, 10);
    int v16 = v4;
    unsigned int v10 = 0;     // endLen
    int v17 = 0;              // startOffset

    int v5 = v4 - 1;
    if (v5 >= 0)
    {
        unsigned char* v6 = &m_lineBreakBytes[v5];
        int v15 = v5 + 1;
        do
        {
            int blockStart = g_IMMList.GetBlockStartPos(m_imeIndex);
            if (*v6 <= static_cast<unsigned char>(blockStart) && v14 < 0)
            {
                v14 = static_cast<int>(v6 - m_lineBreakBytes);
                v17 = blockStart - *v6;
            }
            int v8 = g_IMMList.GetBlockStartPos(m_imeIndex);
            int blockLen = g_IMMList.GetSelectedBlockLen(m_imeIndex);
            if (static_cast<int>(*v6) > (blockLen + v8) || v12 >= 0)
            {
                // v10 unchanged
            }
            else
            {
                int v9 = g_IMMList.GetBlockStartPos(m_imeIndex) - *v6;
                v10 = g_IMMList.GetSelectedBlockLen(m_imeIndex) + v9;
                v12 = static_cast<int>(v6 - m_lineBreakBytes);
            }
            --v6;
            --v15;
        } while (v15);
        v4 = v16;
        v3 = v12;
    }

    int v11 = 0;
    if (v4 <= 0)
        return;

    for (;;)
    {
        if (v11 < v14)
        {
            reinterpret_cast<int*>(a2)[2 * v11 + 1] = 0;
            reinterpret_cast<int*>(a2)[2 * v11] = 0;
        }
        if (v11 != v14)
            break;
        reinterpret_cast<int*>(a2)[2 * v11 + 1] = v17;
        if (v11 == v3)
            goto LABEL_25;
        if (v11)
        {
            v3 = v12;
            reinterpret_cast<int*>(a2)[2 * v11] =
                static_cast<int>(m_lineBreakBytes[v11]) - static_cast<int>(m_lineBreakBytes[v11 - 1]);
        }
        else
        {
            reinterpret_cast<int*>(a2)[0] = static_cast<int>(m_lineBreakBytes[0]);
        }
    LABEL_22:
        if (++v11 >= v4)
            return;
    }
    if (v11 != v3)
        goto LABEL_22;
    reinterpret_cast<int*>(a2)[2 * v11 + 1] = 0;
LABEL_25:
    reinterpret_cast<int*>(a2)[2 * v11] = static_cast<int>(v10);
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
    int a4, int a5, int a6, int a7, const char* a8)
{
    // 對齊反編譯 0041EB80
    const size_t fullLen = strlen(a8);

    if (fullLen == *curCount || a8[*curCount] == 10)
    {
        // 行尾或換行：只檢查左界
        if ((int)ptAbs[0] < a4)
            return FALSE;
    }
    else
    {
        // 一般字元：檢查左右界
        if ((int)ptAbs[0] < a4 || (int)ptAbs[0] > a6)
            return FALSE;
    }

    // 垂直測試
    int v11 = (int)ptAbs[1];
    int fontH = m_Text.GetFontHeight();
    if (v11 < a5 || v11 > a5 + fontH)
    {
        // 對齊反編譯：Y-range fallthrough — 檢查是否在下一段的 Y 範圍
        return (v11 >= a7 && (int)ptAbs[1] <= a7 + m_Text.GetFontHeight()) ? TRUE : FALSE;
    }

    // 判斷靠左或靠右
    if (abs((int)ptAbs[0] - a4) >= abs((int)ptAbs[0] - a6))
    {
        m_caretX = a6;
        m_caretY = a7;
    }
    else
    {
        m_caretX = a4;
        m_caretY = a5;
        --(*curCount);
    }
    return TRUE;
}

char* CControlEditBox::GetText()
{
    return (char *)m_Text.GetText();
}