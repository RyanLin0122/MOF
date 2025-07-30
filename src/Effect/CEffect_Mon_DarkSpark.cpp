#include "Effect/CEffect_Mon_DarkSpark.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x00530EB0
CEffect_Mon_DarkSpark::CEffect_Mon_DarkSpark()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 111, 檔案名稱: "Effect/efn_Mop_DarkSpark.ea"
    CEAManager::GetInstance()->GetEAData(111, "Effect/efn_Mop_DarkSpark.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環
}

// 對應反組譯碼: 0x00530F50
CEffect_Mon_DarkSpark::~CEffect_Mon_DarkSpark()
{
}

// 對應反組譯碼: 0x00530F60
void CEffect_Mon_DarkSpark::SetEffect(ClientCharacter* pTarget)
{
    // 綁定擁有者
    // 原始碼: *((_DWORD *)this + 33) = a2;
    m_pOwnerCharacter = pTarget;
}

// 對應反組譯碼: 0x00530F70
bool CEffect_Mon_DarkSpark::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    // 當 "efn_Mop_DarkSpark.ea" 動畫播放完畢時，此函式會回傳 true。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00530F80
void CEffect_Mon_DarkSpark::Process()
{
    // 檢查擁有者是否存在
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- 核心邏輯：持續追蹤擁有者位置 ---
    // 原始碼: v2[3] = (float)(*(_DWORD *)(v1 + 4384) - dword_A73088);
    //         v2[4] = (float)(*(_DWORD *)(*((_DWORD *)v2 + 24) + 4388) - dword_A7308C);

    // 將世界座標轉換為螢幕座標
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenWidth);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenHeight);

    // 更新內部 CCAEffect 的狀態
    m_ccaEffect.SetPosition(screenX, screenY);
    m_ccaEffect.Process();

    // 設置為可見（裁剪判斷在 CCAEffect::Process 內部完成）
    m_bIsVisible = TRUE;
}

// 對應反組譯碼: 0x00530FD0
void CEffect_Mon_DarkSpark::Draw()
{
    // 直接將繪製任務委託給內部的 CCAEffect 物件
    m_ccaEffect.Draw();
}