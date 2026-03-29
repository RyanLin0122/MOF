#include "UI/CControlChatBallon.h"
#include "Image/CDeviceManager.h"
#include <cstring>

//------------------------------------------------------------------------------
// 004256D0
CControlChatBallon::CControlChatBallon()
    : CControlBase()
    , m_nDirection(3) // DirDown
    , m_piece()
    , m_text()
{
}

//------------------------------------------------------------------------------
// 00425770
CControlChatBallon::~CControlChatBallon()
{
    // 子物件自動逆序解構
}

//------------------------------------------------------------------------------
// 004257E0
void CControlChatBallon::Create(CControlBase* parent,
    uint8_t styleIndex,
    unsigned int wrapWidth,
    uint16_t fontHeight)
{
    // vtbl+44 = Hide()
    this->Hide();

    // 只在尚未建立時初始化（反編譯檢查 m_pFirstChild == nullptr）
    if (!GetFirstChild())
    {
        if (parent)
            CControlBase::Create(parent);

        // 11 片依 styleIndex 建立
        const uint16_t* frames = &g_ChatBallonStyleTable[styleIndex * 11];
        for (int i = 0; i < 11; ++i)
        {
            m_piece[i].Create(this);
            m_piece[i].SetImage(0x2000005Bu, frames[i]);
        }

        // 文字控制
        m_text.Create(this);
        ResizeChatBallon(wrapWidth, fontHeight);
    }
}

//------------------------------------------------------------------------------
// 00425870
void CControlChatBallon::ResizeChatBallon(unsigned int wrapWidth, uint16_t fontHeight)
{
    // 取左上角 piece[0] 的尺寸作為邊框基準
    int cornerW = m_piece[0].GetWidth();
    int cornerH = m_piece[0].GetHeight();

    // 對齊邊框 piece 位置
    m_piece[1].SetX(cornerW);
    m_piece[3].SetY(cornerH);
    m_piece[5].SetPos(cornerW, cornerH);
    m_piece[4].SetY(cornerH);
    m_piece[8].SetX(cornerW);

    // 文字控制配置
    m_text.SetPos(cornerW, cornerH + 1);
    m_text.SetControlSetFont("ChatBallonText");
    m_text.SetMultiLineSpace(3);
    m_text.SetMultiLineSize(100, 0);

    // 儲存 wrapWidth（反編譯：*((_DWORD *)this + 596) = a2）
    m_wrapWidth = wrapWidth;

    if (fontHeight)
        m_text.SetFontHeight(fontHeight);
}

