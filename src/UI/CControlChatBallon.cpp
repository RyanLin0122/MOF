#include "UI/CControlChatBallon.h"
#include "Image/CDeviceManager.h"

//------------------------------------------------------------------------------
// 004256D0
CControlChatBallon::CControlChatBallon()
    : CControlBase()
    , m_text()
{
    // 等價於 'eh vector constructor iterator'：建立 11 個 CControlImage
    for (int i = 0; i < 11; ++i) {
        // 先不掛 parent，Create() 中會掛到 this
        // m_piece[i] 預設建構即可
    }

    // 掛上 vftable 由編譯器處理
    // 設定方向預設值（反編譯：*((_DWORD*)this + 30) = 3）
    m_dir = DirDown;
}

//------------------------------------------------------------------------------
// 00425770
CControlChatBallon::~CControlChatBallon()
{
    // 反編譯是顯式逐一解構；C++ 由成員自動解構
}

//------------------------------------------------------------------------------
// 004257E0
void CControlChatBallon::Create(CControlBase* parent,
    uint8_t styleIndex,
    unsigned int wrapWidth,
    uint16_t fontHeight)
{
    // Hide() 與未建立檢查
    this->Hide();
    if (FirstCreated) {
        if (parent) CControlBase::Create(parent);

        // 把 11 片註冊到自己底下，並根據 styleIndex 套 frame
        // 反編譯：起始表位址 7106256，每種 style 22 uint16 => 11 片
        const uint16_t* frames = reinterpret_cast<const uint16_t*>(0x006C6B10) /*dummy*/; // 只作語意標示
        frames += styleIndex * 22;

        for (int i = 0; i < 11; ++i) {
            m_piece[i].Create(this);
            m_piece[i].SetImage(0x2000005Bu, frames[i]);
        }

        // 建立文字
        m_text.Create(this);
        ResizeChatBallon(wrapWidth, fontHeight); // 內部會配置字型與多行
        FirstCreated = false;
    }
}

//------------------------------------------------------------------------------
// 00425870  排版九宮格與文字框預設屬性
void CControlChatBallon::ResizeChatBallon(unsigned int wrapWidth, uint16_t fontHeight)
{
    // 以左上角塊的尺寸作為邊框基準
    const int cornerW = m_piece[0].GetWidth();
    const int cornerH = m_piece[0].GetHeight();

    // 對齊：把上中、左中、中央、右中等放到基準位置
    m_piece[1].SetX(cornerW);                // TM.x = TL.w
    m_piece[3].SetY(cornerH);                // ML.y = TL.h
    m_piece[5].SetPos(cornerW, cornerH);     // MC = (TL.w, TL.h)
    m_piece[4].SetY(cornerH);                // MM.y = TL.h
    m_piece[8].SetX(cornerW);                // BR 的 x 基準（之後會再加寬度時用）

    // 文字屬性
    m_text.SetPos(cornerW, cornerH + 1);
    m_text.SetControlSetFont("ChatBallonText");
    m_text.SetMultiLineSpace(3);
    m_text.SetMultiLineSize(100, 0);   // 預設 100 寬、動態高
    m_wrapWidthMax = wrapWidth;

    if (fontHeight) {
        m_text.SetFontHeight(fontHeight);
    }
}

