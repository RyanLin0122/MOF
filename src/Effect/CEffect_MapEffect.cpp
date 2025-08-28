#include "Effect/CEffect_MapEffect.h"
#include "Effect/CEAManager.h"
#include "Effect/CEffectManager.h"
#include "Effect/cltMoFC_EffectKindInfo.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;
extern cltMoFC_EffectKindInfo g_clEffectKindInfo;

// 對應反組譯碼: 0x00537780
CEffect_MapEffect::CEffect_MapEffect()
    : m_pStateOwner(nullptr), m_wRequiredStateID(0)
{
}

// 對應反組譯碼: 0x005377C0
CEffect_MapEffect::~CEffect_MapEffect()
{
}

// 對應反組譯碼: 0x005377D0
void CEffect_MapEffect::SetEffect(char* szEffectName, unsigned short requiredStateID, ClientCharacter* pStateOwner, int x, int y)
{
    // 透過名稱獲取特效 Kind ID
    stEffectKindInfo* pKindInfo = CEffectManager::GetInstance()->g_clEffectKindInfo.GetEffectKindInfo(szEffectName);
    if (pKindInfo) {
        // 呼叫另一個多載版本來完成設定
        SetEffect(pKindInfo->usKindID, requiredStateID, pStateOwner, x, y);
    }
}

// 對應反組譯碼: 0x00537810
void CEffect_MapEffect::SetEffect(unsigned short effectKindID, unsigned short requiredStateID, ClientCharacter* pStateOwner, int x, int y)
{
    stEffectKindInfo* pKindInfo = CEffectManager::GetInstance()->g_clEffectKindInfo.GetEffectKindInfo(effectKindID);
    if (!pKindInfo) return;

    // 構建檔案路徑並載入數據
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", pKindInfo->szFileName);
    CEAManager::GetInstance()->GetEAData(effectKindID, szFullPath, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // 持續性特效應循環播放

    // 儲存狀態和位置資訊
    m_wRequiredStateID = requiredStateID;
    m_pStateOwner = pStateOwner;
    m_fCurrentPosX = static_cast<float>(x);
    m_fCurrentPosY = static_cast<float>(y);
}


// 對應反組譯碼: 0x005378B0
bool CEffect_MapEffect::FrameProcess(float fElapsedTime)
{
    if (!m_pStateOwner) {
        return true; // 如果狀態擁有者不存在，則特效立即結束
    }

    // 檢查擁有者的狀態 ID 是否與要求的 ID 匹配
    // 原始碼: *(_WORD *)(*((_DWORD *)this + 33) + 592) == *((_WORD *)this + 68)
    if (m_pStateOwner->GetMapID() == m_wRequiredStateID) {
        // 狀態匹配，更新動畫並讓特效繼續存在
        m_ccaEffect.FrameProcess(fElapsedTime);
        return false;
    }
    else {
        // 狀態不匹配，特效結束
        return true;
    }
}

// 對應反組譯碼: 0x005378E0
void CEffect_MapEffect::Process()
{
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x00537920
void CEffect_MapEffect::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}