#include "Effect/CEffect_Player_EnchantLevel.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x00531530
CEffect_Player_EnchantLevel::CEffect_Player_EnchantLevel()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫
    // 這個類別的特效資源是在 SetEffect 中動態載入的
}

// 對應反組譯碼: 0x00531570
CEffect_Player_EnchantLevel::~CEffect_Player_EnchantLevel()
{
}

// 對應反組譯碼: 0x00531580
void CEffect_Player_EnchantLevel::SetEffect(ClientCharacter* pOwner, unsigned short effectKindID, char* szFileName)
{
    if (!pOwner) return;

    // 構建檔案路徑並載入特效數據
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", szFileName);
    CEAManager::GetInstance()->GetEAData(effectKindID, szFullPath, &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 動畫本身可能不循環，由 FrameProcess 控制其持續性

    // 綁定擁有者
    m_pOwnerCharacter = pOwner;
}

// 對應反組譯碼: 0x005315F0
bool CEffect_Player_EnchantLevel::FrameProcess(float fElapsedTime)
{
    // 更新內部動畫影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    // 檢查擁有者是否存在
    if (!m_pOwnerCharacter) {
        return true;
    }

    // --- 核心生命週期判斷 ---
    // 原始碼: if ( v3[2421] ) return 1; ...
    if (m_pOwnerCharacter->GetActionState() != 0 || // 不再是站立狀態
        m_pOwnerCharacter->IsTransformed() ||       // 處於變身狀態
        m_pOwnerCharacter->IsHide() ||              // 處於隱身狀態
        m_pOwnerCharacter->GetSomeOtherState()) {   // 處於某個其他特殊狀態
        return true; // 任何一個條件滿足，就結束特效
    }

    // 如果所有條件都不滿足，特效繼續存在
    return false;
}

// 對應反組譯碼: 0x00531650
void CEffect_Player_EnchantLevel::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // 將世界座標轉換為螢幕座標
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    // 根據角色朝向設定翻轉
    m_bIsFlip = (m_pOwnerCharacter->GetActionSide() == 1);

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x005316D0
void CEffect_Player_EnchantLevel::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}