//------------------------------------------------------------------------------
// 00425930
void CControlChatBallon::SetString(char* text, int x, int y,
    uint16_t reqW, uint16_t reqH, Direction dir)
{
    // 空字串 → 隱藏
    if (!strcmp(text, ""))
    {
        this->Hide();
        return;
    }

    // 顯示
    this->Show();

    // 檢查氣球已建立且文字有變化（ground truth: *((_DWORD *)this + 304) = m_piece[5].m_pGameImage）
    if (m_piece[5].GetGameImage() && strcmp(m_text.GetText(), text))
    {
        m_nDirection = static_cast<int>(dir);
        m_text.SetText(text);

        uint16_t contentW = reqW;
        uint16_t contentH = reqH;

        if (reqW || reqH)
        {
            // 固定尺寸模式
            m_text.SetMultiLineSize(reqW, reqH);
            m_text.m_isCentered = 1;
            m_text.SetX(static_cast<int>(static_cast<double>(reqW) * 0.5));
        }
        else
        {
            // 自動尺寸模式
            if (strlen(text) <= 0x16)
            {
                DWORD sizeResult = 0;
                contentW = *reinterpret_cast<uint16_t*>(m_text.GetTextLength(&sizeResult));
                contentH = 15;
            }
            else
            {
                contentW = 100;
                contentH = m_text.GetCalcedTextBoxHeight(100);
            }
            m_text.SetMultiLineSize(contentW, contentH);
        }

        // 儲存 content 尺寸到 piece[5] 的 scale 欄位作暫存
        float fContentW = static_cast<float>(contentW);
        float fContentH = static_cast<float>(contentH);
        m_piece[5].SetScaleX(fContentW);
        m_piece[5].SetScaleY(fContentH);

        // 取角落尺寸
        int cornerW = m_piece[0].GetWidth();
        uint16_t cornerH = m_piece[0].GetHeight();

        // 計算縮放後的內容區域
        int scaledW = static_cast<int>(
            static_cast<double>(m_piece[5].GetWidth()) * static_cast<double>(fContentW));
        int scaledH = static_cast<int>(
            static_cast<double>(m_piece[5].GetHeight()) * static_cast<double>(fContentH));

        float fScaledW = static_cast<float>(scaledW);
        float fScaledH = static_cast<float>(scaledH);

        // 設定氣球總尺寸
        SetWidth(static_cast<uint16_t>(scaledW + 2 * cornerW));
        SetHeight(static_cast<uint16_t>(scaledH + 2 * cornerH));

        // 邊框 piece 定位
        m_piece[1].SetScaleX(fScaledW);
        m_piece[2].SetX(scaledW + cornerW);
        m_piece[3].SetScaleY(fScaledH);
        m_piece[4].SetX(m_piece[2].GetX());
        m_piece[4].SetScaleY(fScaledH);
        m_piece[6].SetY(cornerH + scaledH);

        if (dir == DirDown) // dir == 3
        {
            // 底部方向箭頭
            int bmY = static_cast<int>(
                static_cast<double>(m_piece[2].GetHeight()) * static_cast<double>(m_piece[2].GetScaleY())
                + fScaledH);
            m_piece[7].SetPos(m_piece[2].GetX(), bmY);

            int bottomY = m_piece[6].GetY();
            int arrowW = m_piece[9].GetWidth();

            float halfGap = static_cast<float>(static_cast<int>(
                static_cast<double>(scaledW - arrowW) * 0.5));

            m_piece[8].SetY(bottomY);
            m_piece[8].SetScaleX(halfGap);

            float brOffset = static_cast<double>(m_piece[8].GetWidth())
                           * static_cast<double>(m_piece[8].GetScaleX());
            float arrowX = static_cast<float>(m_piece[8].GetX()) + brOffset;

            m_piece[9].SetPos(static_cast<int>(arrowX), bottomY);

            int arrowEndX = m_piece[9].GetX();
            m_piece[10].SetPos(arrowW + arrowEndX, bottomY);
            m_piece[10].SetScaleX(static_cast<float>(fScaledW - halfGap - static_cast<double>(arrowW)));
        }
        else
        {
            // 非底部方向
            int bmY = static_cast<int>(
                static_cast<double>(m_piece[2].GetHeight()) * static_cast<double>(m_piece[2].GetScaleY())
                + fScaledH);
            m_piece[7].SetPos(m_piece[2].GetX(), bmY);

            int bottomY = m_piece[6].GetY();
            m_piece[8].SetY(bottomY);
            m_piece[8].SetScaleX(fScaledW);

            if (dir == DirUp) // dir == 0
            {
                // 箭頭朝上
                int arrowH = m_piece[9].GetHeight();
                int arrowPosY = 4 - arrowH;
                float halfW = static_cast<float>(static_cast<double>(GetWidth()) * 0.5);
                float halfArrowW = static_cast<float>(static_cast<double>(m_piece[9].GetWidth()) * 0.5);
                m_piece[9].SetPos(static_cast<int>(halfW - halfArrowW), arrowPosY);
                m_piece[9].SetAngle(180);
            }
            else if (dir == DirLeft) // dir == 1
            {
                // 箭頭朝左
                float halfH = static_cast<float>(static_cast<double>(GetHeight()) * 0.5);
                float halfArrowW = static_cast<float>(static_cast<double>(m_piece[9].GetWidth()) * 0.5);
                int posY = static_cast<int>(halfH - halfArrowW);
                int arrowH2 = m_piece[9].GetHeight();
                m_piece[9].SetPos(4 - arrowH2, posY);
                m_piece[9].SetAngle(90);
            }
            else if (dir == DirRight) // dir == 2
            {
                // 箭頭朝右
                float halfH = static_cast<float>(static_cast<double>(GetHeight()) * 0.5);
                float halfArrowW = static_cast<float>(static_cast<double>(m_piece[9].GetWidth()) * 0.5);
                m_piece[9].SetPos(GetWidth() - 4, static_cast<int>(halfH - halfArrowW));
                m_piece[9].SetAngle(270);
            }
        }

        // 呼叫 SetArrowPos(x, y) 定位整個氣球（反編譯：vtbl+84）
        SetArrowPos(x, y);
    }
}

