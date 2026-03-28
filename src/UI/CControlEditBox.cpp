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

    // 對齊反編譯：*((_DWORD *)this + 30) = -1
    reinterpret_cast<int*>(this)[30] = -1;

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
    // 對齊反編譯：ground truth 先設色再設文字（this[909]=-7590900 在 SetText 之前）
    m_Caret.SetTextColor(kTextColor);                  // *((this)+909) = -7590900
    m_Caret.SetText("I");

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
    sprintf(buf, "%d", v);
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
int CControlEditBox::GetMaxTextSize() const
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

    // 對齊反編譯：只有 m_writable 時才執行完整邏輯並呼叫 CControlBase::PrepareDrawing
    // ground truth 中 m_focus && !m_writable 時直接 return，不呼叫基底
    if (!m_writable)
        return;

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
    // 對齊反編譯：strcpy，無 null guard
    strcpy(bak, m_Text.GetText());
    m_Text.SetText(src);

    // 密碼遮罩
    if (m_password) {
        // 對齊反編譯：計算字元數（DBCS lead byte 視為 1 字元）
        // ground truth 從 strlen 開始減，且不檢查 src[i+1]
        int v4 = 0;
        int strlenSrc = (int)strlen(src);
        int nChars = strlenSrc;
        if (strlenSrc > 0) {
            do {
                if (IsDBCSLeadByte((BYTE)src[v4])) {
                    ++v4;
                    --nChars;
                }
                ++v4;
            } while (v4 < strlenSrc);
        }

        char Destination[1024]{};
        std::memset(Destination, 0x2A, nChars); // '*'
        // 對齊反編譯：strcpy(v32, Destination)
        strcpy(bak, Destination);
        if (nChars)
            src[nChars + 1023] = 0; // 對齊反編譯 ground truth 原始行為
        m_Mask.SetText(Destination);
        m_Text.Hide();

        // 對齊反編譯：先設 caret X，再呼叫 GetCaretPos，最後設 Y
        int w = 0, h = 0;
        g_MoFFont.GetTextLength(&w, &h, "CharacterName", Destination);
        m_Caret.SetX(w + 2);
        int xy[2]{};
        GetCaretPos(xy, bak, Destination, m_imeIndex, 0);
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
            int v8 = cHeight; // 對齊反編譯：使用全域 cHeight
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
                unsigned int* v10 = stair.len;
                int remaining = v9;
                const char* Source = m_Text.GetText();
                do {
                    if (*v10) {
                        char Dest[1024]{};
                        std::strncpy(Dest, Source, v10[1]);
                        // 對齊反編譯：使用 6-param GetTextLength（DCTTextManager::GetText(3264)）
                        const char* fontFace = g_DCTTextManager.GetText(3264);
                        int w1 = 0, h1 = 0;
                        g_MoFFont.GetTextLength(&w1, &h1, v8, fontFace, Dest, 400);

                        std::memset(Dest, 0, sizeof(Dest));
                        std::strncpy(Dest, &Source[v25], *v10);
                        const char* fontFace2 = g_DCTTextManager.GetText(3264);
                        int w2 = 0, h2 = 0;
                        g_MoFFont.GetTextLength(&w2, &h2, v8, fontFace2, Dest, 400);

                        SetBlockShow(1, 0);
                        SetBlockBox(nullptr, absX + w1, absY,
                            w2, static_cast<uint16_t>(v8), 0);
                    }
                    v10 += 2;
                } while (--remaining);
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

// caret 位置量測（對齊反編譯完全對照 ground truth）
char* CControlEditBox::GetCaretPos(int outXY[2], const char* a3, const char* Source, int imeIndex, size_t Count)
{
    char Destination[1024];
    Destination[0] = '\0';
    std::memset(Destination + 1, 0, sizeof(Destination) - 1);

    int fontH = m_Text.GetFontHeight();
    // 對齊反編譯：無條件呼叫 IsComposition（不檢查 imeIndex != 0xFFFF）
    int isComposition = g_IMMList.IsComposition(imeIndex);

    int v7 = Count ? (int)Count : g_IMMList.GetEditPosition(imeIndex);
    int v8 = (int)strlen(Source);

    int v10 = 0; // width result
    int v11 = 0; // y result

    if (IsMultiLine()) {
        int v26 = m_Text.GetMultiLineSpace();
        // 對齊反編譯：零化內部欄位
        reinterpret_cast<int*>(this)[36] = 0;
        reinterpret_cast<int*>(this)[37] = 0;
        reinterpret_cast<short*>(this)[76] = 0;

        int v12 = m_Text.GetCharByteByLine(m_lineBreakBytes, 10);
        int v13 = (int)(unsigned char)m_visibleLines;

        // 對齊反編譯：當行數超過 visibleLines 時截斷文字
        const char* v14;
        if (v12 <= v13) {
            v14 = Source;
        } else {
            std::strncpy(Destination, Source, (unsigned char)m_lineBreakBytes[v13 - 1]);
            m_Text.SetText(Destination);
            v14 = Destination;
            Source = Destination;
        }

        int v15 = 0;   // lineIdx
        int v16 = 0;
        int v23 = 0;
        if (v12 > 0) {
            // 對齊反編譯：直接比較 caretPos > lineBreakBytes[lineIdx]
            while (v7 > (int)(unsigned char)m_lineBreakBytes[v16] && v16 != v12 - 1) {
                if (++v16 >= v12)
                    goto LABEL_23;
            }
            // 對齊反編譯：換行字元檢查
            if (v14[v7 - 1] == '\n')
                ++v16;
            v23 = v16;
            v15 = v16;
        }

    LABEL_23:
        // 對齊反編譯：組字狀態下的行推進
        if (m_lineBreakBytes[v15 + 1] && v7 == (int)(unsigned char)m_lineBreakBytes[v15] && isComposition)
            v23 = ++v15;

        // 對齊反編譯：visibleLines 上限夾制
        int v17 = (int)(unsigned char)m_visibleLines;
        if (v15 >= v17) {
            v23 = v17 - 1;
            v15 = v17 - 1;
        }

        // 對齊反編譯：取行起始位元組（LOBYTE）
        int v18 = 0;
        if (v15)
            v18 = (unsigned char)m_lineBreakBytes[v15 - 1];

        int v19 = v7 - v18;
        std::strncpy(Destination, &v14[v18], v19);
        Destination[v19] = '\0';

        // 對齊反編譯：使用 6-param GetTextLength（DCTTextManager::GetText(3264) 取字型名稱）
        const char* fontFace = g_DCTTextManager.GetText(3264);
        int v27 = 0, h = 0;
        g_MoFFont.GetTextLength(&v27, &h, fontH, fontFace, Destination, 400);
        v10 = v27;

        if (isComposition && IsDBCSLeadByte((BYTE)Source[v7]))
            v10 += fontH;

        // 對齊反編譯：m_caretIndex 只在多行分支中設定
        m_caretIndex = v7;
        v11 = v23 * (fontH + v26);
    }
    else {
        // 單行：對齊反編譯三種情況
        if (v8 == v7) {
            strcpy(Destination, Source);
        } else if (v8 > v7) {
            std::strncpy(Destination, Source, v7);
        }
        // else: Destination stays empty（ground truth 行為）

        // 對齊反編譯：使用 6-param GetTextLength
        const char* fontFace = g_DCTTextManager.GetText(3264);
        int v26 = 0, h = 0;
        g_MoFFont.GetTextLength(&v26, &h, fontH, fontFace, Destination, 400);
        v10 = v26;

        if (isComposition && IsDBCSLeadByte((BYTE)Source[v7]))
            v10 += fontH;

        v11 = 0;
    }

    outXY[0] = v10;
    outXY[1] = v11;
    return reinterpret_cast<char*>(outXY);
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