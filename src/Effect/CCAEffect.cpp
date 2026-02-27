#include "Effect/CCAEffect.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"

#include <new>
#include <cstring>
#include <cmath>
#include <limits>

//=============================================================================
// FrameSkip（只用於 layout + 預設值；CCAEffect::FrameProcess 直接操作其欄位）
//=============================================================================
FrameSkip::FrameSkip()
    : m_fAccumulatedTime(0.0f)
    , m_fTimePerFrame(1.0f / 60.0f) // IDA：1015580809 -> 0.0166666675
{
}

FrameSkip::~FrameSkip() = default;

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
// CCAEffect：依 IDA 逐行重建
//=============================================================================

void CCAEffect::RenderState_Draw(CCAEffect* self)
{
    if (self) self->DrawRenderState();
}
void CCAEffect::RenderState_DrawEtc(CCAEffect* self)
{
    if (self) self->DrawEtcRenderState();
}

CCAEffect::CCAEffect()
{
    m_pEffectData = nullptr;   // +1
    m_pLayerList = nullptr;   // +2
    m_fPosX = 0.0f;            // +3
    m_fPosY = 0.0f;            // +4
    m_bFlipX = 0;              // +20
    m_pad21 = 0;
    m_pad22_23 = 0;
    m_fRotation = 0.0f;        // +6
    m_nScale = 100;            // +7

    m_bLoop = 0;               // +32
    m_bIsPlaying = 0;          // +33
    m_bVisible = 1;            // +34
    m_pad35 = 0;

    m_nAnimationID = 0;        // +9
    m_nUnknown10 = 0;          // +10
    m_nCurrentFrame = 0;       // +11
    m_nStartFrameIndex = 0;    // +12
    m_nEndFrameIndex = 0;      // +13

    m_ppGameImages = nullptr;  // +14
    m_nMaxImages = 0;          // +15
    m_nActiveImageCount = 0;   // +16

    // render bytes (+80~85)
    m_ucBlendIndex = 0;
    m_ucUnk81 = 0;
    m_ucEtcBlendOp = 0;
    m_ucEtcSrcBlend = 0;
    m_ucEtcDestBlend = 0;
    m_ucRenderMode = 0;
    m_pad86_87 = 0;

    // IDA：把兩個 render state 函式地址寫入 +22/+23
    m_pRenderStateFn[0] = &CCAEffect::RenderState_Draw;
    m_pRenderStateFn[1] = &CCAEffect::RenderState_DrawEtc;
}

CCAEffect::~CCAEffect()
{
    // IDA：若 m_ppGameImages 非空，operator delete，並清 0
    if (m_ppGameImages)
    {
        ::operator delete(m_ppGameImages);
        m_ppGameImages = nullptr;
    }
}

void CCAEffect::Reset()
{
    void* old = m_ppGameImages;

    m_pEffectData = nullptr;
    m_pLayerList = nullptr;
    m_fPosX = 0.0f;
    m_fPosY = 0.0f;
    m_bFlipX = 0;

    m_fRotation = 0.0f;
    m_nScale = 100;

    m_bLoop = 0;
    m_bIsPlaying = 0;
    m_bVisible = 1;

    m_nAnimationID = 0;
    m_nUnknown10 = 0;
    m_nCurrentFrame = 0;
    m_nStartFrameIndex = 0;
    m_nEndFrameIndex = 0;

    if (old)
    {
        ::operator delete(old);
        m_ppGameImages = nullptr;
    }

    m_nActiveImageCount = 0;
    m_nMaxImages = 0;

    // FrameSkip 累積時間在 IDA Reset() 片段中未重設，故此處不動
}

void CCAEffect::SetFrameTime()
{
    if (!m_pEffectData) return;

    const int totalFrames = m_pEffectData->totalFrames;
    if (totalFrames > 0)
        m_FrameSkip.m_fTimePerFrame = 1.0f / static_cast<float>(totalFrames);
    else
        m_FrameSkip.m_fTimePerFrame = 0.0f;

    if (!m_pLayerList) return;

    m_nMaxImages = m_pLayerList->m_nLayerCount;
    if (m_nMaxImages > 0)
        m_ppGameImages = static_cast<GameImage**>(::operator new(sizeof(GameImage*) * m_nMaxImages));
    else
        m_ppGameImages = nullptr;
}

