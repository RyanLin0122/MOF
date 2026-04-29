// =============================================================================
// cltMyCharData — 還原自 mofclient.c 0x517970..0x519930（行 229526..230954）。
//
// 設計策略
//   - 原 binary 把所有子系統內嵌到 cltMyCharData (`(char*)this + N`)；本還原
//     採用既有的「外掛全域子系統」架構：每個 `(char*)this + offset` 的存取
//     都對應到 inc/global.h 中的同名全域實例。
//   - 介面以「靜態方法 + cltMyCharData* self」呈現，與 mofclient.c
//     `cltMyCharData::Method((cltMyCharData*)&g_clMyCharData, …)` 的呼叫式
//     一致；caller (cltTutorialSystem / Map / ClientCharacterManager 等) 不需
//     修改寫法。
//   - 狀態欄位（角色名稱 / 帳號 / 拾取暫存 / 地圖 / PvP 旗標 / 交易暫存…）
//     全部移到 cltMyCharData 自身的成員，不再走 TU-local static。
// =============================================================================

#include "Character/cltMyCharData.h"

#include <cstring>
#include <ctime>

#include "global.h"

#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltMoFC_EquipmentInfo.h"
#include "Info/cltQuestKindInfo.h"
#include "Logic/CPlayerSpirit.h"
#include "Logic/CSpiritSpeechMgr.h"
#include "Logic/Map.h"
#include "Logic/cltBaseInventory.h"
#include "Logic/cltCashShopItem.h"
#include "Logic/cltFieldItem.h"
#include "Logic/cltFieldItemManager.h"
#include "Logic/cltNPCManager.h"
#include "Logic/cltPKManager.h"
#include "Logic/cltSystemMessage.h"
#include "Network/CMoFNetwork.h"
#include "Object/CNPCObject.h"
#include "Sound/GameSound.h"
#include "System/CMeritoriousSystem.h"
#include "System/CQuizEventSystem.h"
#include "System/CSpiritSystem.h"
#include "System/cltBasicAppearSystem.h"
#include "System/cltCashshopSystem.h"
#include "System/cltClassSystem.h"
#include "System/cltClientPartySystem.h"
#include "System/cltEmblemSystem.h"
#include "System/cltEmoticonSystem.h"
#include "System/cltEnchantSystem.h"
#include "System/cltEquipmentSystem.h"
#include "System/cltGradeSystem.h"
#include "System/cltHelpSystem.h"
#include "System/cltLessonSystem.h"
#include "System/cltLevelSystem.h"
#include "System/cltMakingItemSystem.h"
#include "System/cltMarriageSystem.h"
#include "System/cltMoneySystem.h"
#include "System/cltMonsterToleranceSystem.h"
#include "System/cltMyItemSystem.h"
#include "System/cltNPCRecallSystem.h"
#include "System/cltOverMindSystem.h"
#include "System/cltPKRankSystem.h"
#include "System/cltPetKeepingSystem.h"
#include "System/cltPetInventorySystem.h"
#include "System/cltPetMarketMySalePetSystem.h"
#include "System/cltPetSystem.h"
#include "System/cltPlayerAbility.h"
#include "System/cltQuestSystem.h"
#include "System/cltQuickSlotSystem.h"
#include "System/cltSexSystem.h"
#include "System/cltSkillSystem.h"
#include "System/cltSpecialtySystem.h"
#include "System/cltStorageSystem.h"
#include "System/cltTASSystem.h"
#include "System/cltTestingSystem.h"
#include "System/cltTitleSystem.h"
#include "System/cltTradeSystem.h"
#include "System/cltTransformSystem.h"
#include "System/cltUsingItemSystem.h"
#include "UI/CInterfaceDataCommunity.h"
#include "UI/CMessageBoxManager.h"
#include "UI/CUIManager.h"
#include "UI/CUITradeUser.h"

// -----------------------------------------------------------------------------
// 匿名輔助：把 mofclient.c 寫死的「空閒索引 dword_AFAF44」之類雜散
// 變數封裝起來，以後若需要可遷至 global.h。目前 cltMyCharData 內無此需求。
// -----------------------------------------------------------------------------

// =============================================================================
// 建構 / 解構
//
// GT 0x5179?? — 對應 binary 的建構式。原本是把多個子系統的建構子按 offset
// 連續呼叫；這裡因為子系統是全域實例，建構行為已由它們各自的全域變數
// 負責，cltMyCharData 只需要把 per-char 狀態欄位歸零。
// =============================================================================
cltMyCharData::cltMyCharData()
    : m_szCharName{},
      m_uNearItemPos(0),
      m_iNearItemMoney(0),
      m_uNearItemKind(0),
      m_uNearItemNum(0),
      m_iRequestPickUpFlag(0),
      m_iSomething16(0),
      m_iSomething17(0),
      m_byPrevLevel(0),
      m_iSomething19(0),
      m_uSomething40(0),
      m_dwMyAccount(0),     // GT *((_DWORD *)this + 21) = 0
      m_dwServerTime(0),    // GT *((_DWORD *)this + 24) = 0
      m_dwSyncLocalTime(0), // GT *((_DWORD *)this + 25) = 0
      m_iCanNotChatting(0),
      m_iSomething27(0),
      m_iAutoAttack(0),
      m_uMapID(0),
      m_uMapAreaName(0),
      m_uMapVorF(0),
      m_iIsPvPMap(0),
      m_iIsPvPLobbyMap(0),
      m_iIsContinent(0),
      m_iPKUsableItem(0),
      m_iSomething37(0),
      m_byCorporationUser(0),  // GT *((_BYTE *)this + 160) = 0
      m_iIsResponesSuccess(0),
      m_iAcceptParty(0),
      m_dwKickedAccount(0),
      m_dwRequestedTradeAccount(0),
      m_dwTradeMoneyChange(0),
      m_iAcceptTrade(0),
      m_iTradeDecision(0),
      m_byRequestAddPos(0),
      m_uRequestAddNum(0),
      m_byRequestDelPos(0),
      m_uRequestDelNum(0)
{}

cltMyCharData::~cltMyCharData() = default;

