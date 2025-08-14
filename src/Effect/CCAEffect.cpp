#include "Effect/CCAEffect.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include <new> // for std::nothrow

//=============================================================================
// FrameSkip ���O��@ (��������)
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
// CCAEffect ���O��@ (���g����)
//=============================================================================

CCAEffect::CCAEffect()
{
    // ��l�ƩҦ������ܼƬ����b����l���A
    m_pEffectData = nullptr;
    m_fPosX = 0.0f;
    m_fPosY = 0.0f;
    m_bFlipX = false;
    m_fRotation = 0.0f;
    m_dwAlpha = 255; // �ϥ� 0-255 �d��
    m_bIsPlaying = false;
    m_bShow = true;
    m_bIsLooping = false;
    m_nAnimationID = 0;
    m_nCurrentFrame = 0;
    m_nStartFrame = 0;
    m_nEndFrame = 0;

    // �]�w�w�]����t�׬� 60 FPS
    m_FrameSkip.m_fTimePerFrame = 1.0f / 30.0f;
    m_FrameSkip.m_fAccumulatedTime = 0.0f;

    // ��l�ƴ�V���A���禡����
    m_pfnDrawRenderState = &CCAEffect::DrawRenderState;
    m_pfnDrawEtcRenderState = &CCAEffect::DrawEtcRenderState;
}

CCAEffect::~CCAEffect()
{
    // �b�o�ӷs���]�p���ACCAEffect ���A���� GameImage ���������СA
    // �]���Ѻc�禡�����A�ݭn���� GameImage ���޿�C
    // GameImage �����񧹥��� CEffect_Battle_DownCut ���S�Ī��󪺸Ѻc�禡Ĳ�o�C
}

void CCAEffect::Reset()
{
    // ���]�Ҧ����A�A�P�غc�禡�޿�@�P
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
    // ĵ�i�G�o�Ө禡����l�޿� (1.0f / �`�v���) �O�����D���A
    // ���|�ɭP�ʵe����t�פ�í�w�C
    // ��ĳ���n�I�s���禡�A�ӬO���t�Ψϥιw�]�� 60 FPS ����t�v�C
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

        // ���T���P������G
        // �ʵe�v����ެ� 0 �� (N-1)�C�� m_nCurrentFrame �W�[�� N �ɡA
        // �N�� 0 �� (N-1) ���Ҧ��v�泣�w���񧹲��C
        // �]���A`>` �O���T���P�_�l�C
        if (m_nCurrentFrame > m_nEndFrame) {
            if (m_bIsLooping) {
                m_nCurrentFrame = m_nStartFrame; // �`������
            }
            else {
                m_bIsPlaying = false;
                return true; // �ʵe�����A�^�� true �HĲ�o�P��
            }
        }
    }
    return false; // �ʵe���b����A�^�� false
}

void CCAEffect::Process()
{
    // �b�s���]�p���AProcess �禡�u�t�d��s�Dø�Ϭ������޿�C
    // �Ҧ��P GameImage �������p�ⳣ���� Draw �禡���A�H�קK���A�ìV�C
    // �ثe���禡�i�H���šA�Υu���̰򥻪����A�ˬd�C
    if (!m_bShow || !m_bIsPlaying || !m_pEffectData || m_nCurrentFrame < 0) {
        m_bShow = false; // �]�w�@�ӺX�СA�� Draw �禡���D���V����ø�s
    }
    else {
        m_bShow = true;
    }
}

void CCAEffect::Draw()
{
    // 1. �i��ø�s�e���̲��ˬd
    if (!m_bShow || !m_bIsPlaying || !m_pEffectData || m_nCurrentFrame < 0 || m_nCurrentFrame > m_nEndFrame) {
        return;
    }

    // 2. �M���ϼh�A�ǳ�ø�s
    // (��� .ea �榡�A�q�`�u���@�ӹϼh)
    for (int i = 0; i < m_pEffectData->m_nLayerCount; ++i) {
        VERTEXANIMATIONLAYERINFO* pLayer = &m_pEffectData->m_pLayers[i];
        if (m_nCurrentFrame >= pLayer->m_nFrameCount) continue;

        VERTEXANIMATIONFRAMEINFO* pFrame = &pLayer->m_pFrames[m_nCurrentFrame];
        if (pFrame->m_dwImageID == 0) continue;

        // 3. �bø�s���o�@��A�~�V�޲z���u�ɥΡv�@�� GameImage ����
        GameImage* pImage = cltImageManager::GetInstance()->GetGameImage(7, pFrame->m_dwImageID, 0, 1);

        if (pImage) {
            // 4. �p�⳻�I
            GIVertex transformedVertices[4];
            memcpy(transformedVertices, pFrame->m_Vertices, sizeof(transformedVertices));

            // �i��G�мg Alpha �ȡA�קK�S�Ħ]�ɮ׳]�p�Ӵ��e�H�X
            for (int v = 0; v < 4; ++v) {
                if (((transformedVertices[v].diffuse_color >> 24) & 0xFF) > 0) {
                    transformedVertices[v].diffuse_color = (transformedVertices[v].diffuse_color & 0x00FFFFFF) | (m_dwAlpha << 24);
                }
            }

            // 5. ���α���B½��B�������ܴ�
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

            // 6. �N�p��n�����I�`�J GameImage �äW�Ǩ� GPU
            pImage->VertexAnimationCalculator(transformedVertices);
            pImage->UpdateVertexBuffer();

            Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

            // 7. �]�w��V���A�éI�sø�s

			printf("CCAEffect::Draw: AnimationID=%d, Frame=%d, Layer=%d\n", m_nAnimationID, m_nCurrentFrame, i);
            // --- ���q 2: Alpha �V�X���A ---
            Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
            Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            // --- ����ץ��G���q 1: ���z�V�X���A ---
            // �j��ϥμзǪ������k���Ҧ��A�Ӥ��O�|�ɭP�o�������[�k���Ҧ�
            Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

            // �P�ɤ]���T�]�w Alpha ���V�X�覡
            Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

            if (m_ucRenderStateSelector == 0) {
                (this->*m_pfnDrawRenderState)();
            }
            else {
                (this->*m_pfnDrawEtcRenderState)();
            }

            // --- ��L�T�O�ʳ]�w ---
            Device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
            Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            Setup2DState(Device);
            Device->SetFVF(GIVertex::FVF);
            pImage->Draw();

            // �`�N�GGameImage ���k�١A�ѫ��� CCAEffect ���~���S�����O
            // (�p CEffect_Battle_DownCut) �b��Ѻc��Ĳ�o�A�o�O�ثe�[�c���]�p�C
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