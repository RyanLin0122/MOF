#pragma once
#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h"
class GameImage;

/**
 * @class CEffect_Field_Pet_ItemPick_Sub_Complete
 * @brief ���~�B�������ɪ��z��/�l���S�ġC
 */
class CEffect_Field_Pet_ItemPick_Sub_Complete : public CEffectBase {
public:
    CEffect_Field_Pet_ItemPick_Sub_Complete();
    virtual ~CEffect_Field_Pet_ItemPick_Sub_Complete();

    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    void SetEffect(float x, float y, bool bFlip);
};

/**
 * @class CEffect_Field_Pet_ItemPick_Sub_Light
 * @brief ���~������|�W��������I�C
 */
class CEffect_Field_Pet_ItemPick_Sub_Light : public CEffectBase {
public:
    CEffect_Field_Pet_ItemPick_Sub_Light();
    virtual ~CEffect_Field_Pet_ItemPick_Sub_Light();

    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    void SetEffect(float x, float y, bool bFlip);
private:
    GameImage* m_pLightImage;   // �첾 +132 (0x84)
    float m_fAlpha;             // �첾 +140 (0x8C)
    FrameSkip m_FrameSkip;      // �첾 +164 (0xA4)
    // ��L���ϥΩ��~�Ӫ�����
};