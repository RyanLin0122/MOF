#include "Effect/CCAEffect.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include <new> // for std::nothrow

//=============================================================================
// FrameSkip 類別實作 (維持不變)
//=============================================================================
FrameSkip::FrameSkip() : m_fAccumulatedTime(0.0f), m_fTimePerFrame(1.0f / 60.0f) {}
FrameSkip::~FrameSkip() {}

bool FrameSkip::Update(float fElapsedTime, int& outFrameCount) {
    if (m_fTimePerFrame <= 0.0001f) {
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
// CCAEffect 類別實作 (重寫版本)
//=============================================================================

CCAEffect::CCAEffect()
{
    // 初始化所有成員變數為乾淨的初始狀態
    m_pEffectData = nullptr;
    m_fPosX = 0.0f;
    m_fPosY = 0.0f;
    m_bFlipX = false;
    m_fRotation = 0.0f;
    m_dwAlpha = 255; // 使用 0-255 範圍
    m_bIsPlaying = false;
    m_bShow = true;
    m_bIsLooping = false;
    m_nAnimationID = 0;
    m_nCurrentFrame = 0;
    m_nStartFrame = 0;
    m_nEndFrame = 0;

    // 設定預設播放速度為 60 FPS
    m_FrameSkip.m_fTimePerFrame = 1.0f / 30.0f;
    m_FrameSkip.m_fAccumulatedTime = 0.0f;

    // 初始化渲染狀態的函式指標
    m_pfnDrawRenderState = &CCAEffect::DrawRenderState;
    m_pfnDrawEtcRenderState = &CCAEffect::DrawEtcRenderState;
}

CCAEffect::~CCAEffect()
{
    // 在這個新的設計中，CCAEffect 不再持有 GameImage 的長期指標，
    // 因此解構函式中不再需要釋放 GameImage 的邏輯。
    // GameImage 的釋放完全由 CEffect_Battle_DownCut 等特效物件的解構函式觸發。
}

void CCAEffect::Reset()
{
    // 重設所有狀態，與建構函式邏輯一致
    m_pEffectData = nullptr;
    m_fPosX = 0.0f;
    m_fPosY = 0.0f;
    m_bFlipX = false;
    m_fRotation = 0.0f;
    m_dwAlpha = 255;
    m_bIsPlaying = false;
    m_bShow = true;
    m_bIsLooping = false;
    m_nAnimationID = 0;
    m_nCurrentFrame = 0;
    m_nStartFrame = 0;
    m_nEndFrame = 0;
}

void CCAEffect::SetFrameTime()
{
    // 警告：這個函式的原始邏輯 (1.0f / 總影格數) 是有問題的，
    // 它會導致動畫播放速度不穩定。
    // 建議不要呼叫此函式，而是讓系統使用預設的 60 FPS 播放速率。
    if (!m_pEffectData) return;

    //if (m_pEffectData->m_nTotalFrames > 0) {
    //    m_FrameSkip.m_fTimePerFrame = 1.0f / static_cast<float>(m_pEffectData->m_nTotalFrames);
    //}
}

bool CCAEffect::FrameProcess(float fElapsedTime)
{
    if (!m_bIsPlaying) {
        return false;
    }

    int frameCount = 0;
    if (m_FrameSkip.Update(fElapsedTime, frameCount)) {
        m_nCurrentFrame += frameCount;

        // 正確的銷毀條件：
        // 動畫影格索引為 0 到 (N-1)。當 m_nCurrentFrame 增加到 N 時，
        // 代表 0 到 (N-1) 的所有影格都已播放完畢。
        // 因此，`>` 是正確的判斷子。
        if (m_nCurrentFrame > m_nEndFrame) {
            if (m_bIsLooping) {
                m_nCurrentFrame = m_nStartFrame; // 循環播放
            }
            else {
                m_bIsPlaying = false;
                return true; // 動畫結束，回傳 true 以觸發銷毀
            }
        }
    }
    return false; // 動畫仍在播放，回傳 false
}

void CCAEffect::Process()
{
    // 在新的設計中，Process 函式只負責更新非繪圖相關的邏輯。
    // 所有與 GameImage 相關的計算都移至 Draw 函式中，以避免狀態污染。
    // 目前此函式可以為空，或只做最基本的狀態檢查。
    if (!m_bShow || !m_bIsPlaying || !m_pEffectData || m_nCurrentFrame < 0) {
        m_bShow = false; // 設定一個旗標，讓 Draw 函式知道本幀不需繪製
    }
    else {
        m_bShow = true;
    }
}

void CCAEffect::Draw()
{
    // 1. 進行繪製前的最終檢查
    if (!m_bShow || !m_bIsPlaying || !m_pEffectData || m_nCurrentFrame < 0 || m_nCurrentFrame > m_nEndFrame) {
        return;
    }

    // 2. 遍歷圖層，準備繪製
    // (對於 .ea 格式，通常只有一個圖層)
    for (int i = 0; i < m_pEffectData->m_nLayerCount; ++i) {
        VERTEXANIMATIONLAYERINFO* pLayer = &m_pEffectData->m_pLayers[i];
        if (m_nCurrentFrame >= pLayer->m_nFrameCount) continue;

        VERTEXANIMATIONFRAMEINFO* pFrame = &pLayer->m_pFrames[m_nCurrentFrame];
        if (pFrame->m_dwImageID == 0) continue;

        // 3. 在繪製的這一刻，才向管理器「借用」一個 GameImage 物件
        GameImage* pImage = cltImageManager::GetInstance()->GetGameImage(7, pFrame->m_dwImageID, 0, 1);

        if (pImage) {
            // 4. 計算頂點
            GIVertex transformedVertices[4];
            memcpy(transformedVertices, pFrame->m_Vertices, sizeof(transformedVertices));

            // 可選：覆寫 Alpha 值，避免特效因檔案設計而提前淡出
            for (int v = 0; v < 4; ++v) {
                if (((transformedVertices[v].diffuse_color >> 24) & 0xFF) > 0) {
                    transformedVertices[v].diffuse_color = (transformedVertices[v].diffuse_color & 0x00FFFFFF) | (m_dwAlpha << 24);
                }
            }

            // 5. 應用旋轉、翻轉、平移等變換
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
            for (int v = 0; v < 4; ++v) {
                transformedVertices[v].position_x += m_fPosX;
                transformedVertices[v].position_y += m_fPosY;
                if (m_bFlipX) {
                    transformedVertices[v].position_x = (2.0f * m_fPosX) - transformedVertices[v].position_x;
                }
            }

            // 6. 將計算好的頂點注入 GameImage 並上傳到 GPU
            pImage->VertexAnimationCalculator(transformedVertices);
            pImage->UpdateVertexBuffer();

            Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

            // 7. 設定渲染狀態並呼叫繪製

			printf("CCAEffect::Draw: AnimationID=%d, Frame=%d, Layer=%d\n", m_nAnimationID, m_nCurrentFrame, i);
            // --- 階段 2: Alpha 混合狀態 ---
            Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            // --- 關鍵修正：階段 1: 紋理混合狀態 ---
            // 強制使用標準的“乘法”模式，而不是會導致發光的“加法”模式
            Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

            // 同時也明確設定 Alpha 的混合方式
            Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

            if (m_ucRenderStateSelector == 0) {
                (this->*m_pfnDrawRenderState)();
            }
            else {
                (this->*m_pfnDrawEtcRenderState)();
            }

            // --- 其他確保性設定 ---
            Device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
            Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            Setup2DState(Device);
            Device->SetFVF(GIVertex::FVF);
            pImage->Draw();

            // 注意：GameImage 的歸還，由持有 CCAEffect 的外部特效類別
            // (如 CEffect_Battle_DownCut) 在其解構時觸發，這是目前架構的設計。
        }
    }
}

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
    m_bShow = true;
}

void CCAEffect::Pause()
{
    m_bIsPlaying = !m_bIsPlaying;
}

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

void CCAEffect::DrawRenderState()
{
    if (!m_pEffectData) return;
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, m_pEffectData->m_ucBlendOp);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, m_pEffectData->m_ucSrcBlend);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, m_pEffectData->m_ucDestBlend);
}

void CCAEffect::DrawEtcRenderState()
{
    if (!m_pEffectData) return;
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, m_pEffectData->m_ucEtcBlendOp);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, m_pEffectData->m_ucEtcSrcBlend);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, m_pEffectData->m_ucEtcDestBlend);
}

//For Debug
void CCAEffect::Setup2DState(IDirect3DDevice9* Device) {
    Device->SetRenderState(D3DRS_LIGHTING, FALSE);
    Device->SetRenderState(D3DRS_ZENABLE, FALSE);
    Device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

    Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE); //D3DBLEND_INVSRCCOLOR D3DBLEND_ONE

    Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    Device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    Device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    Device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    Device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    if (colorNum > 1) {
        Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
    }
}