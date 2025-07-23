#pragma once

#include <d3d9.h> // ���F D3DRENDERSTATETYPE ���w�q
#include "Image/GameImage.h"

// ========================================================================
// �H�U�O�ھڥ������R���ɥX�� .ea �ɮ׵��c
// ========================================================================

/**
 * @struct VERTEXANIMATIONFRAMEINFO
 * @brief (�j�p: 124 bytes) �x�s�S�İʵe��@�v�檺�ԲӴ�V��T�C
 * �o�O�ʵe�ƾڤ��̩��h�����c�C
 */
struct VERTEXANIMATIONFRAMEINFO {
    /// @brief (�첾 +0, �j�p 4) �n�ϥΪ��Ϥ��귽 ID�C
    unsigned int m_dwImageID;

    /// @brief (�첾 +4, �j�p 112) �c���@�ӯx�Ϊ� 4 �ӳ��I������ƾڡC
    /// CCAEffect::Process �|Ū���o���ƾڨöǵ� GameImage�C
    GIVertex m_Vertices[4];

    /// @brief (�첾 +116, �j�p 4) �����γ~���ƾڡC
    unsigned int m_dwUnknown1;

    /// @brief (�첾 +120, �j�p 4) �����γ~���ƾڡC
    unsigned int m_dwUnknown2;
};

/**
 * @struct VERTEXANIMATIONLAYERINFO
 * @brief (�j�p: 8 bytes) �x�s�@�ӯS�Ĺϼh����T�A�]�t�ӹϼh�Ҧ����v��C
 */
struct VERTEXANIMATIONLAYERINFO {
    /// @brief (�첾 +0, �j�p 4) ���ϼh�֦����`�v��ơC
    int m_nFrameCount;

    /// @brief (�첾 +4, �j�p 4) ���V VERTEXANIMATIONFRAMEINFO �}�C�����СC
    VERTEXANIMATIONFRAMEINFO* m_pFrames;
};

/**
 * @struct KEYINFO
 * @brief (�j�p: 28 bytes) �w�q�@�Ө�W���ʵe���q�]�Ҧp "attack", "idle"�^�C
 */
struct KEYINFO {
    /// @brief (�첾 +0, �j�p 20) �ʵe���q���W�١A�����j�p�� 20 �H����C
    char m_szName[20];

    /// @brief (�첾 +20, �j�p 4) ���ʵe���q���_�l�v����ޡC
    /// CCAEffect::Play �|Ū�����ȡC
    int m_nStartFrame;

    /// @brief (�첾 +24, �j�p 4) ���ʵe���q�������v����ޡC
    /// CCAEffect::Play �|Ū�����ȡC
    int m_nEndFrame;
};

/**
 * @struct EADATALISTINFO
 * @brief (�j�p: 36 bytes) .ea �ɮצb�O���餤���̤W�h�D���c�C
 * �]�t�F��ӯS�Ī��Ҧ��ʵe�ƾکM��V�]�w�C
 */
struct EADATALISTINFO {
    /// @brief (�첾 +0, �j�p 4) �ʵe���q(KEYINFO)���`�ơC
    int m_nAnimationCount;

    /// @brief (�첾 +4, �j�p 4) ���V KEYINFO �}�C�����СC
    KEYINFO* m_pKeyFrames;

    /// @brief (�첾 +8, �j�p 4) �����γ~�A�i��O�`�v��ơC
    int m_nTotalFrames;

    /// @brief (�첾 +12, �j�p 4) �ɮת����C
    int m_nVersion;

    /// @brief (�첾 +16, �j�p 4) �S�Ī��ϼh�ƶq�C
    int m_nLayerCount;

    /// @brief (�첾 +20, �j�p 4) ���V VERTEXANIMATIONLAYERINFO �}�C�����СC
    VERTEXANIMATIONLAYERINFO* m_pLayers;

    // --- ��V���A�]�w (�� CCAEffect Ū��) ---
    unsigned char m_ucBlendOp;      // �첾 +24
    unsigned char m_ucSrcBlend;     // �첾 +25
    unsigned char m_ucDestBlend;    // �첾 +26
    unsigned char m_ucEtcBlendOp;   // �첾 +27
    unsigned char m_ucEtcSrcBlend;  // �첾 +28
    unsigned char m_ucEtcDestBlend; // �첾 +29
    // �첾 +30, +31 �O�sĶ�����F������J�� padding
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