#include "System/cltQuestSystem.h"

#include <cstring>
#include <cstdio>
#include <windows.h>
#include <mmsystem.h>

#include "Info/cltQuestKindInfo.h"
#include "Info/cltNPCInfo.h"
#include "Info/cltLessonKindInfo.h"
#include "Info/cltCharKindInfo.h"
#include "Info/cltClassKindInfo.h"
#include "Info/cltInstantDungeonKindInfo.h"
#include "Logic/cltBaseInventory.h"
#include "Logic/cltItemList.h"
#include "System/cltMoneySystem.h"
#include "System/cltLevelSystem.h"
#include "System/cltLessonSystem.h"
#include "System/cltClassSystem.h"
#include "System/cltEmblemSystem.h"
#include "System/cltTitleSystem.h"
#include "System/cltPetInventorySystem.h"
#include "System/cltTASSystem.h"
#include "System/cltSpecialtySystem.h"
#include "System/CMeritoriousSystem.h"
#include "System/cltPKRankSystem.h"
#include "System/cltGradeSystem.h"
#include "System/cltMyItemSystem.h"
#include "System/cltPlayerAbility.h"

// ---- 靜態成員定義 ----
cltQuestKindInfo*           cltQuestSystem::m_pclQuestKindInfo = nullptr;
cltNPCInfo*                 cltQuestSystem::m_pclNPCInfo = nullptr;
cltLessonKindInfo*          cltQuestSystem::m_pclLessonKindInfo = nullptr;
cltCharKindInfo*            cltQuestSystem::m_pclCharKindInfo = nullptr;
cltClassKindInfo*           cltQuestSystem::m_pclClassKindInfo = nullptr;
cltInstantDungeonKindInfo*  cltQuestSystem::m_pclInstantDungeonKindInfo = nullptr;
int  (*cltQuestSystem::m_pExternIsJoinedCircleFuncPtr)(unsigned int) = nullptr;
void (*cltQuestSystem::m_pQuestMarkRefreshFuncPtr)(std::uint16_t) = nullptr;

// g_clQuestSystem 定義在 global.cpp

// ---- 建構 / 解構 ----

cltQuestSystem::cltQuestSystem()
{
    // GT: loop 清 quest 區 + memset + playingCount + giveupPermanently
    for (int i = 0; i < MAX_QUEST_SLOTS; i++) {
        m_quests[i].wNPCID = 0;
        m_quests[i].wQuestID = 0;
        m_quests[i].bStatus = 0;
        m_quests[i].dwValue = 0;
        m_quests[i].dwStartTime = 0;
        m_quests[i].dwTimeLimit = 0;
    }
    m_bPlayingQuestCount = 0;
    std::memset(m_quests, 0, sizeof(m_quests));
    m_bGiveupPermanently = 0;
}

cltQuestSystem::~cltQuestSystem()
{
    Free();
}

void cltQuestSystem::Free()
{
    m_bPlayingQuestCount = 0;
    std::memset(m_quests, 0, sizeof(m_quests));
}

// ---- 初始化 ----

void cltQuestSystem::Initailize(
    cltBaseInventory* pInventory,
    cltMoneySystem* pMoneySystem,
    cltPlayerAbility* pPlayerAbility,
    cltLevelSystem* pLevelSystem,
    cltLessonSystem* pLessonSystem,
    cltSkillSystem* pSkillSystem,
    cltClassSystem* pClassSystem,
    cltEmblemSystem* pEmblemSystem,
    cltTitleSystem* pTitleSystem,
    cltPetInventorySystem* pPetInventorySystem,
    cltTASSystem* pTASSystem,
    cltSpecialtySystem* pSpecialtySystem,
    CMeritoriousSystem* pMeritoriousSystem,
    cltPKRankSystem* pPKRankSystem,
    cltGradeSystem* pGradeSystem,
    cltMyItemSystem* pMyItemSystem,
    const stPlayingQuestInfo* questData,
    int giveupPermanently,
    unsigned int accountID)
{
    if (!questData)
        return;

    Free();

    m_pInventory          = pInventory;
    m_pMoneySystem        = pMoneySystem;
    m_pPlayerAbility      = pPlayerAbility;
    m_pLevelSystem        = pLevelSystem;
    m_pLessonSystem       = pLessonSystem;
    m_pSkillSystem        = pSkillSystem;
    m_pClassSystem        = pClassSystem;
    m_pEmblemSystem       = pEmblemSystem;
    m_pTitleSystem        = pTitleSystem;
    m_pPetInventorySystem = pPetInventorySystem;
    m_pTASSystem          = pTASSystem;
    m_pSpecialtySystem    = pSpecialtySystem;
    m_pMeritoriousSystem  = pMeritoriousSystem;
    m_pPKRankSystem       = pPKRankSystem;
    m_pGradeSystem        = pGradeSystem;
    m_pMyItemSystem       = pMyItemSystem;
    m_dwAccountID         = accountID;
    m_bGiveupPermanently  = giveupPermanently;

    for (int i = 0; i < MAX_QUEST_SLOTS; i++) {
        if (!questData[i].wNPCID)
            break;
        m_quests[i].wNPCID     = questData[i].wNPCID;
        m_quests[i].wQuestID   = questData[i].wQuestID;
        m_quests[i].bStatus    = questData[i].bStatus;
        m_quests[i].dwValue    = questData[i].dwValue;
        m_quests[i].dwStartTime = timeGetTime() / 1000;
        m_quests[i].dwTimeLimit = questData[i].dwTimeLimit;
        m_bPlayingQuestCount++;
    }
}

void cltQuestSystem::InitializeStaticVariable(
    cltQuestKindInfo* pQuestKindInfo,
    cltNPCInfo* pNPCInfo,
    cltLessonKindInfo* pLessonKindInfo,
    cltCharKindInfo* pCharKindInfo,
    cltClassKindInfo* pClassKindInfo,
    cltInstantDungeonKindInfo* pInstantDungeonKindInfo,
    int (*pExternIsJoinedCircleFunc)(unsigned int),
    void (*pQuestMarkRefreshFunc)(std::uint16_t))
{
    m_pclQuestKindInfo           = pQuestKindInfo;
    m_pclNPCInfo                 = pNPCInfo;
    m_pclLessonKindInfo          = pLessonKindInfo;
    m_pclCharKindInfo            = pCharKindInfo;
    m_pclClassKindInfo           = pClassKindInfo;
    m_pclInstantDungeonKindInfo  = pInstantDungeonKindInfo;
    m_pExternIsJoinedCircleFuncPtr = pExternIsJoinedCircleFunc;
    m_pQuestMarkRefreshFuncPtr   = pQuestMarkRefreshFunc;
}

// ---- 欄位管理 ----

std::uint8_t cltQuestSystem::GetPlayingQuestCount()
{
    return m_bPlayingQuestCount;
}

std::uint8_t cltQuestSystem::GetQuestIndexByQuestID(std::uint16_t questID)
{
    for (std::uint8_t i = 0; i < MAX_QUEST_SLOTS; i++) {
        if (m_quests[i].wQuestID == questID)
            return i;
    }
    return 100;
}

