#include "Image/BackgroundImage.h"
#include "Image/CDeviceManager.h"
#include "Image/CDeviceResetManager.h"
#include <cstring> // for memcpy

// --- �~���̿઺�����ܼ� ---
extern LPDIRECT3DDEVICE9    Device;
extern bool                 DontDraw; // ���쪺 "���nø�s" �X��

BackgroundImage::BackgroundImage()
{
    // �N�Ҧ����ЩM�ƭȦ�����l�Ƭ� 0 �� nullptr
    Reset();

    // ��l�ƥ��a���I�֨�
    for (int i = 0; i < 4; ++i)
    {
        // �ھڤϽsĶ�{���X���޿��l��
        m_imageVertices[i] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
        m_giVertices[i] = GIVertex(); // �ϥ� GIVertex ���w�]�غc�禡
    }
}

BackgroundImage::~BackgroundImage()
{
    // �b�Ѻc������ D3D �귽
    if (m_pTexData) {
        CDeviceResetManager::GetInstance()->DeleteTexture(m_pTexData);
    }
    if (m_pVBData) {
        CDeviceResetManager::GetInstance()->DeleteVertexBuffer(m_pVBData);
    }
}

void BackgroundImage::Reset()
{
    // ���� D3D �귽
    if (m_pTexData) {
        CDeviceResetManager::GetInstance()->DeleteTexture(m_pTexData);
        m_pTexData = nullptr;
    }
    if (m_pVBData) {
        CDeviceResetManager::GetInstance()->DeleteVertexBuffer(m_pVBData);
        m_pVBData = nullptr;
    }

    // ���]�Ҧ��ƭȦ���
    m_fTexWidth = 0.0f;
    m_fTexHeight = 0.0f;
    m_fImgWidth = 0.0f;
    m_fImgHeight = 0.0f;
    m_fU_End = 0.0f;
    m_fV_End = 0.0f;
    m_fPosX = 0.0f;
    m_fPosY = 0.0f;
    m_fU_Start = 0.0f;
    m_fV_Start = 0.0f;
}

void BackgroundImage::CreateImage(const char* szFilename, float imgWidth, float imgHeight, float texWidth, float texHeight)
{
    Reset(); // ���M���¸귽

    m_pTexData = CDeviceResetManager::GetInstance()->CreateTexture(szFilename, 0);
    m_pVBData = CDeviceResetManager::GetInstance()->CreateVertexBuffer(4, 2); // ���� 2: ImageVertex

    m_fImgWidth = imgWidth;
    m_fImgHeight = imgHeight;
    m_fTexWidth = texWidth;
    m_fTexHeight = texHeight;

    m_fU_Start = 0.0f;
    m_fU_End = m_fImgWidth / m_fTexWidth;
    m_fV_End = m_fImgHeight / m_fTexHeight;
    m_fV_Start = 1.0f - m_fV_End; // V �y�бq�����}�l�p��
}

void BackgroundImage::CreateBlackBG(float x, float y, float width, float height)
{
    Reset(); // ���M���¸귽

    m_pVBData = CDeviceResetManager::GetInstance()->CreateVertexBuffer(4, 0); // ���� 0: GIVertex
    if (!m_pVBData) return;

    // �]�w�|�ӳ��I���ù��y�ЩM�C��
    float left = x - 0.5f;
    float top = y - 0.5f;
    float right = x + width - 0.5f;
    float bottom = y + height - 0.5f;
    const DWORD blackColor = 0xFF000000;

    m_giVertices[0].position_x = left;
    m_giVertices[0].position_y = top;
    m_giVertices[0].diffuse_color = blackColor;

    m_giVertices[1].position_x = right;
    m_giVertices[1].position_y = top;
    m_giVertices[1].diffuse_color = blackColor;

    m_giVertices[2].position_x = right;
    m_giVertices[2].position_y = bottom;
    m_giVertices[2].diffuse_color = blackColor;

    m_giVertices[3].position_x = left;
    m_giVertices[3].position_y = bottom;
    m_giVertices[3].diffuse_color = blackColor;

    // �N���I��ƤW�Ǩ� GPU
    if (m_pVBData && m_pVBData->pVertexBuffer) {
        void* pV = nullptr;
        if (SUCCEEDED(m_pVBData->pVertexBuffer->Lock(0, 0, &pV, 0))) {
            memcpy(pV, m_giVertices, sizeof(m_giVertices));
            m_pVBData->pVertexBuffer->Unlock();
        }
    }
}

void BackgroundImage::SetPosition(float x, float y)
{
    m_fPosX = x;
    m_fPosY = y;
}

bool BackgroundImage::SetPositionUP(float delta)
{
    m_fV_Start -= (delta / m_fTexHeight);
    if (m_fV_Start < 0.0f) {
        m_fV_Start = 0.0f;
        return true; // �w�F����
    }
    return false;
}

bool BackgroundImage::SetPositionDOWN(float delta)
{
    m_fV_Start += (delta / m_fTexHeight);
    float maxV = 1.0f - m_fV_End;
    if (m_fV_Start > maxV) {
        m_fV_Start = maxV;
        return true; // �w�F����
    }
    return false;
}

void BackgroundImage::Process()
{
    if (!m_pVBData) return;

    // �ھڦ�m�M UV �_�I�p��̲ת����I���
    float left = m_fPosX - 0.5f;
    float top = m_fPosY - 0.5f;
    float right = left + m_fImgWidth;
    float bottom = top + m_fImgHeight;

    m_imageVertices[0] = { left,  top,    0.5f, 1.0f, m_fU_Start, m_fV_Start };
    m_imageVertices[1] = { right, top,    0.5f, 1.0f, m_fU_End,   m_fV_Start };
    m_imageVertices[2] = { right, bottom, 0.5f, 1.0f, m_fU_End,   m_fV_End + m_fV_Start };
    m_imageVertices[3] = { left,  bottom, 0.5f, 1.0f, m_fU_Start, m_fV_End + m_fV_Start };

    // �N���I��ƤW�Ǩ� GPU
    if (m_pVBData && m_pVBData->pVertexBuffer) {
        void* pV = nullptr;
        if (SUCCEEDED(m_pVBData->pVertexBuffer->Lock(0, 0, &pV, 0))) {
            memcpy(pV, m_imageVertices, sizeof(m_imageVertices));
            m_pVBData->pVertexBuffer->Unlock();
        }
    }
}

void BackgroundImage::Render()
{
    if (DontDraw || !m_pVBData || !m_pTexData) return;

    CDeviceManager::GetInstance()->SetTexture(0, m_pTexData->pTexture);
    CDeviceManager::GetInstance()->SetStreamSource(0, m_pVBData->pVertexBuffer, 0, sizeof(ImageVertex));
    CDeviceManager::GetInstance()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1); // FVF: 0x104
    Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
}

void BackgroundImage::RenderBlackBG()
{
    if (DontDraw || !m_pVBData) return;

    CDeviceManager::GetInstance()->SetTexture(0, nullptr); // ���ϥί��z
    CDeviceManager::GetInstance()->SetStreamSource(0, m_pVBData->pVertexBuffer, 0, sizeof(GIVertex));
    // FVF ���� D3DFVF_XYZRHW | D3DFVF_DIFFUSE (0x44) �Υ]�t���z�y�Ъ� 0x144
    // ���������l�X�A�o�̨ϥΥ��O���� 0x104
    CDeviceManager::GetInstance()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
    Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
}