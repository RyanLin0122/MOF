#include "Effect/CEffect_Portal.h"
#include "Effect/CEAManager.h"
#include "Effect/CEffectmanager.h"
#include "Info/cltMoFC_EffectKindInfo.h" // 需要特效種類資訊管理器
#include "global.h"

CEffect_Portal::CEffect_Portal()
    : m_bIsFinished(true)
{
}

CEffect_Portal::~CEffect_Portal()
{
}

// 對應反組譯碼: 0x00537930
void CEffect_Portal::SetEffect(char* szEffectName, float x, float y)
{
    stEffectKindInfo* pKindInfo = CEffectManager::GetInstance()->g_clEffectKindInfo.GetEffectKindInfo(szEffectName);
    if (!pKindInfo) {
        return;
    }

    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "MoFData/Effect/%s", pKindInfo->eaFile);

    CEAManager::GetInstance()->GetEAData(pKindInfo->kindCode, szFullPath, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true);

    m_bIsFinished = false;
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
}

// 對應反組譯碼: 0x005379D0
bool CEffect_Portal::FrameProcess(float fElapsedTime)
{
    // 更新內部動畫的影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    // 返回生命週期旗標，由外部邏輯決定何時結束
    return m_bIsFinished;
}

// 對應反組譯碼: 0x005379F0
void CEffect_Portal::Process()
{
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x00537A30
void CEffect_Portal::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}
