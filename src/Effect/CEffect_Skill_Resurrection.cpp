#include "Effect/CEffect_Skill_Resurrection.h"
#include "Effect/CEAManager.h"
#include "global.h"

// 對應反組譯碼: 0x00533C90
CEffect_Skill_Resurrection::CEffect_Skill_Resurrection()
    : m_fInitialPosX(0.0f), m_fInitialPosY(0.0f)
{
    // CEffectBase 的建構函式會被自動呼叫
    // 特效資源是在 SetEffect 中動態載入的
}

// 對應反組譯碼: 0x00533CD0
CEffect_Skill_Resurrection::~CEffect_Skill_Resurrection()
{
}

// 對應反組譯碼: 0x00533CE0
void CEffect_Skill_Resurrection::SetEffect(float x, float y, unsigned char skillLevel)
{
    const char* szFileName = nullptr;
    int effectID = 0;

    // 根據技能等級選擇不同的特效資源
    if (skillLevel == 1) {
        effectID = 72;
        szFileName = "Effect/efn_resurrectionLv2.ea";
    }
    else if (skillLevel == 2) {
        effectID = 73;
        szFileName = "Effect/efn_resurrectionLv3.ea";
    }
    else { // 預設為等級 0
        effectID = 71;
        szFileName = "Effect/efn_Resurrection.ea";
    }

    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(effectID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 儲存位置快照
    m_fInitialPosX = x;
    m_fInitialPosY = y;
}

// 對應反組譯碼: 0x00533D60
bool CEffect_Skill_Resurrection::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00533D70
void CEffect_Skill_Resurrection::Process()
{
    // --- 核心邏輯：使用儲存的初始位置 ---
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

// 對應反組譯碼: 0x00533DC0
void CEffect_Skill_Resurrection::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}