#pragma once

#include "Effect/CBulletBase.h"

// 前向宣告
class GameImage;

/**
 * @class CBulletArrow
 * @brief 代表一支直線飛行並逐漸淡出的箭矢。
 *
 * 繼承自 CBulletBase，是一個由程式碼控制動畫的簡單投射物。
 */
class CBulletArrow : public CBulletBase {
public:
    CBulletArrow();
    virtual ~CBulletArrow();

    // --- 虛擬函式覆寫 ---

    /// @brief 創建並設定箭矢的初始狀態。
    virtual void Create(unsigned int dwOwnerID, D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fSpeed) override;

    /// @brief 更新箭矢的位置和透明度。
    /// @return 如果箭矢生命週期結束，則回傳 true。
    virtual bool Process(float fElapsedTime) override;

    /// @brief 繪製箭矢。
    virtual void Draw() override;

private:
    // --- 成員變數 (根據 Effectall.c @ 0x0052D570 推斷) ---
    int m_dwAlpha;          // 位移 +28: 作為生命週期計時器和透明度
    GameImage* m_pArrowImage;  // 位移 +32: 指向箭矢的 GameImage
};