std::uint8_t cltQuestSystem::GetRunningQuestCount()
{
    // 重建 running quest 索引表（未完成且有 NPC ID 的任務）
    std::memset(m_runningQuestIndices, 0, sizeof(m_runningQuestIndices));
    std::uint8_t count = 0;

    for (std::uint8_t i = 0; i < MAX_QUEST_SLOTS; i++) {
        if (m_quests[i].wNPCID && !m_quests[i].bStatus && count < MAX_RUNNING_QUESTS) {
            m_runningQuestIndices[count] = i;
            count++;
        }
    }
    return count;
}

// ---- 資訊存取 ----

stPlayingQuestInfo* cltQuestSystem::GetPlayingQuestInfoByNPCID(std::uint16_t npcKind)
{
    for (int i = 0; i < MAX_QUEST_SLOTS; i++) {
        if (m_quests[i].wNPCID == npcKind)
            return &m_quests[i];
    }
    return nullptr;
}

stPlayingQuestInfo* cltQuestSystem::GetPlayingQuestInfoByQuestID(std::uint16_t questKind)
{
    for (int i = 0; i < MAX_QUEST_SLOTS; i++) {
        if (m_quests[i].wQuestID == questKind)
            return &m_quests[i];
    }
    return nullptr;
}

stPlayingQuestInfo* cltQuestSystem::GetPlayingQuestInfoByIndex(std::uint8_t index)
{
    return &m_quests[index];
}

stPlayingQuestInfo* cltQuestSystem::GetRunningQuestInfoByIndex(std::uint8_t index)
{
    return &m_quests[m_runningQuestIndices[index]];
}

stPlayingQuestInfo* cltQuestSystem::GetRunningQuestInfoByQuestID(std::uint16_t questID)
{
    std::uint8_t count = GetRunningQuestCount();
    if (!count)
        return nullptr;
    for (int i = 0; i < count; i++) {
        if (m_quests[m_runningQuestIndices[i]].wQuestID == questID)
            return &m_quests[m_runningQuestIndices[i]];
    }
    return nullptr;
}

stPlayingQuestInfo* cltQuestSystem::GetPlayingQuestInfoByIndunID(std::uint16_t indunID)
{
    strInstantDungeonKindInfo* pIndun =
        m_pclInstantDungeonKindInfo->GetInstantDungeonKindInfo(indunID);
    if (!pIndun)
        return nullptr;

    for (int i = 0; i < MAX_QUEST_SLOTS; i++) {
        stQuestKindInfo* pQuest =
            m_pclQuestKindInfo->GetQuestKindInfo(m_quests[i].wQuestID);
        if (pQuest) {
            if (pQuest->bPlayType == 8) {
                std::uint16_t dungeonId = pQuest->extra.playIndun.wDungeonId;
                if (dungeonId == indunID || dungeonId == pIndun->wBaseIndunId)
                    return &m_quests[i];
            }
        }
    }
    return nullptr;
}

// ---- 完成 / 結束 ----

void cltQuestSystem::FinishQuest(std::uint16_t index)
{
    m_quests[index].bStatus = 1;
    m_quests[index].dwValue = 0;
}

void cltQuestSystem::FinishQuest(std::uint16_t index, std::uint16_t questID, std::uint16_t npcID)
{
    m_quests[index].wNPCID   = npcID;
    m_quests[index].wQuestID = questID;
    m_quests[index].bStatus  = 1;
    m_quests[index].dwValue  = 0;
}

// ---- 新增任務 ----

void cltQuestSystem::AddQuest(cltItemList* itemList, std::uint8_t* outSlots)
{
    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(m_wPendingNPCID);
    stQuestKindInfo* pQuestKind = nullptr;

    if (pInfo) {
        // 已有此 NPC 的任務欄位，更新之
        pInfo->bStatus  = 0;
        pInfo->wQuestID = m_wPendingQuestID;
        pInfo->dwValue  = 0;
        pQuestKind = m_pclQuestKindInfo->GetQuestKindInfo(m_wPendingQuestID);
        pInfo->dwTimeLimit = pQuestKind->dwTimeLimit;
        pInfo->dwStartTime = timeGetTime() / 1000;
    } else {
        // 新欄位
        std::uint8_t idx = m_bPlayingQuestCount;
        m_quests[idx].wNPCID   = m_wPendingNPCID;
        m_quests[idx].wQuestID = m_wPendingQuestID;
        m_quests[idx].bStatus  = 0;
        m_quests[idx].dwValue  = 0;
        pQuestKind = m_pclQuestKindInfo->GetQuestKindInfo(m_quests[idx].wQuestID);
        m_quests[idx].dwTimeLimit = pQuestKind->dwTimeLimit;
        m_quests[idx].dwStartTime = timeGetTime() / 1000;
        m_bPlayingQuestCount++;
    }

    // 依任務類型給予起始道具
    switch (pQuestKind->bPlayType) {
        case 3: // DELIVERY
            itemList->AddItem(pQuestKind->extra.delivery.wItemId, 1, 0, 0, 0xFFFF, nullptr);
            if (!m_pInventory->CanAddInventoryItems(itemList))
                m_pInventory->AddInventoryItem(itemList, outSlots);
            break;
        case 6: // ONEWAYDELIVERY
            itemList->AddItem(pQuestKind->extra.oneWayDelivery.wItemId, 1, 0, 0, 0xFFFF, nullptr);
            if (!m_pInventory->CanAddInventoryItems(itemList))
                m_pInventory->AddInventoryItem(itemList, outSlots);
            break;
        case 8: // PLAYINDUN
            itemList->AddItem(pQuestKind->extra.playIndun.wDungeonItemCode, 1, 0, 0, 0xFFFF, nullptr);
            if (!m_pInventory->CanAddInventoryItems(itemList))
                m_pInventory->AddInventoryItem(itemList, outSlots);
            break;
    }

    m_wPendingNPCID   = 0;
    m_wPendingQuestID = 0;
}

void cltQuestSystem::AddQuest()
{
    cltItemList tempList;
    AddQuest(&tempList, nullptr);
}

// ---- 獵殺任務 ----

std::uint16_t cltQuestSystem::UpdateHuntQuest(std::uint16_t monsterKind)
{
    std::uint16_t outNPC = 0;
    unsigned int outVal = 0;
    return UpdateHuntQuest(monsterKind, &outNPC, &outVal);
}

std::uint16_t cltQuestSystem::UpdateHuntQuest(std::uint16_t monsterKind, std::uint16_t* outNPCID, unsigned int* outValue)
{
    void* charInfo = m_pclCharKindInfo->GetCharKindInfo(monsterKind);

    // stCharKindInfo offset 6 = 怪物族群代碼
    std::uint16_t tribeCode = *reinterpret_cast<std::uint16_t*>(
        reinterpret_cast<char*>(charInfo) + 6);

    for (std::uint8_t i = 0; i < MAX_QUEST_SLOTS; i++) {
        stQuestKindInfo* pQuest =
            m_pclQuestKindInfo->GetQuestKindInfo(m_quests[i].wQuestID);
        if (!pQuest)
            continue;
        if (pQuest->bPlayType == 2
            && !m_quests[i].bStatus
            && pQuest->extra.hunt.wTargetId1 == tribeCode
            && m_quests[i].dwValue < static_cast<unsigned int>(pQuest->extra.hunt.wCount))
        {
            *outNPCID = m_quests[i].wNPCID;
            m_quests[i].dwValue++;
            *outValue = m_quests[i].dwValue;
            return m_quests[i].wQuestID;
        }
    }
    return 0;
}

