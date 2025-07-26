#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip 的使用
#include "Character/ClientCharacter.h"
#include <d3dx9math.h>


/**
 * @class CEffect_Field_ItemPick
 * @brief 玩家拾取物品的邏輯控制器與軌跡生成器。
 */
class CEffect_Field_ItemPick : public CEffectBase {
public:
    CEffect_Field_ItemPick();
    virtual ~CEffect_Field_ItemPick();

    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    /// @brief 設定特效的起點和目標玩家。
    void SetEffect(D3DXVECTOR2* pStartPosition, unsigned int dwOwnerAccountID);

private:
    ClientCharacter* m_pOwnerCharacter;   // 位移 +132 (0x84): 指向目標玩家
    unsigned int     m_dwOwnerAccountID;  // 位移 +136 (0x88): 玩家的帳號 ID

    unsigned int m_dwTotalFrame;        // 位移 +140 (0x8C): 總計時器
    unsigned char m_ucState;            // 位移 +144 (0x90): 飛行狀態機 (0-4)

    float m_fSpeedFactor;         // 位移 +148 (0x94)
    // char m_cUnk152;            // 位移 +152 (0x98)

    float m_fCurrentPosX;         // 位移 +156 (0x9C)
    float m_fCurrentPosY;         // 位移 +160 (0xA0)
    float m_fTargetPosX;          // 位移 +172 (0xAC)
    float m_fTargetPosY;          // 位移 +176 (0xB0)

    FrameSkip m_FrameSkip;        // 位移 +180 (0xB4)

    int m_nSubEffectTimer;        // 位移 +192 (0xC0): 生成子特效的計時器
};