//------------------------------------------------------------------------------
// 00425E40
void CControlChatBallon::SetImageChange(unsigned int resId,
    const uint16_t* frames11,
    unsigned int wrapWidth,
    uint16_t fontHeight)
{
    for (int i = 0; i < 11; ++i)
        m_piece[i].SetImage(resId, frames11[i]);
    ResizeChatBallon(wrapWidth, fontHeight);
}

//------------------------------------------------------------------------------
// 00425E90
void CControlChatBallon::ResetImage(uint8_t styleIndex,
    unsigned int wrapWidth,
    uint16_t fontHeight)
{
    const uint16_t* frames = &g_ChatBallonStyleTable[styleIndex * 11];
    for (int i = 0; i < 11; ++i)
        m_piece[i].SetImage(0x2000005Bu, frames[i]);
    ResizeChatBallon(wrapWidth, fontHeight);
}

//------------------------------------------------------------------------------
// 00425EF0
int CControlChatBallon::IsSameImage(unsigned int resId, uint16_t frame)
{
    // 檢查 piece[0] 的 GIID 是否匹配
    if (static_cast<unsigned int>(m_piece[0].m_nGIID) != resId)
        return 0;

    // 搜尋任一 piece 的 block 匹配
    for (int i = 0; i < 11; ++i)
    {
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
    // 方向非 DirDown(3) 時隱藏 piece[10]
    if (m_nDirection != 3)
        m_piece[10].Hide();
}

//------------------------------------------------------------------------------
// 00425F80
void CControlChatBallon::SetPos(int x, int y)
{
    CControlBase::SetAbsPos(
        x - static_cast<int>(static_cast<double>(GetWidth()) * 0.5),
        y - GetHeight());
}

//------------------------------------------------------------------------------
// 00425FC0
void CControlChatBallon::SetArrowPos(int x, int y)
{
    int dir = m_nDirection;

    if (dir == 3) // DirDown
    {
        CControlBase::SetPos(
            static_cast<int>(static_cast<double>(x) - static_cast<double>(GetWidth()) * 0.5),
            y - GetHeight());
    }
    else if (dir == 0) // DirUp
    {
        CControlBase::SetPos(
            static_cast<int>(static_cast<double>(x) - static_cast<double>(GetWidth()) * 0.5),
            y);
    }
    else if (dir == 1) // DirLeft
    {
        CControlBase::SetPos(
            x,
            static_cast<int>(static_cast<double>(y) - static_cast<double>(GetHeight()) * 0.5));
    }
    else if (dir == 2) // DirRight
    {
        CControlBase::SetPos(
            x - GetWidth(),
            static_cast<int>(static_cast<double>(y) - static_cast<double>(GetHeight()) * 0.5));
    }
}

//------------------------------------------------------------------------------
// 004260B0
void CControlChatBallon::Draw()
{
    if (!byte_21CB35D)
    {
        if (IsVisible())
        {
            g_clDeviceManager.SetRenderState(D3DRS_SRCBLEND, 5u);
            g_clDeviceManager.SetRenderState(D3DRS_DESTBLEND, 6u);
            CControlBase::Draw();
        }
    }
}
