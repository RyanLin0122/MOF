#include "Effect/CEffect_Mon_DarkBall.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"
#include <cmath>

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x00530770
CEffect_Mon_DarkBall::CEffect_Mon_DarkBall()
    : m_fTraveledDistance(0.0f),
    m_fTotalDistance(0.0f),
    m_pTargetCharacter(nullptr),
    m_fAngle(0.0f),
    m_nHitInfoID(0)
{
    // CEffectBase 建構函式已自動呼叫

    // 載入特效的視覺動畫
    CEAManager::GetInstance()->GetEAData(108, "Effect/efn_Mop_DarkBall.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 動畫不循環，但因為是投射物，通常會自行處理循環邏輯或使用單幀動畫

    // 設定移動計時器的更新頻率 (30 FPS)
    // 原始碼: *((_DWORD *)this + 38) = 995783694;
    m_MovementFrameSkip.m_fTimePerFrame = 1.0f / 30.0f;
}

// 對應反組譯碼: 0x00530840
CEffect_Mon_DarkBall::~CEffect_Mon_DarkBall()
{
}

// 對應反組譯碼: 0x00530860
void CEffect_Mon_DarkBall::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, float fFlip, int hitInfoID)
{
    if (!pCaster || !pTarget) return;

    m_pTargetCharacter = pTarget;
    m_nHitInfoID = hitInfoID;
    m_fSpeed = 2.0f; // 速度在原始碼中被硬編碼為 2.0 (0x40000000)

    // 根據施法者朝向，對起始 X 座標進行偏移
    float startX = static_cast<float>(pCaster->GetPosX());
    if (static_cast<int>(fFlip) != 0) {
        startX -= 50.0f;
    }
    else {
        startX += 50.0f;
    }

    D3DXVECTOR2 startPos(startX, static_cast<float>(pCaster->GetPosY()));
    D3DXVECTOR2 endPos(static_cast<float>(pTarget->GetPosX()), static_cast<float>(pTarget->GetPosY()));

    m_fCurrentPosX = startPos.x;
    m_fCurrentPosY = startPos.y;

    D3DXVECTOR2 vecDirection = endPos - startPos;
    m_fTotalDistance = D3DXVec2Length(&vecDirection) - 5.0f; // 提前 5.0 單位觸發
    if (m_fTotalDistance < 0.0f) m_fTotalDistance = 0.0f;

    D3DXVec2Normalize(&vecDirection, &vecDirection);
    m_fDirectionX = vecDirection.x * m_fSpeed;
    m_fDirectionY = vecDirection.y * m_fSpeed;

    m_bIsFlip = (pCaster->GetPosX() > pTarget->GetPosX());

    D3DXVECTOR2 refVec(m_bIsFlip ? -1.0f : 1.0f, 0.0f);
    float dotProduct = D3DXVec2Dot(&refVec, &vecDirection);
    m_fAngle = acosf(dotProduct);
    if (vecDirection.y < 0) {
        m_fAngle = -m_fAngle;
    }
}

// 對應反組譯碼: 0x00530B60
void CEffect_Mon_DarkBall::SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fFlip, int hitInfoID)
{
    // ... 座標版本的 SetEffect，邏輯與 BowShoot 和 MagicBook 的版本完全相同 ...
    if (!pStartPos || !pEndPos) return;

    m_nHitInfoID = hitInfoID;
    m_fSpeed = 2.0f;
    m_fCurrentPosX = pStartPos->x;
    m_fCurrentPosY = pStartPos->y;

    D3DXVECTOR2 vecDirection = *pEndPos - *pStartPos;
    m_fTotalDistance = D3DXVec2Length(&vecDirection) - 60.0f; // 座標版本使用不同的提前量
    if (m_fTotalDistance < 0.0f) m_fTotalDistance = 0.0f;

    D3DXVec2Normalize(&vecDirection, &vecDirection);
    m_fDirectionX = vecDirection.x * m_fSpeed;
    m_fDirectionY = vecDirection.y * m_fSpeed;

    m_bIsFlip = (static_cast<int>(fFlip) != 0);

    D3DXVECTOR2 refVec(m_bIsFlip ? -1.0f : 1.0f, 0.0f);
    float dotProduct = D3DXVec2Dot(&refVec, &vecDirection);
    m_fAngle = acosf(dotProduct);
    if (vecDirection.y < 0) {
        m_fAngle = -m_fAngle;
    }
}


// 對應反組譯碼: 0x00530D70
bool CEffect_Mon_DarkBall::FrameProcess(float fElapsedTime)
{
    m_ccaEffect.FrameProcess(fElapsedTime);

    int moveFrameCount = 0;
    if (m_MovementFrameSkip.Update(fElapsedTime, moveFrameCount)) {
        float moveDistance = static_cast<float>(moveFrameCount);
        m_fTraveledDistance += moveDistance;

        if (m_fTraveledDistance >= m_fTotalDistance) {
            if (m_pTargetCharacter) {
                m_pTargetCharacter->SetHited(m_nHitInfoID, 16);
            }
            return true;
        }
        else {
            m_fCurrentPosX += m_fDirectionX * moveDistance;
            m_fCurrentPosY += m_fDirectionY * moveDistance;
        }
    }
    return false;
}

// 對應反組譯碼: 0x00530E70
void CEffect_Mon_DarkBall::Process()
{
    // 與 BowShoot 不同，此函式沒有裁剪判斷，而是直接計算
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    m_ccaEffect.SetPosition(screenX, screenY);
    m_ccaEffect.SetRotation(m_fAngle);
    m_ccaEffect.Process();
    m_bIsVisible = TRUE;
}

// 對應反組譯碼: 0x00530EA0
void CEffect_Mon_DarkBall::Draw()
{
    // 沒有 isVisible 檢查，直接呼叫 Draw
    m_ccaEffect.Draw();
}