bool CCAEffect::FrameProcess(float fDeltaTime)
{
    float t = fDeltaTime + m_FrameSkip.m_fAccumulatedTime;
    const bool notEnough = (t < m_FrameSkip.m_fTimePerFrame);
    m_FrameSkip.m_fAccumulatedTime = t;

    int framesToAdvance = 0;
    if (!notEnough && m_FrameSkip.m_fTimePerFrame > 0.0f)
    {
        framesToAdvance = static_cast<int>(t / m_FrameSkip.m_fTimePerFrame);
        if (framesToAdvance)
            m_FrameSkip.m_fAccumulatedTime = t - static_cast<float>(framesToAdvance) * m_FrameSkip.m_fTimePerFrame;
    }

    const int prev = m_nCurrentFrame;
    const int next = prev + framesToAdvance;
    m_nCurrentFrame = next;

    if (next < 0)
        return true;

    if (m_nEndFrameIndex > next)
        return false;

    const uint8_t loop = m_bLoop;
    m_nCurrentFrame = m_nStartFrameIndex;
    return loop == 0; // IDA：return !m_bLoop
}

void CCAEffect::Process()
{
    if (m_nCurrentFrame < 0)
        return;

    m_nActiveImageCount = 0;

    if (!m_bIsPlaying)
        return;

    float v18[28];
    {
        float* p = &v18[2];
        int k = 4;
        do
        {
            p[-2] = 0.0f;
            p[-1] = 0.0f;
            p[0] = 0.5f;
            p[1] = 1.0f;
            p[2] = std::numeric_limits<float>::quiet_NaN();
            p[3] = 0.0f;
            p[4] = 0.0f;
            p += 7;
            --k;
        } while (k);
    }

    if (!m_pLayerList || !m_pLayerList->m_pLayers)
        return;

    const int layerCount = m_pLayerList->m_nLayerCount;

    for (int layerIdx = 0; layerIdx < layerCount; ++layerIdx)
    {
        VERTEXANIMATIONLAYERINFO& layer = m_pLayerList->m_pLayers[layerIdx];

        if (layer.m_nFrameCount > m_nCurrentFrame)
        {
            VERTEXANIMATIONFRAMEINFO* frames = layer.m_pFrames;

            if (!frames) continue;

            const uint32_t imageId = *reinterpret_cast<uint32_t*>(
                reinterpret_cast<uint8_t*>(frames) + sizeof(VERTEXANIMATIONFRAMEINFO) * m_nCurrentFrame + 0);

            if (imageId == 0) continue;

            GameImage* img = cltImageManager::GetInstance()->GetGameImage(7u, imageId, 0, 1);

            if (m_ppGameImages)
                m_ppGameImages[m_nActiveImageCount] = img;

            if (!img)
                continue;

            if (m_nScale != 100)
            {
                img->m_nScale = m_nScale; // +376
                *reinterpret_cast<uint8_t*>(reinterpret_cast<uint8_t*>(img) + 449) = 1;
                *reinterpret_cast<uint8_t*>(reinterpret_cast<uint8_t*>(img) + 444) = 0;
            }

            std::memcpy(
                v18,
                reinterpret_cast<uint8_t*>(frames) + sizeof(VERTEXANIMATIONFRAMEINFO) * m_nCurrentFrame + 4,
                sizeof(v18));

            if (m_fRotation != 0.0f)
            {
                D3DXMATRIX matRot;
                D3DXMatrixRotationZ(&matRot, m_fRotation);
                D3DXVec3TransformCoord((D3DXVECTOR3*)&v18[0], (D3DXVECTOR3*)&v18[0], &matRot);
                D3DXVec3TransformCoord((D3DXVECTOR3*)&v18[7], (D3DXVECTOR3*)&v18[7], &matRot);
                D3DXVec3TransformCoord((D3DXVECTOR3*)&v18[14], (D3DXVECTOR3*)&v18[14], &matRot);
                D3DXVec3TransformCoord((D3DXVECTOR3*)&v18[21], (D3DXVECTOR3*)&v18[21], &matRot);
            }

            v18[0] += m_fPosX; v18[1] += m_fPosY;
            v18[7] += m_fPosX; v18[8] += m_fPosY;
            v18[14] += m_fPosX; v18[15] += m_fPosY;
            v18[21] += m_fPosX; v18[22] += m_fPosY;

            if (m_bFlipX)
            {
                const float twoX = m_fPosX + m_fPosX;
                v18[0] = twoX - v18[0];
                v18[7] = twoX - v18[7];
                v18[14] = twoX - v18[14];
                v18[21] = twoX - v18[21];
            }

            img->VertexAnimationCalculator(reinterpret_cast<const GIVertex*>(v18));

            ++m_nActiveImageCount;
        }
    }
}