// ---- ProcRemainTime ----

unsigned int cltQuestSystem::ProcRemainTime(std::uint16_t questID)
{
    unsigned int currentTime = timeGetTime() / 1000;

    for (std::uint8_t i = 0; i < MAX_QUEST_SLOTS; i++) {
        if (m_quests[i].wQuestID == questID) {
            if (m_quests[i].bStatus)
                return 0;
            stQuestKindInfo* pQuest =
                m_pclQuestKindInfo->GetQuestKindInfo(m_quests[i].wQuestID);
            if (!pQuest || !pQuest->dwTimeLimit)
                return 0;
            return m_quests[i].dwStartTime + m_quests[i].dwTimeLimit - currentTime;
        }
    }
    return 0;
}

// ---- CanReward ----

std::uint16_t cltQuestSystem::CanReward(int npcID)
{
    std::uint16_t dummy = 0;
    return CanReward(npcID, &dummy);
}

std::uint16_t cltQuestSystem::CanReward(int npcID, std::uint16_t* outDeliveryNPC)
{
    std::uint16_t result = 0;
    std::uint16_t rewardNPC = static_cast<std::uint16_t>(npcID);

    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(static_cast<std::uint16_t>(npcID));
    if (pInfo) {
        // 狀態 1=完成, 2=放棄 → 不再判斷
        if (pInfo->bStatus == 2 || pInfo->bStatus == 1) {
            goto check_oneway_delivery;
        }

        if (outDeliveryNPC)
            *outDeliveryNPC = 0;
        m_wRewardNPCID = 0;
        m_wRewardItemKind = 0;
        m_pRewardQuestInfo = nullptr;

        stQuestKindInfo* pQuest =
            m_pclQuestKindInfo->GetQuestKindInfo(pInfo->wQuestID);
        if (!pQuest) {
            char buf[256];
            wsprintfA(buf, "Quest info incorrect : %i", pInfo->wQuestID);
            return 0;
        }

        switch (pQuest->bPlayType) {
            case 1: { // COLLECT
                if (m_pInventory->IsLock())
                    break;
                std::uint16_t qty1_pet =
                    m_pPetInventorySystem->GetInventoryItemQuantity(pQuest->extra.collection.wQuestItem1);
                std::uint16_t total1 = static_cast<std::uint16_t>(
                    m_pInventory->GetInventoryItemQuantity(pQuest->extra.collection.wQuestItem1)) + qty1_pet;
                std::uint16_t qty2_pet =
                    m_pPetInventorySystem->GetInventoryItemQuantity(pQuest->extra.collection.wQuestItem2);
                std::uint16_t total2 = static_cast<std::uint16_t>(
                    m_pInventory->GetInventoryItemQuantity(pQuest->extra.collection.wQuestItem2)) + qty2_pet;
                if (total1 >= pQuest->extra.collection.wQuestItemCount1
                    && total2 >= pQuest->extra.collection.wQuestItemCount2)
                {
                    result = pQuest->wEndDialogue;
                }
                break;
            }
            case 2: // HUNT
                if (pInfo->dwValue >= static_cast<unsigned int>(pQuest->extra.hunt.wCount))
                    result = pQuest->wEndDialogue;
                break;
            case 3: // DELIVERY
                if (pInfo->dwValue == 1)
                    result = pQuest->wEndDialogue;
                break;
            case 4: { // LESSONSETTING
                int matchCount = 0;
                std::uint8_t categories[4] = { 0, 1, 2, 3 };
                for (int c = 0; c < 4; c++) {
                    for (std::uint8_t s = 0; s < 4; s++) {
                        std::uint8_t lessonId = m_pLessonSystem->GetLessonSchedule(s);
                        if (!lessonId)
                            break;
                        strLessonKindInfo* pLesson =
                            m_pclLessonKindInfo->GetLessonKindInfo(lessonId);
                        if (pLesson && categories[c] == pLesson->bLessonType) {
                            matchCount++;
                            break;
                        }
                    }
                }
                if (matchCount > 3)
                    result = pQuest->wEndDialogue;
                break;
            }
            case 5: // LESSON
                if (m_pLessonSystem->GetTotalSwordLessonPt()
                    || m_pLessonSystem->GetTotalBowLessonPt()
                    || m_pLessonSystem->GetTotalTheologyLessonPt()
                    || m_pLessonSystem->GetTotalMagicLessonPt())
                {
                    result = pQuest->wEndDialogue;
                }
                break;
            case 7: // BUYEMBLEM
                if (m_pEmblemSystem->GetGenericEmblemByID(pQuest->extra.buyEmblem.wEmblemItemId))
                    result = pQuest->wEndDialogue;
                break;
            case 8: // PLAYINDUN
                if (pInfo->dwValue == 1)
                    result = pQuest->wEndDialogue;
                break;
            case 9:  // BUYNPCITEM
            case 10: // STORAGEBOXUSE
            case 11: // ADDFRIEND
            case 12: // WHISPER
            case 14: // SENDPOSTIT
            case 17: // WYVERNTAKE
            case 18: // BOATTAKE
            case 19: // PARTY
            case 20: // BUYSAILSAGENCY
            case 21: // SAILSAILSAGENCY
            case 22: // OPENPRIVATESHOP
            case 23: // OPENCASHSHOP
            case 25: // MATERIALSUPPLY
            case 27: // ENCHANTITEM
            case 28: // HUNTMATCH
            case 29: // PVP
                if (pInfo->dwValue == 1)
                    result = pQuest->wEndDialogue;
                break;
            case 13: // ADDSENIOR
                if (m_pTASSystem->AmIStudent()
                    || (cltLevelSystem::GetMaxLevel() - 10 < m_pLevelSystem->GetLevel()))
                {
                    CompleteFunctionQuest(13);
                    result = pQuest->wEndDialogue;
                }
                break;
            case 15: // EXAMINATION
                if (m_pGradeSystem->GetGrade() >= 0x14u || pInfo->dwValue == 1) {
                    CompleteFunctionQuest(15);
                    result = pQuest->wEndDialogue;
                }
                break;
            case 16: // SPECIALTYLEARN
                if (m_pSpecialtySystem->GetAcquiredSpecialtyNum()) {
                    CompleteFunctionQuest(16);
                    result = pQuest->wEndDialogue;
                }
                break;
            case 24: // MERITORIOUS
                if (pInfo->dwValue == 1)
                    result = pQuest->wEndDialogue;
                else if (cltLevelSystem::GetMaxLevel() - 13 < m_pLevelSystem->GetLevel())
                    result = pQuest->wEndDialogue;
                break;
            case 26: // ADDJUNIOR
                if (m_pTASSystem->AmITeacher()) {
                    CompleteFunctionQuest(26);
                    result = pQuest->wEndDialogue;
                }
                break;
            case 30: // JOINCICLE
                if (m_pExternIsJoinedCircleFuncPtr(m_dwAccountID)) {
                    CompleteFunctionQuest(30);
                    result = pQuest->wEndDialogue;
                }
                break;
            case 31: // ADDEMBLEMTOQSL
                if (pInfo->dwValue == 1)
                    result = pQuest->wEndDialogue;
                break;
        }
    }

check_oneway_delivery:
    // 檢查單向配送任務 (oneWayDelivery) 的配送目標
    if (outDeliveryNPC) {
        if (!m_pInventory->IsLock()) {
            std::uint8_t runCount = GetRunningQuestCount();
            for (std::uint8_t ri = 0; ri < runCount; ri++) {
                stPlayingQuestInfo* pRunning = GetRunningQuestInfoByIndex(ri);
                stQuestKindInfo* pQK =
                    m_pclQuestKindInfo->GetQuestKindInfo(pRunning->wQuestID);
                if (pQK && pQK->bPlayType == 6
                    && pQK->extra.oneWayDelivery.wReceiverNpcId == static_cast<std::uint16_t>(npcID))
                {
                    if (m_pInventory->GetInventoryItemQuantity(pQK->extra.oneWayDelivery.wItemId)) {
                        *outDeliveryNPC = pQK->extra.oneWayDelivery.wAssignedNpcId;
                        rewardNPC = pQK->extra.oneWayDelivery.wAssignedNpcId;
                        m_wRewardNPCID = pQK->extra.oneWayDelivery.wAssignedNpcId;
                        m_pRewardQuestInfo = pRunning;
                        m_wRewardItemKind = pQK->extra.oneWayDelivery.wItemId;
                        result = pQK->extra.oneWayDelivery.wReceiverNpcDialogue;
                        break;
                    }
                }
            }
        }
    }

    if (result) {
        if (CanAddRewarItem(rewardNPC) == 605) {
            return 605;
        }
        if (!CanIncreaseRewardMoney(rewardNPC))
            result = 106;
    }
    return result;
}