// =============================================================================
// (00517CF0) Initialize
//
// 真實 binary 的 Initialize 把 60+ 個欄位扇形派發給 22 個內嵌子系統。本還原
// 採用「呼叫對應全域子系統的 Initialize」策略：每個 `(char*)this + N` 對應
// 一個全域實例，呼叫順序 / 參數對齊 GT。對於目前尚未具備全域實例的子系統
// （cltStorageSystem / cltExStorageSystem / cltGradeSystem / cltMakingItemSystem
// / cltTASSystem / cltEnchantSystem / cltTransformSystem / cltCashshopSystem
// / cltOverMindSystem / cltEmblemSystem / CQuizEventSystem / cltTASSystem
// / cltWorkingPassiveSkillSystem / cltMonsterToleranceSystem
// / cltMoFC_EquipmentInfo / cltNPCRecallSystem / cltTitleSystem
// / cltPetKeepingSystem / cltPetMarketMySalePetSystem / cltPKRankSystem
// / cltMyItemSystem / CPlayerSpirit / CSpiritSpeechMgr-embedded /
// cltQuickSlotSystem / cltEquipmentSystem-embedded 等），保留呼叫位置的註解
// 待對應全域加入後即可拆封。
//
// 已具備全域實例的子系統 init 會實際被呼叫。
// =============================================================================
void cltMyCharData::Initialize(
    cltMyCharData* self,
    std::uint16_t charKind,             // a2
    char classKind,                     // a3
    std::int64_t expValue,              // a4
    int hp,                             // a5
    int mp,                             // a6
    std::uint16_t str,                  // a7
    std::uint16_t dex,                  // a8
    std::uint16_t intel,                // a9
    std::uint16_t vit,                  // a10
    std::uint16_t bonusPoint,           // a11
    int teamKind,                       // a12
    cltItemList* pItemList,             // a13
    int initialMoney,                   // a14
    std::uint16_t numSkill,             // a15
    std::uint16_t* skillKindList,       // a16
    stEquipItemInfo* pEquipPrimary,     // a17
    stEquipItemInfo* pEquipSecondary,   // a18
    std::uint16_t lessonItemKind,       // a19
    std::uint8_t* lessonSchedule,       // a20
    std::uint8_t* lessonState,          // a21
    unsigned int swordPt,               // a22
    unsigned int bowPt,                 // a23
    unsigned int theologyPt,            // a24
    unsigned int magicPt,               // a25
    unsigned int totalSwordPt,          // a26
    unsigned int totalBowPt,            // a27
    unsigned int totalTheologyPt,       // a28
    unsigned int totalMagicPt,          // a29
    strLessonHistory (*lessonHistory)[3], // a30
    std::uint8_t sex,                   // a31
    char appearKind,                    // a32
    char hairKind,                      // a33
    unsigned int appearLook,            // a34
    stPlayingQuestInfo* questData,      // a35
    std::int16_t specialtyPt,           // a36
    std::uint16_t acquiredSpecialtyNum, // a37
    std::uint16_t* acquiredSpecialty,   // a38
    std::uint16_t makingItemNum,        // a39
    std::uint16_t* makingItemKinds,     // a40
    std::uint8_t grade,                 // a41
    unsigned int meritPoint,            // a42
    unsigned int meritTotalPoint,       // a43
    std::uint16_t meritGrade,           // a44
    std::uint16_t meritGradePoint,      // a45
    int warQuestPlaying,                // a46
    char supplyQuestPlaying,            // a47
    char warQuestMonCount,              // a48
    std::uint16_t* warQuestMonKinds,    // a49
    std::uint16_t* warQuestMonGoals,    // a50
    std::uint16_t emblemNum,            // a51
    std::uint16_t* emblemKinds,         // a52
    std::uint16_t emblemUsingKind,      // a53
    std::uint16_t quizID,               // a54
    std::uint8_t quizPlayed,            // a55
    std::uint8_t quizAnswerSize,        // a56
    std::uint8_t* quizAnswerPos,        // a57
    std::uint16_t* quizAnswerItem,      // a58
    std::uint16_t currentTitle,         // a59
    std::uint8_t indunMode,             // a60
    CMofMsg* msg)                       // a61
{
    // ------------------------------------------------------------------------
    // GT 0x517CF0：cltBaseInventory::Initialize(self+396, self+5252,
    //                self+40720, self+40740, a12, a13)
    // ------------------------------------------------------------------------
    g_clMyInventory.Initialize(&g_clQuickSlotSystem,
                               &g_clTitleSystem,
                               &g_clPetSystem,
                               teamKind,
                               pItemList);

    // GT: cltMoneySystem::InitiaizeMoney(self+7400, a14)
    g_clMoneySystem.InitiaizeMoney(initialMoney);

    // GT 一連串狀態初值 (對齊行 229654..229661)：
    self->m_iRequestPickUpFlag = 1;          // *(DWORD*)+15 = 1
    self->m_uNearItemPos       = 0;          // *(WORD*)+25 = 0
    self->m_uNearItemKind      = 0;          // *(WORD*)+28 = 0
    self->m_uSomething40       = 0;          // *(WORD*)+40 = 0
    self->m_iSomething16       = 0;          // *(DWORD*)+16 = 0
    self->m_byPrevLevel        = static_cast<std::uint8_t>(classKind);
    self->m_iSomething19       = 0;          // *(DWORD*)+19 = 0
    self->m_iIsResponesSuccess = 0;          // *(DWORD*)+17330 = 0

    // GT: cltSexSystem::Initialize(self+6724, a31)
    g_clSexSystem.Initialize(sex);

    // GT: cltLevelSystem::Initilaize(self+4512, self+4536, self+39308, a3, a4)
    g_clLevelSystem.Initilaize(&g_clPlayerAbility, &g_clEmblemSystem,
                               classKind, expValue);

    // GT: cltPlayerAbility::Initialize — 13 子系統指標 + 7 個 stat
    g_clPlayerAbility.Initialize(
        &g_clLevelSystem,
        &g_clSkillSystem,
        &g_clEquipmentSystem,
        &g_clClassSystem,
        &g_clUsingItemSystem,
        &g_clUsingSkillSystem,
        &g_clPlayerSpirit,
        &g_clEmblemSystem,
        &g_clWorkingPassiveSkillSystem,
        &g_clMonsterToleranceSystem,
        /*restBonus*/ nullptr,             // GT 寫 0
        &g_clPetSystem,
        &g_clMyItemSystem,
        hp, mp, str, dex, intel, vit, bonusPoint);

    // GT: cltEquipmentSystem::Initialize(self+5556, self+6724, self+4512,
    //          self+5896, self+4536, self+396, self+4616, self+14660,
    //          self+6716, a17, a18)
    g_clEquipmentSystem.Initialize(
        &g_clSexSystem,
        &g_clLevelSystem,
        &g_clClassSystem,
        &g_clPlayerAbility,
        &g_clMyInventory,
        &g_clSkillSystem,
        &g_clPlayerSpirit,
        &g_clBasicAppearSystem,
        pEquipPrimary,
        pEquipSecondary);

    // GT: cltClassSystem::Initialize(self+5896, self+4512, self+4536,
    //          self+6568, self+396, self+5556, self+5252, self+6144,
    //          self+40384, self+4616, self+39308, a2)
    g_clClassSystem.Initialize(
        &g_clLevelSystem,
        &g_clPlayerAbility,
        &g_clLessonSystem,
        &g_clMyInventory,
        &g_clEquipmentSystem,
        &g_clQuickSlotSystem,
        &g_clUsingSkillSystem,
        &g_clWorkingPassiveSkillSystem,
        &g_clSkillSystem,
        &g_clEmblemSystem,
        charKind);

    // GT: cltSkillSystem::Initialize(self+4616, self+5252, self+6568,
    //          self+4512, self+5896, self+5556, self+4536, self+40720,
    //          a15, a16, 0)
    g_clSkillSystem.Initialize(
        &g_clQuickSlotSystem,
        &g_clLessonSystem,
        &g_clLevelSystem,
        &g_clClassSystem,
        &g_clEquipmentSystem,
        &g_clPlayerAbility,
        &g_clTitleSystem,
        numSkill,
        skillKindList,
        /*userData1*/ 0u);

    // GT: cltBasicAppearSystem::Initialize(self+6716, a32, a33, a34)
    g_clBasicAppearSystem.Initialize(static_cast<uint8_t>(hairKind),
                                     static_cast<uint8_t>(appearKind),
                                     appearLook);

    // GT: cltUsingItemSystem::Initialize(self+5944, 0, self+396, self+4536,
    //          self+16912, self+17032, self+6144, self+40384, self+39308,
    //          self+4616, self+5896, self+6724, self+5556, self+6716,
    //          self+40740, self+69084, self+69260)
    g_clUsingItemSystem.Initialize(
        /*owner*/ nullptr,
        &g_clMyInventory,
        &g_clPlayerAbility,
        &g_clSpecialtySystem,
        &g_clMakingItemSystem,
        &g_clUsingSkillSystem,
        &g_clWorkingPassiveSkillSystem,
        &g_clEmblemSystem,
        &g_clSkillSystem,
        &g_clClassSystem,
        &g_clSexSystem,
        &g_clEquipmentSystem,
        &g_clBasicAppearSystem,
        &g_clPetSystem,
        &g_clMyItemSystem,
        g_clMarriageSystem);

    // GT: cltUsingSkillSystem::Initialize(self+6144, ...) —
    //   cltUsingSkillSystem 在 inc/System/cltUsingSkillSystem.h 尚未提供
    //   Initialize 介面，待補完後解除註解。

    // GT: cltLessonSystem::Initialize(self+6568, self+396, self+39308,
    //          a19, a20, a21, a22..a25, a26..a29, a30)
    g_clLessonSystem.Initialize(
        &g_clMyInventory,
        &g_clEmblemSystem,
        lessonItemKind,
        lessonSchedule,
        lessonState,
        swordPt, bowPt, theologyPt, magicPt,
        totalSwordPt, totalBowPt, totalTheologyPt, totalMagicPt,
        lessonHistory);

    // GT: v63 = cltPetSystem::GetPetInventorySystem(v70)
    cltPetInventorySystem* petInv = g_clPetSystem.GetPetInventorySystem();

    // GT: cltQuestSystem::Initailize(self+7412, self+396, self+7400,
    //          self+4536, self+4512, self+6568, self+4616, self+5896,
    //          self+39308, self+40720, v63, self+39616, self+16912,
    //          self+39248, self+69044, self+16900, self+69084, a35, a60, 0)
    g_clQuestSystem.Initailize(
        &g_clMyInventory,
        &g_clMoneySystem,
        &g_clPlayerAbility,
        &g_clLevelSystem,
        &g_clLessonSystem,
        &g_clSkillSystem,
        &g_clClassSystem,
        &g_clEmblemSystem,
        &g_clTitleSystem,
        petInv,
        &g_clTASSystem,
        &g_clSpecialtySystem,
        &g_clMeritoriousSystem,
        &g_clPKRankSystem,
        &g_clGradeSystem,
        &g_clMyItemSystem,
        questData,
        indunMode,
        /*accountID*/ 0u);

    // GT: cltHelpSystem::Initialize(self+164, self+4512, self+7412,
    //          self+4616, self+5556, self+6728, self+6568)
    g_clHelpSystem.Initialize(
        &g_clLevelSystem,
        &g_clQuestSystem,
        &g_clSkillSystem,
        &g_clEquipmentSystem,
        /*party*/ &g_clPartySystem,
        &g_clLessonSystem);

    // GT: v64 = cltPetSystem::GetPetInventorySystem(v70)
    cltPetInventorySystem* petInv2 = g_clPetSystem.GetPetInventorySystem();

    // GT: CSpiritSpeechMgr::SetSpiritSpeech(self+14672, self+4512, self+7412,
    //          self+6568, self+4616, self+4536, v62, v64)
    g_clSpiritSpeechMgr.SetSpiritSpeech(
        &g_clLevelSystem,
        &g_clQuestSystem,
        &g_clLessonSystem,
        &g_clSkillSystem,
        &g_clPlayerAbility,
        &g_clMyInventory,
        petInv2);

    // GT: cltSpecialtySystem::Initialize(self+16912, self+16900, self+4616,
    //          self+4512, self+7412, a36, a37, a38)
    g_clSpecialtySystem.Initialize(
        &g_clGradeSystem,
        &g_clSkillSystem,
        &g_clLevelSystem,
        &g_clQuestSystem,
        specialtyPt,
        acquiredSpecialtyNum,
        acquiredSpecialty);

    // GT: cltGradeSystem::Initialize(self+16900, self+16912, self+5896, a41)
    g_clGradeSystem.Initialize(&g_clSpecialtySystem, &g_clClassSystem, grade);

    // GT: cltTestingSystem::Initialize(self+16996, self+7400, self+6568,
    //          self+16900, self+7412, 0)
    g_clTestingSystem.Initialize(
        &g_clMoneySystem,
        &g_clLessonSystem,
        &g_clGradeSystem,
        &g_clQuestSystem,
        /*lastTestingTime*/ 0);

    // GT: cltMakingItemSystem::Initialize(self+17032, v62, self+16912, a39, a40)
    g_clMakingItemSystem.Initialize(
        &g_clMyInventory,
        &g_clSpecialtySystem,
        static_cast<int>(makingItemNum),
        makingItemKinds);

    // GT: v65 = cltMyCharData::GetMyAccount(v61);
    //     cltTransformSystem::Initialize(self+19044, v65)
    const unsigned int myAccount = cltMyCharData::GetMyAccount(self);
    g_clTransformSystem.Initialize(myAccount);

    // GT: cltEnchantSystem::Initialize(self+19056, v62, self+7400, self+39308)
    g_clEnchantSystem.Initialize(&g_clMyInventory,
                                 &g_clMoneySystem,
                                 &g_clEmblemSystem);

    // GT: cltOverMindSystem::Initialize(self+39240, self+5896)
    g_clOverMindSystem.Initialize(&g_clClassSystem);

    // GT: CMeritoriousSystem::Initialize(self+39248, self+7412, v62,
    //          self+16912, self+7400, self+39308, a42, a43, a44, a45,
    //          a46 != 0, a47, (uint8)a48, a49, a50)
    g_clMeritoriousSystem.Initialize(
        &g_clQuestSystem,
        &g_clMyInventory,
        &g_clSpecialtySystem,
        &g_clMoneySystem,
        &g_clEmblemSystem,
        meritPoint,
        meritTotalPoint,
        meritGrade,
        meritGradePoint,
        warQuestPlaying != 0,
        supplyQuestPlaying,
        static_cast<std::uint16_t>(static_cast<std::uint8_t>(warQuestMonCount)),
        warQuestMonKinds,
        warQuestMonGoals);

    // GT: cltEmblemSystem::Initialize(self+39308, self+4512, self+5896,
    //          self+7412, self+7400, self+40720, a51, a52, a53, 0)
    g_clEmblemSystem.Initialize(
        &g_clLevelSystem,
        &g_clClassSystem,
        &g_clQuestSystem,
        &g_clMoneySystem,
        &g_clTitleSystem,
        emblemNum,
        emblemKinds,
        emblemUsingKind,
        /*userData0*/ 0u);

    // GT: CQuizEventSystem::Initialize(self+39544, v62, self+4512, self+7400,
    //          a54, a55, a56, a57, a58)
    g_clQuizEventSystem.Initialize(
        &g_clMyInventory,
        &g_clLevelSystem,
        &g_clMoneySystem,
        quizID,
        static_cast<int>(quizPlayed),
        quizAnswerSize,
        quizAnswerPos,
        quizAnswerItem);

    // GT: cltPetSystem::Initialize(v70, self+4512, self+7400, a61)
    g_clPetSystem.Initialize(&g_clLevelSystem, &g_clMoneySystem, msg);

    // GT: cltPKRankSystem::Initailize(self+69044, self+16912, self+39248,
    //          self+7412, a61)
    g_clPKRankSystem.Initailize(&g_clSpecialtySystem,
                                &g_clMeritoriousSystem,
                                &g_clQuestSystem,
                                msg);

    // GT: cltMyItemSystem::Initialize(self+69084, 0, self+5252, self+69260, a61)
    g_clMyItemSystem.Initialize(/*owner*/ nullptr,
                                &g_clQuickSlotSystem,
                                g_clMarriageSystem,
                                msg);

    // GT: cltQuickSlotSystem::Initialize(self+5252, v62, self+69084,
    //          self+7412, a61)
    g_clQuickSlotSystem.Initialize(&g_clMyInventory,
                                   &g_clMyItemSystem,
                                   &g_clQuestSystem,
                                   msg);

    // GT: cltMarriageSystem::Initialize(self+69260, 0, *((_DWORD*)v61 + 24),
    //          v62, self+5896, self+69084, a61, 0)
    //   *((_DWORD*)v61 + 24) = m_dwServerTime
    if (g_clMarriageSystem) {
        g_clMarriageSystem->Initialize(
            /*owner*/ nullptr,
            static_cast<int>(self->m_dwServerTime),
            &g_clMyInventory,
            &g_clClassSystem,
            &g_clMyItemSystem,
            msg,
            /*outCharged*/ nullptr);
    }

    // GT: cltTASSystem::Initialize(self+39616, self+4512, self+7400,
    //          self+7412, a61) — CMofMsg-form overload
    g_clTASSystem.Initialize(&g_clLevelSystem,
                             &g_clMoneySystem,
                             &g_clQuestSystem,
                             msg);

    // GT: cltWorkingPassiveSkillSystem::Initialize(self+40384, ...) —
    //   cltWorkingPassiveSkillSystem 尚未提供 Initialize 介面，留待補完。

    // GT: v66 = ClientCharacterManager::GetMyCharacterPtr(&g_ClientCharMgr);
    //     cltMonsterToleranceSystem::Initialize(self+40568, v66, self+4616)
    {
        ClientCharacter* my = g_ClientCharMgr.GetMyCharacterPtr();
        g_clMonsterToleranceSystem.Initialize(my, &g_clSkillSystem);
    }

    // GT: v67 = cltClassSystem::GetClass(self+5896);
    //     cltMoFC_EquipmentInfo::Init(self+40576, v67)
    {
        const std::uint16_t classCode = g_clClassSystem.GetClass();
        g_clMoFC_EquipmentInfo.Init(classCode);
    }

    // GT: cltNPCRecallSystem::Initialize(self+40712, self+7400)
    g_clNPCRecallSystem.Initialize(&g_clMoneySystem);

    // GT: cltEmoticonSystem::InitializeStaticVariable(unk_82723C, v62,
    //          &g_clEmoticonKindInfo);
    //     v71 = {1, 9, 3, 13, 17};
    //     cltEmoticonSystem::Initialize(unk_82723C, v71);
    cltEmoticonSystem::InitializeStaticVariable(&g_clMyInventory,
                                                &g_clEmoticonKindInfo);
    {
        std::uint8_t defaultShortcut[5] = {1, 9, 3, 13, 17};
        g_clEmoticonSystem.Initialize(defaultShortcut);
    }

    // GT: cltTitleSystem::Initialize(self+40720, 0, a59, self+4512, v62)
    g_clTitleSystem.Initialize(/*owner*/ nullptr,
                               currentTitle,
                               &g_clLevelSystem,
                               &g_clMyInventory);

    // GT: cltMyCharData::IndunQuestNotice(v61);
    cltMyCharData::IndunQuestNotice(self);

    // GT: dword_18C4E14 = 0;
    //   尚未提升為命名全域，IDA 推測為「升級樂譜框」相關旗標。略。

    // GT: v68 = cltLevelSystem::GetLevel(self+4512);
    //     CSpiritSystem::GetSpiritKind(&g_clSpiritSystem, v68);
    //     v69 = cltPetSystem::GetPetKind(v70);
    //     CSpiritSpeechMgr::SetSpiritID(self+14672, v69);
    //     CSpiritSpeechMgr::PlaySpiritSpeech(self+14672);
    {
        const std::uint8_t lvl = g_clLevelSystem.GetLevel();
        g_clSpiritSystem.GetSpiritKind(lvl);
        const std::uint16_t petKind = g_clPetSystem.GetPetKind();
        g_clSpiritSpeechMgr.SetSpiritID(petKind);
        g_clSpiritSpeechMgr.PlaySpiritSpeech();
    }

    // GT: cltPKManager::Init(&g_clPKManager) — 無參數
    g_clPKManager.Init();
}

