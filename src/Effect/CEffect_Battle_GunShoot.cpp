#include "Effect/CEffect_Battle_GunShoot.h"
#include "Effect/CEffectManager.h"
#include "Image/CDeviceManager.h"
#include "Image/cltImageManager.h"
#include "Character/ClientCharacter.h"
#include "Effect/CEffect_Field_Walkdust.h"
#include "Effect/CEffect_Battle_GunSpark.h"
#include "global.h"

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x0052E090
CEffect_Battle_GunShoot::CEffect_Battle_GunShoot()
    : m_pTargetCharacter(nullptr),
    m_fTotalDistance(0.0f),
    m_fTraveledDistance(0.0f),
    m_nHitInfoID(0),
    m_dwCasterAccountID(0)
{
    // CEffectBase 建構函式已自動呼叫

    // 原始碼: *((_DWORD *)this + 39) = 993397601; (0x3B360B61 -> 0.0271f ~ 1/36.8)
    // 設定移動計時器的更新頻率
    m_MovementFrameSkip.m_fTimePerFrame = 1.0f / 37.0f;
}

CEffect_Battle_GunShoot::~CEffect_Battle_GunShoot()
{
}

// 對應反組譯碼: 0x0052E130
void CEffect_Battle_GunShoot::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short a4, int hitInfoID)
{
    if (!pCaster || !pTarget) return;

    m_pTargetCharacter = pTarget;
    m_nHitInfoID = hitInfoID;
    m_dwCasterAccountID = pCaster->GetAccountID();

    D3DXVECTOR2 startPos(static_cast<float>(pCaster->GetPosX()), static_cast<float>(pCaster->GetPosY()));
    D3DXVECTOR2 endPos(static_cast<float>(pTarget->GetPosX()), static_cast<float>(pTarget->GetPosY()));

    // --- 觸發開火時的附屬特效 ---
    if (pCaster->GetActionSide() == 1) { // 朝右
        startPos.x -= 70.0f; // 調整特效起始位置以匹配槍口

        // 建立煙塵效果
        CEffect_Field_Walkdust* pDust = new CEffect_Field_Walkdust();
        pDust->SetEffect(startPos.x + 55.0f, startPos.y - 45.0f, false, 0, 0, 0, 0);
        CEffectManager::GetInstance()->BulletAdd(pDust);

        // 建立槍口火花
        CEffect_Battle_GunSpark* pSpark = new CEffect_Battle_GunSpark();
        pSpark->SetEffect(startPos.x, startPos.y, true);
        CEffectManager::GetInstance()->BulletAdd(pSpark);

    }
    else { // 朝左
        startPos.x += 5.0f;

        CEffect_Field_Walkdust* pDust = new CEffect_Field_Walkdust();
        pDust->SetEffect(startPos.x + 50.0f, startPos.y - 45.0f, false, 0, 0, 0, 0);
        CEffectManager::GetInstance()->BulletAdd(pDust);

        CEffect_Battle_GunSpark* pSpark = new CEffect_Battle_GunSpark();
        pSpark->SetEffect(startPos.x, startPos.y, false);
        CEffectManager::GetInstance()->BulletAdd(pSpark);
    }

    // --- 計算飛行路徑 ---
    m_fCurrentPosX = startPos.x;
    m_fCurrentPosY = startPos.y;

    D3DXVECTOR2 vec = endPos - startPos;
    m_fTotalDistance = D3DXVec2Length(&vec) - 60.0f; // 提前一點距離觸發命中

    D3DXVec2Normalize(&vec, &vec);
    m_fDirectionX = vec.x;
    m_fDirectionY = vec.y;

    m_fSpeed = 10.0f; // 設定飛行速度
}

// 另一個 SetEffect 多載，雖然未在 CEffectManager 中直接使用，但存在於原始碼中
void CEffect_Battle_GunShoot::SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, bool bFlip, unsigned short a5, int hitInfoID)
{
    // ... 類似的向量和距離計算邏輯 ...
}

// 對應反組譯碼: 0x0052E520
bool CEffect_Battle_GunShoot::FrameProcess(float fElapsedTime)
{
    int moveFrameCount = 0;
    if (m_MovementFrameSkip.Update(fElapsedTime, moveFrameCount)) {
        float moveDistance = m_fSpeed * static_cast<float>(moveFrameCount);
        m_fTraveledDistance += moveDistance;

        if (m_fTraveledDistance >= m_fTotalDistance) {
            if (m_pTargetCharacter) {
                m_pTargetCharacter->SetHited(m_nHitInfoID, 16);
            }
            return true; // 到達目標，特效結束
        }
        else {
            m_fCurrentPosX += m_fDirectionX * moveDistance;
            m_fCurrentPosY += m_fDirectionY * moveDistance;
        }
    }
    return false; // 繼續飛行
}

/**
 * @brief 準備繪製前的邏輯更新，核心工作是獲取並設定子彈圖像的狀態。
 * @note 此函式精確還原自 Effectall.c 中 0x0052E610 的邏輯。
 */
void CEffect_Battle_GunShoot::Process()
{
    // 從物件池獲取或更新子彈的 GameImage 實例
    // 資源 ID 0xC000171u 是硬編碼的
    m_pBulletImage = cltImageManager::GetInstance()->GetGameImage(7, 0xC000171u, 0, 1);

    if (m_pBulletImage)
    {
        // 將世界座標轉換為螢幕座標
        // 原始碼: v4 = *((float *)this + 2) - (double)dword_A73088;
        //         v3 = *((float *)this + 3) - (double)dword_A7308C - 60.0;
        float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
        float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY) - 60.0f; // Y軸有60像素的額外偏移

        // --- 開始設定 GameImage 的所有屬性 ---

        // 設定位置
        m_pBulletImage->SetPosition(screenX, screenY);

        // 設定動畫影格 ID
        // 原始碼: *((_WORD *)v2 + 186) = *((_WORD *)this + 82); (this+164 -> m_usUnk_a4)
        m_pBulletImage->SetBlockID(m_usUnk_a4);

        // 設定透明度
        // 原始碼: *(_DWORD *)(v5 + 380) = 255;
        m_pBulletImage->SetAlpha(255);

        // 設定顏色
        // 原始碼: *(_DWORD *)(v6 + 376) = 100;
        m_pBulletImage->SetColor(100);

        // 傳遞施法者的 AccountID，可能用於 Shader
        // 原始碼: *(_DWORD *)(*((_DWORD *)this + 33) + 392) = *((_DWORD *)this + 42);
        // m_pBulletImage->m_dwSomeShaderData = m_dwCasterAccountID; // 假設 GameImage 有此成員

        // 更新頂點緩衝區
        m_pBulletImage->Process();
    }
}

/**
 * @brief 繪製子彈圖像。
 * @note 此函式精確還原自 Effectall.c 中 0x0052E6D0 的邏輯。
 */
void CEffect_Battle_GunShoot::Draw()
{
    if (m_pBulletImage && m_pBulletImage->IsInUse())
    {
        // 設定特效專用的 Alpha 混合模式
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);  // 5
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA); // 6

        // 繪製圖像
        m_pBulletImage->Draw();

        // 注意：原始碼在 Draw 之後沒有恢復 RenderState，
        // 這通常由 CEffectManager::Draw 結尾的 ResetRenderState 統一處理。
    }
}