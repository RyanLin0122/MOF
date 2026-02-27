#include "Test/EffectSystemTest.h"
#include "Effect/CEffectManager.h"
#include "Effect/CEAManager.h"
#include "Info/cltMoFC_EffectKindInfo.h"
#include "Effect/CEffect_Battle_DownCut.h"
#include "Effect/CEffect_Battle_Hit_Normal.h"
#include "Effect/CEffect_Battle_UpperCut.h"
#include "Effect/CEffect_Battle_HorizonCut.h"
#include "Effect/CEffect_Battle_MagicBook.h"
#include "Effect/CEffect_Battle_TwoHandCut.h"
#include "Effect/CEffect_Battle_Pierce.h"
#include "Effect/CEffect_Battle_GunSpark.h"
#include "Effect/CEffect_Hit_Critical.h"
#include "Effect/CEffect_Field_AttackMiss.h"
#include "Effect/CEffect_Field_Miss.h"
#include "Effect/CEffect_Skill_Hit.h"
#include "Effect/CEffect_Use_Hit.h"
#include "Effect/CEffect_Battle_BowShoot.h"
#include "Effect/CEffect_Battle_GunShoot.h"
#include "Effect/CEffect_Battle_StaffShoot.h"
#include "Effect/CEffect_Field_CriticalNumber.h"
#include "Effect/CEffect_Field_DamageNumber.h"
#include "Effect/CEffect_Field_ExpNumber.h"
#include "Effect/CEffect_Field_HealNumber.h"
#include "Effect/CEffect_Field_ItemPick.h"
#include "Effect/CEffect_Field_ItemPick_Sub.h"
#include "Effect/CEffect_Field_Pet_ItemPick.h"
#include "Effect/CEffect_Field_Pet_ItemPick_Sub.h"
#include "Effect/CEffect_Field_Recall_Potal.h"
#include "Effect/CEffect_Field_Walkdust.h"
#include "Effect/CEffect_Field_Warp.h"
#include "Effect/CEffect_Item_Type_Once.h"
#include "Effect/CEffect_Item_Use_HPPotion.h"
#include "Effect/CEffect_MapEffect.h"
#include "Effect/CEffect_Mon_DarkBall.h"
#include "Effect/CEffect_Mon_DarkSpark.h"
#include "Effect/CEffect_Mon_Die_Ghost.h"
#include "Effect/CEffect_Mon_ElecSpark.h"
#include "Effect/CEffect_Mon_FireSpark.h"
#include "Effect/CEffect_Pet_Base.h"
#include "Effect/CEffect_Player_Dead.h"
#include "Effect/CEffect_Player_EnchantLevel.h"
#include "Effect/CEffect_Player_FItem_Effect.h"
#include "Effect/CEffect_Player_Goggles.h"
#include "Effect/CEffect_Player_MapConqueror.h"
#include "Effect/CEffect_Player_PCRoom_Effect.h"
#include "Effect/CEffect_Player_Special_Bottom_Effect.h"
#include "Effect/CEffect_Player_Special_Top_Effect.h"
#include "Effect/CEffect_Player_Tolerance.h"
#include "Effect/CEffect_Player_WeatherPreView.h"
#include "Effect/CEffect_Portal.h"
#include "Effect/CEffect_Skill_Blocking.h"
#include "Effect/CEffect_Skill_Freezing.h"
#include "Effect/CEffect_Skill_Heal.h"
#include "Effect/CEffect_Skill_HealPray.h"
#include "Effect/CEffect_Skill_Integrity.h"
#include "Effect/CEffect_Skill_OtherHeal.h"
#include "Effect/CEffect_Skill_Resurrection.h"
#include "Effect/CEffect_Skill_SpeedUP.h"
#include "Effect/CEffect_Skill_Take.h"
#include "Effect/CEffect_Skill_Trap_Explosion.h"
#include "Effect/CEffect_Skill_Type_Directed.h"
#include "Effect/CEffect_Skill_Type_Directed_Target.h"
#include "Effect/CEffect_Skill_Type_Once.h"
#include "Effect/CEffect_Skill_Type_ShootUnit.h"
#include "Effect/CEffect_Skill_Type_Sustain.h"
#include "Effect/CEffect_UI_ButtonSide.h"
#include "Effect/CEffect_UI_HP_MP_Number.h"
#include "Effect/CEffect_UI_SkillSet.h"
#include "Effect/CEffect_UI_WorldMap_Light.h"
#include "Effect/CEffect_WeddingHall.h"
#include "Effect/CEffectUseHitMulti.h"

