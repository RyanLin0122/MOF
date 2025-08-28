#include "Effect/CEffect_Mon_Die_Ghost.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x00531240
CEffect_Mon_Die_Ghost::CEffect_Mon_Die_Ghost()
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 112, 檔案名稱: "Effect/efn-mon-dead.ea"
    CEAManager::GetInstance()->GetEAData(112, "Effect/efn-mon-dead.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環
}

// 對應反組譯碼: 0x005312D0
CEffect_Mon_Die_Ghost::~CEffect_Mon_Die_Ghost()
{
}

// 對應反組譯碼: 0x005312E0
void CEffect_Mon_Die_Ghost::SetEffect(ClientCharacter* pDeadMonster)
{
    if (!pDeadMonster) {
        // 如果傳入的指標為空，則不設定位置，特效可能在 (0,0) 播放或不可見
        m_fCurrentPosX = 0.0f;
        m_fCurrentPosY = 0.0f;
        return;
    }

    // --- 核心邏輯：儲存位置快照，並根據朝向進行偏移 ---
    // 原始碼: *((float *)this + 33) = (float)(*((_DWORD *)a2 + 1096) + (*((_DWORD *)a2 + 143) != 1 ? -15 : 15));
    //         *((float *)this + 34) = (float)*((int *)a2 + 1097);

    float offsetX = (pDeadMonster->GetActionSide() != 1) ? -15.0f : 15.0f;
    m_fCurrentPosX = static_cast<float>(pDeadMonster->GetPosX()) + offsetX;
    m_fCurrentPosY = static_cast<float>(pDeadMonster->GetPosY());
}

// 對應反組譯碼: 0x00531330
bool CEffect_Mon_Die_Ghost::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    // 當 "efn-mon-dead.ea" 動畫播放完畢時，此函式會回傳 true。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00531340
void CEffect_Mon_Die_Ghost::Process()
{
    // 將世界座標轉換為螢幕座標
    // 原始碼: v3 = *((float *)this + 33) - (double)dword_A73088;
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // 進行裁剪判斷
    // 原始碼: v2 = CEffectBase::IsCliping(this, v3, 0.0);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        // 原始碼: *((float *)this + 12) = ...; *((float *)this + 13) = ...;
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x00531390
void CEffect_Mon_Die_Ghost::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}