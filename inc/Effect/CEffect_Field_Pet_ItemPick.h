#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip 的使用
#include <d3dx9math.h>

class ClientCharacter;

/**
 * @class CEffect_Field_Pet_ItemPick
 * @brief 寵物拾取物品的邏輯控制器與軌跡生成器。
 */
class CEffect_Field_Pet_ItemPick : public CEffectBase {
public:
    CEffect_Field_Pet_ItemPick();
    virtual ~CEffect_Field_Pet_ItemPick();

    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override; // 此函式為空，因為控制器本身不可見

    /// @brief 設定特效的起點和目標。
    void SetEffect(D3DXVECTOR2* pStartPosition, unsigned int dwOwnerAccountID);

private:
    ClientCharacter* m_pOwnerPet;      // 位移 +132 (0x84): 指向目標寵物
    unsigned int     m_dwOwnerAccountID; // 位移 +136 (0x88): 寵物擁有者的帳號 ID

    unsigned int m_dwTotalFrame;       // 位移 +140 (0x8C): 總計時器
    unsigned char m_ucState;           // 位移 +144 (0x90): 飛行狀態機 (0-4)

    // 飛行軌跡相關
    float m_fSpeedFactor;        // 位移 +148 (0x94)
    float m_fCurrentPosX;        // 位移 +156 (0x9C)
    float m_fCurrentPosY;        // 位移 +160 (0xA0)
    float m_fTargetPosX;         // 位移 +172 (0xAC)
    float m_fTargetPosY;         // 位移 +176 (0xB0)

    FrameSkip m_FrameSkip;       // 位移 +180 (0xB4)

    int m_nSubEffectTimer;       // 位移 +192 (0xC0): 生成子特效的計時器
};