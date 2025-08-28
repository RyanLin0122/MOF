#include "Effect/CEffect_Skill_Trap_Explosion.h"
#include "Effect/CEAManager.h"
#include "global.h"

// 對應反組譯碼: 0x005340B0
CEffect_Skill_Trap_Explosion::CEffect_Skill_Trap_Explosion()
    : m_fInitialPosX(0.0f), m_fInitialPosY(0.0f)
{
    // CEffectBase 的建構函式會被自動呼叫
    // 特效資源是在 SetEffect 中動態載入的
}

// 對應反組譯碼: 0x005340F0
CEffect_Skill_Trap_Explosion::~CEffect_Skill_Trap_Explosion()
{
}

// 對應反組譯碼: 0x00534100
void CEffect_Skill_Trap_Explosion::SetEffect(float x, float y, unsigned char skillLevel)
{
    const char* szFileName = nullptr;
    int effectID = 0;

    // 根據技能等級選擇不同的特效資源
    if (skillLevel == 1) {
        effectID = 90;
        szFileName = "Effect/efn_Trap_ExplosionLv2.ea";
    }
    else if (skillLevel == 2) {
        effectID = 91;
        szFileName = "Effect/efn_Trap_ExplosionLv3.ea";
    }
    else { // 預設為等級 0
        effectID = 89;
        szFileName = "Effect/efn_Trap_Explosion.ea";
    }

    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(effectID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 儲存位置快照
    m_fInitialPosX = x;
    m_fInitialPosY = y;
}

// 對應反組譯碼: 0x00534180
bool CEffect_Skill_Trap_Explosion::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00534190
void CEffect_Skill_Trap_Explosion::Process()
{
    // --- 核心邏輯：使用儲存的初始位置 ---
    // 原始碼: v1[3] = v1[24] - (double)dword_A73088;
    //         v1[4] = v1[25] - (double)dword_A7308C;
    // 這裡的 v1[24] 和 v1[25] 實際上是 m_fInitialPosX 和 m_fInitialPosY
    // 但為了清晰起見，我們不直接操作 CCAEffect 的內部成員

    // 將世界座標轉換為螢幕座標
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // 更新內部 CCAEffect 的狀態
    m_ccaEffect.SetPosition(screenX, screenY);
    m_ccaEffect.Process();

    // 裁剪判斷在 CCAEffect::Process 內部完成，這裡只需設定可見性
    m_bIsVisible = TRUE;
}

// 對應反組譯碼: 0x005341B0
void CEffect_Skill_Trap_Explosion::Draw()
{
    // 直接將繪製任務委託給內部的 CCAEffect 物件
    m_ccaEffect.Draw();
}