// ---- RewardQuest ----

void cltQuestSystem::RewardQuest(std::uint16_t npcID, unsigned int* outExp)
{
    std::uint16_t delNPC = 0;
    int money = 0;
    unsigned int sword = 0, bow = 0, magic = 0, theology = 0;
    std::int16_t major = 0;
    std::uint16_t merit = 0;
    int rank = 0;

    cltItemList delList;
    cltItemList addList;

    RewardQuest(npcID, &delNPC, &money, outExp, &sword, &bow, &magic, &theology,
                &major, &merit, &rank, &delList, &addList, nullptr, nullptr);
}

void cltQuestSystem::RewardQuest(
    std::uint16_t npcID,
    std::uint16_t* outDelNPC,
    int* outMoney,
    unsigned int* outExp,
    unsigned int* outSword,
    unsigned int* outBow,
    unsigned int* outMagic,
    unsigned int* outTheology,
    std::int16_t* outMajor,
    std::uint16_t* outMerit,
    int* outRank,
    cltItemList* delItemList,
    cltItemList* addItemList,
    std::uint8_t* outDelSlots,
    std::uint8_t* outAddSlots)
{
    *outDelNPC = 0;
    stPlayingQuestInfo* pInfo = nullptr;

    if (m_wRewardNPCID) {
        *outDelNPC = m_wRewardNPCID;
        pInfo = GetPlayingQuestInfoByNPCID(m_wRewardNPCID);
    } else {
        pInfo = GetPlayingQuestInfoByNPCID(npcID);
    }
    if (!pInfo)
        return;

    stQuestKindInfo* pQuest =
        m_pclQuestKindInfo->GetQuestKindInfo(pInfo->wQuestID);
    if (!pQuest)
        return;

    // 依任務類型處理扣除道具/給予特殊獎勵
    switch (pQuest->bPlayType) {
        case 1: { // COLLECT
            std::uint16_t invQty1 = static_cast<std::uint16_t>(
                m_pInventory->GetInventoryItemQuantity(pQuest->extra.collection.wQuestItem1));
            std::uint16_t invQty2 = static_cast<std::uint16_t>(
                m_pInventory->GetInventoryItemQuantity(pQuest->extra.collection.wQuestItem2));
            // 寵物也有物品
            m_pPetInventorySystem->GetInventoryItemQuantity(pQuest->extra.collection.wQuestItem1);
            m_pPetInventorySystem->GetInventoryItemQuantity(pQuest->extra.collection.wQuestItem2);

            std::uint16_t need1 = pQuest->extra.collection.wQuestItemCount1;
            // 收集品 1
            if (need1 <= invQty1) {
                m_pInventory->DelInventoryItemKind(
                    pQuest->extra.collection.wQuestItem1, need1, delItemList, outDelSlots);
            } else {
                if (!m_pPetInventorySystem->CanDelItemByItemKind(
                        pQuest->extra.collection.wQuestItem1, need1 - invQty1))
                    return;
                m_pPetInventorySystem->DelItemByItemKind(
                    pQuest->extra.collection.wQuestItem1, need1 - invQty1, outAddSlots);
                m_pInventory->DelInventoryItemKind(
                    pQuest->extra.collection.wQuestItem1, invQty1, delItemList, outDelSlots);
            }

            // 收集品 2
            std::uint16_t need2 = pQuest->extra.collection.wQuestItemCount2;
            if (need2 <= invQty2) {
                m_pInventory->DelInventoryItemKind(
                    pQuest->extra.collection.wQuestItem2, need2, delItemList, outDelSlots);
            } else {
                if (!m_pPetInventorySystem->CanDelItemByItemKind(
                        pQuest->extra.collection.wQuestItem2, need2 - invQty2))
                    return;
                m_pPetInventorySystem->DelItemByItemKind(
                    pQuest->extra.collection.wQuestItem2, need2 - invQty2, outAddSlots);
                m_pInventory->DelInventoryItemKind(
                    pQuest->extra.collection.wQuestItem2, invQty2, delItemList, outDelSlots);
            }
            break;
        }
        case 6: // ONEWAYDELIVERY
            m_pInventory->DelInventoryItemKind(
                pQuest->extra.oneWayDelivery.wItemId, 1, delItemList, outDelSlots);
            break;
        case 8: // PLAYINDUN
            m_pInventory->DelInventoryItemKind(
                pQuest->extra.playIndun.wDungeonItemCode, 1, delItemList, outDelSlots);
            break;
        case 15: // EXAMINATION
        case 16: // SPECIALTYLEARN
            *outMajor = pQuest->wRewardMajorPoints;
            m_pSpecialtySystem->IncreaseSpecialtyPt(pQuest->wRewardMajorPoints);
            break;
        case 24: // MERITORIOUS
        case 25: // MATERIALSUPPLY
            *outMerit = pQuest->wRewardMeritPoints;
            m_pMeritoriousSystem->IncreaseMeritoriousPoint(pQuest->wRewardMeritPoints);
            break;
        case 29: // PVP
            *outRank = pQuest->dwRewardRankPoints;
            m_pPKRankSystem->IncreasePoint(pQuest->dwRewardRankPoints, nullptr);
            break;
    }

    // 獎勵道具 Group 1
    if (GetRewardItem(npcID, 1)) {
        std::uint8_t cnt1 = pQuest->bGroup1ItemCount;
        if (cnt1) {
            std::int16_t itemId = GetRewardItem(npcID, 1);
            addItemList->AddItem(itemId, cnt1, 0, 0, 0xFFFF, nullptr);
        }
    }
    // 獎勵道具 Group 2
    if (GetRewardItem(npcID, 2)) {
        std::uint8_t cnt2 = pQuest->bGroup2ItemCount;
        if (cnt2) {
            std::int16_t itemId = GetRewardItem(npcID, 2);
            addItemList->AddItem(itemId, cnt2, 0, 0, 0xFFFF, nullptr);
        }
    }

    m_pInventory->AddInventoryItem(addItemList, outDelSlots);

    // 金錢
    *outMoney = pQuest->dwRewardLib;
    m_pMoneySystem->IncreaseMoney(pQuest->dwRewardLib);

    // 經驗
    *outExp = pQuest->dwRewardExperience;

    // 課程點數
    *outSword = pQuest->dwRewardClassPoints_Sword;
    m_pLessonSystem->IncLessonPt_Sword(pQuest->dwRewardClassPoints_Sword);

    *outBow = pQuest->dwRewardClassPoints_Bow;
    m_pLessonSystem->IncLessonPt_Bow(pQuest->dwRewardClassPoints_Bow);

    *outMagic = pQuest->dwRewardClassPoints_Magic;
    m_pLessonSystem->IncLessonPt_Magic(pQuest->dwRewardClassPoints_Magic);

    *outTheology = pQuest->dwRewardClassPoints_Theology;
    m_pLessonSystem->IncLessonPt_Theology(pQuest->dwRewardClassPoints_Theology);

    // 事件通知
    m_pEmblemSystem->OnEvent_CompleteQuest(pInfo->wQuestID);
    m_pTitleSystem->OnEvent_clearquest(pInfo->wQuestID);

    // 標記完成
    std::uint8_t idx = GetQuestIndexByQuestID(pInfo->wQuestID);
    FinishQuest(idx);

    m_wRewardNPCID = 0;
    m_pRewardQuestInfo = nullptr;
    m_wRewardItemKind = 0;
}