// --- 新增的小遊戲特效標頭檔 ---
#include "Effect/CEffect_MiniGame_Archer_String.h"
#include "Effect/CEffect_MiniGame_Fighter_Break.h"
#include "Effect/CEffect_MiniGame_Fighter_String.h"
#include "Effect/CEffect_MiniGame_MagicStick_Left.h"
#include "Effect/CEffect_MiniGame_MagicStick_Right.h"
#include "Effect/CEffect_MiniGame_Priest_Heal.h"
#include "Effect/CEffect_MiniGame_Priest_Light.h"
#include "Effect/CEffect_MiniGame_Class_Sword.h"
#include "Effect/CEffect_MiniGame_Wizard_String.h"

#include "Character/ClientCharacter.h"
#include "global.h"
#include <stdio.h>

EffectSystemTest::EffectSystemTest() :
    m_pCaster(nullptr),
    m_pTarget(nullptr),
    m_fTimeSinceLastEffect(0.0f),
    m_currentRenderTestCaseIndex(0)
{
}

EffectSystemTest::~EffectSystemTest()
{
    Cleanup();
}

HRESULT EffectSystemTest::Initialize()
{
    printf("--- [EffectSystemTest] 初始化開始 ---\n");

    CEffectManager::GetInstance();
    CEAManager::GetInstance();

    printf("  正在建立模擬施法者 (ClientCharacter)...\n");
    m_pCaster = new ClientCharacter();
    if (!m_pCaster) {
        printf("錯誤：建立 ClientCharacter 失敗。\n");
        return E_FAIL;
    }

    m_pTarget = new ClientCharacter();
    if (!m_pTarget) {
        printf("錯誤：建立 ClientCharacter 失敗。\n");
        return E_FAIL;
    }

    m_pCaster->SetPosX(450);
    m_pCaster->SetPosY(200);
    m_pTarget->SetPosX(900);
    m_pTarget->SetPosY(200);

    InitializeRenderTestCases();

    printf("--- [EffectSystemTest] 初始化成功 ---\n");
    return S_OK;
}

void EffectSystemTest::Cleanup()
{
    printf("--- [EffectSystemTest] 清理資源 ---\n");

    CEffectManager::GetInstance()->BulletListAllDel();

    if (m_pCaster) {
        delete m_pCaster;
        m_pCaster = nullptr;
        printf("  已刪除模擬施法者。\n");
    }

    if (m_pTarget) {
        delete m_pTarget;
        m_pTarget = nullptr;
        printf("  已刪除模擬目標。\n");
    }
}

void EffectSystemTest::Update(float fElapsedTime)
{
    m_fTimeSinceLastEffect += fElapsedTime;

    if (m_fTimeSinceLastEffect > 3.0f) {
        SpawnNextRenderTestCase();
        m_fTimeSinceLastEffect = 0.0f;
    }

    CEffectManager::GetInstance()->FrameProcess(fElapsedTime, false);
}

void EffectSystemTest::Render()
{
    CEffectManager::GetInstance()->Process();
    CEffectManager::GetInstance()->Draw();
}

