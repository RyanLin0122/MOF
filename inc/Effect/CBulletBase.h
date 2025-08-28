#pragma once

#include <d3dx9math.h> // 為了 D3DXVECTOR2

/**
 * @class CBulletBase
 * @brief 簡單投射物（子彈）的抽象基底類別。
 *
 * 為遊戲中獨立於 CEffectManager 管理的飛行物體提供一個統一的介面。
 */
class CBulletBase {
public:
    CBulletBase();
    virtual ~CBulletBase();

    // --- 核心虛擬函式 ---

    /// @brief 創建並設定投射物的初始狀態。
    /// @param dwOwnerID 發射者的唯一 ID。
    /// @param pStartPos 起始位置。
    /// @param pEndPos 目標位置。
    /// @param fSpeed 飛行速度。
    virtual void Create(unsigned int dwOwnerID, D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fSpeed) = 0; // 純虛擬

    /// @brief 更新投射物的狀態（例如位置）。
    /// @param fElapsedTime 經過的時間。
    /// @return 如果投射物應被銷毀，則回傳 true。
    virtual bool Process(float fElapsedTime);

    /// @brief 繪製投射物。
    virtual void Draw() = 0; // 純虛擬

protected:
    // --- 成員變數 (根據 Effectall.c @ 0x0052D700 推斷) ---
    void* m_pVftable;        // 位移 +0

    unsigned int m_dwOwnerID;  // 位移 +4: 發射者的 ID

    // 位置與方向
    D3DXVECTOR2 m_vecPos;      // 位移 +8, +12
    D3DXVECTOR2 m_vecDir;      // 位移 +16, +20

    float m_fSpeed;            // 位移 +24
};