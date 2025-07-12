#include "Image/Image.h"
#include "Image/CDeviceManager.h"       // ���] CDeviceManager ���w�q�b��

// --- �~���̿઺�����ܼ� ---
extern LPDIRECT3DDEVICE9    Device;
extern bool                 DontDraw = 0; // ���쪺 "���nø�s" �X��

Image::Image()
{
    // ��l�ƫ��ЩM�X��
    m_pVBData = nullptr;
    m_pTexData = nullptr;
    m_bIsCreated = false;

    // ��l�Ʀ����ܼ�
    m_nGridX = 0;
    m_fFrameWidth = 0.0f;
    m_fFrameHeight = 0.0f;
    m_fGridWidth = 0.0f;
    m_fGridHeight = 0.0f;
    m_nWord_28 = 0;
    g_Device_Reset_Manager = CDeviceResetManager::GetInstance();
    // ��l�Ƴ��I�}�C�M��¦���I�}�C
    for (int i = 0; i < 4; ++i)
    {
        m_vertices[i] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
        m_baseVertices[i] = { 0.0f, 0.0f, 0.0f };
    }
}

Image::~Image()
{
    // �b�Ѻc�ɩI�s Free ������귽
    this->Free();
}

bool Image::CreateImage(const char* szFilename, int texWidth, int texHeight, int frameWidth, int frameHeight)
{
    // �]�w�ؤo�M��u��T
    m_fFrameWidth = (float)frameWidth;
    m_fFrameHeight = (float)frameHeight;
    m_nGridX = texWidth / frameWidth;
    m_fGridWidth = (float)frameWidth / (float)texWidth;
    m_fGridHeight = (float)frameHeight / (float)texHeight;

    // �إ߳��I�w�İϩM���z
    m_pVBData = g_Device_Reset_Manager->CreateVertexBuffer(4, 2); // ���� 2 ���� ImageVertex
    m_pTexData = g_Device_Reset_Manager->CreateTexture(szFilename, 0);

    if (!m_pVBData || !m_pTexData)
    {
        Free(); // �p�G���@���إߥ��ѡA�h�M�z�Ҧ��귽
        return false;
    }

    // �p��Ω����M�w�쪺��¦���I�]�H (0,0) �����ߡ^
    float halfW = m_fFrameWidth * 0.5f;
    float halfH = m_fFrameHeight * 0.5f;
    m_baseVertices[0] = { -halfW, -halfH, 0.0f }; // ���W
    m_baseVertices[1] = { halfW, -halfH, 0.0f }; // �k�W
    m_baseVertices[2] = { halfW,  halfH, 0.0f }; // �k�U
    m_baseVertices[3] = { -halfW,  halfH, 0.0f }; // ���U

    m_bIsCreated = true;
    return true;
}

void Image::Free()
{
    // �ϥ� CDeviceResetManager �Ӧw���a�R���귽
    if (m_pTexData)
    {
        g_Device_Reset_Manager->DeleteTexture(m_pTexData);
        m_pTexData = nullptr;
    }
    if (m_pVBData)
    {
        g_Device_Reset_Manager->DeleteVertexBuffer(m_pVBData);
        m_pVBData = nullptr;
    }
    m_bIsCreated = false;
}

void Image::SetImage(float x, float y, unsigned short frameID)
{
    if (!m_bIsCreated) return;

    // 1. �p��ù��y��
    // �`�N�G��l�X���� -0.5 �������A�o�q�`�O���F�����������
    m_vertices[0].x = x - 0.5f;
    m_vertices[0].y = y - 0.5f;
    m_vertices[1].x = x + m_fFrameWidth - 0.5f;
    m_vertices[1].y = y - 0.5f;
    m_vertices[2].x = x + m_fFrameWidth - 0.5f;
    m_vertices[2].y = y + m_fFrameHeight - 0.5f;
    m_vertices[3].x = x - 0.5f;
    m_vertices[3].y = y + m_fFrameHeight - 0.5f;

    // 2. �p�⯾�z (UV) �y��
    float tx = (float)(frameID % m_nGridX) * m_fGridWidth;
    float ty = (float)(frameID / m_nGridX) * m_fGridHeight;
    m_vertices[0].u = tx;
    m_vertices[0].v = ty;
    m_vertices[1].u = tx + m_fGridWidth;
    m_vertices[1].v = ty;
    m_vertices[2].u = tx + m_fGridWidth;
    m_vertices[2].v = ty + m_fGridHeight;
    m_vertices[3].u = tx;
    m_vertices[3].v = ty + m_fGridHeight;

    // 3. �N���I��Ƨ�s��w��w�İ�
    this->Process();
}

void Image::SetPosition(float x, float y)
{
    // �N��¦���I�ھګ��w��m�i�業���A�o��̲ת��ù����I�y��
    // �P�ˡA-0.5f �O���F�������
    for (int i = 0; i < 4; ++i)
    {
        m_vertices[i].x = x + m_baseVertices[i].x - 0.5f;
        m_vertices[i].y = y + m_baseVertices[i].y - 0.5f;
    }
}