void EffectSystemTest::InitializeRenderTestCases()
{
    m_renderTestCases = {
        //{ "Battle_DownCut", &EffectSystemTest::SpawnDownCutEffect },
        //{ "Battle_Hit_Normal", &EffectSystemTest::SpawnHitNormalEffect },
        //{ "Battle_UpperCut", &EffectSystemTest::SpawnUpperCutEffect },
        //{ "Battle_HorizonCut", &EffectSystemTest::SpawnHorizenCutEffect },
        //{ "Battle_MagicBook", &EffectSystemTest::SpawnMagicBookEffect },
        //{ "Battle_TwoHandCut", &EffectSystemTest::SpawnTwoHandCutEffect },
        //{ "Battle_Pierce", &EffectSystemTest::SpawnPierceEffect },
        //{ "Battle_GunSpark", &EffectSystemTest::SpawnGunSparkEffect },
        //{ "Hit_Critical", &EffectSystemTest::SpawnHitCriticalEffect },
        //{ "Field_AttackMiss", &EffectSystemTest::SpawnFieldAttackMissEffect },
        //{ "Field_Miss", &EffectSystemTest::SpawnFieldMissEffect }, //沒東西
        //{ "Skill_Hit", &EffectSystemTest::SpawnSkillHitEffect }//,
        //{ "Use_Hit", &EffectSystemTest::SpawnUseHitEffect },

        //{ "MiniGame_Archer_String", &EffectSystemTest::SpawnMiniGameArcherStringEffect }, //有bug，截掉右邊
        //{ "MiniGame_Fighter_Break", &EffectSystemTest::SpawnMiniGameFighterBreakEffect },
        //{ "MiniGame_Fighter_String", &EffectSystemTest::SpawnMiniGameFighterStringEffect }, //gi id問題
        //{ "MiniGame_MagicStick_Left", &EffectSystemTest::SpawnMiniGameMagicStickLeftEffect },
        //{ "MiniGame_MagicStick_Right", &EffectSystemTest::SpawnMiniGameMagicStickRightEffect },
        //{ "MiniGame_Priest_Heal", &EffectSystemTest::SpawnMiniGamePriestHealEffect },
        //{ "MiniGame_Priest_Light", &EffectSystemTest::SpawnMiniGamePriestLightEffect },
        //{ "MiniGame_Class_Sword", &EffectSystemTest::SpawnMiniGameClassSwordEffect },
        //{ "MiniGame_Wizard_String", &EffectSystemTest::SpawnMiniGameWizardStringEffect },
        //{ "Battle_BowShoot", &EffectSystemTest::SpawnBowShootEffect },
        //{ "Battle_GunShoot", &EffectSystemTest::SpawnGunShootEffect },
        //{ "Battle_StaffShoot", &EffectSystemTest::SpawnStaffShootEffect },
        //{ "Field_CriticalNumber", &EffectSystemTest::SpawnFieldCriticalNumberEffect },
        //{ "Field_DamageNumber", &EffectSystemTest::SpawnFieldDamageNumberEffect },
        //{ "Field_ExpNumber", &EffectSystemTest::SpawnFieldExpNumberEffect },
        //{ "Field_HealNumber", &EffectSystemTest::SpawnFieldHealNumberEffect },
        //{ "Field_ItemPick", &EffectSystemTest::SpawnFieldItemPickEffect },
        //{ "Field_ItemPick_Sub", &EffectSystemTest::SpawnFieldItemPickSubEffect },
        //{ "Field_Pet_ItemPick", &EffectSystemTest::SpawnFieldPetItemPickEffect },
        //{ "Field_Pet_ItemPick_Sub", &EffectSystemTest::SpawnFieldPetItemPickSubEffect },
        //{ "Field_Recall_Potal", &EffectSystemTest::SpawnFieldRecallPotalEffect },
        //{ "Field_Walkdust", &EffectSystemTest::SpawnFieldWalkdustEffect },
        //{ "Field_Warp", &EffectSystemTest::SpawnFieldWarpEffect },
        //{ "Item_Type_Once", &EffectSystemTest::SpawnItemTypeOnceEffect },
        //{ "Item_Use_HPPotion", &EffectSystemTest::SpawnItemUseHPPotionEffect },
        //{ "MapEffect", &EffectSystemTest::SpawnMapEffect },
        //{ "Mon_DarkBall", &EffectSystemTest::SpawnMonDarkBallEffect },
        //{ "Mon_DarkSpark", &EffectSystemTest::SpawnMonDarkSparkEffect },
        //{ "Mon_Die_Ghost", &EffectSystemTest::SpawnMonDieGhostEffect },
        //{ "Mon_ElecSpark", &EffectSystemTest::SpawnMonElecSparkEffect },
        //{ "Mon_FireSpark", &EffectSystemTest::SpawnMonFireSparkEffect },
        //{ "Pet_Base", &EffectSystemTest::SpawnPetBaseEffect },
        //{ "Player_Dead", &EffectSystemTest::SpawnPlayerDeadEffect },
        //{ "Player_EnchantLevel", &EffectSystemTest::SpawnPlayerEnchantLevelEffect },
        //{ "Player_FItem_Effect", &EffectSystemTest::SpawnPlayerFItemEffect },
        //{ "Player_Goggles", &EffectSystemTest::SpawnPlayerGogglesEffect },
        //{ "Player_MapConqueror", &EffectSystemTest::SpawnPlayerMapConquerorEffect },
        //{ "Player_PCRoom_Effect", &EffectSystemTest::SpawnPlayerPCRoomEffect },
        //{ "Player_Special_Bottom_Effect", &EffectSystemTest::SpawnPlayerSpecialBottomEffect },
        //{ "Player_Special_Top_Effect", &EffectSystemTest::SpawnPlayerSpecialTopEffect },
        //{ "Player_Tolerance", &EffectSystemTest::SpawnPlayerToleranceEffect },
        //{ "Player_WeatherPreView", &EffectSystemTest::SpawnPlayerWeatherPreviewEffect },
        { "Portal", &EffectSystemTest::SpawnPortalEffect },
        { "Skill_Blocking", &EffectSystemTest::SpawnSkillBlockingEffect },
        { "Skill_Freezing", &EffectSystemTest::SpawnSkillFreezingEffect },
        { "Skill_Heal", &EffectSystemTest::SpawnSkillHealEffect },
        { "Skill_HealPray", &EffectSystemTest::SpawnSkillHealPrayEffect },
        { "Skill_Integrity", &EffectSystemTest::SpawnSkillIntegrityEffect },
        { "Skill_OtherHeal", &EffectSystemTest::SpawnSkillOtherHealEffect },
        { "Skill_Resurrection", &EffectSystemTest::SpawnSkillResurrectionEffect },
        { "Skill_SpeedUP", &EffectSystemTest::SpawnSkillSpeedUpEffect },
        { "Skill_Take", &EffectSystemTest::SpawnSkillTakeEffect },
        { "Skill_Trap_Explosion", &EffectSystemTest::SpawnSkillTrapExplosionEffect },
        //{ "Skill_Type_Directed", &EffectSystemTest::SpawnSkillTypeDirectedEffect },
        //{ "Skill_Type_Directed_Target", &EffectSystemTest::SpawnSkillTypeDirectedTargetEffect },
        //{ "Skill_Type_Once", &EffectSystemTest::SpawnSkillTypeOnceEffect },
        //{ "Skill_Type_ShootUnit", &EffectSystemTest::SpawnSkillTypeShootUnitEffect },
        //{ "Skill_Type_Sustain", &EffectSystemTest::SpawnSkillTypeSustainEffect },
        //{ "UI_ButtonSide", &EffectSystemTest::SpawnUIButtonSideEffect },
        //{ "UI_HP_MP_Number", &EffectSystemTest::SpawnUIHpMpNumberEffect },
        //{ "UI_SkillSet", &EffectSystemTest::SpawnUISkillSetEffect },
        //{ "UI_WorldMap_Light", &EffectSystemTest::SpawnUIWorldMapLightEffect },
        //{ "WeddingHall", &EffectSystemTest::SpawnWeddingHallEffect },
        //{ "UseHitMulti", &EffectSystemTest::SpawnUseHitMultiEffect },
    };
}