// =============================================================================
// (005184A0) UnInit
// =============================================================================
void cltMyCharData::UnInit(cltMyCharData* self) {
    // GT: 數個 WORD/DWORD 欄位 reset
    self->m_uNearItemPos = 0;
    self->m_uNearItemKind = 0;
    self->m_uNearItemNum = 0;
    self->m_iRequestPickUpFlag = 1;
    self->m_iNearItemMoney = 0;
    self->m_iSomething16 = 0;       // GT *(DWORD*)+16
    self->m_byPrevLevel = 0;
    self->m_iSomething19 = 0;
    self->m_uSomething40 = 0;
    self->m_dwMyAccount = 0;
    self->m_iSomething27 = 0;
    self->m_iCanNotChatting = 0;
    self->m_uMapID = 0;
    self->m_uMapAreaName = 0;
    self->m_uMapVorF = 0;
    self->m_iIsPvPMap = 0;
    self->m_iIsPvPLobbyMap = 0;
    self->m_iIsContinent = 0;
    self->m_iSomething37 = 0;

    // GT: 對齊 mofclient.c 行 229979..229993 的 Free 序列。
    //   cltUsingSkillSystem / cltBasicAppearSystem 在本還原無 Free() 介面，
    //   故略；其餘對應全域均呼叫一次。CPlayerSpirit 不是 Free 而是 Initialize
    //   重置（GT 行 229991）。
    g_clMyInventory.Free();           // GT: cltBaseInventory::Free(self+396)
    g_clSkillSystem.Free();           // GT: cltSkillSystem::Free(self+4616)
    g_clQuickSlotSystem.Free();       // GT: cltQuickSlotSystem::Free(self+5252)
    g_clEquipmentSystem.Free();       // GT: cltEquipmentSystem::Free(self+5556)
    g_clClassSystem.Free();           // GT: cltClassSystem::Free(self+5896)
    g_clUsingItemSystem.Free();       // GT: cltUsingItemSystem::Free(self+5944)
    // GT: cltUsingSkillSystem::Free(self+6144) — 無對應介面，略。
    g_clLessonSystem.Free();          // GT: cltLessonSystem::Free(self+6568)
    // GT: cltBasicAppearSystem::Free(self+6716) — 無對應介面，略。
    g_clTradeSystem.Free();           // GT: cltTradeSystem::Free(self+7152)
    g_clNPCRecallSystem.Free();       // GT: cltNPCRecallSystem::Free(self+40712)
    g_clSpiritSpeechMgr.Free();       // GT: CSpiritSpeechMgr::Free(self+14672)
    g_clPlayerSpirit.Initialize();    // GT: CPlayerSpirit::Initialize(self+14660)
    g_clPetSystem.Free();             // GT: cltPetSystem::Free(self+40740)

    // GT: 若隊伍存在 → 離隊
    if (g_clPartySystem.IsCreated()) {
        // GT: cltClientPartySystem::Leave(self+6728, dword_1844428)
        //   dword_1844428 為當前 leader 帳號暫存；尚未還原為命名全域，
        //   暫以 0 帶入；對齊呼叫位置即可。
        g_clPartySystem.Leave(0);
    }
}

