#ifndef CCONTROLBUTTONBASE_H
#define CCONTROLBUTTONBASE_H

#include "UI/CControlImage.h"
#include "UI/CControlText.h"

/**
 * @class CControlButtonBase
 * @brief 所有可點擊按鈕的基底類別。
 *
 * 結合了 CControlImage (背景圖) 和 CControlText (文字標籤) 的功能，
 * 並增加了對點擊、滑鼠懸停等狀態的處理，以及點擊時的視覺與聲音反饋。
 */
class CControlButtonBase : public CControlImage
{
public:
    // 建構函式與解構函式
    CControlButtonBase();
    virtual ~CControlButtonBase();

    // --- 初始化 ---
    // 創建子控制項 (文字) 並進行基本設定
    void CreateChildren();
    // 初始化按鈕狀態和預設音效
    void Init();

    // --- 功能函式 ---
    // 設定按鈕上顯示的文字
    void SetText(const char* szText);
    void SetText(int nDCTID);

    // 播放點擊音效
    void PlaySoundClick();

    // --- 狀態與外觀 ---
    // 處理按鈕被按下的視覺效果 (讓子物件位移)
    void ButtonPosDown();
    // 處理按鈕被放開的視覺效果
    void ButtonPosUp();

    // 設定是否啟用點擊位移效果以及位移的像素值
    void SetChildMoveByClick(bool bEnable, int nOffset);

    // 檢查滑鼠是否在按鈕上方
    bool IsMouseOver() const;

protected:
    // --- 成員變數 ---

    // 點擊效果相關
    bool m_bCanMoveChildren;  // 是否啟用點擊時子物件的位移效果
    bool m_bIsButtonDown;     // 按鈕目前是否處於被按下的狀態
    int  m_nChildMoveOffset;  // 點擊時子物件位移的像素量

    // 音效
    char m_szClickSoundName[32]; // 儲存點擊音效的名稱

    // 複合控制項
    CControlText m_Text;      // 內含的文字控制項，用於顯示按鈕標籤
};

#endif // CCONTROLBUTTONBASE_H
