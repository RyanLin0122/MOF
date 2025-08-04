#include "Effect/CEffect_WeddingHall.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x00537E50
CEffect_WeddingHall::CEffect_WeddingHall()
    : m_pOwnerPlayer(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫
}

// 對應反組譯碼: 0x00537E90
CEffect_WeddingHall::~CEffect_WeddingHall()
{
}

// 對應反組譯碼: 0x00537EA0
void CEffect_WeddingHall::Init()
{
    // 向 CEAManager 請求特效數據
    // 特效 ID: 107, 檔案名稱: "Effect/efn_wedding_hall.ea"
    CEAManager::GetInstance()->GetEAData(107, "Effect/efn_wedding_hall.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // 婚禮特效應循環播放
}

// 對應反組譯碼: 0x00537ED0
void CEffect_WeddingHall::SetEffect(float x, float y, ClientCharacter* pPlayer)
{
    if (!pPlayer) return;

    // 初始化特效資源
    Init();

    // 儲存位置和綁定的玩家
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
    m_pOwnerPlayer = pPlayer;
}


// 對應反組譯碼: 0x00537F00
bool CEffect_WeddingHall::FrameProcess(float fElapsedTime)
{
    if (!m_pOwnerPlayer) {
        return true; // 如果狀態擁有者不存在，則特效立即結束
    }

    // --- 核心生命週期判斷 ---
    // 檢查擁有者的地圖 ID 是否為 4
    // 原始碼: if ( *(_WORD *)(*((_DWORD *)this + 33) + 592) == 4 ) ...
    if (m_pOwnerPlayer->GetMapID() == 4) { // 假設 GetMapID() 讀取位移 +592 的屬性
        // 狀態匹配，更新動畫並讓特效繼續存在
        m_ccaEffect.FrameProcess(fElapsedTime);
        return false;
    }
    else {
        // 狀態不匹配，特效結束
        return true;
    }
}

// 對應反組譯碼: 0x00537F30
void CEffect_WeddingHall::Process()
{
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x00537F70
void CEffect_WeddingHall::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}