void EffectSystemTest::SpawnNextRenderTestCase()
{
    if (m_renderTestCases.empty()) {
        return;
    }

    const EffectRenderTestCase& testCase = m_renderTestCases[m_currentRenderTestCaseIndex];
    printf("  [RenderTest] Case %zu/%zu: %s\n",
        m_currentRenderTestCaseIndex + 1,
        m_renderTestCases.size(),
        testCase.name);

    (this->*testCase.spawnFunc)();

    m_currentRenderTestCaseIndex = (m_currentRenderTestCaseIndex + 1) % m_renderTestCases.size();
}

void EffectSystemTest::GetCasterPosition(float& x, float& y) const
{
    x = static_cast<float>(m_pCaster->GetPosX());
    y = static_cast<float>(m_pCaster->GetPosY());
}

void EffectSystemTest::GetTargetPosition(float& x, float& y) const
{
    x = static_cast<float>(m_pTarget->GetPosX());
    y = static_cast<float>(m_pTarget->GetPosY());
}

void EffectSystemTest::SpawnDownCutEffect()
{
    if (!m_pCaster) return;

    printf("  正在產生 CEffect_Battle_DownCut 特效...\n");
    CEffect_Battle_DownCut* pEffect = new CEffect_Battle_DownCut();

    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);

    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(x, y, bFlip, 0);

    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnHitNormalEffect()
{
    if (!m_pCaster) return;

    printf("  正在產生 CEffect_Battle_Hit_Normal 特效...\n");

    CEffect_Battle_Hit_Normal* pEffect = new CEffect_Battle_Hit_Normal();

    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);

    pEffect->SetEffect(x, y);

    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnUpperCutEffect()
{
    if (!m_pCaster) return;

    printf("  正在產生 CEffect_Battle_UpperCut 特效...\n");

    CEffect_Battle_UpperCut* pEffect = new CEffect_Battle_UpperCut();

    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);

    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(x, y, bFlip, 0);

    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnHorizenCutEffect()
{
    if (!m_pCaster) return;

    printf("  正在產生 CEffect_Battle_HorizenCut 特效...\n");

    CEffect_Battle_HorizonCut* pEffect = new CEffect_Battle_HorizonCut();

    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);

    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(x, y, bFlip, 0);

    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMagicBookEffect()
{
    if (!m_pCaster || !m_pTarget) return;

    printf("  正在產生 CEffect_Battle_MagicBook 特效...\n");

    CEffect_Battle_MagicBook* pEffect = new CEffect_Battle_MagicBook();

    bool bFlip = (rand() % 2 == 0);
    pEffect->SetEffect(m_pCaster, m_pTarget, bFlip, 0);

    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnTwoHandCutEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_Battle_TwoHandCut 特效...\n");
    CEffect_Battle_TwoHandCut* pEffect = new CEffect_Battle_TwoHandCut();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y, (rand() % 2 == 0), 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPierceEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_Battle_Pierce 特效...\n");
    CEffect_Battle_Pierce* pEffect = new CEffect_Battle_Pierce();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y, (rand() % 2 == 0), 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnGunSparkEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_Battle_GunSpark 特效...\n");
    CEffect_Battle_GunSpark* pEffect = new CEffect_Battle_GunSpark();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y, (rand() % 2 == 0));
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnHitCriticalEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_Hit_Critical 特效...\n");
    CEffect_Hit_Critical* pEffect = new CEffect_Hit_Critical();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldAttackMissEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_Field_AttackMiss 特效...\n");
    CEffect_Field_AttackMiss* pEffect = new CEffect_Field_AttackMiss();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldMissEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_Field_Miss 特效...\n");
    CEffect_Field_Miss* pEffect = new CEffect_Field_Miss();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillHitEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_Skill_Hit 特效...\n");
    CEffect_Skill_Hit* pEffect = new CEffect_Skill_Hit();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnUseHitEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_Use_Hit 特效...\n");
    CEffect_Use_Hit* pEffect = new CEffect_Use_Hit();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

