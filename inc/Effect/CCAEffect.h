#pragma once

#include <d3d9.h> // 為了 D3DRENDERSTATETYPE 等定義
#include "Image/GameImage.h"

// ========================================================================
// 以下是根據全局分析推導出的 .ea 檔案結構
// ========================================================================

/**
 * @struct VERTEXANIMATIONFRAMEINFO
 * @brief (大小: 124 bytes) 儲存特效動畫單一影格的詳細渲染資訊。
 * 這是動畫數據中最底層的結構。
 */
struct VERTEXANIMATIONFRAMEINFO {
    /// @brief (位移 +0, 大小 4) 要使用的圖片資源 ID。
    unsigned int m_dwImageID;

    /// @brief (位移 +4, 大小 112) 構成一個矩形的 4 個頂點的完整數據。
    /// CCAEffect::Process 會讀取這塊數據並傳給 GameImage。
    GIVertex m_Vertices[4];

    /// @brief (位移 +116, 大小 4) 未知用途的數據。
    unsigned int m_dwUnknown1;

    /// @brief (位移 +120, 大小 4) 未知用途的數據。
    unsigned int m_dwUnknown2;
};

/**
 * @struct VERTEXANIMATIONLAYERINFO
 * @brief (大小: 8 bytes) 儲存一個特效圖層的資訊，包含該圖層所有的影格。
 */
struct VERTEXANIMATIONLAYERINFO {
    /// @brief (位移 +0, 大小 4) 此圖層擁有的總影格數。
    int m_nFrameCount;

    /// @brief (位移 +4, 大小 4) 指向 VERTEXANIMATIONFRAMEINFO 陣列的指標。
    VERTEXANIMATIONFRAMEINFO* m_pFrames;
};

/**
 * @struct KEYINFO
 * @brief (大小: 28 bytes) 定義一個具名的動畫片段（例如 "attack", "idle"）。
 */
struct KEYINFO {
    /// @brief (位移 +0, 大小 20) 動畫片段的名稱，推測大小為 20 以對齊。
    char m_szName[20];

    /// @brief (位移 +20, 大小 4) 此動畫片段的起始影格索引。
    /// CCAEffect::Play 會讀取此值。
    int m_nStartFrame;

    /// @brief (位移 +24, 大小 4) 此動畫片段的結束影格索引。
    /// CCAEffect::Play 會讀取此值。
    int m_nEndFrame;
};

/**
 * @struct EADATALISTINFO
 * @brief (大小: 36 bytes) .ea 檔案在記憶體中的最上層主結構。
 * 包含了整個特效的所有動畫數據和渲染設定。
 */
struct EADATALISTINFO {
    /// @brief (位移 +0, 大小 4) 動畫片段(KEYINFO)的總數。
    int m_nAnimationCount;

    /// @brief (位移 +4, 大小 4) 指向 KEYINFO 陣列的指標。
    KEYINFO* m_pKeyFrames;

    /// @brief (位移 +8, 大小 4) 未知用途，可能是總影格數。
    int m_nTotalFrames;

    /// @brief (位移 +12, 大小 4) 檔案版本。
    int m_nVersion;

    /// @brief (位移 +16, 大小 4) 特效的圖層數量。
    int m_nLayerCount;

    /// @brief (位移 +20, 大小 4) 指向 VERTEXANIMATIONLAYERINFO 陣列的指標。
    VERTEXANIMATIONLAYERINFO* m_pLayers;

    // --- 渲染狀態設定 (由 CCAEffect 讀取) ---
    unsigned char m_ucBlendOp;      // 位移 +24
    unsigned char m_ucSrcBlend;     // 位移 +25
    unsigned char m_ucDestBlend;    // 位移 +26
    unsigned char m_ucEtcBlendOp;   // 位移 +27
    unsigned char m_ucEtcSrcBlend;  // 位移 +28
    unsigned char m_ucEtcDestBlend; // 位移 +29
    // 位移 +30, +31 是編譯器為了對齊插入的 padding
};

/**
 * @class FrameSkip
 * @brief 處理基於時間的影格跳躍邏輯，確保動畫播放速度穩定。
 */
class FrameSkip {
public:
    FrameSkip();
    virtual ~FrameSkip();

    /// @brief 根據經過的時間更新影格計數。
    /// @param fElapsedTime 經過的時間（秒）。
    /// @param outFrameCount 輸出參數，返回計算出的影格推進數。
    /// @return 如果有影格推進，返回 true。
    bool Update(float fElapsedTime, int& outFrameCount);

public:
    float m_fAccumulatedTime;  // 累計的時間
    float m_fTimePerFrame;     // 每影格需要的時間
};


/**
 * @class CCAEffect
 * @brief 核心的特效動畫類別，負責播放 .ea 檔案定義的視覺效果。
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

    // 渲染狀態選擇旗標 (由 CEAManager 設定)
    unsigned char   m_ucRenderStateSelector; // 位移 +85: 0 使用 DrawRenderState, 1 使用 DrawEtcRenderState

    // 兩個預先儲存的成員函式指標
    void (CCAEffect::* m_pfnDrawRenderState)();    // 位移 +88 (原始碼中為 +22 * 4)
    void (CCAEffect::* m_pfnDrawEtcRenderState)(); // 位移 +92 (原始碼中為 +23 * 4)
};