#pragma once
#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip 的使用

class GameImage;

/**
 * @class CEffect_Field_ItemPick_Sub_Complete
 * @brief 物品拾取完成時的爆炸/吸收特效。
 */
class CEffect_Field_ItemPick_Sub_Complete : public CEffectBase {
public:
    CEffect_Field_ItemPick_Sub_Complete();
    virtual ~CEffect_Field_ItemPick_Sub_Complete();

    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    void SetEffect(float x, float y, bool bFlip);
};

/**
 * @class CEffect_Field_ItemPick_Sub_Light
 * @brief 物品飛行路徑上的拖尾光點。
 */
class CEffect_Field_ItemPick_Sub_Light : public CEffectBase {
public:
    CEffect_Field_ItemPick_Sub_Light();
    virtual ~CEffect_Field_ItemPick_Sub_Light();

    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    void SetEffect(float x, float y, bool bFlip);
private:
    GameImage* m_pLightImage;   // 位移 +132 (0x84)
    float m_fAlpha;             // 位移 +140 (0x8C)
    FrameSkip m_FrameSkip;      // 位移 +164 (0xA4)
};