//------------------------------------------------------------------------------
// 00425930  設定內容與方向、計算尺寸與箭頭位置
void CControlChatBallon::SetString(char* text,
    int x, int y,
    uint16_t reqW, uint16_t reqH,
    Direction dir)
{
    if (!strcmp(text, "")) {
        this->Hide();
        return;
    }

    this->Show();

    // 方向與文字
    if (strcmp(m_text.GetText(), text) != 0) {
        m_dir = dir;
        m_text.SetText(text);

        uint16_t contentW = reqW;
        uint16_t contentH = reqH;

        // 若沒有給固定尺寸，依長度計算
        if (!reqW && !reqH) {
            if (strlen(text) <= 0x16) {
                // 直接用文字長度（寬）與 15 高（反編譯的分支）
                int px = 0;
                int py = 0;
                (void)m_text.GetTextPixelSize(&px, &py); // 你的引擎如果回傳指標，請改為實際 API
                contentW = static_cast<uint16_t>(px);
                contentH = 15;
            }
            else {
                contentW = 100;
                contentH = static_cast<uint16_t>(m_text.GetCalcedTextBoxHeight(100));
            }
            m_text.SetMultiLineSize(contentW, contentH);
            m_multilineFixed = false;
        }
        else {
            m_text.SetMultiLineSize(contentW, contentH);
            m_multilineFixed = true;      // *((_DWORD*)this + 595) = 1
            // 反編譯：把文字 x 移到 contentW/2，維持大致置中
            m_text.SetX(static_cast<int>(contentW * 0.5f));
        }

        // 依九宮格與箭頭重排
        layoutPieces(contentW, contentH);
        placeArrow(m_textW, m_textH);

        // 反編譯最後呼叫：(*(this->vtbl+84))(this, a3, a4)
        // 這裡對齊語意：把氣球定位到 (x,y)
        SetArrowPos(x, y);
    }
}

//------------------------------------------------------------------------------
// 00425E40  換圖（直接給 11 個 frame）
void CControlChatBallon::SetImageChange(unsigned int resId,
    const uint16_t* frames11,
    unsigned int wrapWidth,
    uint16_t fontHeight)
{
    for (int i = 0; i < 11; ++i) {
        m_piece[i].SetImage(resId, frames11[i]);
    }
    ResizeChatBallon(wrapWidth, fontHeight);
}

//------------------------------------------------------------------------------
// 00425E90  依 styleIndex 套用預設圖（table: 每種 22 個 uint16 -> 11 片）
void CControlChatBallon::ResetImage(uint8_t styleIndex,
    unsigned int wrapWidth,
    uint16_t fontHeight)
{
    // 在反編譯中：v7 = (unsigned __int16 *)(22 * a2 + 7106256)
    const uint16_t* frames = reinterpret_cast<const uint16_t*>(0x006C6B10) /*dummy*/;
    frames += styleIndex * 22;

    for (int i = 0; i < 11; ++i) {
        m_piece[i].SetImage(0x2000005Bu, frames[i]);
    }
    ResizeChatBallon(wrapWidth, fontHeight);
}

//------------------------------------------------------------------------------
// 00425EF0  檢查是否相同資源/其中任一片等於該 frame
int CControlChatBallon::IsSameImage(unsigned int resId, uint16_t frame) const
{
    if (m_piece[1].m_nGIID != resId) // 反編譯用的是某個偏移，語意上檢查任一/首片資源
        return 0;

    for (int i = 0; i < 11; ++i) {
        if (m_piece[i].m_usBlockID == frame)
            return 1;
    }
    return 0;
}

//------------------------------------------------------------------------------
// 00425F60
void CControlChatBallon::Show()
{
    CControlBase::Show();
    if (m_dir != DirDown) {
        m_piece[10].Hide(); // 反編譯：方向非 3 時隱藏 +2044 那片
    }
}

//------------------------------------------------------------------------------
// 00425F80  置中到 (x, y)（以整體寬的一半與整體高）
void CControlChatBallon::SetPos(int x, int y)
{
    const int halfW = static_cast<int>(m_textW * 0.5f);
    SetAbsPos(x - halfW, y - m_textH);
}

//------------------------------------------------------------------------------
// 00425FC0  依箭頭方向決定如何將整個氣球對齊到 (x,y)
void CControlChatBallon::SetArrowPos(int x, int y)
{
    switch (m_dir) {
    case DirDown:
        SetPos(static_cast<int>(x - m_textW * 0.5f), y - m_textH);
        break;
    case DirUp:
        SetPos(static_cast<int>(x - m_textW * 0.5f), y);
        break;
    case DirLeft:
        SetPos(x, static_cast<int>(y - m_textH * 0.5f));
        break;
    case DirRight:
        SetPos(x - m_textW, static_cast<int>(y - m_textH * 0.5f));
        break;
    }
}

