#include "Effect/CCAEffect.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include <new> // for std::nothrow

//=============================================================================
// FrameSkip 類別實作
//=============================================================================

FrameSkip::FrameSkip() : m_fAccumulatedTime(0.0f), m_fTimePerFrame(1.0f / 60.0f) {}
FrameSkip::~FrameSkip() {}

// 邏輯對應 Effectall.c @ 005286C0 (CCAEffect::FrameProcess)
bool FrameSkip::Update(float fElapsedTime, int& outFrameCount) {
    if (m_fTimePerFrame <= 0.0001f) { // 避免除以零
        outFrameCount = 0;
        return false;
    }

    m_fAccumulatedTime += fElapsedTime;

    if (m_fAccumulatedTime >= m_fTimePerFrame) {
        outFrameCount = static_cast<int>(m_fAccumulatedTime / m_fTimePerFrame);
        m_fAccumulatedTime -= static_cast<float>(outFrameCount) * m_fTimePerFrame;
        return true;
    }

    outFrameCount = 0;
    return false;
}

//=============================================================================
// CCAEffect 類別實作
//=============================================================================

// 對應反組譯碼: 0x00528560
CCAEffect::CCAEffect()
{
    m_pEffectData = nullptr;
    m_fPosX = 0.0f;
    m_fPosY = 0.0f;
    m_bFlipX = false;
    m_fRotation = 0.0f;
    m_dwAlpha = 100;
    m_bIsPlaying = false;
    m_bShow = true;
    m_bIsLooping = false;
    m_nAnimationID = 0;
    m_nCurrentFrame = 0;
    m_nStartFrame = 0;
    m_nEndFrame = 0;
    m_pGameImages = nullptr;
    m_nMaxImagesInAnimation = 0;
    m_nImageCountInFrame = 0;
    m_pfnDrawRenderState = &CCAEffect::DrawRenderState;

    // 初始化 FrameSkip 成員
    // 原始碼: *((_DWORD *)this + 19) = 1015580809; (0x3C888889 -> 0.016666667f ~ 1/60)
    m_FrameSkip.m_fTimePerFrame = 1.0f / 60.0f;
    m_FrameSkip.m_fAccumulatedTime = 0.0f;
}

// 對應反組譯碼: 0x00528600
CCAEffect::~CCAEffect()
{
    delete[] m_pGameImages;
}

// 對應反組譯碼: 0x00528630
void CCAEffect::Reset()
{
    m_pEffectData = nullptr;
    m_fPosX = 0.0f;
    m_fPosY = 0.0f;
    m_bFlipX = false;
    m_fRotation = 0.0f;
    m_dwAlpha = 100;
    m_bIsPlaying = false;
    m_bShow = true;
    m_bIsLooping = false;
    m_nAnimationID = 0;
    m_nCurrentFrame = 0;
    m_nStartFrame = 0;
    m_nEndFrame = 0;

    delete[] m_pGameImages;
    m_pGameImages = nullptr;

    m_nMaxImagesInAnimation = 0;
    m_nImageCountInFrame = 0;
}

// 對應反組譯碼: 0x00528690
void CCAEffect::SetFrameTime()
{
    if (!m_pEffectData) return;

    // 原始碼: *((float *)this + 19) = 1.0 / (double)*(int *)(*((_DWORD *)this + 1) + 8);
    // 其中 this+1 是 m_pAnimationInfo，+8 是 m_nTotalFrames。此處假設它代表幀率。
    if (m_pEffectData->m_nTotalFrames > 0) {
        m_FrameSkip.m_fTimePerFrame = 1.0f / static_cast<float>(m_pEffectData->m_nTotalFrames);
    }

    // 原始碼: v1 = *(_DWORD *)(*((_DWORD *)this + 2) + 4);
    // 其中 this+2 是 m_pTimelineInfo，+4 是 m_nLayerCount
    m_nMaxImagesInAnimation = m_pEffectData->m_nLayerCount;
    if (m_nMaxImagesInAnimation > 0) {
        // 分配足夠的 GameImage 指標空間
        m_pGameImages = new (std::nothrow) GameImage * [m_nMaxImagesInAnimation];
    }
}

