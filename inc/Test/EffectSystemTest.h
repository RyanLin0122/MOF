#pragma once
#include <windows.h> // for HRESULT
#include <cstddef>
#include <vector>

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
    struct EffectRenderTestCase {
        const char* name;
        void (EffectSystemTest::*spawnFunc)();
    };

    void InitializeRenderTestCases();
    void SpawnNextRenderTestCase();
    void GetCasterPosition(float& x, float& y) const;
    void GetTargetPosition(float& x, float& y) const;

    /// @brief 一個輔助函式，用於在角色位置產生新的下劈斬特效。
    void SpawnDownCutEffect();

    void SpawnHitNormalEffect();

    void SpawnUpperCutEffect();

	void SpawnHorizenCutEffect();

    void SpawnMagicBookEffect();

    void SpawnMiniGameArcherStringEffect();

    void SpawnMiniGameFighterBreakEffect();

    void SpawnMiniGameFighterStringEffect();

    void SpawnMiniGameMagicStickLeftEffect();

    void SpawnMiniGameMagicStickRightEffect();

    void SpawnMiniGamePriestHealEffect();

    void SpawnMiniGamePriestLightEffect();

    void SpawnMiniGameClassSwordEffect();

    void SpawnMiniGameWizardStringEffect();

    void SpawnTwoHandCutEffect();

    void SpawnPierceEffect();

    void SpawnGunSparkEffect();

    void SpawnHitCriticalEffect();

    void SpawnFieldAttackMissEffect();

    void SpawnFieldMissEffect();

    void SpawnSkillHitEffect();

    void SpawnUseHitEffect();


    void SpawnBowShootEffect();
    void SpawnGunShootEffect();
    void SpawnStaffShootEffect();
    void SpawnFieldCriticalNumberEffect();
    void SpawnFieldDamageNumberEffect();
    void SpawnFieldExpNumberEffect();
    void SpawnFieldHealNumberEffect();
    void SpawnFieldItemPickEffect();
    void SpawnFieldItemPickSubEffect();
    void SpawnFieldPetItemPickEffect();
    void SpawnFieldPetItemPickSubEffect();
    void SpawnFieldRecallPotalEffect();
    void SpawnFieldWalkdustEffect();
    void SpawnFieldWarpEffect();
    void SpawnItemTypeOnceEffect();
    void SpawnItemUseHPPotionEffect();
    void SpawnMapEffect();
    void SpawnMonDarkBallEffect();
    void SpawnMonDarkSparkEffect();
    void SpawnMonDieGhostEffect();
    void SpawnMonElecSparkEffect();
    void SpawnMonFireSparkEffect();
    void SpawnPetBaseEffect();
    void SpawnPlayerDeadEffect();
    void SpawnPlayerEnchantLevelEffect();
    void SpawnPlayerFItemEffect();
    void SpawnPlayerGogglesEffect();
    void SpawnPlayerMapConquerorEffect();
    void SpawnPlayerPCRoomEffect();
    void SpawnPlayerSpecialBottomEffect();
    void SpawnPlayerSpecialTopEffect();
    void SpawnPlayerToleranceEffect();
    void SpawnPlayerWeatherPreviewEffect();
    void SpawnPortalEffect();
    void SpawnSkillBlockingEffect();
    void SpawnSkillFreezingEffect();
    void SpawnSkillHealEffect();
    void SpawnSkillHealPrayEffect();
    void SpawnSkillIntegrityEffect();
    void SpawnSkillOtherHealEffect();
    void SpawnSkillResurrectionEffect();
    void SpawnSkillSpeedUpEffect();
    void SpawnSkillTakeEffect();
    void SpawnSkillTrapExplosionEffect();
    void SpawnSkillTypeDirectedEffect();
    void SpawnSkillTypeDirectedTargetEffect();
    void SpawnSkillTypeOnceEffect();
    void SpawnSkillTypeShootUnitEffect();
    void SpawnSkillTypeSustainEffect();
    void SpawnUIButtonSideEffect();
    void SpawnUIHpMpNumberEffect();
    void SpawnUISkillSetEffect();
    void SpawnUIWorldMapLightEffect();
    void SpawnWeddingHallEffect();
    void SpawnUseHitMultiEffect();

    // --- 私有成員 ---

    /// @brief 模擬的施法者角色。
    ClientCharacter* m_pCaster;
    ClientCharacter* m_pTarget;
    /// @brief 用於計時，以固定間隔產生新特效。
    float m_fTimeSinceLastEffect;
    std::vector<EffectRenderTestCase> m_renderTestCases;
    size_t m_currentRenderTestCaseIndex;
};