//------------------------------------------------------------------------------
// 004260B0  繪製（套 SRC/DSTBLEND 再走 base::Draw）
void CControlChatBallon::Draw()
{
    if (!g_bRenderStateLocked && this->IsVisible()) {
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, 5u);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, 6u);
        Draw();
    }
}

//------------------------------------------------------------------------------
// 依反編譯中的幾何關係，把九宮格按 contentW/H 擺好，並更新總寬高(m_textW/H)
void CControlChatBallon::layoutPieces(uint16_t contentW, uint16_t contentH)
{
    const int cw = m_piece[0].GetWidth();
    const int ch = m_piece[0].GetHeight();

    // 角落
    m_piece[0].SetPos(0, 0);                    // TL
    m_piece[2].SetPos(cw + contentW, 0);        // TR
    m_piece[6].SetPos(0, ch + contentH);        // BL
    m_piece[8].SetPos(cw + contentW, ch + contentH); // BR

    // 邊（僅定位；實際是否拉伸/平鋪由 CControlImage 內部決定）
    m_piece[1].SetPos(cw, 0);                   // TM
    m_piece[3].SetPos(0, ch);                   // ML
    m_piece[5].SetPos(cw, ch);                  // MC
    m_piece[4].SetPos(cw, ch);                  // MM（中央）
    m_piece[7].SetPos(cw, ch + contentH);       // BM

    // 文字框與總尺寸
    m_text.SetPos(cw, ch + 1);
    m_textW = static_cast<uint16_t>(cw + contentW + cw);
    m_textH = static_cast<uint16_t>(ch + contentH + ch);
}

//------------------------------------------------------------------------------
// 安放箭頭：反編譯會旋轉 +1852 那片，並依方向挪到邊中央
void CControlChatBallon::placeArrow(uint16_t totalW, uint16_t totalH)
{
    // m_piece[9] 當作箭頭主體
    // m_piece[10] 當作底座/補塊（反編譯在 DirDown 以外會隱藏）
    auto& arrow = m_piece[9];

    switch (m_dir) {
    case DirDown: {
        // 置於底邊中央，朝下（反編譯給 180 度給「朝上」；此處視引擎角度定義）
        const int ax = static_cast<int>(totalW * 0.5f - arrow.GetWidth() * 0.5f);
        const int ay = m_piece[7].GetY(); // 底邊 y
        arrow.SetPos(ax, ay);
        arrow.SetAngle(180); // 反編譯：DirUp 用 180；DirDown 用 0？（原碼中有 180/90/270 分支）
        break;
    }
    case DirUp: {
        const int ax = static_cast<int>(totalW * 0.5f - arrow.GetWidth() * 0.5f);
        const int ay = 4; // 反編譯有 4 的偏移
        arrow.SetPos(ax, ay);
        arrow.SetAngle(180);
        break;
    }
    case DirLeft: {
        const int ax = 4;
        const int ay = static_cast<int>(totalH * 0.5f - arrow.GetHeight() * 0.5f);
        arrow.SetPos(ax, ay);
        arrow.SetAngle(90);
        break;
    }
    case DirRight: {
        const int ax = totalW - 4;
        const int ay = static_cast<int>(totalH * 0.5f - arrow.GetHeight() * 0.5f);
        arrow.SetPos(ax, ay);
        arrow.SetAngle(270);
        break;
    }
    }

    // 反編譯：若不是 DirDown，會把第 11 片（索引 10）隱藏
    if (m_dir != DirDown) m_piece[10].Hide(); else m_piece[10].Show();
}