// --- 以下為新增的小遊戲特效生成函式 ---

void EffectSystemTest::SpawnMiniGameArcherStringEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_MiniGame_Archer_String 特效...\n");
    CEffect_MiniGame_Archer_String* pEffect = new CEffect_MiniGame_Archer_String();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(0, x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMiniGameFighterBreakEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_MiniGame_Fighter_Break 特效...\n");
    CEffect_MiniGame_Fighter_Break* pEffect = new CEffect_MiniGame_Fighter_Break();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMiniGameFighterStringEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_MiniGame_Fighter_String 特效...\n");
    CEffect_MiniGame_Fighter_String* pEffect = new CEffect_MiniGame_Fighter_String();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(0, x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMiniGameMagicStickLeftEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_MiniGame_MagicStick_Left 特效...\n");
    CEffect_MiniGame_MagicStick_Left* pEffect = new CEffect_MiniGame_MagicStick_Left();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMiniGameMagicStickRightEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_MiniGame_MagicStick_Right 特效...\n");
    CEffect_MiniGame_MagicStick_Right* pEffect = new CEffect_MiniGame_MagicStick_Right();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMiniGamePriestHealEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_MiniGame_Priest_Heal 特效...\n");
    CEffect_MiniGame_Priest_Heal* pEffect = new CEffect_MiniGame_Priest_Heal();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMiniGamePriestLightEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_MiniGame_Priest_Light 特效...\n");
    CEffect_MiniGame_Priest_Light* pEffect = new CEffect_MiniGame_Priest_Light();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMiniGameClassSwordEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_MiniGame_Class_Sword 特效...\n");
    CEffect_MiniGame_Class_Sword* pEffect = new CEffect_MiniGame_Class_Sword();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMiniGameWizardStringEffect()
{
    if (!m_pCaster) return;
    printf("  正在產生 CEffect_MiniGame_Wizard_String 特效...\n");
    CEffect_MiniGame_Wizard_String* pEffect = new CEffect_MiniGame_Wizard_String();
    float x = 0.0f;
    float y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(0, x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}


void EffectSystemTest::SpawnBowShootEffect()
{
    if (!m_pCaster || !m_pTarget) return;
    printf("  正在產生 CEffect_Battle_BowShoot 特效...\n");
    CEffect_Battle_BowShoot* pEffect = new CEffect_Battle_BowShoot();
    pEffect->SetEffect(m_pCaster, m_pTarget, false, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnGunShootEffect()
{
    if (!m_pCaster || !m_pTarget) return;
    printf("  正在產生 CEffect_Battle_GunShoot 特效...\n");
    CEffect_Battle_GunShoot* pEffect = new CEffect_Battle_GunShoot();
    pEffect->SetEffect(m_pCaster, m_pTarget, 0, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnStaffShootEffect()
{
    if (!m_pCaster || !m_pTarget) return;
    printf("  正在產生 CEffect_Battle_StaffShoot 特效...\n");
    CEffect_Battle_StaffShoot* pEffect = new CEffect_Battle_StaffShoot();
    float x = 0.0f, y = 0.0f, tx = 0.0f, ty = 0.0f;
    GetCasterPosition(x, y);
    GetTargetPosition(tx, ty);
    D3DXVECTOR2 startPos(x, y);
    D3DXVECTOR2 endPos(tx, ty);
    pEffect->SetEffect(&startPos, &endPos, false);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldCriticalNumberEffect()
{
    if (!m_pCaster) return;
    CEffect_Field_CriticalNumber* pEffect = new CEffect_Field_CriticalNumber();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(9999, x, y, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldDamageNumberEffect()
{
    if (!m_pCaster) return;
    CEffect_Field_DamageNumber* pEffect = new CEffect_Field_DamageNumber();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(1234, x, y, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldExpNumberEffect()
{
    if (!m_pCaster) return;
    CEffect_Field_ExpNumber* pEffect = new CEffect_Field_ExpNumber();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(888, x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldHealNumberEffect()
{
    if (!m_pCaster) return;
    CEffect_Field_HealNumber* pEffect = new CEffect_Field_HealNumber();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(666, x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldItemPickEffect()
{
    if (!m_pCaster) return;
    CEffect_Field_ItemPick* pEffect = new CEffect_Field_ItemPick();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    D3DXVECTOR2 startPos(x, y);
    pEffect->SetEffect(&startPos, 1);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldItemPickSubEffect()
{
    if (!m_pCaster) return;
    CEffect_Field_ItemPick_Sub_Light* pEffect = new CEffect_Field_ItemPick_Sub_Light();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y, false);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldPetItemPickEffect()
{
    if (!m_pCaster) return;
    CEffect_Field_Pet_ItemPick* pEffect = new CEffect_Field_Pet_ItemPick();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    D3DXVECTOR2 startPos(x, y);
    pEffect->SetEffect(&startPos, 1);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldPetItemPickSubEffect()
{
    if (!m_pCaster) return;
    CEffect_Field_Pet_ItemPick_Sub_Light* pEffect = new CEffect_Field_Pet_ItemPick_Sub_Light();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y, false);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldRecallPotalEffect()
{
    if (!m_pCaster) return;
    CEffect_Field_Recall_Potal* pEffect = new CEffect_Field_Recall_Potal();
    float x = 0.0f, y = 0.0f;
    char effectName[] = "efn_recallpotal.ea";
    GetCasterPosition(x, y);
    pEffect->SetEffect(effectName, x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldWalkdustEffect()
{
    if (!m_pCaster) return;
    CEffect_Field_Walkdust* pEffect = new CEffect_Field_Walkdust();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y, false, 0, 0, 0, false);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnFieldWarpEffect()
{
    if (!m_pCaster) return;
    CEffect_Field_Warp* pEffect = new CEffect_Field_Warp();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(0, x, y, m_pCaster);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnItemTypeOnceEffect()
{
    if (!m_pCaster) return;
    CEffect_Item_Type_Once* pEffect = new CEffect_Item_Type_Once();
    char effectName[] = "efn_item.ea";
    pEffect->SetEffect(m_pCaster, 0, effectName);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnItemUseHPPotionEffect()
{
    if (!m_pCaster) return;
    CEffect_Item_Use_HPPotion* pEffect = new CEffect_Item_Use_HPPotion();
    pEffect->SetEffect(m_pCaster);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMapEffect()
{
    if (!m_pCaster) return;
    CEffect_MapEffect* pEffect = new CEffect_MapEffect();
    char effectName[] = "efn_mapeffect.ea";
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(effectName, 0, m_pCaster, static_cast<int>(x), static_cast<int>(y));
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMonDarkBallEffect()
{
    if (!m_pCaster || !m_pTarget) return;
    CEffect_Mon_DarkBall* pEffect = new CEffect_Mon_DarkBall();
    pEffect->SetEffect(m_pCaster, m_pTarget, 0.0f, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMonDarkSparkEffect()
{
    if (!m_pTarget) return;
    CEffect_Mon_DarkSpark* pEffect = new CEffect_Mon_DarkSpark();
    pEffect->SetEffect(m_pTarget);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMonDieGhostEffect()
{
    if (!m_pTarget) return;
    CEffect_Mon_Die_Ghost* pEffect = new CEffect_Mon_Die_Ghost();
    pEffect->SetEffect(m_pTarget);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMonElecSparkEffect()
{
    if (!m_pTarget) return;
    CEffect_Mon_ElecSpark* pEffect = new CEffect_Mon_ElecSpark();
    pEffect->SetEffect(m_pTarget);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnMonFireSparkEffect()
{
    if (!m_pTarget) return;
    CEffect_Mon_FireSpark* pEffect = new CEffect_Mon_FireSpark();
    pEffect->SetEffect(m_pTarget);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPetBaseEffect()
{
    CEffect_Pet_Base* pEffect = new CEffect_Pet_Base();
    char effectName[] = "efn_petbase.ea";
    pEffect->SetEffect(nullptr, 0, effectName);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPlayerDeadEffect()
{
    if (!m_pCaster) return;
    CEffect_Player_Dead* pEffect = new CEffect_Player_Dead();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(m_pCaster, x, y, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPlayerEnchantLevelEffect()
{
    if (!m_pCaster) return;
    CEffect_Player_EnchantLevel* pEffect = new CEffect_Player_EnchantLevel();
    char effectName[] = "efn_enchant.ea";
    pEffect->SetEffect(m_pCaster, 0, effectName);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPlayerFItemEffect()
{
    if (!m_pCaster) return;
    CEffect_Player_FItem_Effect* pEffect = new CEffect_Player_FItem_Effect();
    char effectName[] = "efn_fitem.ea";
    pEffect->SetEffect(m_pCaster, 0, effectName);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPlayerGogglesEffect()
{
    if (!m_pCaster) return;
    CEffect_Player_Goggles* pEffect = new CEffect_Player_Goggles();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y, false, 0, 1);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPlayerMapConquerorEffect()
{
    if (!m_pCaster) return;
    CEffect_Player_MapConqueror* pEffect = new CEffect_Player_MapConqueror();
    pEffect->SetEffect(m_pCaster);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPlayerPCRoomEffect()
{
    if (!m_pCaster) return;
    CEffect_Player_PCRoom_Effect* pEffect = new CEffect_Player_PCRoom_Effect();
    pEffect->SetEffect(m_pCaster);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPlayerSpecialBottomEffect()
{
    if (!m_pCaster) return;
    CEffect_Player_Special_Bottom_Effect* pEffect = new CEffect_Player_Special_Bottom_Effect();
    pEffect->SetEffect(m_pCaster, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPlayerSpecialTopEffect()
{
    if (!m_pCaster) return;
    CEffect_Player_Special_Top_Effect* pEffect = new CEffect_Player_Special_Top_Effect();
    pEffect->SetEffect(m_pCaster, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPlayerToleranceEffect()
{
    if (!m_pCaster) return;
    CEffect_Player_Tolerance* pEffect = new CEffect_Player_Tolerance();
    pEffect->SetEffect(m_pCaster, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPlayerWeatherPreviewEffect()
{
    CEffect_Player_WeatherPreView* pEffect = new CEffect_Player_WeatherPreView();
    char effectName[] = "efn_weatherpreview.ea";
    pEffect->SetEffect(100.0f, 100.0f, 0, effectName);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnPortalEffect()
{
    if (!m_pCaster) return;
    CEffect_Portal* pEffect = new CEffect_Portal();
    // CEffect_Portal::SetEffect 依照 mofclient.c 需傳入 kind code（例如 E0741），不是 ea 檔名。
    char effectName[] = "E0741";
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(effectName, x, y);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillBlockingEffect()
{
    if (!m_pTarget) return;
    CEffect_Skill_Blocking* pEffect = new CEffect_Skill_Blocking();
    pEffect->SetEffect(m_pTarget);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillFreezingEffect()
{
    if (!m_pTarget) return;
    CEffect_Skill_Freezing* pEffect = new CEffect_Skill_Freezing();
    pEffect->SetEffect(m_pTarget, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillHealEffect()
{
    if (!m_pTarget) return;
    CEffect_Skill_Heal* pEffect = new CEffect_Skill_Heal();
    pEffect->SetEffect(m_pTarget, 1);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillHealPrayEffect()
{
    if (!m_pCaster) return;
    CEffect_Skill_HealPray* pEffect = new CEffect_Skill_HealPray();
    pEffect->SetEffect(m_pCaster, 1);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillIntegrityEffect()
{
    if (!m_pTarget) return;
    CEffect_Skill_Integrity* pEffect = new CEffect_Skill_Integrity();
    pEffect->SetEffect(m_pTarget, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillOtherHealEffect()
{
    if (!m_pTarget) return;
    CEffect_Skill_OtherHeal* pEffect = new CEffect_Skill_OtherHeal();
    pEffect->SetEffect(m_pTarget, 1);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillResurrectionEffect()
{
    if (!m_pTarget) return;
    CEffect_Skill_Resurrection* pEffect = new CEffect_Skill_Resurrection();
    float x = 0.0f, y = 0.0f;
    GetTargetPosition(x, y);
    pEffect->SetEffect(x, y, 1);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillSpeedUpEffect()
{
    if (!m_pCaster) return;
    CEffect_Skill_SpeedUP* pEffect = new CEffect_Skill_SpeedUP();
    pEffect->SetEffect(m_pCaster);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillTakeEffect()
{
    if (!m_pCaster) return;
    CEffect_Skill_Take* pEffect = new CEffect_Skill_Take();
    pEffect->SetEffect(m_pCaster);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillTrapExplosionEffect()
{
    if (!m_pCaster) return;
    CEffect_Skill_Trap_Explosion* pEffect = new CEffect_Skill_Trap_Explosion();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y, 1);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillTypeDirectedEffect()
{
    if (!m_pCaster || !m_pTarget) return;
    CEffect_Skill_Type_Directed* pEffect = new CEffect_Skill_Type_Directed();
    char effectName[] = "efn_skill_directed.ea";
    pEffect->SetEffect(m_pCaster, m_pTarget, 0, effectName, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillTypeDirectedTargetEffect()
{
    if (!m_pCaster || !m_pTarget) return;
    CEffect_Skill_Type_Directed_Target* pEffect = new CEffect_Skill_Type_Directed_Target();
    char effectName[] = "efn_skill_target.ea";
    pEffect->SetEffect(m_pCaster, m_pTarget, 0, effectName);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillTypeOnceEffect()
{
    if (!m_pCaster) return;
    CEffect_Skill_Type_Once* pEffect = new CEffect_Skill_Type_Once();
    char effectName[] = "efn_skill_once.ea";
    pEffect->SetEffect(m_pCaster, 0, effectName);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillTypeShootUnitEffect()
{
    if (!m_pCaster || !m_pTarget) return;
    CEffect_Skill_Type_ShootUnit* pEffect = new CEffect_Skill_Type_ShootUnit();
    char effectName[] = "efn_skill_shoot.ea";
    pEffect->SetEffect(m_pCaster, m_pTarget, 0, effectName, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnSkillTypeSustainEffect()
{
    if (!m_pCaster) return;
    CEffect_Skill_Type_Sustain* pEffect = new CEffect_Skill_Type_Sustain();
    char effectName[] = "efn_skill_sustain.ea";
    pEffect->SetEffect(m_pCaster, 0, effectName, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnUIButtonSideEffect()
{
    CEffect_UI_ButtonSide* pEffect = new CEffect_UI_ButtonSide();
    pEffect->SetEffect(120.0f, 120.0f);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnUIHpMpNumberEffect()
{
    CEffect_UI_HP_MP_Number* pEffect = new CEffect_UI_HP_MP_Number();
    pEffect->SetEffect(500, 160.0f, 160.0f, 0);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnUISkillSetEffect()
{
    CEffect_UI_SkillSet* pEffect = new CEffect_UI_SkillSet();
    pEffect->SetEffect(200.0f, 200.0f);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnUIWorldMapLightEffect()
{
    CEffect_UI_WorldMap_Light* pEffect = new CEffect_UI_WorldMap_Light();
    pEffect->SetEffect(240.0f, 240.0f);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnWeddingHallEffect()
{
    if (!m_pCaster) return;
    CEffect_WeddingHall* pEffect = new CEffect_WeddingHall();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect(x, y, m_pCaster);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}

void EffectSystemTest::SpawnUseHitMultiEffect()
{
    if (!m_pCaster) return;
    CEffectUseHitMulti* pEffect = new CEffectUseHitMulti();
    float x = 0.0f, y = 0.0f;
    GetCasterPosition(x, y);
    pEffect->SetEffect2(x, y, 0, 1);
    CEffectManager::GetInstance()->BulletAdd(pEffect);
}
