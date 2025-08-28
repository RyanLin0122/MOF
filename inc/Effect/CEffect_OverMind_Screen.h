#pragma once

// 前向宣告
class ClientCharacter;
class GameImage;

/**
 * @class CEffect_OverMind_Screen
 * @brief 在角色頭頂顯示一個特殊的全螢幕/UI層級效果。
 *
 * 這個類別不繼承 CEffectBase，而是作為 CEffectManager 的一個成員，
 * 用於處理一些特殊的、常駐的視覺效果。
 */
class CEffect_OverMind_Screen {
public:
    CEffect_OverMind_Screen();
    ~CEffect_OverMind_Screen();

    /// @brief 啟用並設定特效的目標。
    /// @param pTarget 要附加特效的目標角色。
    void SetActive(ClientCharacter* pTarget);

    /// @brief 準備繪製數據，更新特效狀態。
    void PrepareDrawing();

    /// @brief 繪製特效。
    void Draw();

    /// @brief 輪詢函式 (在原始碼中被呼叫，但無實作)。
    void Poll() {}

private:
    // --- 成員變數 (根據 Effectall.c @ 0x00539F80 推斷) ---

    ClientCharacter* m_pTargetCharacter; // 位移 +0
    bool             m_bIsActive;        // 位移 +4
    // 原始碼中位移 +8 處有一個未使用的 DWORD
    int              m_dwAlpha;          // 位移 +12: 作為生命週期計時器和透明度
    GameImage* m_pEffectImage;     // 位移 +16: 指向要繪製的 GameImage
};