// ---- CompleteInstansDungeonMission ----

int cltQuestSystem::CompleteInstansDungeonMission(std::uint16_t indunID)
{
    strInstantDungeonKindInfo* pIndun =
        m_pclInstantDungeonKindInfo->GetInstantDungeonKindInfo(indunID);
    std::uint8_t runCount = GetRunningQuestCount();
    if (!pIndun || !runCount)
        return 0;

    stPlayingQuestInfo* pFound = nullptr;
    int found = 0;

    for (int i = 0; i < runCount; i++) {
        stPlayingQuestInfo* pInfo = GetRunningQuestInfoByIndex(static_cast<std::uint8_t>(i));
        if (!pInfo)
            continue;
        stQuestKindInfo* pQuest =
            m_pclQuestKindInfo->GetQuestKindInfo(pInfo->wQuestID);
        if (!pQuest)
            continue;
        if (pQuest->bPlayType == 8) {
            std::uint16_t did = pQuest->extra.playIndun.wDungeonId;
            if (did == indunID || did == pIndun->wBaseIndunId) {
                pFound = pInfo;
                found = 1;
                break;
            }
        }
    }

    if (!pFound || !found)
        return 0;
    pFound->dwValue = 1;
    return 1;
}

// ---- CanAddRewarItem ----

std::uint16_t cltQuestSystem::CanAddRewarItem(std::uint16_t npcID)
{
    if (m_pInventory->IsLock() == 1)
        return 605;

    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(npcID);
    if (!pInfo)
        return 605;

    stQuestKindInfo* pQuest =
        m_pclQuestKindInfo->GetQuestKindInfo(pInfo->wQuestID);
    if (!pQuest)
        return 605;

    // 模擬扣除後是否能放入獎勵道具
    // 建立臨時背包副本（以原始記憶體複製方式）
    cltBaseInventory tempInv;
    std::memcpy(&tempInv, m_pInventory, sizeof(cltBaseInventory));

    cltItemList tempList;

    char playType = pQuest->bPlayType;
    if (playType == 1) {
        tempInv.DelInventoryItemKind(pQuest->extra.collection.wQuestItem1,
                                     pQuest->extra.collection.wQuestItemCount1, nullptr, nullptr);
        tempInv.DelInventoryItemKind(pQuest->extra.collection.wQuestItem2,
                                     pQuest->extra.collection.wQuestItemCount2, nullptr, nullptr);
    } else if (playType == 3 || playType == 6) {
        tempInv.DelInventoryItemKind(pQuest->extra.delivery.wItemId, 1, nullptr, nullptr);
    }

    // 獎勵道具 Group 1
    std::uint16_t rewardItem1 = pQuest->wReward1ItemId_Group1;
    if (rewardItem1) {
        std::uint8_t cnt1 = pQuest->bGroup1ItemCount;
        if (cnt1)
            tempList.AddItem(rewardItem1, cnt1, 0, 0, 0xFFFF, nullptr);
    }
    // 獎勵道具 Group 2
    std::uint16_t rewardItem2 = pQuest->wReward1ItemId_Group2;
    if (rewardItem2) {
        std::uint8_t cnt2 = pQuest->bGroup2ItemCount;
        if (cnt2)
            tempList.AddItem(rewardItem2, cnt2, 0, 0, 0xFFFF, nullptr);
    }

    int canAdd = tempInv.CanAddInventoryItems(&tempList);
    return canAdd != 0 ? 605 : 0;
}

// ---- CanIncreaseRewardMoney ----

bool cltQuestSystem::CanIncreaseRewardMoney(std::uint16_t npcID)
{
    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(npcID);
    if (!pInfo)
        return false;
    stQuestKindInfo* pQuest =
        m_pclQuestKindInfo->GetQuestKindInfo(pInfo->wQuestID);
    return m_pMoneySystem->CanIncreaseMoney(pQuest->dwRewardLib);
}

// ---- GetPrecedenceQuestInfo ----

