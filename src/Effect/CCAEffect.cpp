#include "Effect/CCAEffect.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include <new> // for std::nothrow

//=============================================================================
// FrameSkip ���O��@
//=============================================================================

FrameSkip::FrameSkip() : m_fAccumulatedTime(0.0f), m_fTimePerFrame(1.0f / 60.0f) {}
FrameSkip::~FrameSkip() {}

// �޿���� Effectall.c @ 005286C0 (CCAEffect::FrameProcess)
bool FrameSkip::Update(float fElapsedTime, int& outFrameCount) {
    if (m_fTimePerFrame <= 0.0001f) { // �קK���H�s
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
// CCAEffect ���O��@
//=============================================================================

// �����ϲ�Ķ�X: 0x00528560
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

    // ��l�� FrameSkip ����
    // ��l�X: *((_DWORD *)this + 19) = 1015580809; (0x3C888889 -> 0.016666667f ~ 1/60)
    m_FrameSkip.m_fTimePerFrame = 1.0f / 60.0f;
    m_FrameSkip.m_fAccumulatedTime = 0.0f;
}

// �����ϲ�Ķ�X: 0x00528600
CCAEffect::~CCAEffect()
{
    delete[] m_pGameImages;
}

// �����ϲ�Ķ�X: 0x00528630
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

// �����ϲ�Ķ�X: 0x00528690
void CCAEffect::SetFrameTime()
{
    if (!m_pEffectData) return;

    // ��l�X: *((float *)this + 19) = 1.0 / (double)*(int *)(*((_DWORD *)this + 1) + 8);
    // �䤤 this+1 �O m_pAnimationInfo�A+8 �O m_nTotalFrames�C���B���]���N��V�v�C
    if (m_pEffectData->m_nTotalFrames > 0) {
        m_FrameSkip.m_fTimePerFrame = 1.0f / static_cast<float>(m_pEffectData->m_nTotalFrames);
    }

    // ��l�X: v1 = *(_DWORD *)(*((_DWORD *)this + 2) + 4);
    // �䤤 this+2 �O m_pTimelineInfo�A+4 �O m_nLayerCount
    m_nMaxImagesInAnimation = m_pEffectData->m_nLayerCount;
    if (m_nMaxImagesInAnimation > 0) {
        // ���t������ GameImage ���ЪŶ�
        m_pGameImages = new (std::nothrow) GameImage * [m_nMaxImagesInAnimation];
    }
}

// �����ϲ�Ķ�X: 0x005286C0
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
                return true; // �ʵe����
            }
        }
    }
    return false;
}

// �����ϲ�Ķ�X: 0x00528740
void CCAEffect::Process()
{
    if (!m_bShow || !m_bIsPlaying || !m_pEffectData || m_nCurrentFrame < 0) {
        m_nImageCountInFrame = 0;
        return;
    }

    m_nImageCountInFrame = 0;

    // �M���Ҧ��ϼh
    for (int i = 0; i < m_pEffectData->m_nLayerCount; ++i) {
        VERTEXANIMATIONLAYERINFO* pLayer = &m_pEffectData->m_pLayers[i];

        if (m_nCurrentFrame < pLayer->m_nFrameCount) {
            VERTEXANIMATIONFRAMEINFO* pFrame = &pLayer->m_pFrames[m_nCurrentFrame];

            if (pFrame->m_dwImageID != 0) {
                // �q�������� GameImage
                GameImage* pImage = cltImageManager::GetInstance()->GetGameImage(7, pFrame->m_dwImageID, 0, 1);

                if (pImage && m_nImageCountInFrame < m_nMaxImagesInAnimation) {
                    m_pGameImages[m_nImageCountInFrame++] = pImage;

                    GIVertex transformedVertices[4];
                    memcpy(transformedVertices, pFrame->m_Vertices, sizeof(transformedVertices));

                    // 1. �B�z����
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

                    // 2. �B�z��m�M½��
                    for (int v = 0; v < 4; ++v) {
                        transformedVertices[v].position_x += m_fPosX;
                        transformedVertices[v].position_y += m_fPosY;
                        if (m_bFlipX) {
                            // ��l�X��½���޿�������A���B��²�ƥܷN
                            // float tempX = transformedVertices[v].position_x;
                            // transformedVertices[v].position_x = (m_fPosX * 2.0f) - tempX;
                        }
                    }

                    // 3. �ǻ����I�ƾ�
                    pImage->VertexAnimationCalculator(transformedVertices);
                }
            }
        }
    }
}


// �����ϲ�Ķ�X: 0x005289A0
void CCAEffect::Draw()
{
    if (!m_bShow || !m_bIsPlaying || m_nImageCountInFrame == 0) return;

    if (m_pfnDrawRenderState) {
        (this->*m_pfnDrawRenderState)();
    }

    for (int i = 0; i < m_nImageCountInFrame; ++i) {
        GameImage* pImage = m_pGameImages[i];
        if (pImage && pImage->IsInUse()) {
            pImage->Draw();
        }
    }
}

// �����ϲ�Ķ�X: 0x00528A80
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

// �����ϲ�Ķ�X: 0x00528AD0
void CCAEffect::Pause()
{
    m_bIsPlaying = !m_bIsPlaying;
}

// �����ϲ�Ķ�X: 0x00528AE0
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

// �����ϲ�Ķ�X: 0x005289E0
void CCAEffect::DrawRenderState()
{
    if (!m_pEffectData) return;
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, m_pEffectData->m_ucBlendOp);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, m_pEffectData->m_ucSrcBlend);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, m_pEffectData->m_ucDestBlend);
}

// �����ϲ�Ķ�X: 0x00528A40
void CCAEffect::DrawEtcRenderState()
{
    if (!m_pEffectData) return;
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, m_pEffectData->m_ucEtcBlendOp);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, m_pEffectData->m_ucEtcSrcBlend);
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, m_pEffectData->m_ucEtcDestBlend);
}