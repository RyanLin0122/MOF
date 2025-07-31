#include "Effect/CEffect_Battle_StaffShoot.h"
#include "Effect/CEAManager.h"
#include "global.h"
#include <cmath>

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x0052F420
CEffect_Battle_StaffShoot::CEffect_Battle_StaffShoot()
    : m_fTraveledDistance(0.0f), m_fTotalDistance(0.0f)
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 11, 檔案名稱: "Effect/efn_staffshoot.ea"
    CEAManager::GetInstance()->GetEAData(11, "Effect/efn_staffshoot.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 動畫只播放一次
}

// 對應反組譯碼: 0x0052F4C0
CEffect_Battle_StaffShoot::~CEffect_Battle_StaffShoot()
{
}

// 對應反組譯碼: 0x0052F4D0
void CEffect_Battle_StaffShoot::SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, bool bFlip)
{
    if (!pStartPos || !pEndPos) return;

    // 步驟 1: 設定初始位置
    m_fCurrentPosX = pStartPos->x;
    m_fCurrentPosY = pStartPos->y;

    // 步驟 2: 計算飛行向量
    D3DXVECTOR2 vecDirection = *pEndPos - *pStartPos;

    // 步驟 3: 計算總距離和速度
    m_fTotalDistance = D3DXVec2Length(&vecDirection);
    m_fSpeed = m_fTotalDistance * 0.02f; // 速度是總距離的 2%

    // 步驟 4: 將向量單位化以獲得純方向
    D3DXVec2Normalize(&vecDirection, &vecDirection);
    m_fDirectionX = vecDirection.x;
    m_fDirectionY = vecDirection.y;

    // 步驟 5: 設定翻轉
    m_bIsFlip = bFlip;
}

// 對應反組譯碼: 0x0052F570
bool CEffect_Battle_StaffShoot::FrameProcess(float fElapsedTime)
{
    // 更新視覺動畫的影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    // 檢查是否已到達或超過目標距離
    // 原始碼: return *((float *)this + 33) > (double)*((float *)this + 34);
    return m_fTraveledDistance > m_fTotalDistance;
}

// 對應反組譯碼: 0x0052F5A0
void CEffect_Battle_StaffShoot::Process()
{
    // 步驟 1: 更新位置
    // 原始碼: v5 = *((float *)this + 4) + *((float *)this + 2);
    //         v2 = *((float *)this + 5) + *((float *)this + 3);
    // 注意：原始碼中將速度乘入了方向向量，我們的版本將在 Process 中應用速度
    m_fCurrentPosX += m_fDirectionX * m_fSpeed;
    m_fCurrentPosY += m_fDirectionY * m_fSpeed;

    // 步驟 2: 累計已飛行距離
    // 原始碼: *((float *)this + 33) = *((float *)this + 6) + *((float *)this + 33);
    m_fTraveledDistance += m_fSpeed;

    // 步驟 3: 轉換為螢幕座標並進行裁剪判斷
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x0052F620
void CEffect_Battle_StaffShoot::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}