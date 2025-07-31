#include "Effect/CEffect_Field_ItemPick.h"
#include "Effect/CEffect_Field_ItemPick_Sub.h"
#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "Effect/CEffectManager.h"
#include "global.h"

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;
extern int SETTING_FRAME;

// 對應反組譯碼: 0x00535BD0
CEffect_Field_ItemPick::CEffect_Field_ItemPick()
{
    m_pOwnerCharacter = nullptr;
    m_dwOwnerAccountID = 0;
    m_dwTotalFrame = 0;
    m_ucState = 0;
    m_fSpeedFactor = 1.0f;
    m_nSubEffectTimer = 0;
    m_fCurrentPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_fTargetPosX = 0.0f;
    m_fTargetPosY = 0.0f;
    // 1001786209 -> 1.0f / 75.0f
    m_FrameSkip.m_fTimePerFrame = 1.0f / 75.0f;
}

CEffect_Field_ItemPick::~CEffect_Field_ItemPick() {}

// 對應反組譯碼: 0x00535C80
void CEffect_Field_ItemPick::SetEffect(D3DXVECTOR2* pStartPosition, unsigned int dwOwnerAccountID)
{
    m_fCurrentPosX = pStartPosition->x;
    m_fCurrentPosY = pStartPosition->y - 50.0f;
    m_dwOwnerAccountID = dwOwnerAccountID;
    m_pOwnerCharacter = nullptr;
}

// 對應反組譯碼: 0x00535CC0
bool CEffect_Field_ItemPick::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }
    float fFrameCount = static_cast<float>(frameCount);

    m_pOwnerCharacter = ClientCharacterManager::GetInstance()->GetCharByAccount(m_dwOwnerAccountID);
    if (!m_pOwnerCharacter) return true; // 目標消失，特效結束

    m_dwTotalFrame += frameCount;

    m_fTargetPosX = static_cast<float>(m_pOwnerCharacter->GetPosX());
    m_fTargetPosY = static_cast<float>(m_pOwnerCharacter->GetPosY() - 50);

    D3DXVECTOR2 vecToTarget(m_fTargetPosX - m_fCurrentPosX, m_fTargetPosY - m_fCurrentPosY);

    if (D3DXVec2LengthSq(&vecToTarget) < 15.0f * 15.0f || m_dwTotalFrame > 2000) {
        CEffect_Field_ItemPick_Sub_Complete* pComplete = new CEffect_Field_ItemPick_Sub_Complete();
        pComplete->SetEffect(m_fTargetPosX, m_fTargetPosY, false);
        CEffectManager::GetInstance()->BulletAdd(pComplete);
        return true;
    }

    D3DXVec2Normalize(&vecToTarget, &vecToTarget);

    // 狀態機控制的移動邏輯
    switch (m_ucState) {
    case 0:
        m_fCurrentPosY -= fFrameCount * 0.3f;
        m_fCurrentPosX += vecToTarget.x * fFrameCount;
        m_fCurrentPosY += vecToTarget.y * fFrameCount;
        if (m_dwTotalFrame > 70) { m_ucState = 1; m_dwTotalFrame = 70; }
        break;
    case 1:
        m_fCurrentPosY -= fFrameCount * 0.2f;
        m_fCurrentPosX += vecToTarget.x * fFrameCount;
        m_fCurrentPosY += vecToTarget.y * fFrameCount;
        if (m_dwTotalFrame > 140) { m_ucState = 2; m_dwTotalFrame = 140; }
        break;
    case 2:
        m_fCurrentPosY -= fFrameCount * 0.1f;
        m_fCurrentPosX += vecToTarget.x * fFrameCount;
        m_fCurrentPosY += vecToTarget.y * fFrameCount;
        if (m_dwTotalFrame > 210) { m_ucState = 3; m_dwTotalFrame = 210; }
        break;
    case 3:
        m_fCurrentPosX += vecToTarget.x * fFrameCount;
        m_fCurrentPosY += vecToTarget.y * fFrameCount;
        if (m_dwTotalFrame > 200) { m_ucState = 4; m_dwTotalFrame = 200; }
        break;
    case 4:
        m_fCurrentPosX += vecToTarget.x * m_fSpeedFactor * fFrameCount;
        m_fCurrentPosY += vecToTarget.y * m_fSpeedFactor * fFrameCount;
        m_fSpeedFactor += fFrameCount * 0.01f;
        break;
    }

    return false;
}

// 對應反組譯碼: 0x00536210
void CEffect_Field_ItemPick::Process()
{
    m_nSubEffectTimer++;
    if (m_nSubEffectTimer >= SETTING_FRAME) {
        m_nSubEffectTimer = 0;
        float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
        m_bIsVisible = IsCliping(screenX, 0.0f);

        if (m_bIsVisible) {
            CEffect_Field_ItemPick_Sub_Light* pLight = new CEffect_Field_ItemPick_Sub_Light();
            pLight->SetEffect(m_fCurrentPosX, m_fCurrentPosY, false);
            CEffectManager::GetInstance()->BulletAdd(pLight);
        }
    }
}

void CEffect_Field_ItemPick::Draw()
{
    // 主控制器不可見
}