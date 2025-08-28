#include "Effect/CEffect_Skill_OtherHeal.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x00533B20
CEffect_Skill_OtherHeal::CEffect_Skill_OtherHeal()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫
    // 特效資源是在 SetEffect 中動態載入的
}

// 對應反組譯碼: 0x00533B60
CEffect_Skill_OtherHeal::~CEffect_Skill_OtherHeal()
{
}

// 對應反組譯碼: 0x00533B70
void CEffect_Skill_OtherHeal::SetEffect(ClientCharacter* pTarget, unsigned char skillLevel)
{
    if (!pTarget) return;

    const char* szFileName = nullptr;
    int effectID = 0;

    // 根據技能等級選擇不同的特效資源
    if (skillLevel == 1) {
        effectID = 78;
        szFileName = "Effect/efn_OtherHeal_TopLv2.ea";
    }
    else if (skillLevel == 2) {
        effectID = 79;
        szFileName = "Effect/efn_OtherHeal_TopLv3.ea";
    }
    else { // 預設為等級 0
        effectID = 77;
        szFileName = "Effect/efn_OtherHeal.ea";
    }

    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(effectID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 綁定擁有者
    m_pOwnerCharacter = pTarget;
}

// 對應反組譯碼: 0x00533BE0
bool CEffect_Skill_OtherHeal::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00533BF0
void CEffect_Skill_OtherHeal::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- 核心邏輯：持續追蹤擁有者位置 ---
    // 將世界座標轉換為螢幕座標
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x00533C70
void CEffect_Skill_OtherHeal::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}