// 對應反組譯碼: 0x005286C0
bool CCAEffect::FrameProcess(float fElapsedTime)
{
    if (!m_bIsPlaying) {
        return false;
    }

    int frameCount = 0;
    if (m_FrameSkip.Update(fElapsedTime, frameCount)) {
        m_nCurrentFrame += frameCount;
        if (m_nCurrentFrame > m_nEndFrame) {
            if (m_bIsLooping) {
                m_nCurrentFrame = m_nStartFrame;
            }
            else {
                m_bIsPlaying = false;
                return true; // 動畫結束
            }
        }
    }
    return false;
}

/**
 * @brief 處理特效當前影格的邏輯，準備所有需要繪製的 GameImage 物件。
 * * 這是 CCAEffect 的核心函式之一。它根據 m_nCurrentFrame 的值，
 * 從 .ea 特效數據中讀取對應的圖層和影格資訊，然後：
 * 1. 從物件池中獲取 GameImage 實例。
 * 2. 讀取影格定義的頂點數據。
 * 3. 依次應用旋轉、平移和翻轉等矩陣變換。
 * 4. 將最終計算好的頂點數據傳遞給 GameImage 物件。
 * 5. 將準備好的 GameImage 物件加入到當前影格的繪製列表中。
 * * 此函式的行為精確模擬了 Effectall.c 中 0x00528740 處的原始邏輯。
 */
void CCAEffect::Process()
{
    // 檢查基本播放狀態，如果未播放、不顯示、沒有數據或影格為負，則直接返回。
    if (!m_bShow || !m_bIsPlaying || !m_pEffectData || m_nCurrentFrame < 0) {
        m_nImageCountInFrame = 0;
        return;
    }

    // 重設當前影格要繪製的圖片數量
    m_nImageCountInFrame = 0;

    // 遍歷此特效的所有圖層
    for (int i = 0; i < m_pEffectData->m_nLayerCount; ++i) {
        VERTEXANIMATIONLAYERINFO* pLayer = &m_pEffectData->m_pLayers[i];

        // 確保當前影格在此圖層的時間軸範圍內
        if (m_nCurrentFrame < pLayer->m_nFrameCount) {
            VERTEXANIMATIONFRAMEINFO* pFrame = &pLayer->m_pFrames[m_nCurrentFrame];

            // 如果該影格定義了一個有效的圖片資源 ID
            if (pFrame->m_dwImageID != 0) {

                // 從物件池獲取一個 GameImage 實例
                // 資源群組 7 通常對應 Effect
                GameImage* pImage = cltImageManager::GetInstance()->GetGameImage(7, pFrame->m_dwImageID, 0, 1);

                // 確保成功獲取且未超過最大圖片數量限制
                if (pImage && m_nImageCountInFrame < m_nMaxImagesInAnimation) {
                    // 將此 GameImage 加入到本幀的繪製列表中
                    m_pGameImages[m_nImageCountInFrame++] = pImage;

                    // 創建一個臨時頂點陣列，用於進行變換計算
                    GIVertex transformedVertices[4];
                    memcpy(transformedVertices, pFrame->m_Vertices, sizeof(transformedVertices));

                    // --- 開始頂點變換 ---

                    // 1. 旋轉 (Rotation)
                    if (m_fRotation != 0.0f) {
                        D3DXMATRIX matRotation;
                        D3DXMatrixRotationZ(&matRotation, m_fRotation);
                        for (int v = 0; v < 4; ++v) {
                            D3DXVECTOR3 pos = { transformedVertices[v].position_x, transformedVertices[v].position_y, 0.0f };
                            D3DXVec3TransformCoord(&pos, &pos, &matRotation);
                            transformedVertices[v].position_x = pos.x;
                            transformedVertices[v].position_y = pos.y;
                        }
                    }

                    // 2. 平移 (Translation) 與 翻轉 (Flip)
                    // 原始碼中將平移和翻轉合併處理，此處為忠實還原其邏輯
                    for (int v = 0; v < 4; ++v) {
                        // 首先加上特效自身的座標
                        transformedVertices[v].position_x += m_fPosX;
                        transformedVertices[v].position_y += m_fPosY;

                        // 如果設定了水平翻轉
                        if (m_bFlipX) {
                            // 原始碼邏輯: v16 = m_fPosX + m_fPosX; v18[0] = v16 - v18[0];
                            // 這等同於以 m_fPosX 所在的垂直線為對稱軸進行鏡像反射
                            transformedVertices[v].position_x = (2.0f * m_fPosX) - transformedVertices[v].position_x;
                        }
                    }

                    // --- 頂點變換結束 ---

                    // 將計算完成的頂點數據傳遞給 GameImage
                    pImage->VertexAnimationCalculator(transformedVertices);

                    // 設定其他渲染屬性
                    if (m_dwAlpha != 100) { // 原始碼中有此判斷
                        pImage->SetAlpha(m_dwAlpha);
                    }
                }
            }
        }
    }
}