int cltQuestSystem::GetPrecedenceQuestInfo(std::uint16_t npcID, std::uint16_t* outPrereqNPC, std::uint16_t* outPrereqQuest)
{
    stNPCInfo* pNPC = m_pclNPCInfo->GetNPCInfoByID(npcID);
    if (!pNPC->m_wQuestIDs[0])
        return 0;

    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(npcID);
    std::uint16_t nextQuestID = 0;

    if (pInfo) {
        if (pInfo->bStatus != 1)
            return 0;
        // 找到目前完成的任務在 NPC 任務鏈中的位置，取得下一個
        std::uint16_t curQuestID = pInfo->wQuestID;
        for (int i = 0; i < 70; i++) {
            if (pNPC->m_wQuestIDs[i] == curQuestID && pNPC->m_wQuestIDs[i + 1]) {
                nextQuestID = pNPC->m_wQuestIDs[i + 1];
                break;
            }
        }
        if (!nextQuestID)
            return 0;
    } else {
        nextQuestID = pNPC->m_wQuestIDs[0];
    }

    stQuestKindInfo* pQuest =
        m_pclQuestKindInfo->GetQuestKindInfo(nextQuestID);
    if (!pQuest)
        return 0;
    *outPrereqQuest = pQuest->wPrerequisiteQuest;
    *outPrereqNPC   = pQuest->wPrerequisiteQuestNpc;
    return 1;
}

// ---- CanAccept ----

std::uint16_t cltQuestSystem::CanAccept(std::uint16_t npcID)
{
    std::uint16_t currentNPC = npcID;

    while (true) {
        stNPCInfo* pNPC = m_pclNPCInfo->GetNPCInfoByID(currentNPC);
        if (!m_pclNPCInfo) {
            char buf[256];
            wsprintfA(buf, "NPC Info Error :%i", currentNPC);
            MessageBoxA(nullptr, buf, "error", 0);
            return 600;
        }
        if (!pNPC || !pNPC->m_wQuestIDs[0])
            return 600;

        stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(currentNPC);

        if (pInfo) {
            // 有進行中的任務
            if (!pInfo->bStatus)
                return 603; // 還在進行中

            stQuestKindInfo* pQuestKind =
                m_pclQuestKindInfo->GetQuestKindInfo(pInfo->wQuestID);

            if (pInfo->bStatus == 2) {
                // 已放棄，可以重新接取
                if (GetRunningQuestCount() >= MAX_RUNNING_QUESTS)
                    return 601;

                char pt = pQuestKind->bPlayType;
                std::uint16_t checkItem = 0;
                if (pt == 3) {
                    checkItem = pQuestKind->extra.delivery.wItemId;
                } else if (pt == 6) {
                    checkItem = pQuestKind->extra.oneWayDelivery.wItemId;
                } else if (pt == 8) {
                    if (m_pInventory->CanAddInventoryItem(pQuestKind->extra.playIndun.wDungeonItemCode, 1))
                        return 604;
                    m_wPendingNPCID = currentNPC;
                    m_wPendingQuestID = pInfo->wQuestID;
                    return 605;
                } else {
                    m_wPendingNPCID = currentNPC;
                    m_wPendingQuestID = pInfo->wQuestID;
                    return 605;
                }

                if (m_pInventory->CanAddInventoryItem(checkItem, 1))
                    return 604;
                m_wPendingNPCID = currentNPC;
                m_wPendingQuestID = pInfo->wQuestID;
                return 605;
            }

            if (pInfo->bStatus == 1) {
                // 已完成，尋找任務鏈中的下一個
                int foundNext = 0;
                for (int i = 0; i < 70; i++) {
                    if (pInfo->wQuestID == pNPC->m_wQuestIDs[i] && pNPC->m_wQuestIDs[i + 1]) {
                        std::uint16_t nextQuestID = pNPC->m_wQuestIDs[i + 1];
                        stQuestKindInfo* pNextQuest =
                            m_pclQuestKindInfo->GetQuestKindInfo(nextQuestID);
                        if (!pNextQuest)
                            return 600;

                        // 等級檢查
                        if (m_pLevelSystem->GetLevel() < pNextQuest->bConditionLevel)
                            return 602;

                        // 前置任務 NPC 檢查
                        if (pNextQuest->wPrerequisiteQuestNpc) {
                            stNPCInfo* pPreNPC =
                                m_pclNPCInfo->GetNPCInfoByID(pNextQuest->wPrerequisiteQuestNpc);
                            if (!pPreNPC)
                                return 606;
                            stPlayingQuestInfo* pPreInfo =
                                GetPlayingQuestInfoByNPCID(pNextQuest->wPrerequisiteQuestNpc);
                            if (!pPreInfo)
                                return 606;

                            // 確認前置任務的進度順序
                            int preIdx = 0, curIdx = 0;
                            std::uint16_t preQuestID = pPreInfo->wQuestID;
                            for (int j = 0; j < 70; j++) {
                                if (preQuestID == pPreNPC->m_wQuestIDs[j] && pPreInfo->bStatus == 1)
                                    preIdx = j;
                                if (pNextQuest->wPrerequisiteQuest == pPreNPC->m_wQuestIDs[j])
                                    curIdx = j;
                                if (preIdx > 0 && curIdx > 0)
                                    break;
                            }
                            if (preIdx < curIdx)
                                return 606;
                        }

                        // 職業檢查
                        std::uint16_t myClass = m_pClassSystem->GetClass();
                        if (!pNextQuest->CanAcceptQuestByClass(myClass)) {
                            FinishQuest(static_cast<std::uint16_t>(i + 1), nextQuestID, npcID);
                            foundNext = 0;
                            break; // 重新進入迴圈
                        }

                        if (GetRunningQuestCount() >= MAX_RUNNING_QUESTS)
                            return 601;

                        m_wPendingNPCID = npcID;
                        m_wPendingQuestID = nextQuestID;

                        // 需要起始道具的任務類型
                        char npt = pNextQuest->bPlayType;
                        std::uint16_t needItem = 0;
                        if (npt == 3) {
                            needItem = pNextQuest->extra.delivery.wItemId;
                        } else if (npt == 6) {
                            needItem = pNextQuest->extra.oneWayDelivery.wItemId;
                        } else if (npt == 8) {
                            needItem = pNextQuest->extra.playIndun.wDungeonItemCode;
                        } else {
                            return 605;
                        }
                        if (!m_pInventory->CanAddInventoryItem(needItem, 1))
                            return 605;
                        return 604;
                    }
                }
                // 沒找到下一個任務
                if (!foundNext)
                    return 600;
            }
            return 600;
        }

        // 沒有進行中的任務 → 嘗試接取首個任務
        stQuestKindInfo* pFirstQuest =
            m_pclQuestKindInfo->GetQuestKindInfo(pNPC->m_wQuestIDs[0]);
        if (!pFirstQuest)
            return 600;

        std::uint16_t myClass = m_pClassSystem->GetClass();
        if (!pFirstQuest->CanAcceptQuestByClass(myClass)) {
            FinishQuest(0, pNPC->m_wQuestIDs[0], currentNPC);
            // 迴圈重新檢查
            pNPC = m_pclNPCInfo->GetNPCInfoByID(currentNPC);
            if (!m_pclNPCInfo)
                goto error_label;
            continue;
        }

        // 等級檢查
        if (m_pLevelSystem->GetLevel() < pFirstQuest->bConditionLevel)
            return 602;

        // 前置任務 NPC 檢查
        if (pFirstQuest->wPrerequisiteQuestNpc) {
            stNPCInfo* pPreNPC =
                m_pclNPCInfo->GetNPCInfoByID(pFirstQuest->wPrerequisiteQuestNpc);
            stPlayingQuestInfo* pPreInfo =
                GetPlayingQuestInfoByNPCID(pFirstQuest->wPrerequisiteQuestNpc);
            if (!pPreInfo)
                return 606;

            int preIdx = 0, curIdx = 0;
            for (int j = 0; j < 70; j++) {
                if (pPreInfo->wQuestID == pPreNPC->m_wQuestIDs[j] && pPreInfo->bStatus == 1)
                    preIdx = j;
                if (pFirstQuest->wPrerequisiteQuest == pPreNPC->m_wQuestIDs[j])
                    curIdx = j;
                if (preIdx > 0 && curIdx > 0)
                    break;
            }
            if (preIdx < curIdx)
                return 606;
        }

        if (GetRunningQuestCount() >= MAX_RUNNING_QUESTS)
            return 601;

        char pt = pFirstQuest->bPlayType;
        std::uint16_t needItem = 0;
        if (pt == 3) {
            needItem = pFirstQuest->extra.delivery.wItemId;
        } else if (pt == 6) {
            needItem = pFirstQuest->extra.oneWayDelivery.wItemId;
        } else if (pt == 8) {
            if (m_pInventory->CanAddInventoryItem(pFirstQuest->extra.playIndun.wDungeonItemCode, 1))
                return 604;
            m_wPendingNPCID = currentNPC;
            m_wPendingQuestID = pNPC->m_wQuestIDs[0];
            return 605;
        } else {
            m_wPendingNPCID = currentNPC;
            m_wPendingQuestID = pNPC->m_wQuestIDs[0];
            return 605;
        }
        if (m_pInventory->CanAddInventoryItem(needItem, 1))
            return 604;
        m_wPendingNPCID = currentNPC;
        m_wPendingQuestID = pNPC->m_wQuestIDs[0];
        return 605;
    }

error_label:
    {
        char buf[256];
        wsprintfA(buf, "NPC Info Error :%i", currentNPC);
        MessageBoxA(nullptr, buf, "error", 0);
        return 600;
    }
}

