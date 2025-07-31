#include "Effect/CEffect_Player_PCRoom_Effect.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x00531E80
CEffect_Player_PCRoom_Effect::CEffect_Player_PCRoom_Effect()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫
    // 這個類別的特效資源是在 SetEffect 中動態載入的
}

// 對應反組譯碼: 0x00531EC0
CEffect_Player_PCRoom_Effect::~CEffect_Player_PCRoom_Effect()
{
}

// 對應反組譯碼: 0x00531ED0
void CEffect_Player_PCRoom_Effect::SetEffect(ClientCharacter* pOwner)
{
    if (!pOwner) return;

    // 載入特效數據
    CEAManager::GetInstance()->GetEAData(49, "Effect/efn_PCroom.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 動畫本身可能不循環，由 FrameProcess 控制其持續性

    // 綁定擁有者
    m_pOwnerCharacter = pOwner;
}

// 對應反組譯碼: 0x00531F10
bool CEffect_Player_PCRoom_Effect::FrameProcess(float fElapsedTime)
{
    // 更新內部動畫影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    // --- 核心生命週期判斷 ---
    ClientCharacter* pOwner = m_pOwnerCharacter;
    if (!pOwner) {
        return true; // 如果擁有者指標為空，特效結束
    }

    // 檢查角色是否仍處於 PC Room 狀態
    // 原始碼: if ( *(_DWORD *)(v3 + 11556) ) return 0;
    //         if ( *(_BYTE *)(v3 + 11560) == 1 ) return 0;
    if (pOwner->IsPCRoomUser() || pOwner->IsPCRoomPremium()) { // 假設的函式
        // 如果是，特效繼續存在
        return false;
    }

    // 否則，特效結束
    return true;
}

// 對應反組譯碼: 0x00531F80
void CEffect_Player_PCRoom_Effect::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // 如果角色處於隱身等特殊狀態，則不顯示
    if (m_pOwnerCharacter->IsHide() || m_pOwnerCharacter->GetSomeOtherState()) {
        m_bIsVisible = FALSE;
        return;
    }

    // 將世界座標轉換為螢幕座標
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    // 根據角色朝向設定翻轉
    m_bIsFlip = (m_pOwnerCharacter->GetActionSide() != 0);

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x00532040
void CEffect_Player_PCRoom_Effect::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}