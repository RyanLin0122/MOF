#pragma once
#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 為了 FrameSkip
#include <d3dx9math.h>

// 前向宣告
class ClientCharacter;

class CEffect_Skill_Type_ShootUnit : public CEffectBase {
public:
    CEffect_Skill_Type_ShootUnit();
    virtual ~CEffect_Skill_Type_ShootUnit();

    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;

    void SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short effectKindID, char* szFileName, int hitInfoID);

protected:
    ClientCharacter* m_pTargetCharacter;

    float   m_fTotalDistance;
    float   m_fTraveledDistance;

    // --- 關鍵修正 ---
    // 這個 FrameSkip 物件專門用於控制投射物的移動和生命週期
    FrameSkip m_MovementFrameSkip; // 位移 +144 (0x90)

    float   m_fAngle;
    int     m_nHitInfoID;
};