// ---- GetNewQuestID ----

std::uint16_t cltQuestSystem::GetNewQuestID(std::uint16_t npcID)
{
    stNPCInfo* pNPC = m_pclNPCInfo->GetNPCInfoByID(npcID);
    if (!m_pclNPCInfo || !pNPC || !pNPC->m_wQuestIDs[0])
        return 0;

    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(npcID);
    if (!pInfo) {
        return pNPC->m_wQuestIDs[0] ? pNPC->m_wQuestIDs[0] : 0;
    }

    if (!pInfo->bStatus)
        return 0;
    if (pInfo->bStatus == 2)
        return pInfo->wQuestID;
    if (pInfo->bStatus != 1)
        return 0;

    // 尋找下一個任務
    std::uint16_t curQuest = pInfo->wQuestID;
    for (int i = 0; i < 70; i++) {
        if (pNPC->m_wQuestIDs[i] == curQuest && pNPC->m_wQuestIDs[i + 1]) {
            return pNPC->m_wQuestIDs[i + 1];
        }
    }
    return 0;
}

// ---- CanShowQuestAcceptMark ----

int cltQuestSystem::CanShowQuestAcceptMark(std::uint16_t npcID)
{
    std::uint16_t v = CanAccept(npcID);
    if (v == 605) return 1;
    if (v == 604) return 1;
    if (v == 601) return 1;
    return 0;
}

// ---- GetNPCIDByLevel ----

char cltQuestSystem::GetNPCIDByLevel(std::uint8_t level, std::uint16_t* outNPCIDs, std::uint16_t* outQuestIDs)
{
    std::memset(outNPCIDs, 0, 0xC8); // 100 entries * 2 bytes
    char count = 0;
    if (outQuestIDs)
        std::memset(outQuestIDs, 0, 0xC8);

    int totalNPC = m_pclNPCInfo->GetTotalNPCNum();
    if (!totalNPC)
        return 0;

    for (int i = 0; i < totalNPC; i++) {
        stNPCInfo* pNPC = m_pclNPCInfo->GetNPCInfoByIndex(i);
        if (!pNPC)
            continue;
        if (pNPC->_field_192)
            continue;
        if (CanShowQuestAcceptMark(pNPC->m_wKind)) {
            outNPCIDs[static_cast<std::uint8_t>(count)] = pNPC->m_wKind;
            count++;
        }
    }
    return count;
}

// ---- OnMeetNPC ----

std::uint16_t cltQuestSystem::OnMeetNPC(std::uint16_t npcID)
{
    std::uint16_t dummy = 0;
    return OnMeetNPC(npcID, &dummy);
}

std::uint16_t cltQuestSystem::OnMeetNPC(std::uint16_t npcID, std::uint16_t* outNPCID)
{
    m_wDeliveryItemKind = 0;

    std::uint8_t runCount = GetRunningQuestCount();
    if (!runCount)
        return 0;

    for (std::uint8_t i = 0; i < runCount; i++) {
        stPlayingQuestInfo* pInfo = GetRunningQuestInfoByIndex(i);
        stQuestKindInfo* pQuest =
            m_pclQuestKindInfo->GetQuestKindInfo(pInfo->wQuestID);
        if (!pQuest)
            continue;

        if (pQuest->bPlayType == 3
            && pQuest->extra.delivery.wReceiveNpcId == npcID
            && pInfo->dwValue != 1)
        {
            if (m_pInventory->GetInventoryItemQuantity(pQuest->extra.delivery.wItemId)) {
                *outNPCID = pInfo->wNPCID;
                m_pDeliveryQuestInfo = pInfo;
                m_wDeliveryItemKind = pQuest->extra.delivery.wItemId;
                return pQuest->extra.delivery.wReceiveNpcDialogue;
            }
        }
    }
    return 0;
}

// ---- DelDeliveryItem ----

void cltQuestSystem::DelDeliveryItem()
{
    cltItemList tempList;
    DelDeliveryItem(&tempList, nullptr);
}

void cltQuestSystem::DelDeliveryItem(cltItemList* itemList, std::uint8_t* outSlots)
{
    m_pDeliveryQuestInfo->dwValue = 1;
    m_pInventory->DelInventoryItemKind(m_wDeliveryItemKind, 1, itemList, outSlots);
    m_pDeliveryQuestInfo = nullptr;
    m_wDeliveryItemKind = 0;
}

// ---- 放棄任務 ----

unsigned int cltQuestSystem::CanGiveupQuest(std::uint16_t npcID)
{
    if (m_pInventory->IsLock() == 1)
        return 607;

    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(npcID);
    if (!pInfo)
        return 608;
    if (pInfo->bStatus)
        return 609;

    stQuestKindInfo* pQuest =
        m_pclQuestKindInfo->GetQuestKindInfo(pInfo->wQuestID);
    if (!pQuest)
        return 610;

    char pt = pQuest->bPlayType;
    if (pt == 5 || pt == 4)
        return 611;

    if (pt == 3) {
        if (!m_pInventory->GetInventoryItemQuantity(pQuest->extra.delivery.wItemId))
            return 612;
    }
    return 0;
}

