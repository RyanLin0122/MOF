#include "UI/CControlButtonBase.h"
#include <cstring>

// --- 模擬的外部依賴 ---
class GameSound {
public:
    void PlaySoundA(const char* soundName, int, int) {}
};
GameSound g_GameSoundManager;
// --- 模擬結束 ---

/**
 * CControlButtonBase 建構函式
 * 透過初始化列表呼叫基底和成員的建構函式。
 */
CControlButtonBase::CControlButtonBase()
    : CControlImage(), m_Text()
{
    m_bCanMoveChildren = false;
    m_bIsButtonDown = false;
    m_nChildMoveOffset = 2; // 預設位移量
    memset(m_szClickSoundName, 0, sizeof(m_szClickSoundName));
}

/**
 * CControlButtonBase 解構函式
 */
CControlButtonBase::~CControlButtonBase()
{
}

/**
 * 創建並設定子控制項 (文字標籤)。
 */
void CControlButtonBase::CreateChildren()
{
    // 將 m_Text 物件作為自己的子控制項加入到UI階層中
    m_Text.Create(this);
}

/**
 * 初始化按鈕，設定預設音效。
 */
void CControlButtonBase::Init()
{
    ButtonPosUp(); // 確保按鈕回到彈起狀態
    strcpy(m_szClickSoundName, "J0003"); // 設定預設點擊音效
}

/**
 * 設定按鈕上的文字。
 * @param szText 要顯示的文字。
 */
void CControlButtonBase::SetText(const char* szText)
{
    // 這是一個便捷的包裝函式，將操作轉發給內部的 m_Text 物件
    m_Text.SetText(szText);
}

/**
 * 透過文字ID設定按鈕上的文字。
 * @param nDCTID 在文字管理器中的ID。
 */
void CControlButtonBase::SetText(int nDCTID)
{
    m_Text.SetText(nDCTID);
}

/**
 * 播放點擊音效。
 */
void CControlButtonBase::PlaySoundClick()
{
    g_GameSoundManager.PlaySoundA(m_szClickSoundName, 0, 0);
}

/**
 * 處理按鈕按下的視覺效果。
 * 如果啟用位移效果，將所有子控制項（主要是文字）向下、向右移動指定像素。
 */
void CControlButtonBase::ButtonPosDown()
{
    if (m_bCanMoveChildren && !m_bIsButtonDown)
    {
        SetChildPosMove(m_nChildMoveOffset, m_nChildMoveOffset);
        m_bIsButtonDown = true;
    }
}

/**
 * 處理按鈕彈起的視覺效果。
 * 如果按鈕處於按下狀態，將所有子控制項移回原始位置。
 */
void CControlButtonBase::ButtonPosUp()
{
    if (m_bCanMoveChildren && m_bIsButtonDown)
    {
        SetChildPosMove(-m_nChildMoveOffset, -m_nChildMoveOffset);
        m_bIsButtonDown = false;
    }
}

/**
 * 設定是否啟用點擊位移效果以及位移量。
 * @param bEnable true為啟用，false為停用。
 * @param nOffset 位移的像素值。
 */
void CControlButtonBase::SetChildMoveByClick(bool bEnable, int nOffset)
{
    m_bCanMoveChildren = bEnable;
    m_nChildMoveOffset = nOffset;
}

/**
 * 檢查滑鼠是否在按鈕上方。
 * 這個 IsMouseOver 是 CControlImage 繼承來的成員 m_bIsMouseOver
 * 在 CControlImage::ControlKeyInputProcess 中被更新。
 */
bool CControlButtonBase::IsMouseOver() const
{
    return m_bIsMouseOver;
}
