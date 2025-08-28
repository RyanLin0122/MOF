#include "Effect/CEffect_Pet_Base.h"
#include "Effect/CEAManager.h"
#include "Pet/cltPetObject.h" // 假設寵物物件的定義在此
#include "global.h"

// 對應反組譯碼: 0x00539090
CEffect_Pet_Base::CEffect_Pet_Base()
    : m_pOwnerPet(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫
}

// 對應反組譯碼: 0x005390D0
CEffect_Pet_Base::~CEffect_Pet_Base()
{
}

// 對應反組譯碼: 0x005390E0
void CEffect_Pet_Base::SetEffect(cltPetObject* pPetOwner, unsigned short effectKindID, char* szFileName)
{
    if (!pPetOwner) return;

    // 構建完整的檔案路徑
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", szFileName);

    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(effectKindID, szFullPath, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 綁定擁有者
    m_pOwnerPet = pPetOwner;
}

// 對應反組譯碼: 0x00539150
bool CEffect_Pet_Base::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00539160
void CEffect_Pet_Base::Process()
{
    // 檢查擁有者是否存在
    if (!m_pOwnerPet) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- 核心邏輯：持續追蹤寵物位置 ---
    // 原始碼: v4 = (float)(*(_DWORD *)(v2 + 2948) - dword_A73088);

    // 將世界座標轉換為螢幕座標
    float screenX = static_cast<float>(m_pOwnerPet->GetPosX() - g_Game_System_Info.ScreenWidth);
    float screenY = static_cast<float>(m_pOwnerPet->GetPosY() - g_Game_System_Info.ScreenHeight);

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x005391E0
void CEffect_Pet_Base::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}