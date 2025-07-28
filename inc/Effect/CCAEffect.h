#pragma once

#include <d3d9.h> // 為了 D3DRENDERSTATETYPE 等定義
#include "Image/GameImage.h"

// ========================================================================
// 以下是根據全局分析推導出的 .ea 檔案結構
// ========================================================================

/**
 * @struct VERTEXANIMATIONFRAMEINFO
 * @brief 代表動畫中的單一影格。
 *
 * MODIFICATION: 結構大小被修改為 120 bytes，以精確對應 `ea_parser.py` 的解析。
 * 移除了 m_dwUnknown2，並將 m_dwUnknown1 重新命名以增加清晰度。
 * (大小: 4 + 4*28 + 4 = 120 bytes)
 */
struct VERTEXANIMATIONFRAMEINFO
{
    uint32_t   m_dwImageID;   // +0:  要使用的圖片 ID
    GIVertex   m_Vertices[4]; // +4:  定義圖片形狀的 4 個頂點 (112 bytes)
    uint32_t   m_pExtraData;  // +116: 對應 python 腳本中的 'ptrExtra'
};

/**
 * @struct VERTEXANIMATIONLAYERINFO
 * @brief 代表一個動畫圖層，包含一組影格。
 * Python 腳本解析的格式可視為只包含一個圖層。
 */
struct VERTEXANIMATIONLAYERINFO
{
    int                           m_nFrameCount; // 此圖層的總影格數
    VERTEXANIMATIONFRAMEINFO* m_pFrames;     // 指向影格資料陣列的指標
};

/**
 * @struct KEYINFO
 * @brief 定義一個動畫片段 (clip)，例如 "run", "attack"。
 */
struct KEYINFO
{
    char m_szName[20];
    int  m_nStartFrame;
    int  m_nEndFrame;
};

/**
 * @struct EADATALISTINFO
 * @brief .ea 檔案在記憶體中的最上層結構。
 *
 * MODIFICATION: 移除了與 `ea_parser.py` 解析格式不符的 SIMPLESPRITE 相關成員，
 * 使其專注於基於 GIVertex 的影格動畫格式。
 */
struct EADATALISTINFO
{
    // 動畫片段資訊
    int m_nAnimationCount;
    KEYINFO* m_pKeyFrames;

    // 整體資訊
    int m_nTotalFrames;
    int m_nVersion;

    // 圖層與影格資料
    int m_nLayerCount;
    VERTEXANIMATIONLAYERINFO* m_pLayers;

    // 渲染狀態 (Render States)
    uint8_t m_ucBlendOp;
    uint8_t m_ucSrcBlend;
    uint8_t m_ucDestBlend;
    uint8_t m_ucEtcBlendOp;
    uint8_t m_ucEtcSrcBlend;
    uint8_t m_ucEtcDestBlend;
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