#include "Effect/CEffect_Skill_Integrity.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x00533560
CEffect_Skill_Integrity::CEffect_Skill_Integrity()
    : m_pOwnerCharacter(nullptr),
    m_nUnknown_a3(0),
    m_fInitialPosX(0.0f),
    m_fInitialPosY(0.0f)
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 95, 檔案名稱: "Effect/efn_Integrity.ea"
    CEAManager::GetInstance()->GetEAData(95, "Effect/efn_Integrity.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 動畫本身可能不循環，由 FrameProcess 控制其持續性
}

// 對應反組譯碼: 0x00533600
CEffect_Skill_Integrity::~CEffect_Skill_Integrity()
{
}

// 對應反組譯碼: 0x00533610
void CEffect_Skill_Integrity::SetEffect(ClientCharacter* pTarget, int a3)
{
    if (!pTarget) return;

    // 綁定擁有者
    m_pOwnerCharacter = pTarget;
    m_nUnknown_a3 = a3;

    // --- 核心邏輯：儲存帶有垂直偏移的位置快照 ---
    // 原始碼: *((float *)this + 36) = (float)*((int *)a2 + 1096);
    //         v3 = *((_DWORD *)a2 + 1097);
    //         *((float *)this + 37) = (float)(v3 - ClientCharacter::GetCharHeight(a2) + 70);
    m_fInitialPosX = static_cast<float>(pTarget->GetPosX());
    m_fInitialPosY = static_cast<float>(pTarget->GetPosY() - pTarget->GetCharHeight() + 70);
}

// 對應反組譯碼: 0x00533660
bool CEffect_Skill_Integrity::FrameProcess(float fElapsedTime)
{
    // 更新內部動畫影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    if (!m_pOwnerCharacter) {
        return true;
    }

    // --- 核心生命週期判斷 ---
    // 原始碼: return (*(_BYTE *)(*((_DWORD *)this + 33) + 9700) & 1) == 0;
    // 檢查角色的特殊狀態旗標中，代表「Integrity」的位元是否被清除
    if (m_pOwnerCharacter->IsIntegrityActive()) { // 假設 IsIntegrityActive() 實現了該位元檢查
        return false; // 如果仍在狀態中，特效繼續
    }

    return true; // 否則，特效結束
}

// 對應反組譯碼: 0x00533690
void CEffect_Skill_Integrity::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // 將世界座標轉換為螢幕座標
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x005336F0
void CEffect_Skill_Integrity::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}