// =============================================================================
// 帳號 / 名稱 / 伺服器時間 / GameMoney
// =============================================================================
void cltMyCharData::SetMyAccount(cltMyCharData* self, unsigned int account) {
    // GT: *((DWORD*)+21) = a2;
    self->m_dwMyAccount = account;
    // 兼容既有客戶端：global g_dwMyAccountID 也同步更新。
    g_dwMyAccountID = account;
}

unsigned int cltMyCharData::GetMyAccount(cltMyCharData* self) {
    return self->m_dwMyAccount;
}

void cltMyCharData::SetMyCharName(cltMyCharData* self, const char* name) {
    // GT: strcpy((char*)this, a2);
    if (!name) {
        self->m_szCharName[0] = '\0';
        return;
    }
    std::strncpy(self->m_szCharName, name, sizeof(self->m_szCharName) - 1);
    self->m_szCharName[sizeof(self->m_szCharName) - 1] = '\0';
}

cltMyCharData* cltMyCharData::GetMyCharName(cltMyCharData* self) {
    // GT 0x518610: `return this;` — 第一個 member m_szCharName 起點即字串。
    return self;
}

void cltMyCharData::SetServerTime(cltMyCharData* self, unsigned int serverTime) {
    // GT: *((DWORD*)+24) = a2; *((DWORD*)+25) = _time(0);
    self->m_dwServerTime = serverTime;
    self->m_dwSyncLocalTime = static_cast<unsigned int>(std::time(nullptr));
}