// 對應反組譯碼: 0x005289A0
void CCAEffect::Draw()
{
    if (!m_bShow || !m_bIsPlaying || m_nImageCountInFrame == 0) return;

    // --- 關鍵變更 ---
    // 根據 m_ucRenderStateSelector 旗標來決定呼叫哪個函式指標
    if (m_ucRenderStateSelector == 0) {
        if (m_pfnDrawRenderState) {
            (this->*m_pfnDrawRenderState)();
        }
    }
    else {
        if (m_pfnDrawEtcRenderState) {
            (this->*m_pfnDrawEtcRenderState)();
        }
    }

    // 後續繪製 GameImage 的邏輯不變
    for (int i = 0; i < m_nImageCountInFrame; ++i) {
        GameImage* pImage = m_pGameImages[i];
        if (pImage && pImage->IsInUse()) {
            pImage->Draw();
        }
    }
}

// 對應反組譯碼: 0x00528A80
void CCAEffect::Play(int nAnimationID, bool bLoop)
{
    if (!m_pEffectData || nAnimationID >= m_pEffectData->m_nAnimationCount) return;

    m_nAnimationID = nAnimationID;
    m_bIsLooping = bLoop;

    KEYINFO* pKey = &m_pEffectData->m_pKeyFrames[nAnimationID];
    m_nStartFrame = pKey->m_nStartFrame;
    m_nEndFrame = pKey->m_nEndFrame;

    m_nCurrentFrame = m_nStartFrame;
    m_bIsPlaying = true;
}

// 對應反組譯碼: 0x00528AD0
void CCAEffect::Pause()
{
    m_bIsPlaying = !m_bIsPlaying;
}

// 對應反組譯碼: 0x00528AE0
void CCAEffect::LoadImageA()
{
    if (!m_pEffectData || m_nCurrentFrame < 0) return;

    for (int i = 0; i < m_pEffectData->m_nLayerCount; ++i) {
        VERTEXANIMATIONLAYERINFO* pLayer = &m_pEffectData->m_pLayers[i];
        if (m_nCurrentFrame < pLayer->m_nFrameCount) {
            VERTEXANIMATIONFRAMEINFO* pFrame = &pLayer->m_pFrames[m_nCurrentFrame];
            if (pFrame->m_dwImageID != 0) {
                cltImageManager::GetInstance()->GetGameImage(7, pFrame->m_dwImageID, 0, 1);
            }
        }
    }
}

// 對應反組譯碼: 0x005289E0
void CCAEffect::DrawRenderState()
{
    if (!m_pEffectData) return;
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, m_pEffectData->m_ucBlendOp);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, m_pEffectData->m_ucSrcBlend);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, m_pEffectData->m_ucDestBlend);
}

// 對應反組譯碼: 0x00528A40
void CCAEffect::DrawEtcRenderState()
{
    if (!m_pEffectData) return;
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, m_pEffectData->m_ucEtcBlendOp);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, m_pEffectData->m_ucEtcSrcBlend);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, m_pEffectData->m_ucEtcDestBlend);
}