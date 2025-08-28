#include "Effect/CEffect_Item_Use_HPPotion.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x00538F60
CEffect_Item_Use_HPPotion::CEffect_Item_Use_HPPotion()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 121, 檔案名稱: "Effect/efn_heal.ea"
    CEAManager::GetInstance()->GetEAData(121, "Effect/efn_heal.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環
}

// 對應反組譯碼: 0x00539000
CEffect_Item_Use_HPPotion::~CEffect_Item_Use_HPPotion()
{
}

// 對應反組譯碼: 0x00539010
void CEffect_Item_Use_HPPotion::SetEffect(ClientCharacter* pUser)
{
    // 綁定擁有者
    // 原始碼: *((_DWORD *)this + 33) = a2;
    m_pOwnerCharacter = pUser;
}

// 對應反組譯碼: 0x00539020
bool CEffect_Item_Use_HPPotion::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    // 當 "efn_heal.ea" 動畫播放完畢時，此函式會回傳 true。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00539030
void CEffect_Item_Use_HPPotion::Process()
{
    // 檢查擁有者是否存在
    if (!m_pOwnerCharacter) {
        // 如果擁有者消失（例如下線），提前結束特效
        // 雖然原始碼沒有明確處理，但這是穩健的作法
        m_bIsVisible = FALSE;
        return;
    }

    // --- 核心邏輯：持續追蹤擁有者位置 ---
    // 原始碼: v1 = *((_DWORD *)this + 33);
    //         v2 = (float *)((char *)this + 36);
    //         v2[3] = (float)(*(_DWORD *)(v1 + 4384) - dword_A73088);
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

// 對應反組譯碼: 0x00539080
void CEffect_Item_Use_HPPotion::Draw()
{
    // 直接將繪製任務委託給內部的 CCAEffect 物件
    m_ccaEffect.Draw();
}