int cltMyCharData::GetGameMoney(cltMyCharData* /*self*/) {
    // GT: cltMoneySystem::GetGameMoney(self+7400)
    return g_clMoneySystem.GetGameMoney();
}

// =============================================================================
// PickUpItem / SetNearItemInfo / RequestPickUpItem
// =============================================================================
void cltMyCharData::SetNearItemInfo(cltMyCharData* self,
                                    std::uint16_t fieldItemID, int money,
                                    std::uint16_t itemKind, std::uint16_t itemNum) {
    self->m_uNearItemPos  = fieldItemID;
    self->m_iNearItemMoney = money;
    self->m_uNearItemKind = itemKind;
    self->m_uNearItemNum  = itemNum;
}

namespace {
// GT 0x4FFCD0 `getScriptParameter`：把 template 中的 "%(name)" 替換成
// parameters 對應的字串。原 binary 用 std::map<string,string> + 複雜的
// _Tree 操作；此處以等價但精簡的雙鍵替換實作（PickUpItem 只用到
// "Parameter0" / "Parameter1" 兩鍵）。
//
// 行為：
//   - 把 tmpl 內所有 "%(<name>)" 模板替換成對應 value，寫入 out。
//   - 如果 tmpl 不包含任何 "%(" 開頭的模板，回傳 false 表示未替換 →
//     呼叫端應改用 printf-style fallback。
bool ApplyScriptParameter(char* out, std::size_t outSize, const char* tmpl,
                          const char* k0, const char* v0,
                          const char* k1, const char* v1) {
    if (!tmpl || !out || outSize == 0) {
        if (out && outSize) out[0] = '\0';
        return false;
    }
    bool didAny = false;
    std::size_t writeIdx = 0;
    for (std::size_t i = 0; tmpl[i] != '\0'; ) {
        if (tmpl[i] == '%' && tmpl[i + 1] == '(') {
            const char* close = std::strchr(tmpl + i + 2, ')');
            if (close) {
                const std::size_t nameLen =
                    static_cast<std::size_t>(close - (tmpl + i + 2));
                const char* repl = nullptr;
                if (k0 && std::strlen(k0) == nameLen
                    && std::strncmp(tmpl + i + 2, k0, nameLen) == 0) {
                    repl = v0 ? v0 : "";
                } else if (k1 && std::strlen(k1) == nameLen
                           && std::strncmp(tmpl + i + 2, k1, nameLen) == 0) {
                    repl = v1 ? v1 : "";
                }
                if (repl) {
                    didAny = true;
                    while (*repl && writeIdx + 1 < outSize) {
                        out[writeIdx++] = *repl++;
                    }
                    i = static_cast<std::size_t>(close - tmpl) + 1;
                    continue;
                }
            }
        }
        if (writeIdx + 1 < outSize) out[writeIdx++] = tmpl[i];
        ++i;
    }
    out[writeIdx < outSize ? writeIdx : outSize - 1] = '\0';
    return didAny;
}
}  // namespace

// GT 0x518680 — 拾取確認流程。
// 物品分支：
//   - 取 textCode 4462 的範本，先嘗試把 "%(Parameter0)" → 物品名稱、
//     "%(Parameter1)" → 數量字串 替換進去；若範本不含 "%(" 模板，
//     退回原 binary 的 sprintf fallback (`_sprintf(Buffer, fmt)`，把後續
//     的 v19=itemName / v20=itemNum 當作 vararg 傳入)。
//   - 之後執行 cltBaseInventory::AddInventoryItem 與 SpiritSpeech 通知。
// 金錢分支：直接 sprintf textCode 4463 + 金額，並 IncreaseMoney。
// 兩個分支最後都丟到 cltSystemMessage::SetSystemMessage 顯示。
void cltMyCharData::PickUpItem(cltMyCharData* self) {
    char buffer[1024] = {0};

    if (self->m_uNearItemKind) {
        // 取物品名稱
        const char* itemName = "";
        if (auto* itemInfo = g_clItemKindInfo.GetItemKindInfo(self->m_uNearItemKind)) {
            const char* n = g_DCTTextManager.GetText(itemInfo->m_wTextCode);
            if (n) itemName = n;
        }
        // 數量字串
        char numStr[16];
        std::snprintf(numStr, sizeof(numStr), "%u",
                      static_cast<unsigned>(self->m_uNearItemNum));

        const char* tmpl = g_DCTTextManager.GetText(4462);
        const bool replaced = ApplyScriptParameter(
            buffer, sizeof(buffer), tmpl,
            "Parameter0", itemName,
            "Parameter1", numStr);

        if (!replaced) {
            // GT fallback：_sprintf(Buffer, fmt) 把 fmt 當 printf 樣式，
            //   把 itemName / itemNum 當作 vararg。
            if (tmpl) {
                std::snprintf(buffer, sizeof(buffer), tmpl,
                              itemName, self->m_uNearItemNum);
            }
        }

        // GT: cltBaseInventory::AddInventoryItem(self+396, &v36, 0, 0)
        strInventoryItem item{};
        item.itemKind = self->m_uNearItemKind;
        item.itemQty  = self->m_uNearItemNum;
        item.value0   = 0;
        item.value1   = 0;
        std::uint8_t  outSlots[8] = {0};
        std::uint16_t outNewPos   = 0;
        g_clMyInventory.AddInventoryItem(&item, outSlots, &outNewPos);

        // GT: CSpiritSpeechMgr::UpdateQuestCollection(self+14672, m_uNearItemKind)
        g_clSpiritSpeechMgr.UpdateQuestCollection(self->m_uNearItemKind);
    } else {
        // GT: _sprintf(Buffer, GetText(4463), m_iNearItemMoney)
        const char* fmt = g_DCTTextManager.GetText(4463);
        if (fmt) {
            std::snprintf(buffer, sizeof(buffer), fmt, self->m_iNearItemMoney);
        }
        // GT: cltMoneySystem::IncreaseMoney(self+7400, m_iNearItemMoney)
        g_clMoneySystem.IncreaseMoney(self->m_iNearItemMoney);
    }

    cltSystemMessage::SetSystemMessage(&g_clSysemMessage, buffer, 0, 0, 0);
}

// GT 0x518BF0 — 主動拾取請求。
void cltMyCharData::RequestPickUpItem(cltMyCharData* self) {
    if (!self->m_iRequestPickUpFlag) return;

    int posY = g_ClientCharMgr.GetMyPositionY();
    int posX = g_ClientCharMgr.GetMyPositionX();
    auto fx = static_cast<float>(posX);
    auto fy = static_cast<float>(posY);

    std::uint16_t fieldItemID = 0;
    float outX = 0.0f, outY = 0.0f;
    if (g_clFieldItemMgr.GetNearItemInfo(fx, fy, &fieldItemID, &outX, &outY,
                                         0, 0, 0)) {
        cltFieldItem* item = g_clFieldItemMgr.GetFieldItem(fieldItemID);
        if (item) {
            int itemCount = 0;
            std::uint16_t itemKind = 0;
            std::uint16_t itemInfo = 0;
            item->GetItemInfo(&itemCount, &itemKind, &itemInfo);

            if (itemKind) {
                if (!g_clMyInventory.CanAddInventoryItem(itemKind, itemInfo)) {
                    cltMyCharData::SetNearItemInfo(self, fieldItemID, itemCount,
                                                   itemKind, itemInfo);
                    g_Network.PickUpItem(fieldItemID, itemInfo);
                    self->m_iRequestPickUpFlag = 0;
                    return;
                }
                const char* msg = g_DCTTextManager.GetText(58001);
                cltSystemMessage::SetSystemMessage(&g_clSysemMessage, msg, 0, 0, 0);
            } else {
                if (g_clMoneySystem.GetGameMoney() < 2000000000) {
                    cltMyCharData::SetNearItemInfo(self, fieldItemID, itemCount,
                                                   itemKind, itemInfo);
                    g_Network.PickUpItem(fieldItemID, 0);
                    self->m_iRequestPickUpFlag = 0;
                    return;
                }
                const char* msg = g_DCTTextManager.GetText(8117);
                cltSystemMessage::SetSystemMessage(&g_clSysemMessage, msg, 0, 0, 0);
            }
        }
    }
}

