#include "Effect/CEffect_Skill_Type_ShootUnit.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: CEffect_Battle_BowShoot::CEffect_Battle_BowShoot (結構類似)
CEffect_Skill_Type_ShootUnit::CEffect_Skill_Type_ShootUnit()
    : m_pTargetCharacter(nullptr),
    m_fTotalDistance(0.0f),
    m_fTraveledDistance(0.0f),
    m_fAngle(0.0f),
    m_nHitInfoID(0)
{
    // CEffectBase 的建構函式會被自動呼叫
    m_fSpeed = 10.0f;

    // --- 關鍵修正 ---
    // 初始化用於移動的 FrameSkip。
    // 原始碼 (0x0052D770 @ CEffect_Battle_BowShoot) 中，此計時器的每影格時間被設為 0x3D088889
    // 這個十六進位值對應的浮點數約為 0.03333f，即 1.0f / 30.0f。
    // 這表示物理更新的頻率是固定的 30 FPS。
    m_MovementFrameSkip.m_fTimePerFrame = 1.0f / 30.0f;
}

CEffect_Skill_Type_ShootUnit::~CEffect_Skill_Type_ShootUnit()
{
}

void CEffect_Skill_Type_ShootUnit::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short effectKindID, char* szFileName, int hitInfoID)
{
    // ... (此函式內容不變，保持原樣) ...
    if (!pCaster || !pTarget) return;
    CEAManager::GetInstance()->GetEAData(effectKindID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true);
    m_pTargetCharacter = pTarget;
    m_nHitInfoID = hitInfoID;
    D3DXVECTOR2 startPos(static_cast<float>(pCaster->GetPosX()), static_cast<float>(pCaster->GetPosY() - 25));
    D3DXVECTOR2 endPos(static_cast<float>(pTarget->GetPosX()), static_cast<float>(pTarget->GetPosY() - 15));
    m_fCurrentPosX = startPos.x;
    m_fCurrentPosY = startPos.y;
    D3DXVECTOR2 vec = endPos - startPos;
    m_fTotalDistance = D3DXVec2Length(&vec);
    D3DXVec2Normalize(&vec, &vec);
    m_fDirectionX = vec.x;
    m_fDirectionY = vec.y;
    m_bIsFlip = (pCaster->GetActionSide() == 1);
    D3DXVECTOR2 refVec(m_bIsFlip ? -1.0f : 1.0f, 0.0f);
    float dotProduct = D3DXVec2Dot(&refVec, &vec);
    m_fAngle = acosf(dotProduct);
    if (vec.y < 0) {
        m_fAngle = -m_fAngle;
    }
}

// 對應反組譯碼: 0x0052DE10 (CEffect_Battle_BowShoot::FrameProcess)
bool CEffect_Skill_Type_ShootUnit::FrameProcess(float fElapsedTime)
{
    // 步驟 1: 更新內部的視覺動畫 (火球燃燒、箭矢旋轉等)
    m_ccaEffect.FrameProcess(fElapsedTime);

    // 步驟 2: 使用專門的移動計時器來計算物理更新
    int moveFrameCount = 0;
    if (m_MovementFrameSkip.Update(fElapsedTime, moveFrameCount))
    {
        // 根據計算出的影格數來更新位置
        // 這確保了即使遊戲幀率波動，投射物的飛行速度也保持恆定
        float moveDistance = m_fSpeed * static_cast<float>(moveFrameCount);
        m_fTraveledDistance += moveDistance;

        // 步驟 3: 檢查是否到達終點
        if (m_fTraveledDistance >= m_fTotalDistance) {
            // 已到達，通知目標角色被擊中
            if (m_pTargetCharacter) {
                // 原始碼: *(_BYTE *)(32 * m_nHitInfoID + pTarget + 8099) = 16;
                m_pTargetCharacter->SetHited(m_nHitInfoID, 16); // 假設的函式
            }
            return true; // 返回 true，表示特效生命週期結束
        }
        else {
            // 尚未到達，更新當前世界座標
            m_fCurrentPosX += m_fDirectionX * moveDistance;
            m_fCurrentPosY += m_fDirectionY * moveDistance;
        }
    }

    return false; // 特效繼續存在
}

// 對應反組譯碼: 0x0052DF10 (CEffect_Battle_BowShoot::Process)
void CEffect_Skill_Type_ShootUnit::Process()
{
    // 將世界座標轉換為螢幕座標
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, screenY);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetRotation(m_fAngle);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x0052DF60 (CEffect_Battle_BowShoot::Draw)
void CEffect_Skill_Type_ShootUnit::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}