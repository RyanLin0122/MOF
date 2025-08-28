#include "Effect/CEffect_Skill_SpeedUP.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x00533DD0
CEffect_Skill_SpeedUP::CEffect_Skill_SpeedUP()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 99, 檔案名稱: "Effect/efn_SpeedUp.ea"
    CEAManager::GetInstance()->GetEAData(99, "Effect/efn_SpeedUp.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環
}

// 對應反組譯碼: 0x00533E70
CEffect_Skill_SpeedUP::~CEffect_Skill_SpeedUP()
{
}

// 對應反組譯碼: 0x00533E80
void CEffect_Skill_SpeedUP::SetEffect(ClientCharacter* pOwner)
{
    if (!pOwner) return;

    // 綁定擁有者
    m_pOwnerCharacter = pOwner;

    // 原始碼: *((_DWORD *)a2 + 174) = 0;
    // 立即設定角色的一個旗標（可能是「準備加速」）
    pOwner->SetPreparingSpeedUp(false); // 假設的函式
}

// 對應反組譯碼: 0x00533EA0
bool CEffect_Skill_SpeedUP::FrameProcess(float fElapsedTime)
{
    // --- 核心生命週期與觸發邏輯 ---

    // 檢查動畫是否播放完畢
    if (m_ccaEffect.FrameProcess(fElapsedTime)) {
        // 動畫已結束
        if (m_pOwnerCharacter) {
            // 原始碼: *(_DWORD *)(v3 + 696) = 1;
            // 設定角色身上真正的「加速」旗標
            m_pOwnerCharacter->SetCanSpeedUp(true); // 假設的函式
        }
        return true; // 特效結束
    }

    // 動畫尚未結束，特效繼續存在
    return false;
}

// 對應反組譯碼: 0x00533EE0
void CEffect_Skill_SpeedUP::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- 持續追蹤擁有者位置 ---
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    // 更新內部 CCAEffect 的狀態
    m_ccaEffect.SetPosition(screenX, screenY);
    m_ccaEffect.Process();

    m_bIsVisible = TRUE;
}

// 對應反組譯碼: 0x00533F30
void CEffect_Skill_SpeedUP::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}