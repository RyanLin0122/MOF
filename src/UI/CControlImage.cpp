#include "UI/CControlImage.h"

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
    m_fadeCurA = 0;
    SetAlpha(0);
}

void CControlImage::Create(int x, int y, CControlBase* pParent)
{
    CControlBase::Create(x, y, pParent);
    m_fadeCurA = 0;
    SetAlpha(0);
}

void CControlImage::Create(int x, int y, unsigned int giid, unsigned short block, CControlBase* pParent)
{
    m_usBlockID = block;
    m_nGIID = static_cast<int>(giid);
    SetImageSize();
    CControlBase::Create(x, y, pParent);
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

// 對齊反編譯 00420A90：無條件版本 SetImageID
void CControlImage::SetImageID(int giGroup, int giid, short block)
{
    m_nGIGroup = giGroup;
    m_nGIID = giid;
    m_usBlockID = block;
    SetImageSize();
}

// ======================================================================
// 依資源大小回填控制寬高（對齊反編譯 00420F90）
// ======================================================================
void CControlImage::SetImageSize()
{
    if (m_usBlockID == 0xFFFF)
    {
        m_usWidth = 0;
        m_usHeight = 0;
        return;
    }

    GameImage* v2 = cltImageManager::GetInstance()->GetGameImage(m_nGIGroup, m_nGIID, 0, 1);
    m_pGameImage = v2;  // 對齊反編譯：*((_DWORD *)this + 33) = v2;
    if (!v2)
        return;

    // 對齊反編譯：從 GameImage 內部結構直接讀取 block 尺寸
    // v4 = *((_DWORD *)v2 + 2);  → GameImage+8 = pBlockData
    char* pGIRaw = reinterpret_cast<char*>(v2);
    char* pBlockData = *reinterpret_cast<char**>(pGIRaw + 8);
    uint16_t blockID = m_usBlockID;

    int v10 = 0; // height
    if (pBlockData)
    {
        if (*reinterpret_cast<uint16_t*>(pBlockData + 28) > blockID)
        {
            // v10 = *(_DWORD *)(*(_DWORD *)(v4 + 32) + 52 * blockID + 24);
            char* pEntries = *reinterpret_cast<char**>(pBlockData + 32);
            v10 = *reinterpret_cast<int*>(pEntries + 52 * static_cast<unsigned short>(m_usBlockID) + 24);
        }
    }

    // 對齊反編譯：LABEL_8 起
    char* pGI2 = reinterpret_cast<char*>(m_pGameImage);
    uint16_t v7 = m_usBlockID;
    char* v8 = *reinterpret_cast<char**>(pGI2 + 8);
    int v9 = 0; // width
    if (v8)
    {
        if (*reinterpret_cast<uint16_t*>(v8 + 28) > v7)
        {
            // v9 = *(_DWORD *)(*(_DWORD *)(v8 + 32) + 52 * blockID + 20);
            char* pEntries2 = *reinterpret_cast<char**>(v8 + 32);
            v9 = *reinterpret_cast<int*>(pEntries2 + 52 * static_cast<unsigned short>(m_usBlockID) + 20);
        }
    }

    m_usWidth = static_cast<uint16_t>(v9);
    m_usHeight = static_cast<uint16_t>(v10);
    cltImageManager::GetInstance()->ReleaseGameImage(m_pGameImage);
}

void CControlImage::SetShadeMode(int enable)
{
    // [41] = enable
    m_bFadeIn = enable;

    if (enable) {
        // 開啟淡入：從 0 透明度開始（[39]）
        m_fadeCurA = 0;
    }
    else {
        // 關閉淡入：直接滿透明度，並同步一般 alpha（[39] 與 [34]）
        m_fadeCurA = 255;
        m_nAlpha = 255;
    }
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

    // 對齊反編譯：向池取 GameImage 並存入 m_pGameImage
    GameImage* v3 = cltImageManager::GetInstance()->GetGameImage(m_nGIGroup, m_nGIID, 0, 1);
    m_pGameImage = v3;  // *((_DWORD *)this + 33) = v3;
    if (!v3) return;

    // ---- 淡入邏輯 ----
    if (m_bFadeIn == 1)
    {
        float speedFactor = 1.0f;
        if ((timeGetTime() - m_lastTick) / 0x23 >= 2)
            speedFactor = static_cast<float>(SETTING_FRAME) * 2.5f;
        m_lastTick = timeGetTime();

        if (m_fadeCurA < 255)
        {
            long long v4 = static_cast<long long>(
                static_cast<double>(m_fadeStep) * static_cast<double>(speedFactor) * 30.0
                / static_cast<double>(SETTING_FRAME)
                + static_cast<double>(m_fadeCurA));
            m_fadeCurA = static_cast<int>(v4);
            if (m_fadeCurA > 255)
                m_fadeCurA = 255;
        }
    }

    // 對齊反編譯：m_bPrepared = 1 在 block check 之前
    m_bPrepared = 1;

    // ---- 把狀態寫回 GameImage（對齊反編譯順序）----
    char* pGI = reinterpret_cast<char*>(m_pGameImage);

    // 對齊反編譯：*(_DWORD *)(v11 + 388) = *((_DWORD *)this + 35);  → rotation
    *reinterpret_cast<int*>(pGI + 388) = m_nAngle;
    // *(_BYTE *)(v11 + 448) = 1;
    pGI[448] = 1;
    // *(_BYTE *)(v11 + 444) = 0;
    pGI[444] = 0;

    // 對齊反編譯：position
    float fAbsY = static_cast<float>(GetAbsY());
    float fAbsX = static_cast<float>(GetAbsX());
    pGI = reinterpret_cast<char*>(m_pGameImage);
    *reinterpret_cast<float*>(pGI + 332) = fAbsX;
    *reinterpret_cast<float*>(pGI + 336) = fAbsY;
    // *(_BYTE *)(v13 + 447) = 1;
    pGI[447] = 1;

    // 對齊反編譯：scaleX
    pGI = reinterpret_cast<char*>(m_pGameImage);
    *reinterpret_cast<float*>(pGI + 424) = GetScaleX();
    pGI[449] = 1;
    pGI[444] = 0;

    // 對齊反編譯：scaleY
    pGI = reinterpret_cast<char*>(m_pGameImage);
    *reinterpret_cast<float*>(pGI + 428) = GetScaleY();
    pGI[449] = 1;
    pGI[444] = 0;

    // 對齊反編譯：alpha
    pGI = reinterpret_cast<char*>(m_pGameImage);
    if (m_bFadeIn)
        *reinterpret_cast<int*>(pGI + 380) = m_fadeCurA;
    else
        *reinterpret_cast<int*>(pGI + 380) = m_nAlpha;
    pGI[450] = 1;
    pGI[444] = 0;

    // 對齊反編譯：blockID
    pGI = reinterpret_cast<char*>(m_pGameImage);
    *reinterpret_cast<uint16_t*>(pGI + 372) = m_usBlockID;
    // *(_BYTE *)(v17 + 446) = 1;
    pGI[446] = 1;

    // 對齊反編譯：drawFlag
    char v18 = m_drawFlag;
    if (v18 != 3)
        *reinterpret_cast<int*>(reinterpret_cast<char*>(m_pGameImage) + 392) = (v18 != 0) ? 1 : 0;

    // 對齊反編譯：*(_BYTE *)(... + 445) = *((_BYTE *)this + 152);
    reinterpret_cast<char*>(m_pGameImage)[445] = static_cast<char>(m_outputPercentFromTop);

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

    // 對齊反編譯：result = (int *)pt.y;
    int* pResult = reinterpret_cast<int*>(y);

    if (a6 != 1 && (msg == 7 || msg == 4))  // a6 != 1 時啟用去抖；7/4 為滑鼠移動/相關訊息
    {
        if (s_lastX == x && s_lastY == y)
        {
            return pResult;
        }
        s_lastX = x;
        s_lastY = y;
    }

    // 對齊反編譯：this[47] = CControlBase::PtInCtrl((int)this, pt);
    m_bHover = PtInCtrl(stPoint{ x, y });

    // 其餘交給基底流程（可能上拋給父）
    return reinterpret_cast<int*>(
        CControlBase::ControlKeyInputProcess(msg, key, x, y, a6, a7)
        );
}