// =============================================================================
// IsLevelUp / LevelUpProcess / ReLoadQSLData
// =============================================================================
int cltMyCharData::IsLevelUp(cltMyCharData* self) {
    // GT: prev>=cur → 0；否則寫 prev=cur 並執行升級流程
    std::uint8_t cur = g_clLevelSystem.GetLevel();
    if (self->m_byPrevLevel >= cur) return 0;
    self->m_byPrevLevel = cur;
    cltMyCharData::LevelUpProcess(self);
    return 1;
}

void cltMyCharData::LevelUpProcess(cltMyCharData* self) {
    ClientCharacter* my = g_ClientCharMgr.GetMyCharacterPtr();
    if (!my) return;

    // GT 0x518D90 — vtable cycle (透過 cltBaseAbility / cltPlayerAbility 介面
    // 重新計算等級提升後的 max HP / max MP，並把 HP 回滿)：
    //
    //   v3 = cltPartySystem::IsCreated(self+6728);
    //   v4 = vtable[+24](self+4536, v3 ? party : 0);   // GetMaxHP(party)
    //   vtable[+36](self+4536, v4);                    // SetHP(v4)  → HP 回滿
    //   v5 = cltPartySystem::IsCreated(self+6728);
    //   v6 = cltPlayerAbility::GetMaxMP(self+4536, ebp_a2, v5 ? party : 0);
    //   cltPlayerAbility::SetMana(self+4536, v6);      // mana 回滿
    //   v7 = cltPartySystem::IsCreated(self+6728);
    //   self->ClientChar[2814] = vtable[+24](self+4536, v7 ? party : 0);
    //   ClientCharacter::SetHP(my);                    // 立即重繪 HP bar
    //
    // 我們的 cltPlayerAbility 沒有 vtable，但 vtable[+24] 對應到 GT 的
    // GetMaxHP(int, party*) userpurge overload；vtable[+36] 為繼承自
    // cltBaseAbility 的 SetHP(int)。兩個方法都已在 cltPlayerAbility 補齊。
    auto* party1 = g_clPartySystem.IsCreated()
                       ? static_cast<void*>(&g_clPartySystem) : nullptr;
    const int newMaxHP = g_clPlayerAbility.GetMaxHP(/*a2 ebp*/ 0, party1);
    g_clPlayerAbility.SetHP(newMaxHP);

    auto* party2 = g_clPartySystem.IsCreated()
                       ? static_cast<void*>(&g_clPartySystem) : nullptr;
    const int newMaxMP = g_clPlayerAbility.GetMaxMP(/*a2 ebp*/ 0, party2);
    g_clPlayerAbility.SetMana(newMaxMP);

    auto* party3 = g_clPartySystem.IsCreated()
                       ? static_cast<void*>(&g_clPartySystem) : nullptr;
    const int newMaxHP2 = g_clPlayerAbility.GetMaxHP(/*a2 ebp*/ 0, party3);
    // GT: v2[2814] = newMaxHP2 — store at byte offset 11256 on ClientCharacter.
    //   ClientCharacter 在本還原是 typed class，沒有命名欄位對應到該 offset；
    //   保留與 GT byte-equal 的 raw 寫入，與其他 ClientCharacter 內部還原中
    //   仍在用的 offset-based 寫法一致。
    *reinterpret_cast<int*>(reinterpret_cast<char*>(my) + 11256) = newMaxHP2;

    my->SetHP();

    // GT: PlayerLevelUp(level)
    my->PlayerLevelUp(static_cast<char>(g_clLevelSystem.GetLevel()));

    // GT: GameSound::PlaySoundA(&g_GameSoundManager, "J0014", x, y)
    g_GameSoundManager.PlaySoundA(const_cast<char*>("J0014"),
                                  g_ClientCharMgr.GetMyPositionX(),
                                  g_ClientCharMgr.GetMyPositionY());
    g_clHelpSystem.OnLevelUp();
    g_clSpiritSpeechMgr.OnLevelUp();

    // GT: CPlayerSpirit::UpdatePlayerSpiritInfo(self+14660, level)
    g_clPlayerSpirit.UpdatePlayerSpiritInfo(static_cast<std::uint16_t>(g_clLevelSystem.GetLevel()));

    g_clEquipmentSystem.UpdateValidity();

    const char* msg = g_DCTTextManager.GetText(58021);
    cltSystemMessage::SetSystemMessage(&g_clSysemMessage, msg, 0, 0, 0);

    cltMyCharData::AddQuestMark(self);

    // GT: 等級 ≤10 時跳出 4441 訊息框；GT 之後還會把訊息框位置上推到 y=100，
    //  本還原省略 SetPos 細節，僅保留主要的 AddOK 呼叫。
    if (g_clLevelSystem.GetLevel() <= 10) {
        if (g_pMsgBoxMgr) {
            g_pMsgBoxMgr->AddOK(/*a2 textCode*/ 4441, 0, 0, 0, /*a6*/ -1);
        }
    }
}

void cltMyCharData::ReLoadQSLData(cltMyCharData* /*self*/, CMofMsg* msg) {
    // GT: cltQuickSlotSystem::Initialize(self+5252, self+396, self+69084,
    //                                    self+7412, msg);
    g_clQuickSlotSystem.Initialize(&g_clMyInventory,
                                   &g_clMyItemSystem,
                                   &g_clQuestSystem,
                                   msg);
}

// =============================================================================
// SetMapID / GetPKUsableItem
// =============================================================================
void cltMyCharData::SetMapID(cltMyCharData* self, std::uint16_t mapID) {
    self->m_uMapID = mapID;
    self->m_uMapAreaName = g_Map.GetMapAreaName(mapID);
    self->m_uMapVorF     = g_Map.GetMapVorF(self->m_uMapID);

    self->m_iIsPvPMap        = g_Map.IsPvPMap(self->m_uMapID) ? 1 : 0;
    self->m_iIsPvPLobbyMap   = g_Map.IsPvPLobbyMap(self->m_uMapID) ? 1 : 0;
    self->m_iIsContinent     = g_Map.IsContinent(self->m_uMapID) ? 1 : 0;

    // GT: 非 PvP 地圖才預設 PK 使用道具為 1。
    if (!self->m_iIsPvPMap) {
        self->m_iPKUsableItem = 1;
    }
}

int cltMyCharData::GetPKUsableItem(cltMyCharData* self) {
    return self->m_iIsPvPMap ? self->m_iPKUsableItem : 1;
}

// =============================================================================
// SetItem / ResetItem / InitMyCharItemInfo
// =============================================================================
void cltMyCharData::SetItem(cltMyCharData* /*self*/, std::uint16_t itemKind, int slot) {
    if (itemKind) g_ClientCharMgr.SetMyItem(itemKind, slot);
}

void cltMyCharData::ResetItem(cltMyCharData* /*self*/, std::uint8_t slot) {
    g_ClientCharMgr.ResetMyItem(slot);
}

void cltMyCharData::InitMyCharItemInfo(cltMyCharData* /*self*/) {
    if (!dword_21BA32C) return;
    for (int i = 0; i < 11; ++i) {
        std::uint16_t kind = dword_21BA32C->GetEquipItem(0, i);
        if (kind) g_ClientCharMgr.SetMyItem(kind, 1);
    }
    for (int i = 0; i < 11; ++i) {
        std::uint16_t kind = dword_21BA32C->GetEquipItem(1, i);
        if (kind) g_ClientCharMgr.SetMyItem(kind, 1);
    }
}

