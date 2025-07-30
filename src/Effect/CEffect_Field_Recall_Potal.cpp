#include "Effect/CEffect_Field_Recall_Potal.h"
#include "Effect/CEAManager.h"
#include "Effect/cltMoFC_EffectKindInfo.h" // 需要特效種類資訊管理器
#include "Effect/CEffectManager.h"
#include "global.h"


// 雖然原始碼沒有提供建構/解構函式，但為了完整性而添加
CEffect_Field_Recall_Potal::CEffect_Field_Recall_Potal()
    : m_bIsFinished(false)
{
}

CEffect_Field_Recall_Potal::~CEffect_Field_Recall_Potal()
{
}

// 對應反組譯碼: 0x005370C0
void CEffect_Field_Recall_Potal::SetEffect(char* szEffectName, float x, float y)
{
    // 步驟 1: 根據名稱查找特效資訊
    stEffectKindInfo* pKindInfo = CEffectManager::GetInstance()->g_clEffectKindInfo.GetEffectKindInfo(szEffectName);
    if (!pKindInfo) {
        return; // 找不到對應的特效定義
    }

    // 步驟 2: 構建完整的檔案路徑
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", pKindInfo->szFileName);

    // 步驟 3: 載入特效數據並開始播放
    CEAManager::GetInstance()->GetEAData(pKindInfo->usKindID, szFullPath, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 步驟 4: 設定特效位置和初始狀態
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
    m_bIsFinished = false;
}

// 對應反組譯碼: 0x00537160
bool CEffect_Field_Recall_Potal::FrameProcess(float fElapsedTime)
{
    // 更新內部動畫的影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    // 返回生命週期旗標，由外部邏輯決定何時結束
    return m_bIsFinished;
}

// 對應反組譯碼: 0x00537180
void CEffect_Field_Recall_Potal::Process()
{
    // 將世界座標轉換為螢幕座標
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x005371C0
void CEffect_Field_Recall_Potal::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}