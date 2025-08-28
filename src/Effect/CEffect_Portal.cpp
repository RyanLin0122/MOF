#include "Effect/CEffect_Portal.h"
#include "Effect/CEAManager.h"
#include "Effect/CEffectmanager.h"
#include "Effect/cltMoFC_EffectKindInfo.h" // 需要特效種類資訊管理器
#include "global.h"

CEffect_Portal::CEffect_Portal()
    : m_bIsFinished(false)
{
    // CEffectBase 的建構函式會被自動呼叫
}

CEffect_Portal::~CEffect_Portal()
{
}

// 對應反組譯碼: 0x00537930
void CEffect_Portal::SetEffect(char* szEffectName, float x, float y)
{
    // 步驟 1: 根據名稱查找特效資訊
    stEffectKindInfo* pKindInfo = CEffectManager::GetInstance()->g_clEffectKindInfo.GetEffectKindInfo(szEffectName);
    if (!pKindInfo) {
        m_bIsFinished = true; // 如果找不到資源，直接標記為結束
        return;
    }

    // 步驟 2: 構建完整的檔案路徑
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", pKindInfo->szFileName);

    // 步驟 3: 載入特效數據並開始播放
    CEAManager::GetInstance()->GetEAData(pKindInfo->usKindID, szFullPath, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // 傳送門通常是循環播放的

    // 步驟 4: 設定特效位置和初始狀態
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
    m_bIsFinished = false;
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

// 對應反組譯碼: 0x00537A30
void CEffect_Portal::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}