void CCAEffect::Draw()
{
    if (!m_bVisible)
        return;

    const uint8_t mode = m_ucRenderMode;
    if (mode < 2 && m_pRenderStateFn[mode])
        m_pRenderStateFn[mode](this);

    for (int i = 0; i < m_nActiveImageCount; ++i)
    {
        GameImage* img = m_ppGameImages ? m_ppGameImages[i] : nullptr;
        if (!img)
            continue;

        // IDA：if (*((DWORD*)img + 2)) Draw
        if (img->m_pGIData)
            img->Draw();
    }
}

void CCAEffect::DrawRenderState()
{
    const uint8_t idx = m_ucBlendIndex;

    CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, D3DRS_BLENDOP_TYPE[idx]);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DRS_SRCBLEND_TYPE[idx]);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DRS_DESTBLEND_TYPE[idx]);
}

void CCAEffect::DrawEtcRenderState()
{
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, m_ucEtcBlendOp);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, m_ucEtcSrcBlend);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, m_ucEtcDestBlend);
}

void CCAEffect::Play(int nAnimationID, bool bLoop)
{
    if (!m_pEffectData) return;

    if (m_pEffectData->animationCount > nAnimationID)
    {
        m_bIsPlaying = 1;
        m_bLoop = 0;

        m_nAnimationID = nAnimationID;

        KEYINFO* keys = m_pEffectData->keys;
        if (keys)
        {
            m_nStartFrameIndex = *reinterpret_cast<int*>(reinterpret_cast<uint8_t*>(keys) + sizeof(KEYINFO) * nAnimationID + 20);
            m_nEndFrameIndex = *reinterpret_cast<int*>(reinterpret_cast<uint8_t*>(keys) + sizeof(KEYINFO) * nAnimationID + 24);
        }
        else
        {
            m_nStartFrameIndex = 0;
            m_nEndFrameIndex = 0;
        }

        m_bLoop = bLoop ? 1u : 0u;
    }
}

void CCAEffect::Pause()
{
    m_bIsPlaying = (m_bIsPlaying == 0) ? 1u : 0u;
}

void CCAEffect::LoadImageA()
{
    m_nActiveImageCount = 0;

    if (!m_bIsPlaying)
        return;

    if (!m_pLayerList || !m_pLayerList->m_pLayers || !m_ppGameImages)
        return;

    const int layerCount = m_pLayerList->m_nLayerCount;

    for (int i = 0; i < layerCount; ++i)
    {
        VERTEXANIMATIONLAYERINFO& layer = m_pLayerList->m_pLayers[i];

        if (layer.m_nFrameCount > m_nCurrentFrame)
        {
            VERTEXANIMATIONFRAMEINFO* frames = layer.m_pFrames;
            if (!frames) continue;

            const uint32_t imageId = *reinterpret_cast<uint32_t*>(
                reinterpret_cast<uint8_t*>(frames) + 124 * m_nCurrentFrame + 0);

            if (imageId)
            {
                m_ppGameImages[m_nActiveImageCount++] =
                    cltImageManager::GetInstance()->GetGameImage(7u, imageId, 0, 1);
            }
        }
    }
}
