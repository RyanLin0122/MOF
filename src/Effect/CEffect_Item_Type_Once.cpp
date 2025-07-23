#include "Effect/CEffect_Item_Type_Once.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x00539200
CEffect_Item_Type_Once::CEffect_Item_Type_Once()
    : m_pOwnerCharacter(nullptr),
    m_fInitialPosX(0.0f),
    m_fInitialPosY(0.0f)
{
    // CEffectBase 的建構函式會被自動呼叫
}

// 對應反組譯碼: 0x00539250
CEffect_Item_Type_Once::~CEffect_Item_Type_Once()
{
}

// 對應反組譯碼: 0x00539260
void CEffect_Item_Type_Once::SetEffect(ClientCharacter* pUser, unsigned short effectKindID, char* szFileName)
{
    if (!pUser) return;

    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(effectKindID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 綁定擁有者
    m_pOwnerCharacter = pUser;

    // --- 核心邏輯：儲存位置快照 ---
    // 原始碼: v5 = *((_DWORD *)this + 33); 
    //         *((_DWORD *)this + 34) = *(_DWORD *)(v5 + 4384);
    //         *((_DWORD *)this + 35) = *(_DWORD *)(v5 + 4388);
    m_fInitialPosX = static_cast<float>(pUser->GetPosX());
    m_fInitialPosY = static_cast<float>(pUser->GetPosY());

    // 根據角色朝向設定翻轉
    // 原始碼: *((_BYTE *)this + 56) = *((_DWORD *)a2 + 143) != 0;
    m_bIsFlip = (pUser->GetActionSide() != 0);
}

// 對應反組譯碼: 0x00539300
bool CEffect_Item_Type_Once::FrameProcess(float fElapsedTime)
{
    // 生命週期完全由內部動畫決定
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00539310
void CEffect_Item_Type_Once::Process()
{
    // 檢查擁有者是否還存在（例如，可能在特效播放期間下線或死亡）
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- 核心邏輯：使用儲存的初始位置 ---
    // 原始碼: v3 = (float)(*((_DWORD *)this + 34) - dword_A73088);
    //         v2 = CEffectBase::IsCliping(this, v3, 0.0);
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 將世界座標轉換為螢幕座標後傳遞給 CCAEffect
        // 原始碼: *((float *)this + 12) = (float)(*((_DWORD *)this + 34) - dword_A73088);
        //         *((float *)this + 13) = (float)(*((_DWORD *)this + 35) - dword_A7308C);
        m_ccaEffect.SetPosition(screenX, m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenHeight));
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x00539390
void CEffect_Item_Type_Once::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}