// =============================================================================
// MiniGame / Lesson
// =============================================================================
std::uint16_t cltMyCharData::GetMiniGameKind(cltMyCharData* /*self*/) {
    std::uint8_t* type = g_clNPCManager.GetNPCType();
    if (!type) return 0;
    if (type[3]) return 1;  // Sword
    if (type[4]) return 2;  // Bow
    if (type[5]) return 3;  // Magic
    return type[6] ? 4 : 0; // Exorcist or none
}

unsigned int cltMyCharData::GetLessonPoint(cltMyCharData* self) {
    switch (cltMyCharData::GetMiniGameKind(self)) {
        case 1u: return g_clLessonSystem.GetSwordLessonPt();
        case 2u: return g_clLessonSystem.GetBowLessonPt();
        case 3u: return g_clLessonSystem.GetMagicLessonPt();
        case 4u: return g_clLessonSystem.GetTheologyLessonPt();
        default: return 0;
    }
}

void cltMyCharData::IncLessonPt_Sword(cltMyCharData* /*self*/, unsigned int v) {
    g_clLessonSystem.IncLessonPt_Sword(v);
}
void cltMyCharData::IncLessonPt_Bow(cltMyCharData* /*self*/, unsigned int v) {
    g_clLessonSystem.IncLessonPt_Bow(v);
}
void cltMyCharData::IncLessonPt_Theology(cltMyCharData* /*self*/, unsigned int v) {
    g_clLessonSystem.IncLessonPt_Theology(v);
}
void cltMyCharData::IncLessonPt_Magic(cltMyCharData* /*self*/, unsigned int v) {
    g_clLessonSystem.IncLessonPt_Magic(v);
}

// =============================================================================
// Party
// =============================================================================
int cltMyCharData::RequestJoinParty(cltMyCharData* /*self*/, char* name) {
    return g_Network.RequestJoinParty(name);
}

int cltMyCharData::ResponesJoinParty(cltMyCharData* self, std::uint8_t accept) {
    self->m_iIsResponesSuccess = 1;
    self->m_iAcceptParty = accept;
    return g_Network.ResponseJoinParty(accept);
}

void cltMyCharData::SetCSR_ResponesParty(cltMyCharData* self, int v) {
    self->m_iIsResponesSuccess = v;
}

int cltMyCharData::IsResponesSuccess(cltMyCharData* self) {
    return self->m_iIsResponesSuccess;
}

int cltMyCharData::RequestLeaveParty(cltMyCharData* /*self*/) {
    return g_Network.LeaveParty();
}

int cltMyCharData::GetAcceptParty(cltMyCharData* self) {
    return self->m_iAcceptParty;
}

int cltMyCharData::RequestKickOutParty(cltMyCharData* self, unsigned int kickedAccount) {
    self->m_dwKickedAccount = kickedAccount;
    return g_Network.KickOutParty(kickedAccount);
}

unsigned int cltMyCharData::GetKickedAccount(cltMyCharData* self) {
    return self->m_dwKickedAccount;
}

cltClientPartySystem* cltMyCharData::GetPartySystemByIsCreated(cltMyCharData* /*self*/) {
    return g_clPartySystem.IsCreated() ? &g_clPartySystem : nullptr;
}

void cltMyCharData::ReactiveCommunity(cltMyCharData* /*self*/) {
    if (g_pInterfaceDataCommunity) g_pInterfaceDataCommunity->ReactiveCommunity();
}

// =============================================================================
// Trade
// =============================================================================
int cltMyCharData::RequestTrade(cltMyCharData* self, unsigned int otherAccount) {
    self->m_dwRequestedTradeAccount = otherAccount;
    return g_Network.RequestTrade(otherAccount);
}

int cltMyCharData::RequestAddTradeBasket(cltMyCharData* self, std::uint8_t pos, std::uint16_t num) {
    self->m_byRequestAddPos = pos;
    self->m_uRequestAddNum  = num;
    return g_Network.RequestAddTradeItem(pos, num);
}

void cltMyCharData::AddTradeBasket(cltMyCharData* self) {
    g_clTradeSystem.AddTadeBasketByInventoryPos(self->m_dwMyAccount,
                                                self->m_byRequestAddPos,
                                                self->m_uRequestAddNum);
}

void cltMyCharData::AddTradeBasket(cltMyCharData* self, std::uint16_t itemKind,
                                   std::uint16_t num, unsigned int time) {
    g_clTradeSystem.AddTadeBasketByItemKind(self->m_dwRequestedTradeAccount,
                                            itemKind, num, time);
}

int cltMyCharData::RequestDeleteTradeBasket(cltMyCharData* self,
                                            std::uint8_t slot, std::uint16_t num) {
    self->m_byRequestDelPos = slot;
    self->m_uRequestDelNum  = num;
    return g_Network.RequestDeleteTradeItem(slot, num);
}

void cltMyCharData::SetRequestedTradeAccount(cltMyCharData* self, unsigned int otherAccount) {
    self->m_dwRequestedTradeAccount = otherAccount;
}

int cltMyCharData::RequestResponseTrade(cltMyCharData* self, int accept) {
    self->m_iAcceptTrade = accept;
    return g_Network.ResponseTrade(static_cast<std::uint8_t>(accept));
}

int cltMyCharData::GetAcceptTrade(cltMyCharData* self) {
    return self->m_iAcceptTrade;
}

void cltMyCharData::InitializeTradeSystem(cltMyCharData* self) {
    // GT: cltTradeSystem::Initialize(self+7152, my, &g_clMyInventory, &g_clMoneySystem,
    //                                otherAcc, 0, 0)
    //  我們的 cltTradeSystem::Initialize 簽章為 (myAcc, myInv, myMoney,
    //                                            otherAcc, otherInv, otherMoney)
    g_clTradeSystem.Initialize(self->m_dwMyAccount, &g_clMyInventory,
                               &g_clMoneySystem,
                               self->m_dwRequestedTradeAccount, nullptr, nullptr);
}

void cltMyCharData::DeleteTradeBasket(cltMyCharData* self) {
    g_clTradeSystem.DeleteTadeBasket(self->m_dwMyAccount,
                                     self->m_byRequestDelPos,
                                     self->m_uRequestDelNum);
}

void cltMyCharData::DeleteTradeBasket(cltMyCharData* self,
                                      std::uint8_t slot, std::uint16_t num) {
    g_clTradeSystem.DeleteTadeBasket(self->m_dwRequestedTradeAccount, slot, num);
}

int cltMyCharData::RequestChangeTradeMoney(cltMyCharData* self, unsigned int money) {
    self->m_dwTradeMoneyChange = money;
    return g_Network.RequestChangeTradeMoney(money);
}

void cltMyCharData::ChangeMoney(cltMyCharData* self) {
    g_clTradeSystem.SetChangeMoney(self->m_dwMyAccount,
                                   static_cast<int>(self->m_dwTradeMoneyChange));
}

void cltMyCharData::ChangeMoney(cltMyCharData* self, unsigned int money) {
    g_clTradeSystem.SetChangeMoney(self->m_dwRequestedTradeAccount,
                                   static_cast<int>(money));
}

int cltMyCharData::RequestTradeDecision(cltMyCharData* self, int decision) {
    self->m_iTradeDecision = decision;
    return g_Network.RequestTradeDecision();
}

int cltMyCharData::RequestTradeCancel(cltMyCharData* /*self*/) {
    return g_Network.RequestTradeCancel();
}

int cltMyCharData::SetTradeDecision(cltMyCharData* self) {
    return g_clTradeSystem.SetTradeOK(self->m_dwMyAccount, self->m_iTradeDecision) == 0;
}

