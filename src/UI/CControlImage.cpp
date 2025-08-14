#include "UI/CControlImage.h"
#undef max
#undef min
#include <algorithm>

// ======================================================================
// 建構 / 解構（完全對齊你提供的基準預設）
// ======================================================================
CControlImage::CControlImage()
{
    // CControlBase::CControlBase() 已先被呼叫（由編譯器完成）

    m_nGIGroup = 5;        // [30]
    m_nGIID = 0;        // [31]
    m_usBlockID = 0xFFFF;   // [64]
    m_pGameImage = nullptr;  // [33]

    m_nAlpha = 255;        // [34]
    m_nAngle = 0;          // [35]
    m_drawFlag = 3;          // [144]
    m_bPrepared = 0;          // [37]

    m_outputPercentFromTop = 0; // [152]

    m_fadeCurA = 0;          // [39]
    m_lastTick = timeGetTime(); // [40]
    m_bFadeIn = 1;          // [41]
    m_fadeStep = 1;          // [42]

    m_fColorR = 1.0f;        // [43]
    m_fColorG = 1.0f;        // [44]
    m_fColorB = 1.0f;        // [45]
    m_fColorA = 1.0f;        // [46]
}

CControlImage::~CControlImage()
{
    // 交由 ImageManager 管理池；這裡不持有所有權，不需要 delete
    // Base 解構即可
}

// ======================================================================
// 建立 / 清理
// ======================================================================
void CControlImage::Create(CControlBase* pParent)
{
    CControlBase::Create(pParent);
    m_pGameImage = nullptr;
    m_fadeCurA = 0;
    SetAlpha(0); // 基準 Create() 會把 alpha 設 0，配合後續淡入
}

void CControlImage::Create(int x, int y, CControlBase* pParent)
{
    CControlBase::Create(x, y, pParent);
    m_pGameImage = nullptr;
    m_fadeCurA = 0;
    SetAlpha(0);
}

// 這個多載對應基準：Create(x, y, giid, block, parent)
void CControlImage::Create(int x, int y, unsigned int giid, unsigned short block, CControlBase* pParent)
{
    m_usBlockID = block;
    m_nGIID = static_cast<int>(giid);
    SetImageSize(); // 先以資源尺寸回填自身寬高
    CControlBase::Create(x, y, pParent);
    m_pGameImage = nullptr;
    m_fadeCurA = 0;
    SetAlpha(0);
}

void CControlImage::ClearData()
{
    CControlBase::ClearData();
    m_nGIID = 0;
    m_usBlockID = 0xFFFF;
}

// ======================================================================
// 設定圖像 ID / 區塊
// ======================================================================
void CControlImage::SetImage(unsigned int giid, unsigned short block)
{
    m_nGIID = static_cast<int>(giid);
    m_usBlockID = block;
    SetImageSize();
}

void CControlImage::SetImageID(unsigned int giGroup, unsigned int giid, unsigned short block)
{
    if (m_nGIGroup != static_cast<int>(giGroup) ||
        m_nGIID != static_cast<int>(giid) ||
        m_usBlockID != block)
    {
        m_nGIGroup = static_cast<int>(giGroup);
        m_nGIID = static_cast<int>(giid);
        m_usBlockID = block;
        SetImageSize();
    }
}

void CControlImage::SetImageID(unsigned int giid, unsigned short block)
{
    if (m_nGIID != static_cast<int>(giid) || m_usBlockID != block)
    {
        m_nGIID = static_cast<int>(giid);
        m_usBlockID = block;
        SetImageSize();
    }
}

// ======================================================================
// 依資源大小回填控制寬高（基準 SetImageSize）
// ======================================================================
bool CControlImage::SetImageSize()
{
    if (m_usBlockID == 0xFFFF)
    {
        m_usWidth = 0;
        m_usHeight = 0;
        return false;
    }

    GameImage* gi = cltImageManager::GetInstance()->GetGameImage(m_nGIGroup, m_nGIID, 0, 1);
    if (!gi)
    {
        m_usWidth = m_usHeight = 0;
        return false;
    }

    // 指定 block 後用 GetBlockRect 取寬高
    gi->SetBlockID(m_usBlockID);
    RECT rc{}; gi->GetBlockRect(&rc);
    const int w = std::max(0, (int)(rc.right - rc.left));
    const int h = std::max(0, (int)(rc.bottom - rc.top));

    m_usWidth = static_cast<uint16_t>(w);
    m_usHeight = static_cast<uint16_t>(h);

    cltImageManager::GetInstance()->ReleaseGameImage(gi);
    return (w | h) != 0;
}

