#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了FrameSkip 的使用

// 前向宣告
class GameImage;

struct Particle {
    GameImage* pImage;         // 位移 +0: 指向此粒子對應的 GameImage
    float       fAlpha;         // 位移 +4: 當前透明度/生命週期
    float       fScale;         // 位移 +8: 當前縮放/顏色值
    char        cFrameIndex;    // 位移 +12: 粒子的動畫影格
    char        ucOwnerDirection; // 位移 +13: 創建時擁有者的方向
    char        padding[2];     // 位移 +14
    float       fPosX;          // 位移 +16
    float       fPosY;          // 位移 +20
    float       fAngle;         // 位移 +28: 旋轉角度
    float       fVelX;          // 位移 +32: X 軸速度/方向
    float       fVelY;          // 位移 +36: Y 軸速度/方向
    float       fRotation_Unused; // 位移 +40 (原始碼 v13[9])
    float       fSpeed;         // 位移 +48: 速度係數
    bool        bIsActive;      // 位移 +52
    bool        bIsVisible;     // 位移 +53
    char        padding2[10];
};

/**
 * @class CEffectUseHitMulti
 * @brief 一個基於粒子系統的複合特效，用於產生爆炸、衝擊波等多粒子效果。
 */
class CEffectUseHitMulti : public CEffectBase {
public:
    CEffectUseHitMulti();
    virtual ~CEffectUseHitMulti();

    // --- 虛擬函式覆寫 ---
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    // --- 專有函式 ---
    /// @brief 設定並初始化粒子系統特效。
    /// @param x 特效中心點 X 座標。
    /// @param y 特效中心點 Y 座標。
    /// @param ucOwnerDirection 擁有者的方向 (0: 左, 1: 右)。
    /// @param effectType 特效的類型 (1-4)，決定了粒子的行為和外觀。
    void SetEffect2(float x, float y, unsigned char ucOwnerDirection, int effectType);

private:
    static const int MAX_PARTICLES = 15;

    // --- 成員變數 (根據 Effectall.c @ 0x0052FCD0 推斷) ---
    Particle m_Particles[MAX_PARTICLES]; // 位移 +132 (0x84), 15 * 64 = 960 bytes

    unsigned int m_dwResourceID; // 位移 +1092 (0x444), 特效使用的 GameImage 資源 ID
    unsigned int m_uParticleCount; // 位移 +1096 (0x448), 此特效實際產生的粒子數量
    int          m_nEffectType;    // 位移 +1100 (0x44C), 儲存 SetEffect2 傳入的類型

    FrameSkip    m_FrameSkip;      // 位移 +1104 (0x450)

    bool         m_bUseAdditiveBlend; // 位移 +1116 (0x45C), 用於決定 Draw 時的混合模式
};