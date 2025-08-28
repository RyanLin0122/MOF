#include "Effect/CEffect_Skill_Type_Once.h"
#include "Effect/CEAManager.h"       // 需要從 CEAManager 獲取特效數據
#include "Character/ClientCharacter.h" // 需要 ClientCharacter 的定義
#include "global.h"                      // 用於獲取全域變數 g_Game_System_Info

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;


// 對應反組譯碼: (在 CEffectManager::AddEffect 中被 new)
CEffect_Skill_Type_Once::CEffect_Skill_Type_Once()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫
}

// 對應反組譯碼: (在 CEffectManager::FrameProcess 中被 delete)
CEffect_Skill_Type_Once::~CEffect_Skill_Type_Once()
{
}

// 對應反組譯碼: (在 CEffectManager::AddEffect 中被呼叫)
void CEffect_Skill_Type_Once::SetEffect(ClientCharacter* pCaster, unsigned short effectKindID, char* szFileName)
{
    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(effectKindID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，並且不循環

    // 綁定擁有者
    m_pOwnerCharacter = pCaster;
}

// 對應反組譯碼: (在 CEffectManager::FrameProcess 中被呼叫)
bool CEffect_Skill_Type_Once::FrameProcess(float fElapsedTime)
{
    // 直接將生命週期管理的任務委託給內部的 CCAEffect 物件。
    // 當 CCAEffect 的動畫播放完畢時，此函式會回傳 true，
    // CEffectManager 隨後會將此特效實例銷毀。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: (在 CEffectManager::Process 中被呼叫)
void CEffect_Skill_Type_Once::Process()
{
    // 檢查擁有者是否存在
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // 從擁有者獲取當前位置
    float ownerX = static_cast<float>(m_pOwnerCharacter->GetPosX());
    float ownerY = static_cast<float>(m_pOwnerCharacter->GetPosY());

    // 設定特效自身的繪製座標
    // 原始碼: *((float *)this + 12) = (float)(v2[1096] - g_Game_System_Info.ScreenX);
    //         *((float *)this + 13) = (float)(v2[1097] - g_Game_System_Info.ScreenY);
    m_fCurrentPosX = ownerX;
    m_fCurrentPosY = ownerY;

    // 進行裁剪判斷
    // 減去攝影機座標，轉換為螢幕座標
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    m_bIsVisible = IsCliping(screenX, 0.0f); // 原始碼只檢查 X 軸

    // 如果可見，則更新內部 CCAEffect 的狀態並處理其動畫
    if (m_bIsVisible) {
        // 將世界座標轉換為螢幕座標後傳遞給 CCAEffect
        m_ccaEffect.SetPosition(screenX, m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY));
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: (在 CEffectManager::Draw 中被呼叫)
void CEffect_Skill_Type_Once::Draw()
{
    // 只有在 Process() 函式中被標記為可見時才進行繪製
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}