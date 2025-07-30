#include "Effect/CEffect_Field_Warp.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"


CEffect_Field_Warp::CEffect_Field_Warp()
    : m_pOwnerCharacter(nullptr)
{
}

CEffect_Field_Warp::~CEffect_Field_Warp()
{
}

// 對應反組譯碼: 0x00537610
void CEffect_Field_Warp::SetEffect(unsigned short warpType, float x, float y, ClientCharacter* pOwner)
{
    const char* szFileName = nullptr;
    int effectID = 0;

    // 根據傳入的 warpType 選擇要載入的特效資源
    switch (warpType)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 0x65: // 101
    case 0x6F: // 111
        szFileName = "Effect/efn_warp.ea";
        effectID = 103;
        break;

    case 5:
    case 0x70: // 112
        szFileName = "Effect/efn-Inwarp.ea";
        effectID = 104;
        break;

    default:
        // 如果傳入未知的類型，則不創建特效
        return;
    }

    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(effectID, (char*)szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 設定初始位置和跟隨目標
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
    m_pOwnerCharacter = pOwner;
}

// 對應反組譯碼: 0x00537700
bool CEffect_Field_Warp::FrameProcess(float fElapsedTime)
{
    // 生命週期由內部動畫決定
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00537710
void CEffect_Field_Warp::Process()
{
    // 如果綁定了角色，則持續更新位置為該角色的位置
    if (m_pOwnerCharacter) {
        m_fCurrentPosX = static_cast<float>(m_pOwnerCharacter->GetPosX());
        m_fCurrentPosY = static_cast<float>(m_pOwnerCharacter->GetPosY());
    }

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

// 對應反組譯碼: 0x00537770
void CEffect_Field_Warp::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}