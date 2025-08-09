#include "Effect/CEffect_Battle_MagicBook.h"
#include "Effect/CEAManager.h"
#include "Image/CDeviceManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"
#include <cmath>

// 對應反組譯碼: 0x0052EBA0
CEffect_Battle_MagicBook::CEffect_Battle_MagicBook()
    : m_pTargetCharacter(nullptr),
    m_fTotalDistance(0.0f),
    m_fTraveledDistance(0.0f),
    m_fAngle(0.0f),
    m_nHitInfoID(0)
{
    // CEffectBase 建構函式已自動呼叫

    // 載入特效的視覺動畫
    CEAManager::GetInstance()->GetEAData(6, "Effect/efn-firebids01.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // 投射物動畫應循環播放

    // 設定移動計時器的更新頻率 (30 FPS)
    // 原始碼: *((_DWORD *)this + 38) = 995783694;
    m_MovementFrameSkip.m_fTimePerFrame = 1.0f / 30.0f;
}

// 對應反組譯碼: 0x0052EC70
CEffect_Battle_MagicBook::~CEffect_Battle_MagicBook()
{
}

// 對應反組譯碼: 0x0052EC90
void CEffect_Battle_MagicBook::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, bool bFlip, int hitInfoID)
{
    if (!pCaster || !pTarget) return;

    m_pTargetCharacter = pTarget;
    m_nHitInfoID = hitInfoID;
    m_fSpeed = 10.0f;

    // --- 步驟 1: 正常載入特效數據 ---
    CEAManager::GetInstance()->GetEAData(6, "MoFData/Effect/efn-firebids01.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true);

    // --- 步驟 2: 手動覆寫混合模式 (關鍵修正) ---
    // 無論檔案中定義的是什麼，我們都強制其使用加法混合
    m_ccaEffect.m_ucSrcBlend = D3DBLEND_SRCALPHA; // 值為 5
    m_ccaEffect.m_ucDestBlend = D3DBLEND_ONE;      // 值為 2

    // --- 後續的物理計算不變 ---
    D3DXVECTOR2 startPos(static_cast<float>(pCaster->GetPosX()), static_cast<float>(pCaster->GetPosY() - 15));
    D3DXVECTOR2 endPos(static_cast<float>(pTarget->GetPosX()), static_cast<float>(pTarget->GetPosY() - (pTarget->GetCharHeight() >> 2)));

    m_fCurrentPosX = startPos.x;
    m_fCurrentPosY = startPos.y;

    D3DXVECTOR2 vecDirection = endPos - startPos;
    m_fTotalDistance = D3DXVec2Length(&vecDirection);

    D3DXVec2Normalize(&vecDirection, &vecDirection);
    m_fDirectionX = vecDirection.x;
    m_fDirectionY = vecDirection.y;

    m_bIsFlip = (pCaster->GetPosX() > pTarget->GetPosX());

    D3DXVECTOR2 refVec(m_bIsFlip ? -1.0f : 1.0f, 0.0f);
    float dotProduct = D3DXVec2Dot(&refVec, &vecDirection);
    m_fAngle = acosf(dotProduct);
    if (vecDirection.y < 0) {
        m_fAngle = -m_fAngle;
    }
}

// 對應反組譯碼: 0x0052EF90
void CEffect_Battle_MagicBook::SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fFlip, int hitInfoID)
{
    if (!pStartPos || !pEndPos) return;

    m_nHitInfoID = hitInfoID;
    m_fSpeed = 10.0f; // 同樣提升速度
    m_fCurrentPosX = pStartPos->x;
    m_fCurrentPosY = pStartPos->y;

    D3DXVECTOR2 vecDirection = *pEndPos - *pStartPos;
    m_fTotalDistance = D3DXVec2Length(&vecDirection);

    D3DXVec2Normalize(&vecDirection, &vecDirection);
    m_fDirectionX = vecDirection.x;
    m_fDirectionY = vecDirection.y;

    m_bIsFlip = (static_cast<int>(fFlip) != 0);

    D3DXVECTOR2 refVec(m_bIsFlip ? -1.0f : 1.0f, 0.0f);
    float dotProduct = D3DXVec2Dot(&refVec, &vecDirection);
    m_fAngle = acosf(dotProduct);
    if (vecDirection.y < 0) {
        m_fAngle = -m_fAngle;
    }
}

// 對應反組譯碼: 0x0052F1A0
bool CEffect_Battle_MagicBook::FrameProcess(float fElapsedTime)
{
    m_ccaEffect.FrameProcess(fElapsedTime);

    int moveFrameCount = 0;
    if (m_MovementFrameSkip.Update(fElapsedTime, moveFrameCount)) {
        // --- 銷毀 Bug 修正 ---
        // 實際移動距離需要乘上速度
        float moveDistance = m_fSpeed * static_cast<float>(moveFrameCount);
        m_fTraveledDistance += moveDistance;

        if (m_fTraveledDistance >= m_fTotalDistance) {
            if (m_pTargetCharacter) {
                m_pTargetCharacter->SetHited(m_nHitInfoID, 16);
            }
            return true; // 到達目標，返回 true 以銷毀特效
        }
        else {
            // 更新位置
            m_fCurrentPosX += m_fDirectionX * moveDistance;
            m_fCurrentPosY += m_fDirectionY * moveDistance;
        }
    }
    return false;
}

// 對應反組譯碼: 0x0052F2A0
void CEffect_Battle_MagicBook::Process()
{
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetRotation(m_fAngle);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x0052F2F0
void CEffect_Battle_MagicBook::Draw()
{
    if (m_bIsVisible) {
        // --- 繪製邏輯簡化 ---
        // 現在不再需要手動設定渲染狀態，因為 m_ccaEffect 內部會自動
        // 使用我們在 SetEffect 中修正過的正確混合模式。
        m_ccaEffect.Draw();
    }
}