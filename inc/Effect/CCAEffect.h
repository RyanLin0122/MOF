#pragma once
#include <cstdint>
#include <cstddef>
#include <d3d9.h>
#include <d3dx9.h>

#include "global.h"
#include "Image/GameImage.h"

// 注意：IDA 顯示 VERTEXANIMATIONFRAMEINFO stride = 124 bytes（見 124 * frameIndex）。
struct VERTEXANIMATIONFRAMEINFO
{
    uint32_t  m_dwImageID;        // +0   影格圖片 ID
    GIVertex  m_Vertices[4];      // +4   4 個頂點（共 112 bytes）
    uint32_t  m_ptrExtra;         // +116
};

// 圖層：每個圖層是一個 8-byte entry（frameCount + frames pointer），符合 IDA 以 8*i 走訪
struct VERTEXANIMATIONLAYERINFO
{
    int                     m_nFrameCount;  // +0
    VERTEXANIMATIONFRAMEINFO* m_pFrames;    // +4
};

// IDA 中 this+2 指向的不是單一 layer，而是「layer list container」：
// *(ptr+4)=layerCount, *(ptr+8)=layers pointer
#pragma pack(push, 4)
struct VERTEXANIMATIONLAYERLISTINFO
{
    int                     m_nUnknown0;    // +0（IDA 未使用）
    int                     m_nLayerCount;  // +4
    VERTEXANIMATIONLAYERINFO* m_pLayers;    // +8
};

// 動畫 clip
struct KEYINFO
{
    char m_szName[20];   // +0
    int  m_nStartFrame;  // +24
    int  m_nEndFrame;    // +28
};

// .ea 最高層資料（至少需滿足 IDA 存取：+0 animationCount, +4 keyframes, +8 totalFrames）
struct EADATALISTINFO
{
    uint32_t                      animationCount; // offset 0  ( **((_DWORD**)this+idx) )
    KEYINFO* keys;                                // offset 4
    uint32_t                      totalFrames;    // offset 8
    uint32_t                      unknown;        // offset 12 (dword_C24CF4)
    uint32_t                      layerCount;     // offset 16 (dword_C24CF8)
    VERTEXANIMATIONLAYERINFO* layers;             // offset 20

    uint8_t                       m_ucBlendOp;    // offset 24
    uint8_t                       m_ucSrcBlend;   // offset 25
    uint8_t                       m_ucEtcBlendOp;      // offset 26 (只在 blendOp==8 時會被寫)
    uint8_t                       m_ucEtcSrcBlend; // offset 27 (只在 blendOp==8 時會被寫)
    uint8_t                       m_ucEtcDestBlend;// offset 28 (只在 blendOp==8 時會被寫)
    // IDA 沒寫 offset 29，表示「不一定存在第6個 byte」
};
#pragma pack(pop)
// ============================================================================
// FrameSkip：依 IDA layout，必須是「含 vptr 的子物件」並放在 CCAEffect +68。
// ============================================================================

class FrameSkip
{
public:
    FrameSkip();
    virtual ~FrameSkip();
    float m_fAccumulatedTime;  // +4  （在 CCAEffect 中為 +72）
    float m_fTimePerFrame;     // +8  （在 CCAEffect 中為 +76）
    bool Update(float fElapsedTime, int& outFrameCount);
};

// ============================================================================
// CCAEffect：依 IDA member offset 重建
// ============================================================================

class CCAEffect
{
public:
    CCAEffect();
    virtual ~CCAEffect();

    void Reset();
    void SetFrameTime();
    bool FrameProcess(float fDeltaTime);
    void Process();
    void Draw();

    void Play(int nAnimationID, bool bLoop);
    void Pause();
    void LoadImageA();

    // 依 IDA：建構子把這兩個方法的地址寫入物件內（+22/+23），Draw() 再用 renderMode dispatch
    void DrawRenderState();
    void DrawEtcRenderState();

    // setters（不影響 IDA 片段，但方便外部餵資料）
    void SetEffectData(EADATALISTINFO* pData) { m_pEffectData = pData; }
    void SetLayerList(VERTEXANIMATIONLAYERLISTINFO* pList) { m_pLayerList = pList; }

    void SetPosition(float x, float y) { m_fPosX = x; m_fPosY = y; }
    void SetRotation(float radians) { m_fRotation = radians; }
    void SetFlipX(bool bFlip) { m_bFlipX = bFlip ? 1u : 0u; }
    void SetScale(int scale) { m_nScale = scale; }

    void SetBlendIndex(uint8_t idx) { m_ucBlendIndex = idx; }
    void SetEtcBlend(uint8_t op, uint8_t src, uint8_t dst) { m_ucEtcBlendOp = op; m_ucEtcSrcBlend = src; m_ucEtcDestBlend = dst; }
    void SetRenderMode(uint8_t mode) { m_ucRenderMode = mode; }
    

    // +80~+85
    uint8_t m_ucBlendIndex;   // +80
    uint8_t m_ucUnk81;        // +81（IDA 初始化為 0，但片段未用）
    uint8_t m_ucEtcBlendOp;   // +82
    uint8_t m_ucEtcSrcBlend;  // +83
    uint8_t m_ucEtcDestBlend; // +84
    uint8_t m_ucRenderMode;   // +85
private:
    // +4
    EADATALISTINFO* m_pEffectData;

    // +8
    VERTEXANIMATIONLAYERLISTINFO* m_pLayerList;

    // +12, +16
    float  m_fPosX;
    float  m_fPosY;

    // +20 (其後需 padding 到 +24)
    uint8_t m_bFlipX;
    uint8_t m_pad21;
    uint16_t m_pad22_23;

    // +24, +28
    float m_fRotation;
    int   m_nScale;

    // +32~+34 (其後 padding 到 +36)
    uint8_t m_bLoop;       // +32
    uint8_t m_bIsPlaying;  // +33
    uint8_t m_bVisible;    // +34
    uint8_t m_pad35;

    // +36~+52
    int m_nAnimationID;       // +36
    int m_nUnknown10;         // +40
    int m_nCurrentFrame;      // +44
    int m_nStartFrameIndex;   // +48
    int m_nEndFrameIndex;     // +52

    // +56~+64
    GameImage** m_ppGameImages;   // +56
    int         m_nMaxImages;     // +60
    int         m_nActiveImageCount; // +64

    // +68~+79
    FrameSkip   m_FrameSkip;

    // +86~+87 padding
    uint16_t m_pad86_87;

    // +88~+95：函式指標表（IDA：+22/+23）
    using RenderStateFn = void(*)(CCAEffect* self);
    RenderStateFn m_pRenderStateFn[2];

private:
    static void RenderState_Draw(CCAEffect* self);
    static void RenderState_DrawEtc(CCAEffect* self);
};
