#include "Effect/CEffect_Player_Dead.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"
#include <windows.h> // for timeGetTime


// 對應反組譯碼: 0x005313A0
CEffect_Player_Dead::CEffect_Player_Dead()
    : m_pOwnerCharacter(nullptr),
    m_bIsPK_Mode(0)
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 12, 檔案名稱: "Effect/efn_dead.ea"
    CEAManager::GetInstance()->GetEAData(12, "Effect/efn_dead.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 記錄創建時的時間戳，用於超時判斷
    m_dwStartTime = timeGetTime();
}

// 對應反組譯碼: 0x00531450
CEffect_Player_Dead::~CEffect_Player_Dead()
{
}

// 對應反組譯碼: 0x00531460
void CEffect_Player_Dead::SetEffect(ClientCharacter* pDeadChar, float x, float y, int isPkMode)
{
    // 設定特效的初始世界座標
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;

    // 綁定角色物件和PK模式旗標
    m_pOwnerCharacter = pDeadChar;
    m_bIsPK_Mode = isPkMode;
}

// 對應反組譯碼: 0x00531490
bool CEffect_Player_Dead::FrameProcess(float fElapsedTime)
{
    // 更新內部動畫的影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    // 檢查擁有者是否存在
    if (!m_pOwnerCharacter) {
        return true; // 如果角色指標失效，直接結束特效
    }

    // --- 核心生命週期判斷 ---

    // 條件一: 檢查角色是否已復活
    // 原始碼: if ( *(_DWORD *)(*((_DWORD *)this + 33) + 9684) != 7 ) return 1;
    if (m_pOwnerCharacter->GetActionState() != 7) { // 假設 7 代表死亡狀態
        return true;
    }

    // 條件二: 檢查是否超時 (30秒) 且不在 PK 模式
    // 原始碼: return timeGetTime() - *((_DWORD *)this + 34) > 0x7530 && !*((_DWORD *)this + 35);
    if (!m_bIsPK_Mode && (timeGetTime() - m_dwStartTime > 30000)) {
        return true;
    }

    // 如果以上條件都不滿足，則特效繼續存在
    return false;
}

// 對應反組譯碼: 0x005314E0
void CEffect_Player_Dead::Process()
{
    // 將世界座標轉換為螢幕座標
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x00531520
void CEffect_Player_Dead::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}