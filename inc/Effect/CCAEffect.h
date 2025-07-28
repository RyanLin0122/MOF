#pragma once

#include <d3d9.h> // ���F D3DRENDERSTATETYPE ���w�q
#include "Image/GameImage.h"

// ========================================================================
// �H�U�O�ھڥ������R���ɥX�� .ea �ɮ׵��c
// ========================================================================

/**
 * @struct VERTEXANIMATIONFRAMEINFO
 * @brief �N��ʵe������@�v��C
 *
 * MODIFICATION: ���c�j�p�Q�קאּ 120 bytes�A�H��T���� `ea_parser.py` ���ѪR�C
 * �����F m_dwUnknown2�A�ñN m_dwUnknown1 ���s�R�W�H�W�[�M���סC
 * (�j�p: 4 + 4*28 + 4 = 120 bytes)
 */
struct VERTEXANIMATIONFRAMEINFO
{
    uint32_t   m_dwImageID;   // +0:  �n�ϥΪ��Ϥ� ID
    GIVertex   m_Vertices[4]; // +4:  �w�q�Ϥ��Ϊ��� 4 �ӳ��I (112 bytes)
    uint32_t   m_pExtraData;  // +116: ���� python �}������ 'ptrExtra'
};

/**
 * @struct VERTEXANIMATIONLAYERINFO
 * @brief �N��@�Ӱʵe�ϼh�A�]�t�@�ռv��C
 * Python �}���ѪR���榡�i�����u�]�t�@�ӹϼh�C
 */
struct VERTEXANIMATIONLAYERINFO
{
    int                           m_nFrameCount; // ���ϼh���`�v���
    VERTEXANIMATIONFRAMEINFO* m_pFrames;     // ���V�v���ư}�C������
};

/**
 * @struct KEYINFO
 * @brief �w�q�@�Ӱʵe���q (clip)�A�Ҧp "run", "attack"�C
 */
struct KEYINFO
{
    char m_szName[20];
    int  m_nStartFrame;
    int  m_nEndFrame;
};

/**
 * @struct EADATALISTINFO
 * @brief .ea �ɮצb�O���餤���̤W�h���c�C
 *
 * MODIFICATION: �����F�P `ea_parser.py` �ѪR�榡���Ū� SIMPLESPRITE ���������A
 * �Ϩ�M�`���� GIVertex ���v��ʵe�榡�C
 */
struct EADATALISTINFO
{
    // �ʵe���q��T
    int m_nAnimationCount;
    KEYINFO* m_pKeyFrames;

    // �����T
    int m_nTotalFrames;
    int m_nVersion;

    // �ϼh�P�v����
    int m_nLayerCount;
    VERTEXANIMATIONLAYERINFO* m_pLayers;

    // ��V���A (Render States)
    uint8_t m_ucBlendOp;
    uint8_t m_ucSrcBlend;
    uint8_t m_ucDestBlend;
    uint8_t m_ucEtcBlendOp;
    uint8_t m_ucEtcSrcBlend;
    uint8_t m_ucEtcDestBlend;
};

/**
 * @class FrameSkip
 * @brief �B�z���ɶ����v����D�޿�A�T�O�ʵe����t��í�w�C
 */
class FrameSkip {
public:
    FrameSkip();
    virtual ~FrameSkip();

    /// @brief �ھڸg�L���ɶ���s�v��p�ơC
    /// @param fElapsedTime �g�L���ɶ��]��^�C
    /// @param outFrameCount ��X�ѼơA��^�p��X���v����i�ơC
    /// @return �p�G���v����i�A��^ true�C
    bool Update(float fElapsedTime, int& outFrameCount);

public:
    float m_fAccumulatedTime;  // �֭p���ɶ�
    float m_fTimePerFrame;     // �C�v��ݭn���ɶ�
};


/**
 * @class CCAEffect
 * @brief �֤ߪ��S�İʵe���O�A�t�d���� .ea �ɮשw�q����ı�ĪG�C
 */
class CCAEffect {
public:
    CCAEffect();
    virtual ~CCAEffect();

    void Reset();
    void SetFrameTime();
    bool FrameProcess(float fElapsedTime);
    void Process();
    void Draw();
    void Play(int nAnimationID, bool bLoop);
    void Pause();
    void LoadImageA();

    // --- Setters for properties used in Process() ---
    void SetPosition(float x, float y) { m_fPosX = x; m_fPosY = y; }
    void SetRotation(float radians) { m_fRotation = radians; }
    void SetFlipX(bool flip) { m_bFlipX = flip; }
    void SetData(EADATALISTINFO* pData) { m_pEffectData = pData; }
    friend class CEAManager;

private:
    void DrawRenderState();
    void DrawEtcRenderState();

protected:
    EADATALISTINFO* m_pEffectData;

    float           m_fPosX;
    float           m_fPosY;
    bool            m_bFlipX;
    float           m_fRotation;
    unsigned int    m_dwAlpha;

    bool            m_bIsPlaying;
    bool            m_bShow;
    bool            m_bIsLooping;

    int             m_nAnimationID;
    int             m_nCurrentFrame;
    int             m_nStartFrame;
    int             m_nEndFrame;

    GameImage** m_pGameImages;
    int             m_nMaxImagesInAnimation;
    int             m_nImageCountInFrame;

    FrameSkip       m_FrameSkip;

    // ��V���A��ܺX�� (�� CEAManager �]�w)
    unsigned char   m_ucRenderStateSelector; // �첾 +85: 0 �ϥ� DrawRenderState, 1 �ϥ� DrawEtcRenderState

    // ��ӹw���x�s�������禡����
    void (CCAEffect::* m_pfnDrawRenderState)();    // �첾 +88 (��l�X���� +22 * 4)
    void (CCAEffect::* m_pfnDrawEtcRenderState)(); // �첾 +92 (��l�X���� +23 * 4)
};