void Image::SetAngle(float radians)
{
    D3DXMATRIX matRotation;
    D3DXMatrixRotationZ(&matRotation, radians);

    // �ϥα���x�}�ܴ���¦���I
    // �o�̤��ઽ���ק� m_baseVertices�A�_�h�|�ֿn����
    // ��l�X���޿�O�����ܴ� m_baseVertices�A�o�i��O�@�� bug �ίS�w�]�p
    // ���B�������٭�
    for (int i = 0; i < 4; ++i)
    {
        D3DXVec3TransformCoord(&m_baseVertices[i], &m_baseVertices[i], &matRotation);
    }
}

void Image::DrawImage()
{
    if (!m_bIsCreated || DontDraw || !m_pVBData || !m_pVBData->pVertexBuffer) return;

    // �ϥΦۨ������I�w�İϨ�ø�s
    this->DrawImage(m_pVBData->pVertexBuffer); // ���] pVertexBuffer �b VertexBufferData ��
}

void Image::DrawImage(IDirect3DVertexBuffer9* pVB)
{
    // �p�G�귽���إߡB����X�Ь� true �ΨS�����z�A�h������
    if (!m_bIsCreated || !m_pTexData || !m_pTexData->pTexture) return;

    // �ϥΥ��쪺 CDeviceManager �ӳ]�w��V���A��ø�s
    // �o�٭�F�ϽsĶ�{���X�����I�s�y�{

    // 1. �]�w�n�ϥΪ����z
    CDeviceManager::GetInstance()->SetTexture(0, m_pTexData->pTexture);

    // 2. �]�w���I��y�ӷ�
    CDeviceManager::GetInstance()->SetStreamSource(0, pVB, 0, sizeof(ImageVertex));

    // 3. �]�w���I�榡 (FVF)
    // 0x104 �N�� D3DFVF_XYZRHW | D3DFVF_TEX1
    CDeviceManager::GetInstance()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

    // 4. �I�s Direct3D �� DrawPrimitive �禡�i��ø�s
    // D3DPT_TRIANGLEFAN �|�ϥ� 4 �ӳ��Iø�s�@�ӥѨ�ӤT���βզ����x��
    Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
}

void Image::Process()
{
    // �p�G�귽���إߡB����X�Ь� true �ΨS�����I�w�İϡA�h������
    if (!m_bIsCreated || DontDraw || !m_pVBData || !m_pVBData->pVertexBuffer) return;

    // ��w���I�w�İ�
    void* pV = nullptr;
    HRESULT hr = m_pVBData->pVertexBuffer->Lock(0, sizeof(m_vertices), &pV, 0);

    if (SUCCEEDED(hr) && pV != nullptr)
    {
        // �N�ثe�����I��� (m_vertices) �ƻs��w��w�İϤ�
        memcpy(pV, m_vertices, sizeof(m_vertices));
        // ����w�İ�
        m_pVBData->pVertexBuffer->Unlock();
    }
}

void Image::SetLRReverse()
{
    // �o�Ө禡�z�L�洫���k�ⰼ����¦���I�ӹ�{����½��C
    // �洫 ���W(0) �M �k�W(1)
    std::swap(m_baseVertices[0], m_baseVertices[1]);
    // �洫 ���U(3) �M �k�U(2)
    std::swap(m_baseVertices[3], m_baseVertices[2]);
}

void Image::SetBlockID(unsigned short blockID)
{
    if (!m_bIsCreated || m_nGridX == 0) return;

    // �ھڰ϶� ID (�v��s��) �p�⯾�z�� UV �y��
    float tx = (float)(blockID % m_nGridX) * m_fGridWidth;
    float ty = (float)(blockID / m_nGridX) * m_fGridHeight;

    // �N�p��X�� UV �y�г]�w��̲ת����I��Ƥ�
    m_vertices[0].u = tx;
    m_vertices[0].v = ty;
    m_vertices[1].u = tx + m_fGridWidth;
    m_vertices[1].v = ty;
    m_vertices[2].u = tx + m_fGridWidth;
    m_vertices[2].v = ty + m_fGridHeight;
    m_vertices[3].u = tx;
    m_vertices[3].v = ty + m_fGridHeight;
}

void Image::GetVerTextInfo(int* gridX, float* width, float* height, float* gridWidth, float* gridHeight)
{
    // �ˬd�ǤJ�����ЬO�_���ġA�קK�{���Y��
    if (gridX)      *gridX = m_nGridX;
    if (width)      *width = m_fFrameWidth;
    if (height)     *height = m_fFrameHeight;
    if (gridWidth)  *gridWidth = m_fGridWidth;
    if (gridHeight) *gridHeight = m_fGridHeight;
}