// ======================================================================
// 準備繪製（對齊基準行為）
// - 取一個 GameImage*（池中物件）
// - 淡入：更新 m_fadeCurA
// - 把座標/縮放/角度/Alpha/Block/旗標 等資料塞回 GameImage
// ======================================================================
void CControlImage::PrepareDrawing()
{
    m_bPrepared = 0;

    if (!IsVisible()) return;
    if (m_usBlockID == 0xFFFF) return;

    // 向池取一個 GI；若你的引擎在別處已經持有，也可以只在第一次取
    m_pGameImage = cltImageManager::GetInstance()->GetGameImage(m_nGIGroup, m_nGIID, 0, 1);
    if (!m_pGameImage) return;

    // ---- 淡入邏輯（與基準相符）----
    if (m_bFadeIn)
    {
        float speedFactor = 1.0f;
        const unsigned long now = timeGetTime();
        const unsigned long dt = now - m_lastTick;
        // 基準：若 (dt/0x23) >= 2，則用 SETTING_FRAME*2.5 當速度倍數
        if (dt / 0x23 >= 2) speedFactor = static_cast<float>(SETTING_FRAME) * 2.5f;
        m_lastTick = now;

        if (m_fadeCurA < 255)
        {
            // (__int64)((m_fadeStep * speedFactor * 30.0 / SETTING_FRAME) + m_fadeCurA)
            const float add = (static_cast<float>(m_fadeStep) * speedFactor * 30.0f) / static_cast<float>(SETTING_FRAME);
            m_fadeCurA = std::min(255, static_cast<int>(add + static_cast<float>(m_fadeCurA)));
        }
    }

    // ---- 把狀態寫回 GameImage ----
    const int ax = GetAbsX();
    const int ay = GetAbsY();

    m_pGameImage->SetPosition(static_cast<float>(ax), static_cast<float>(ay));
    m_pGameImage->SetRotation(m_nAngle);

    // 縮放：沿用 CControlBase 的 Scale
    m_pGameImage->SetScaleXY(GetScaleX(), GetScaleY());

    // Alpha：若淡入啟用，用 m_fadeCurA；否則用 m_nAlpha
    m_pGameImage->SetAlpha(static_cast<unsigned int>(m_bFadeIn ? m_fadeCurA : m_nAlpha));

    // Block
    m_pGameImage->SetBlockID(m_usBlockID);

    // 旗標（對齊基準預設）
    m_pGameImage->m_bVertexAnimation = false; // +444 = 0
    m_pGameImage->m_bFlag_447 = true;         // +447 = 1
    m_pGameImage->m_bFlag_448 = true;         // +448 = 1
    m_pGameImage->m_bFlag_449 = true;         // +449 = 1
    m_pGameImage->m_bFlag_450 = true;         // +450 = 1

    // 輸出比例旗標（基準：+445 = *((BYTE*)this + 152)）
    m_pGameImage->m_bFlag_445 = (m_outputPercentFromTop != 0);

    // 其餘 drawFlag 在原版可能會影響 GI 內部不同模式；
    // 這裡不亂動翻轉/顏色運算，維持 GameImage 預設。

    m_bPrepared = 1;

    // 遞迴子物件
    CControlBase::PrepareDrawing();
}

// ======================================================================
// Draw（完全對齊基準：覆蓋顏色 → Draw → 還原預設顏色）
// 基準把四個 float 0..1（R,G,B,A）轉成 0xAARRGGBB
// ======================================================================
void CControlImage::Draw()
{
    if (!IsVisible()) return;
    if (m_usBlockID == 0xFFFF) return;
    if (!m_bPrepared) return;
    if (!m_pGameImage) return;

    const uint8_t R = ToByte01(m_fColorR); // [43]
    const uint8_t G = ToByte01(m_fColorG); // [44]
    const uint8_t B = ToByte01(m_fColorB); // [45]
    const uint8_t A = ToByte01(m_fColorA); // [46]

    const unsigned int ARGB =
        (static_cast<unsigned int>(A) << 24) |
        (static_cast<unsigned int>(R) << 16) |
        (static_cast<unsigned int>(G) << 8) |
        (static_cast<unsigned int>(B));

    m_pGameImage->SetOverWriteTextureColor(ARGB);
    m_pGameImage->Draw();
    m_pGameImage->SetDefaultTextureColor();

    // 子物件
    CControlBase::Draw();
}

// ======================================================================
// 隱藏（基準：若啟用淡入，則把目前淡入值/目標 alpha 歸零）
// ======================================================================
void CControlImage::Hide()
{
    if (m_bFadeIn)
    {
        m_fadeCurA = 0;
        m_nAlpha = 0;
    }
    CControlBase::Hide();
}

// ======================================================================
// 事件流程：更新命中快取，並把事件往父層丟（對齊基準精神）
// ======================================================================
int* CControlImage::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    // 反編譯顯示在 msg == 7 或 4（滑鼠相關）時，若座標未變且 a6 != 1，則直接略過處理
    // 原程式以靜態區域變數記錄上次座標；這裡沿用該設計（跨所有 CControlImage 實例共享）
    static int s_lastX = 0x7FFFFFFF;
    static int s_lastY = 0x7FFFFFFF;

    // 與你已修改的基底一致：預設回傳值為父物件指標（不轉交時維持與原版一致的非空語義）
    int* pResult = reinterpret_cast<int*>(m_pParent);

    if (a6 != 1 && (msg == 7 || msg == 4))  // a6 != 1 時啟用去抖；7/4 為滑鼠移動/相關訊息
    {
        if (s_lastX == x && s_lastY == y)
        {
            // 座標未變：略過後續處理與上拋，直接回傳父指標語義
            return pResult;
        }
        s_lastX = x;
        s_lastY = y;
    }

    // 更新「是否滑入命中」快取（對應 this[47] = CControlBase::PtInCtrl(...)）
    m_bHover = PtInCtrl(stPoint{ x, y }) ? 1 : 0;

    // 其餘交給基底流程（可能上拋給父）
    return reinterpret_cast<int*>(
        CControlBase::ControlKeyInputProcess(msg, key, x, y, a6, a7)
        );
}
