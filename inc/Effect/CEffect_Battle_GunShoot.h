#pragma once

#include "Effect/CEffectBase.h"
#include "Effect/CCAEffect.h" // 包含 FrameSkip 的定義
#include "Character/ClientCharacter.h"
#include <d3dx9math.h>

class CEffect_Field_Walkdust;
class CEffect_Battle_GunSpark;

class CEffect_Battle_GunShoot : public CEffectBase {
public:
    CEffect_Battle_GunShoot();
    virtual ~CEffect_Battle_GunShoot();

    // ... (其他函式宣告不變) ...
    virtual bool FrameProcess(float fElapsedTime) override;
    virtual void Process() override;
    virtual void Draw() override;
    void SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short a4, int hitInfoID);
    void SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, bool bFlip, unsigned short a5, int hitInfoID);

private:
    // --- 成員變數 (修正與新增) ---

    // 原始碼 *((_DWORD *)this + 33) 的位置，現在明確其用途
    GameImage* m_pBulletImage;      // 位移 +132 (0x84): 代表子彈/彈道的 GameImage

    ClientCharacter* m_pTargetCharacter;  // 位移 +136 (0x88): 目標角色

    float   m_fTotalDistance;    // 位移 +140 (0x8C): 總距離
    float   m_fTraveledDistance; // 位移 +144 (0x90): 已飛行距離

    FrameSkip m_MovementFrameSkip; // 位移 +148 (0x94)

    int     m_nHitInfoID;        // 位移 +160 (0xA0)
    int     m_dwCasterAccountID; // 位移 +164 (0xA4)
    unsigned short m_usUnk_a4;   // 位移 +168 (0xA8): SetEffect 傳入的未知參數 a4
};