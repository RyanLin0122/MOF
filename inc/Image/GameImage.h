#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "Image/GIVertex.h" // GameImage �ϥ� GIVertex �榡
#include "Image/ImageResource.h"
#include "Image/VertexBufferDataMgr.h"
#include "Image/ImageResourceListDataMgr.h"


/// @class GameImage
/// @brief �@�ӽ������C���Ϲ�����A����Y��B����B�C��V�X���h�دS�ġC
///
/// �����O�q�`�� cltImageManager ������޲z�A�Ω󰪮Ħa��ܤj�q�ʺA�Ϲ��C
class GameImage {
public:
    // --- �֤ߥͩR�g���Pø�s�禡 ---

    /// @brief �غc�禡�G��l�ƩҦ����A���w�]�ȡC
    GameImage();

    /// @brief �Ѻc�禡�G������������I�w�İϡC
    virtual ~GameImage();

    /// @brief �����Ϲ��إߤ@�ӥi�e��8�ӳ��I���w�İϡC
    void CreateVertexBuffer();

    /// @brief ���]�Ϲ����A������Ҧ������귽�]���I�w�İϩM�Ϥ��귽�^�C
    void ResetGI();

    /// @brief �q�귽�޲z������Ϥ���ƨûP������j�w�C
    void GetGIData(unsigned int dwGroupID, unsigned int dwResourceID, int a4, int a5);

    /// @brief �����Ϥ���ƪ��ޥΡA�N���k�ٵ��귽�޲z���C
    void ReleaseGIData();

    /// @brief �ھڷ�e�Ҧ����A�]��m�B�Y��B���൥�^�p��̲ת����I��ơC
    bool Process();

    /// @brief �N�Ϲ�ø�s��ù��W�C
    bool Draw();

    // --- ��L�\��禡 ---

    /// @brief �����e�v��b���z�W�������ϰ�C
    void GetBlockRect(RECT* pOutRect) const;

    /// @brief �ϥΥ~�����Ѫ����I����л\�������I�A�Ω��I�ʵe�C
    void VertexAnimationCalculator(const GIVertex* pSourceVertices);

    /// @brief �N�C���л\�ȭ��]���w�](�զ�)�C
    void SetDefaultTextureColor();

    /// @brief �]�w�Ω��л\���z���C��C
    void SetOverWriteTextureColor(DWORD color);

    /// @brief ����ثe�]�w���л\�C��C
    void GetOverWriteTextureColor(float* pOutColor) const;

    /// @brief �]�w�O�_�ҥ��C���л\ø�s�Ҧ��C
    void SetOverWriteTextureColorDraw(bool bEnable);

    /// @brief �ˬd�O�_�ҥ��C���л\ø�s�Ҧ��C
    bool IsOverWriteTextureColorDraw() const;

    // --- ��K�� Setters / Getters ---

    void SetBlockID(unsigned short wBlockID) { m_wBlockID = wBlockID; }
    void SetPosition(float x, float y) { m_fPosX = x; m_fPosY = y; }
    void SetScale(int nScale) { m_nScale = nScale; }
    void SetScaleXY(float sx, float sy) { m_fScaleX = sx; m_fScaleY = sy; }
    void SetRotation(int nRotation) { m_nRotation = nRotation; }
    void SetFlipX(bool bFlip) { m_bFlipX = bFlip; }
    void SetFlipY(bool bFlip) { m_bFlipY = bFlip; }
    void SetAlpha(unsigned int dwAlpha) { m_dwAlpha = dwAlpha; }
    void SetColor(unsigned int dwColor) { m_dwColor = dwColor; }

    /// @brief �ˬd�� GameImage ����O�_���b�Q�ϥΡC
    bool IsInUse() const { return m_pGIData != nullptr; }

    VertexBufferData* m_pVBData;       // �첾+4: ���I�w�İϪ��޲z�`�I
    ImageResourceListData* m_pGIData;    // �첾+8: �Ϥ��귽���޲z�`�I

    GIVertex m_Vertices[8];              // �첾+12: �x�s8�ӳ��I�����

    D3DXVECTOR3 m_baseVertices[4];       // �첾+236: �Ω��ܴ�����¦���I�y��
    D3DXVECTOR3 m_transformedVertices[4]; // �첾+284: �g�L�ܴ��᪺���I�y��

    float m_fPosX;                       // �첾+332: ø�s�� X �y��
    float m_fPosY;                       // �첾+336: ø�s�� Y �y��
    float m_fDrawWidth;                  // �첾+340: ø�s�e��
    float m_fDrawHeight;                 // �첾+344: ø�s����
    float m_fAngleX;                     // �첾+348: X�b�ɱ�
    float m_fAngleY;                     // �첾+352: Y�b�ɱ�

    float m_fHotspotX;                   // �첾+356
    float m_fHotspotY;                   // �첾+360
    float m_fHotspotWidth;               // �첾+364
    float m_fHotspotHeight;              // �첾+368

    unsigned short m_wBlockID;           // �첾+372: �ʵe�v��/�϶�ID

    int m_nScale;                        // �첾+376: �����Y���� (e.g., 100)
    unsigned int m_dwAlpha;              // �첾+380: �z���� (0-255)
    unsigned int m_dwColor;              // �첾+384: �C���л\ (0-255)
    int m_nRotation;                     // �첾+388: ���ਤ�� (0-359)

    bool m_bFlipX;                       // �첾+392: ����½��
    bool m_bFlipY;                       // �첾+396: ����½��

    unsigned int m_dwColorOp;            // �첾+400: �C��ާ@�Ҧ�
    float m_fCenterX;                    // �첾+404: ����½�઺�����I

    unsigned int m_dwGroupID;            // �첾+408: �귽�s��ID
    unsigned int m_dwResourceID;         // �첾+412: �귽ID

    float m_fScaleX;                     // �첾+416: �W�ߪ� X �b�Y��
    float m_fScaleY;                     // �첾+420: �W�ߪ� Y �b�Y��

    bool m_bDrawPart1;                   // �첾+432: �O�_ø�s�Ĥ@�ӤT����
    bool m_bDrawPart2;                   // �첾+436: �O�_ø�s�ĤG�ӤT����

    bool m_bIsProcessed;                 // �첾+440: �аO���I��ƬO�_�w�B�z

    bool m_bVertexAnimation;             // �첾+444: ���I�ʵe�X��
    bool m_bFlag_445;                    // �첾+445
    bool m_bFlag_446;
    bool m_bFlag_447;
    bool m_bFlag_448;
    bool m_bFlag_449;
    bool m_bFlag_450;
    bool m_bFlag_451;

    float m_OverwriteColor[4];           // �첾+456: RGBA�C��A�Ω��л\���z�C��
    bool  m_bUseOverwriteColor;          // �첾+472: �O�_�ҥ��C���л\ø�s
};