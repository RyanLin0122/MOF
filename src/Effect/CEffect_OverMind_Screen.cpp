#include "Effect/CEffect_OverMind_Screen.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"
#include <cstdlib>

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x00539F60
CEffect_OverMind_Screen::CEffect_OverMind_Screen()
    : m_pTargetCharacter(nullptr),
    m_bIsActive(false),
    m_dwAlpha(0),
    m_pEffectImage(nullptr)
{
    // 原始碼的建構函式為空，所有初始化都在 SetActive 中進行
}

CEffect_OverMind_Screen::~CEffect_OverMind_Screen()
{
    // GameImage 由 cltImageManager 管理，此處不需 delete
}

// 對應反組譯碼: 0x00539F80
void CEffect_OverMind_Screen::SetActive(ClientCharacter* pTarget)
{
    m_pTargetCharacter = pTarget;
    m_bIsActive = true;
    m_dwAlpha = 255; // 初始化透明度/生命週期計時器
}

// 對應反組譯碼: 0x00539FB0
void CEffect_OverMind_Screen::PrepareDrawing()
{
    if (!m_bIsActive || !m_pTargetCharacter) {
        return;
    }

    // 獲取特效使用的 GameImage
    m_pEffectImage = cltImageManager::GetInstance()->GetGameImage(7, 0x20000091u, 0, 1);
    if (!m_pEffectImage) {
        m_bIsActive = false; // 如果獲取不到圖片，則直接結束特效
        return;
    }

    // 計算螢幕座標
    float screenX = static_cast<float>(m_pTargetCharacter->GetPosX() - g_Game_System_Info.ScreenWidth);
    float screenY = static_cast<float>(m_pTargetCharacter->GetPosY() - g_Game_System_Info.ScreenHeight - 30); // 在角色頭頂上方30像素

    // 設定 GameImage 屬性
    m_pEffectImage->SetPosition(screenX, screenY);
    m_pEffectImage->SetBlockID(0);

    // 設定透明度，隨著時間遞減
    m_pEffectImage->SetAlpha(m_dwAlpha % 255);

    // 設定一個隨機的顏色/縮放值，產生閃爍效果
    m_pEffectImage->SetColor(rand() % 500 + 100);

    // 更新 GameImage 的頂點緩衝區
    m_pEffectImage->Process();

    // 更新生命週期計時器
    m_dwAlpha -= 25;
    if (m_dwAlpha < 1) {
        m_bIsActive = false; // 生命週期結束
    }
}

// 對應反組譯碼: 0x0053A090
void CEffect_OverMind_Screen::Draw()
{
    if (m_bIsActive && m_pEffectImage && m_pEffectImage->IsInUse())
    {
        // 雖然原始碼沒有設定混合模式，但此類特效通常使用 Alpha 混合
        // CDeviceManager::GetInstance()->ResetRenderState();
        m_pEffectImage->Draw();
    }
}