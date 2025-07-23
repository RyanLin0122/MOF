#pragma once
#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // ���F FrameSkip
#include <d3dx9math.h>

// �e�V�ŧi
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

    // --- ����ץ� ---
    // �o�� FrameSkip ����M���Ω󱱨��g�������ʩM�ͩR�g��
    FrameSkip m_MovementFrameSkip; // �첾 +144 (0x90)

    float   m_fAngle;
    int     m_nHitInfoID;
};