void cltQuestSystem::GiveupQuest(std::uint16_t npcID)
{
    cltItemList tempList;
    GiveupQuest(npcID, &tempList, nullptr);
}

void cltQuestSystem::GiveupQuest(std::uint16_t npcID, cltItemList* itemList, std::uint8_t* outSlots)
{
    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(npcID);
    if (!pInfo)
        return;

    stQuestKindInfo* pQuest =
        m_pclQuestKindInfo->GetQuestKindInfo(pInfo->wQuestID);
    pInfo->bStatus = 2;
    pInfo->dwValue = 0;

    if (pQuest->bPlayType == 3)
        m_pInventory->DelInventoryItemKind(pQuest->extra.delivery.wItemId, 1, itemList, outSlots);
    if (pQuest->bPlayType == 6)
        m_pInventory->DelInventoryItemKind(pQuest->extra.oneWayDelivery.wItemId, 1, itemList, outSlots);
    if (pQuest->bPlayType == 8)
        m_pInventory->DelInventoryItemKind(pQuest->extra.playIndun.wDungeonItemCode, 1, itemList, outSlots);

    m_pDeliveryQuestInfo = nullptr;
    m_wDeliveryItemKind = 0;
}

// ---- 永久放棄 ----

stPlayingQuestInfo* cltQuestSystem::CanEternalGiveupQuest(std::uint16_t npcID)
{
    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(npcID);
    if (!pInfo)
        return nullptr;
    if (m_pInventory->IsLock() == 1)
        return nullptr;
    stQuestKindInfo* pQuest =
        m_pclQuestKindInfo->GetQuestKindInfo(pInfo->wQuestID);
    if (!pQuest)
        return nullptr;
    return pQuest->dwPermanentAbandon ? reinterpret_cast<stPlayingQuestInfo*>(1) : nullptr;
}

int cltQuestSystem::EternalGiveUpQuest(std::uint16_t npcID)
{
    cltItemList tempList;
    return EternalGiveUpQuest(npcID, &tempList, nullptr);
}

int cltQuestSystem::EternalGiveUpQuest(std::uint16_t npcID, cltItemList* itemList, std::uint8_t* outSlots)
{
    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(npcID);
    if (!pInfo)
        return 0;

    stQuestKindInfo* pQuest =
        m_pclQuestKindInfo->GetQuestKindInfo(pInfo->wQuestID);
    if (!pQuest->dwPermanentAbandon)
        return 0;

    pInfo->bStatus = 1;
    pInfo->dwValue = 0;

    if (pQuest->bPlayType == 3)
        m_pInventory->DelInventoryItemKind(pQuest->extra.delivery.wItemId, 1, itemList, outSlots);
    if (pQuest->bPlayType == 6)
        m_pInventory->DelInventoryItemKind(pQuest->extra.oneWayDelivery.wItemId, 1, itemList, outSlots);
    if (pQuest->bPlayType == 8)
        m_pInventory->DelInventoryItemKind(pQuest->extra.playIndun.wDungeonItemCode, 1, itemList, outSlots);

    m_pDeliveryQuestInfo = nullptr;
    m_wDeliveryItemKind = 0;
    m_bGiveupPermanently = 1;
    return 1;
}

// ---- GetRewardItem ----

std::uint16_t cltQuestSystem::GetRewardItem(std::uint16_t npcID, std::uint8_t groupIndex)
{
    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(npcID);
    if (!pInfo)
        return 0;

    stQuestKindInfo* pQuest =
        m_pclQuestKindInfo->GetQuestKindInfo(pInfo->wQuestID);
    std::uint16_t myClass = m_pClassSystem->GetClass();
    strClassKindInfo* pClassInfo = m_pclClassKindInfo->GetClassKindInfo(myClass);
    if (!pClassInfo)
        return 0;

    return pQuest->GetRewardItem(pClassInfo->kind, groupIndex);
}

std::uint8_t cltQuestSystem::GetRewardItemNum(std::uint16_t /*npcID*/, std::uint8_t /*groupIndex*/)
{
    return 0;
}

// ---- IsQuestMonster ----

int cltQuestSystem::IsQuestMonster(std::uint16_t monsterKind)
{
    void* charInfo = m_pclCharKindInfo->GetCharKindInfo(monsterKind);

    std::uint16_t tribeCode = *reinterpret_cast<std::uint16_t*>(
        reinterpret_cast<char*>(charInfo) + 6);

    for (int i = 0; i < MAX_QUEST_SLOTS; i++) {
        stQuestKindInfo* pQuest =
            m_pclQuestKindInfo->GetQuestKindInfo(m_quests[i].wQuestID);
        if (!pQuest)
            continue;
        if (pQuest->bPlayType == 2
            && !m_quests[i].bStatus
            && pQuest->extra.hunt.wTargetId1 == tribeCode
            && m_quests[i].dwValue < static_cast<unsigned int>(pQuest->extra.hunt.wCount))
        {
            return 1;
        }
    }
    return 0;
}

// ---- IsCompleteQuest ----

int cltQuestSystem::IsCompleteQuest(int questID)
{
    int npcIndex = 0;
    std::uint16_t npcKind = m_pclNPCInfo->GetNPCIDByQuestID(questID, &npcIndex);
    if (!npcKind) {
        MessageBoxA(nullptr, "Quest ID incorrect", "error", 0);
        return 0;
    }

    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(npcKind);
    if (!pInfo)
        return 0;

    int curIndex = 0;
    m_pclNPCInfo->GetNPCIDByQuestID(pInfo->wQuestID, &curIndex);

    if (npcIndex < curIndex)
        return 1;
    if (npcIndex > curIndex)
        return 0;

    // npcIndex == curIndex
    return (pInfo->bStatus && pInfo->bStatus == 1) ? 1 : 0;
}

// ---- IsGiveupQuestPermanently ----

int cltQuestSystem::IsGiveupQuestPermanently()
{
    return m_bGiveupPermanently;
}

// ---- CompleteFunctionQuest ----

int cltQuestSystem::CompleteFunctionQuest(std::int16_t functionType)
{
    std::uint16_t outNPC = 0;
    unsigned int outVal = 0;
    return CompleteFunctionQuest(static_cast<char>(functionType), &outNPC, &outVal);
}

int cltQuestSystem::CompleteFunctionQuest(char functionType, std::uint16_t* outNPCID, unsigned int* outValue)
{
    // 0x689C = 26780 — 功能型任務的固定 NPC ID
    stPlayingQuestInfo* pInfo = GetPlayingQuestInfoByNPCID(0x689C);
    if (!pInfo)
        return 0;
    if (pInfo->bStatus == 1)
        return 0;
    if (m_pclQuestKindInfo->GetQuestPlayType(pInfo->wQuestID) != functionType)
        return 0;

    pInfo->dwValue = 1;

    if (outNPCID)
        *outNPCID = 0x689C;
    if (outValue)
        *outValue = 1;

    if (m_pQuestMarkRefreshFuncPtr)
        m_pQuestMarkRefreshFuncPtr(0x689C);

    return 1;
}