int cltMyCharData::SetTradeDecision(cltMyCharData* self, int decision) {
    return g_clTradeSystem.SetTradeOK(self->m_dwRequestedTradeAccount, decision) == 0;
}

int cltMyCharData::GetMyTradeOK(cltMyCharData* /*self*/) {
    return g_clTradeSystem.GetMyTradeOK();
}

int cltMyCharData::GetOtherTradeOK(cltMyCharData* /*self*/) {
    return g_clTradeSystem.GetOtherTradeOK();
}

void cltMyCharData::CancelTrade(cltMyCharData* /*self*/) {
    g_clTradeSystem.Free();
}

void cltMyCharData::DelCharCancelTrade(cltMyCharData* /*self*/) {
    g_clTradeSystem.Free();
    if (g_UIMgr) {
        if (auto* w = static_cast<CUITradeUser*>(g_UIMgr->GetUIWindow(16))) {
            w->CompleteTradeCanceled(1);
        }
    }
}

// =============================================================================
// Quest
// =============================================================================
cltQuestSystem* cltMyCharData::GetQuestSystem(cltMyCharData* /*self*/) {
    // GT: 回傳指向內嵌 QuestSystem 的指標 (this+7412)。我們的全域單例：
    return &g_clQuestSystem;
}

int cltMyCharData::CanReward(cltMyCharData* /*self*/, int npcID) {
    return g_clQuestSystem.CanReward(npcID);
}

std::uint16_t cltMyCharData::UpdateHuntQuest(cltMyCharData* /*self*/, std::uint16_t monsterKind) {
    return g_clQuestSystem.UpdateHuntQuest(monsterKind);
}

void cltMyCharData::SetSpiritSpeechMonsterKill(cltMyCharData* /*self*/, std::uint16_t monsterKind) {
    g_clSpiritSpeechMgr.OnMonsterKill(monsterKind);
    g_clSpiritSpeechMgr.OnMonsterKillAll();
}

void cltMyCharData::AddQuestMark(cltMyCharData* /*self*/) {
    // GT: 遍歷 dword_7C3684（NPC 數量）對每個 NPC 重新貼任務 / 獎勵 mark。
    //  dword_7C3684 是 cltNPCManager 內部 NPC 計數器；目前還原中沒對應命名
    //  全域。退而求其次，我們用 256 為上界（與 cltNPCManager 的工作集大致
    //  匹配），對所有有效 NPC ID 做相同處理。
    for (int i = 0; i < 256; ++i) {
        std::uint16_t npcID = g_clNPCManager.GetNPCID(i);
        if (!npcID) continue;
        g_clNPCObject.DelRewardMark(npcID);
        g_clNPCObject.DelQuestMark(npcID);
        if (g_clQuestSystem.CanReward(npcID))
            g_clNPCObject.AddRewardMark(npcID);
        if (g_clQuestSystem.CanShowQuestAcceptMark(npcID))
            g_clNPCObject.AddQuestMark(npcID);
    }
}

void cltMyCharData::CompleteFunctionQuest(cltMyCharData* /*self*/, std::uint16_t npcID) {
    g_clNPCObject.DelQuestMark(npcID);
    g_clNPCObject.DelRewardMark(npcID);
    g_clNPCObject.AddRewardMark(npcID);
}

// =============================================================================
// Effect / 禁言 / 副本提示
// =============================================================================
// GT 5992：idb-only。本還原以 no-op 實作。
void cltMyCharData::LoadEffectImage(cltMyCharData* /*self*/) {}

void cltMyCharData::SetCanNotChatting(cltMyCharData* self, std::uint8_t active) {
    self->m_iCanNotChatting = active;
}

int cltMyCharData::GetCanNotChatting(cltMyCharData* self) {
    return self->m_iCanNotChatting;
}

// GT 0x519700 — 進入地圖時若有「副本任務」尚未取得對應道具，跳出多行訊息框。
//
// 流程（與 mofclient.c 行 230849..230941 等價）：
//   for i in 0..99:
//     qi = questSystem.GetPlayingQuestInfoByIndex(i)
//     if !qi || qi.bStatus != 0: continue
//     ki = questKindInfo.GetQuestKindInfo(qi.wQuestID)
//     if !ki || ki.bPlayType != 8: continue        // 8 = 副本任務
//     reqItemKind = ki.extra.playIndun.wDungeonItemCode
//     找背包 item type=1 範圍內是否持有 reqItemKind
//     if (qi.dwValue == 0) && !found: 把 ki.wQuestNameCode 對應字串
//                                       串成「任務A, 任務B, …」
//   若有任一條：用 textCode 4875 的範本格式化後丟到 AddMultLineOK。
void cltMyCharData::IndunQuestNotice(cltMyCharData* /*self*/) {
    char buf[256] = {};
    char line[64];
    int  hasAny = 0;

    for (std::uint8_t i = 0; i < 100; ++i) {
        stPlayingQuestInfo* qi = g_clQuestSystem.GetPlayingQuestInfoByIndex(i);
        if (!qi || qi->bStatus) continue;

        stQuestKindInfo* ki = g_clQuestKindInfo.GetQuestKindInfo(qi->wQuestID);
        if (!ki) continue;
        if (ki->bPlayType != 8) continue;          // GT: ki+18 == 8

        // GT: *(WORD*)(ki + 98) — 副本任務的素材 itemKind。stQuestKindInfo
        //   的 union extra 在 playType==8 時為 playIndun，offset 96 = wDungeonId、
        //   offset 98 = wDungeonItemCode；後者即玩家需在背包中持有的素材。
        const std::uint16_t reqItemKind = ki->extra.playIndun.wDungeonItemCode;

        int slotLo = 0, slotHi = 0;
        g_clMyInventory.GetItemSlotIndexRangeByItemType(1, &slotLo, &slotHi);

        bool found = false;
        for (int s = slotLo; s < slotHi; ++s) {
            strInventoryItem* it = g_clMyInventory.GetInventoryItem(
                static_cast<std::uint16_t>(s));
            if (it && it->itemKind == reqItemKind) { found = true; break; }
        }
        // GT: !*((_DWORD *)v4 + 2) && !v11
        //   *((_DWORD *)v4 + 2) = stPlayingQuestInfo::dwValue (offset 8)
        //   非零代表任務已在進行（獵殺累積數 / 配送旗標等），不必再提醒。
        if (qi->dwValue || found) continue;

        const char* name = g_DCTTextManager.GetText(ki->wQuestNameCode);
        std::snprintf(line, sizeof(line), "%s, ", name ? name : "");
        std::strncat(buf, line, sizeof(buf) - std::strlen(buf) - 1);
        hasAny = 1;
    }

    if (hasAny && g_pMsgBoxMgr) {
        char body[512];
        const char* tmpl = g_DCTTextManager.GetText(4875);
        std::snprintf(body, sizeof(body), tmpl ? tmpl : "%s", buf);
        g_pMsgBoxMgr->AddMultLineOK(static_cast<CUIBase*>(nullptr), body, 1u,
                                    0, 0, 0, 1, 1, 0);
    }
}

// =============================================================================
// 自動攻擊 / 事件
// =============================================================================
void cltMyCharData::SetAutoAttack(cltMyCharData* self, int active) {
    self->m_iAutoAttack = active;
}
int cltMyCharData::GetAutoAttack(cltMyCharData* self) {
    return self->m_iAutoAttack;
}

// GT 5997/5998：idb-only 宣告，本體未在反編譯結果中出現 → no-op。
void cltMyCharData::StartEventAPowerAdvantage(cltMyCharData* /*self*/, std::uint16_t /*a2*/) {}
void cltMyCharData::StartEventDPowerAdvantage(cltMyCharData* /*self*/, std::uint16_t /*a2*/) {}

void cltMyCharData::SetCorporationUser(cltMyCharData* /*self*/, std::uint8_t active) {
    if (!active) g_clCashShopItem.DeleteSpecialItem();
}
