#include "Effect/CEffect_Player_FItem_Effect.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x005316E0
CEffect_Player_FItem_Effect::CEffect_Player_FItem_Effect()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫
}

// 對應反組譯碼: 0x00531720
CEffect_Player_FItem_Effect::~CEffect_Player_FItem_Effect()
{
}

// 對應反組譯碼: 0x00531730
void CEffect_Player_FItem_Effect::SetEffect(ClientCharacter* pOwner, unsigned short effectKindID, char* szFileName)
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

// 對應反組譯碼: 0x005317B0
bool CEffect_Player_FItem_Effect::FrameProcess(float fElapsedTime)
{
    // 更新內部動畫影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    // --- 核心生命週期判斷 ---
    ClientCharacter* pOwner = m_pOwnerCharacter;
    if (!pOwner) {
        return true; // 如果擁有者指標為空，特效結束
    }

    // 條件一: 檢查角色是否處於某個通用特殊狀態
    // 原始碼: if ( *(_DWORD *)(v3 + 11528) ) result = 1;
    if (pOwner->GetSomeOtherState()) { // 假設 GetSomeOtherState() 讀取位移 +11528 的旗標
        return true;
    }

    // 條件二: 檢查觸發此特效的時裝是否還在裝備中
    // 原始碼: result = *(_DWORD *)(v3 + 4376) == 0;
    // 假設 IsFashionItemActive() 讀取位移 +4376 的旗標
    if (!pOwner->IsFashionItemActive()) {
        return true;
    }

    // 如果所有條件都不滿足，特效繼續存在
    return false;
}

// 對應反組譯碼: 0x005317F0
void CEffect_Player_FItem_Effect::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // 將世界座標轉換為螢幕座標
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    // 根據角色朝向設定翻轉
    m_bIsFlip = (m_pOwnerCharacter->GetActionSide() != 0);

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x00531890
void CEffect_Player_FItem_Effect::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}