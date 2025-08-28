#include "Effect/CEffect_Skill_Take.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x00533F40
CEffect_Skill_Take::CEffect_Skill_Take()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 93, 檔案名稱: "Effect/efn_TakeSkill.ea"
    CEAManager::GetInstance()->GetEAData(93, "Effect/efn_TakeSkill.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環
}

// 對應反組譯碼: 0x00533FE0
CEffect_Skill_Take::~CEffect_Skill_Take()
{
}

// 對應反組譯碼: 0x00533FF0
void CEffect_Skill_Take::SetEffect(ClientCharacter* pOwner)
{
    // 綁定擁有者
    // 原始碼: *((_DWORD *)this + 33) = a2;
    m_pOwnerCharacter = pOwner;
}

// 對應反組譯碼: 0x00534000
bool CEffect_Skill_Take::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    // 當 "efn_TakeSkill.ea" 動畫播放完畢時，此函式會回傳 true。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00534010
void CEffect_Skill_Take::Process()
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

// 對應反組譯碼: 0x00534090
void CEffect_Skill_Take::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}