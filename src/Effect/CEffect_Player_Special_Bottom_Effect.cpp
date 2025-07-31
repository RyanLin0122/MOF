#include "Effect/CEffect_Player_Special_Bottom_Effect.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x00532060
CEffect_Player_Special_Bottom_Effect::CEffect_Player_Special_Bottom_Effect()
    : m_pOwnerCharacter(nullptr), m_ucEffectType(0)
{
    // CEffectBase 的建構函式會被自動呼叫
}

// 對應反組譯碼: 0x005320A0
CEffect_Player_Special_Bottom_Effect::~CEffect_Player_Special_Bottom_Effect()
{
}

// 對應反組譯碼: 0x005320B0
void CEffect_Player_Special_Bottom_Effect::SetEffect(ClientCharacter* pOwner, unsigned char effectType)
{
    if (!pOwner) return;

    m_ucEffectType = effectType;

    const char* szFileName = nullptr;
    int effectID = 0;

    // 根據傳入的 effectType 選擇要載入的特效資源
    if (effectType == 1) {
        effectID = 52;
        szFileName = "Effect/efn-LiveeventMc-Bottom.ea";
    }
    else if (effectType == 0) {
        effectID = 49;
        szFileName = "Effect/efn_PCroom.ea";
    }
    else {
        // 原始碼中 v5 未定義，表示如果 effectType 不是 0 或 1，可能會導致錯誤
        // 這裡我們假設有一個預設或錯誤處理
        return;
    }

    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(effectID, (char*)szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 動畫本身可能不循環，由 FrameProcess 控制其持續性

    // 綁定擁有者
    m_pOwnerCharacter = pOwner;
}

// 對應反組譯碼: 0x00532180
bool CEffect_Player_Special_Bottom_Effect::FrameProcess(float fElapsedTime)
{
    // 更新內部動畫影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    // --- 核心生命週期判斷 ---
    // 原始碼: return *((_DWORD *)this + 33) == 0;
    // 如果擁有者指標被外部設為 nullptr，則特效結束
    return (m_pOwnerCharacter == nullptr);
}

// 對應反組譯碼: 0x005321C0
void CEffect_Player_Special_Bottom_Effect::Process()
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

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x00532270
void CEffect_Player_Special_Bottom_Effect::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}