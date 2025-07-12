#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "Image/CDeviceResetManager.h"

// �e�V�ŧi�A�קK�����n�����Y�ɤޥ�
struct VertexBufferData;
struct TextureListData;

/// @struct ImageVertex
/// @brief �Ω�ø�s2D�Ϥ������I�榡�A���� D3DFVF_XYZRHW | D3DFVF_TEX1�C
struct ImageVertex {
    float x, y, z, rhw; // ��m (Reciprocal of Homogeneous W)
    float u, v;         // ���z�y��
};

/// @class Image
/// @brief �t�d�޲z�Mø�s��@ 2D �Ϥ�����¦���O�C
///
/// �o�����O�B�z�@�ӥѥ|�ӳ��I�զ����x�ΡA�ñN����������z�϶�ø�s��ù��W�C
class Image {
public:
    Image();
    virtual ~Image(); // �����O��ĳ�ϥε����Ѻc�禡

    /// @brief �إ߹Ϥ��귽�A�]�A���z�M���I�w�İϡC
    /// @param szFilename ���z�ɮת����|�C
    /// @param texWidth ��ӯ��z���e�סC
    /// @param texHeight ��ӯ��z�����סC
    /// @param frameWidth �Ϥ�����@�v�檺�e�סC
    /// @param frameHeight �Ϥ�����@�v�檺���סC
    /// @return ���\��^ true�A���Ѫ�^ false�C
    bool CreateImage(const char* szFilename, int texWidth, int texHeight, int frameWidth, int frameHeight);

    /// @brief ����� CreateImage �إߪ��귽�C
    void Free();

    /// @brief �]�w�Ϥ���ø�s��m�M�n��ܪ��v��C
    /// @param x �ù��W�� X �y�СC
    /// @param y �ù��W�� Y �y�СC
    /// @param frameID �n��ܪ��v��s���C
    void SetImage(float x, float y, unsigned short frameID);

    /// @brief �]�w�Ϥ����̲�ø�s��m�C
    void SetPosition(float x, float y);

    /// @brief �]�w�Ϥ������ਤ�ס]¶ Z �b�^�C
    /// @param radians ���઺���סC
    void SetAngle(float radians);

    /// @brief ���k½��Ϥ��C
    void SetLRReverse();

    /// @brief �����]�w�n��ܪ����z�϶��]�v��^�C
    void SetBlockID(unsigned short blockID);

    /// @brief �N�ثe�����I��Ƨ�s�쳻�I�w�İϤ��C
    void Process();

    /// @brief �ϥΦۨ������I�w�İϨ�ø�s�Ϥ��C
    void DrawImage();

    /// @brief �ϥΥ~�����Ѫ����I�w�İϨ�ø�s�Ϥ��C
    void DrawImage(IDirect3DVertexBuffer9* pVB);

    /// @brief ���o�Ϥ������z�P��u��T�C
    void GetVerTextInfo(int* gridX, float* width, float* height, float* gridWidth, float* gridHeight);

protected:
    // --- �����ܼ� (�ھڤϽsĶ�{���X���O����첾���_) ---

    VertexBufferData* m_pVBData;      // �첾 0:   ���V���I�w�İϺ޲z�`�I������
    TextureListData* m_pTexData;     // �첾 4:   ���V���z�޲z�`�I������
    int              m_nGridX;         // �첾 8:   ���z�b X �b�W�i�e�Ǫ��v���
    float            m_fFrameWidth;    // �첾 12:  ��@�v�檺�e��
    float            m_fFrameHeight;   // �첾 16:  ��@�v�檺����
    float            m_fGridWidth;     // �첾 20:  ��@�v��e�צ��`�e�ת���� (U)
    float            m_fGridHeight;    // �첾 24:  ��@�v�氪�ץe�`���ת���� (V)
    unsigned short   m_nWord_28;       // �첾 28:  �����γ~�� WORD

    ImageVertex      m_vertices[4];    // �첾 32:  �̲׭nø�s��4�ӳ��I��� (96 bytes)

    D3DXVECTOR3      m_baseVertices[4]; // �첾 128: �Ω����M�w�쪺��¦���I�y�� (48 bytes)

    // �����������γ~�� padding

    bool             m_bIsCreated;     // �첾 176: �аO�귽�O�_�w�إ�
private:
    CDeviceResetManager* g_Device_Reset_Manager;
};