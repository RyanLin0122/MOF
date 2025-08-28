#include "Effect/CEffect_Battle_BowShoot.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x0052D770
CEffect_Battle_BowShoot::CEffect_Battle_BowShoot()
    : m_pTargetCharacter(nullptr),
    m_fTotalDistance(0.0f),
    m_fTraveledDistance(0.0f),
    m_fAngle(0.0f),
    m_nHitInfoID(0)
{
    // CEffectBase 建構函式已自動呼叫

    // 載入箭矢的視覺動畫
    CEAManager::GetInstance()->GetEAData(3, "Effect/efn_bowshoot.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // 箭矢飛行動畫應循環播放

    // 設定移動計時器的更新頻率 (30 FPS)
    // 原始碼: *((_DWORD *)this + 38) = 995783694;
    m_MovementFrameSkip.m_fTimePerFrame = 1.0f / 30.0f;
}

// 對應反組譯碼: 0x0052D850
CEffect_Battle_BowShoot::~CEffect_Battle_BowShoot()
{
}

// 對應反組譯碼: 0x0052D870
void CEffect_Battle_BowShoot::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, bool a4, int hitInfoID)
{
    if (!pCaster || !pTarget) return;

    m_pTargetCharacter = pTarget;
    m_nHitInfoID = hitInfoID;
    m_fSpeed = 2.0f; // 原始碼硬編碼

    D3DXVECTOR2 startPos(static_cast<float>(pCaster->GetPosX()), static_cast<float>(pCaster->GetPosY()));
    D3DXVECTOR2 endPos(static_cast<float>(pTarget->GetPosX()), static_cast<float>(pTarget->GetPosY()));

    m_fCurrentPosX = startPos.x;
    m_fCurrentPosY = startPos.y;

    D3DXVECTOR2 vec = endPos - startPos;
    m_fTotalDistance = D3DXVec2Length(&vec) - 60.0f;
    if (m_fTotalDistance < 0.0f) m_fTotalDistance = 0.0f;

    D3DXVec2Normalize(&vec, &vec);
    m_fDirectionX = vec.x * m_fSpeed; // 原始碼在此處將速度乘入了方向向量
    m_fDirectionY = vec.y * m_fSpeed;

    // 根據施法者與目標的相對位置決定是否翻轉
    m_bIsFlip = (pCaster->GetPosX() - pTarget->GetPosX()) > 0;

    // 計算飛行角度
    D3DXVECTOR2 refVec(m_bIsFlip ? -1.0f : 1.0f, 0.0f);
    float dotProduct = D3DXVec2Dot(&refVec, &vec);
    m_fAngle = acosf(dotProduct);

    // 原始碼中有一個複雜的判斷決定角度正負，可簡化為檢查 Y 分量
    if (vec.y < 0) {
        m_fAngle = -m_fAngle;
    }
}

/**
 * @brief 設定弓箭特效 (座標版本)。
 * @param pStartPos 投射物的起始世界座標。
 * @param pEndPos 投射物的目標世界座標。
 * @param fFlip 翻轉旗標 (在原始碼中為 float，但行為類似 bool)。
 * @param hitInfoID 擊中目標時傳遞的資訊 ID。
 * @note 此函式的邏輯精確還原自 Effectall.c 中 0x0052DC00 的同名函式。
 */
void CEffect_Battle_BowShoot::SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fFlip, int hitInfoID)
{
    if (!pStartPos || !pEndPos) return;

    // 步驟 1: 設定初始屬性
    m_nHitInfoID = hitInfoID;
    m_fSpeed = 2.0f; // 參照角色版本的設定，設定一個基礎速度

    // 設定特效的初始位置
    // 原始碼: *((_DWORD *)this + 2) = *(_DWORD *)a2;
    //         *((_DWORD *)this + 3) = *((_DWORD *)a2 + 1);
    m_fCurrentPosX = pStartPos->x;
    m_fCurrentPosY = pStartPos->y;

    // 步驟 2: 計算飛行向量和總距離
    // 原始碼: v19 = *(float *)a3 - *(float *)a2;
    //         v7 = *((float *)a3 + 1) - *((float *)a2 + 1);
    D3DXVECTOR2 vecDirection = *pEndPos - *pStartPos;

    // 原始碼: *((float *)this + 34) = sqrt(v8 * v8 + v9) - 60.0;
    m_fTotalDistance = D3DXVec2Length(&vecDirection) - 60.0f;
    if (m_fTotalDistance < 0.0f) {
        m_fTotalDistance = 0.0f;
    }

    // 步驟 3: 將向量單位化以獲得純方向，並乘上速度
    // 原始碼: D3DXVec2Normalize((char *)this + 16, (char *)this + 16);
    //         v20 = *((float *)this + 6) * *v6;
    D3DXVec2Normalize(&vecDirection, &vecDirection);
    m_fDirectionX = vecDirection.x * m_fSpeed;
    m_fDirectionY = vecDirection.y * m_fSpeed;

    // 步驟 4: 設定翻轉旗標
    // 原始碼: *((_BYTE *)this + 56) = LOBYTE(a4);
    m_bIsFlip = (static_cast<int>(fFlip) != 0);

    // 步驟 5: 計算飛行角度以便正確渲染
    // 這段邏輯與角色版本的 SetEffect 完全相同，用於使箭矢視覺上朝向目標
    D3DXVECTOR2 refVec(m_bIsFlip ? -1.0f : 1.0f, 0.0f); // 根據翻轉狀態設定水平參考向量

    // 計算參考向量與飛行方向向量的點積
    float dotProduct = D3DXVec2Dot(&refVec, &vecDirection);

    // 使用反餘弦函數計算夾角（弧度）
    m_fAngle = acosf(dotProduct);

    // 根據 Y 軸方向決定角度的正負
    // 原始碼中 v14 | v15 是一個複雜且可能因反組譯出錯的判斷，
    // 其根本目的是判斷向量是在參考向量的順時針還是逆時針方向。
    // 檢查 Y 分量是更直接且等效的實現。
    if (vecDirection.y < 0) {
        m_fAngle = -m_fAngle;
    }
}

// 對應反組譯碼: 0x0052DE10
bool CEffect_Battle_BowShoot::FrameProcess(float fElapsedTime)
{
    m_ccaEffect.FrameProcess(fElapsedTime);

    int moveFrameCount = 0;
    if (m_MovementFrameSkip.Update(fElapsedTime, moveFrameCount)) {
        float moveDistance = static_cast<float>(moveFrameCount); // 速度已乘入向量，這裡係數為1
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

// 對應反組譯碼: 0x0052DF10
void CEffect_Battle_BowShoot::Process()
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

// 對應反組譯碼: 0x0052DF60
void CEffect_Battle_BowShoot::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}