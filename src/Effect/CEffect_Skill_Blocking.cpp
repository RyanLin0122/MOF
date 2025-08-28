#include "Effect/CEffect_Skill_Blocking.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x005330E0
CEffect_Skill_Blocking::CEffect_Skill_Blocking()
    : m_pOwnerCharacter(nullptr),
    m_fInitialPosX(0.0f),
    m_fInitialPosY(0.0f)
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 96, 檔案名稱: "Effect/efn_Weight_01.ea"
    CEAManager::GetInstance()->GetEAData(96, "Effect/efn_Weight_01.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 動畫本身可能不循環，由 FrameProcess 控制其持續性
}

// 對應反組譯碼: 0x00533180
CEffect_Skill_Blocking::~CEffect_Skill_Blocking()
{
}

// 對應反組譯碼: 0x00533190
void CEffect_Skill_Blocking::SetEffect(ClientCharacter* pTarget)
{
    if (!pTarget) return;

    // 綁定擁有者
    m_pOwnerCharacter = pTarget;

    // --- 核心邏輯：儲存帶有垂直偏移的位置快照 ---
    // 原始碼: *((float *)this + 34) = (float)*((int *)a2 + 1096);
    //         *((float *)this + 35) = (float)(*((_DWORD *)a2 + 1097) - *((unsigned __int16 *)a2 + 2216) + 70);
    m_fInitialPosX = static_cast<float>(pTarget->GetPosX());
    m_fInitialPosY = static_cast<float>(pTarget->GetPosY() - pTarget->GetCharHeight() + 70);
}

// 對應反組譯碼: 0x005331D0
bool CEffect_Skill_Blocking::FrameProcess(float fElapsedTime)
{
    // 更新內部動畫影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    // 檢查擁有者是否存在
    if (!m_pOwnerCharacter) {
        return true;
    }

    // --- 核心生命週期判斷 ---
    // 原始碼: return (~*(_BYTE *)(*((_DWORD *)this + 33) + 9700) & 2) != 0;
    // 檢查角色的特殊狀態旗標中，代表「無法移動」的位元是否被清除
    if (m_pOwnerCharacter->CanMove()) { // 假設 CanMove() 實現了該位元檢查
        return true; // 如果可以移動了，特效結束
    }

    // 否則，特效繼續存在
    return false;
}

// 對應反組譯碼: 0x00533200
void CEffect_Skill_Blocking::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- 核心邏輯：使用儲存的初始位置 ---
    // 原始碼: v1[3] = v1[25] - (double)dword_A73088;
    //         v1[4] = v1[26] - (double)dword_A7308C;
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // 更新內部 CCAEffect 的狀態
    m_ccaEffect.SetPosition(screenX, screenY);
    m_ccaEffect.Process();

    // 因為位置固定，裁剪判斷可以簡化
    m_bIsVisible = IsCliping(screenX, 0.0f);
}

// 對應反組譯碼: 0x00533230
void CEffect_Skill_Blocking::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}