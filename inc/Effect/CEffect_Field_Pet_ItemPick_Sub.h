#pragma once
#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip 的使用

class GameImage;

class CEffect_Field_Pet_ItemPick_Sub_Complete : public CEffectBase {
public:
    CEffect_Field_Pet_ItemPick_Sub_Complete();
    virtual ~CEffect_Field_Pet_ItemPick_Sub_Complete();

    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    void SetEffect(float x, float y, bool bFlip);
};

class CEffect_Field_Pet_ItemPick_Sub_Light : public CEffectBase {
public:
    CEffect_Field_Pet_ItemPick_Sub_Light();
    virtual ~CEffect_Field_Pet_ItemPick_Sub_Light();

    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    void SetEffect(float x, float y, bool bFlip);
private:
    GameImage* m_pLightImage;
    float m_fAlpha;
    FrameSkip m_FrameSkip;
    // 儲存位置的成員已在 CEffectBase 中
};