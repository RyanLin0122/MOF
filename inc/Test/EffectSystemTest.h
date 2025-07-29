#pragma once
#include <windows.h> // for HRESULT

// 前向宣告 (Forward Declarations)
class ClientCharacter;

/**
 * @class EffectSystemTest
 * @brief 負責初始化與驅動整個特效系統的測試類別。
 *
 * 這個類別會取代您 main.cpp 中的 ImageDrawTest，
 * 用於模擬遊戲主循環，並持續產生、管理和渲染特效。
 */
class EffectSystemTest {
public:
    EffectSystemTest();
    ~EffectSystemTest();

    /// @brief 初始化測試環境，建立管理器與模擬角色。
    HRESULT Initialize();

    /// @brief 清理所有資源。
    void Cleanup();

    /// @brief 每幀的邏輯更新。
    /// @param fElapsedTime 自上一幀以來經過的時間(秒)。
    void Update(float fElapsedTime);

    /// @brief 渲染所有可見的特效。
    void Render();

private:
    /// @brief 一個輔助函式，用於在角色位置產生新的下劈斬特效。
    void SpawnDownCutEffect();

    void SpawnHitNormalEffect();

    void SpawnUpperCutEffect();

    // --- 私有成員 ---

    /// @brief 模擬的施法者角色。
    ClientCharacter* m_pCaster;

    /// @brief 用於計時，以固定間隔產生新特效。
    float m_fTimeSinceLastEffect;
};