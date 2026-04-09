#include "System/cltClassSystem.h"

#include <cstdint>

#include "Info/cltClassKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "System/cltLevelSystem.h"
#include "System/cltLessonSystem.h"
#include "System/cltSkillSystem.h"
#include "System/cltUsingSkillSystem.h"
#include "System/cltWorkingPassiveSkillSystem.h"
#include "Logic/cltBaseInventory.h"
#include "System/cltEquipmentSystem.h"
#include "System/cltQuickSlotSystem.h"
#include "Logic/cltItemList.h"

cltClassKindInfo* cltClassSystem::m_pstClassKindInfo = nullptr;
cltItemKindInfo* cltClassSystem::m_pclItemKindInfo = nullptr;

void cltClassSystem::InitializeStaticVariable(cltClassKindInfo* classKindInfo, cltItemKindInfo* itemKindInfo) {
    m_pstClassKindInfo = classKindInfo;
    m_pclItemKindInfo = itemKindInfo;
}

cltClassSystem::cltClassSystem() = default;

void cltClassSystem::Initialize(
    cltLevelSystem* levelSystem,
    cltPlayerAbility* playerAbility,
    cltLessonSystem* lessonSystem,
    cltBaseInventory* baseInventory,
    cltEquipmentSystem* equipmentSystem,
    cltQuickSlotSystem* quickSlotSystem,
    cltUsingSkillSystem* usingSkillSystem,
    cltWorkingPassiveSkillSystem* workingPassiveSkillSystem,
    cltSkillSystem* skillSystem,
    cltEmblemSystem* emblemSystem,
    std::uint16_t classCode) {
    SetClass(classCode);
    m_pLevelSystem = levelSystem;
    m_pPlayerAbility = playerAbility;
    m_pLessonSystem = lessonSystem;
    m_pBaseInventory = baseInventory;
    m_pEquipmentSystem = equipmentSystem;
    m_pQuickSlotSystem = quickSlotSystem;
    m_pUsingSkillSystem = usingSkillSystem;
    m_pWorkingPassiveSkillSystem = workingPassiveSkillSystem;
    m_pSkillSystem = skillSystem;
    m_pEmblemSystem = emblemSystem;
}

void cltClassSystem::Free() {
    SetClass(0);
    m_pLevelSystem = nullptr;
    m_pPlayerAbility = nullptr;
    m_pLessonSystem = nullptr;
    m_pBaseInventory = nullptr;
    m_pEquipmentSystem = nullptr;
    m_pQuickSlotSystem = nullptr;
    m_pUsingSkillSystem = nullptr;
    m_pWorkingPassiveSkillSystem = nullptr;
    m_pSkillSystem = nullptr;
    m_pEmblemSystem = nullptr;
}

void cltClassSystem::SetClass(std::uint16_t classCode) {
    m_wClassCode = classCode;
    if (classCode) {
        m_pClassKindInfo = m_pstClassKindInfo->GetClassKindInfo(classCode);
    } else {
        m_pClassKindInfo = nullptr;
    }
}

std::uint16_t cltClassSystem::GetClass() {
    return m_wClassCode;
}

unsigned int cltClassSystem::CanUpgradeClass(
    std::uint16_t targetClassCode,
    int extraItemCount,
    std::uint16_t* extraItemCodes,
    std::uint16_t* extraItemCounts) {

    cltItemList requiredItems;

    strClassKindInfo* classInfo = m_pstClassKindInfo->GetClassKindInfo(targetClassCode);
    if (!classInfo) {
        return 6;
    }
    if (classInfo->bMinLevel > m_pLevelSystem->GetLevel()) {
        return 602;
    }
    if (classInfo->wTransferableClasses != GetClass()) {
        return 6;
    }
    if (classInfo->wMinAttack > m_pPlayerAbility->GetBaseStr()) {
        return 7;
    }
    if (classInfo->wMinAgility > m_pPlayerAbility->GetBaseDex()) {
        return 8;
    }
    if (classInfo->wMinIntelligence > m_pPlayerAbility->GetBaseInt()) {
        return 9;
    }
    if (classInfo->wMinHealth > m_pPlayerAbility->GetBaseVit()) {
        return 10;
    }
    if (classInfo->wSwordsmanshipSkill > m_pLessonSystem->GetSwordLessonPt()) {
        return 11;
    }
    if (classInfo->wMagicSkill > m_pLessonSystem->GetBowLessonPt()) {
        return 12;
    }
    if (classInfo->wPriestSkill > m_pLessonSystem->GetTheologyLessonPt()) {
        return 13;
    }
    if (classInfo->wArcherySkill > m_pLessonSystem->GetMagicLessonPt()) {
        return 14;
    }

    if (classInfo->wItem1CodeOnTransfer) {
        requiredItems.AddItem(classInfo->wItem1CodeOnTransfer, classInfo->wItem1QuantityOnTransfer, 0, 0, 0xFFFFu, nullptr);
    }
    if (classInfo->wItem2CodeOnTransfer) {
        requiredItems.AddItem(classInfo->wItem2CodeOnTransfer, classInfo->wItem2QuantityOnTransfer, 0, 0, 0xFFFFu, nullptr);
    }
    for (int i = 0; i < extraItemCount; ++i) {
        requiredItems.AddItem(extraItemCodes[i], extraItemCounts[i], 0, 0, 0xFFFFu, nullptr);
    }

    if (m_pBaseInventory->CanAddInventoryItems(&requiredItems)) {
        return 103;
    }
    return 0;
}

void cltClassSystem::UpgradeClass(
    std::uint16_t targetClassCode,
    int sealedTimeValue,
    int extraItemCount,
    std::uint16_t* extraItemCodes,
    std::uint16_t* extraItemCounts,
    std::uint8_t* inventoryChangeFlags,
    cltItemList* externalItemList) {

    cltItemList localItemList;

    SetClass(targetClassCode);
    strClassKindInfo* classInfo = m_pstClassKindInfo->GetClassKindInfo(targetClassCode);
    if (classInfo) {
        cltItemList* targetList = externalItemList ? externalItemList : &localItemList;

        if (classInfo->wItem1CodeOnTransfer) {
            targetList->AddItem(classInfo->wItem1CodeOnTransfer, classInfo->wItem1QuantityOnTransfer, 0, 0, 0xFFFFu, nullptr);
        }
        if (classInfo->wItem2CodeOnTransfer) {
            targetList->AddItem(classInfo->wItem2CodeOnTransfer, classInfo->wItem2QuantityOnTransfer, 0, 0, 0xFFFFu, nullptr);
        }

        for (int i = 0; i < extraItemCount; ++i) {
            int timeValue = 0;
            stItemKindInfo* itemInfo = m_pclItemKindInfo->GetItemKindInfo(extraItemCodes[i]);
            if (itemInfo && itemInfo->m_wUseTerm != 0) {
                timeValue = sealedTimeValue;
            }
            targetList->AddItem(extraItemCodes[i], extraItemCounts[i], timeValue, 0, 0xFFFFu, nullptr);
        }

        m_pBaseInventory->AddInventoryItem(targetList, inventoryChangeFlags);
        m_pEmblemSystem->OnEvent_ChangeClass();
    }
}

BOOL cltClassSystem::CanResetClass(std::uint16_t targetClassCode) {
    if (m_pUsingSkillSystem->GetUsingSkillNum()) {
        return 0;
    }
    if (m_pWorkingPassiveSkillSystem->GetWorkingSkillNum()) {
        return 0;
    }
    if (m_pEquipmentSystem->IsEquipedBattleItem() == 1) {
        return 0;
    }
    if (GetClass() == targetClassCode) {
        return 0;
    }

    return targetClassCode == cltClassKindInfo::TranslateKindCode(const_cast<char*>("FIG"))
        || targetClassCode == cltClassKindInfo::TranslateKindCode(const_cast<char*>("ARC"))
        || targetClassCode == cltClassKindInfo::TranslateKindCode(const_cast<char*>("MAG"))
        || targetClassCode == cltClassKindInfo::TranslateKindCode(const_cast<char*>("CLA"));
}

void cltClassSystem::ResetClass(std::uint16_t targetClassCode) {
    SetClass(targetClassCode);
    m_pQuickSlotSystem->OnClassReseted();
    m_pSkillSystem->UpdateValidity();
}

std::uint16_t cltClassSystem::GetUpgradeableClasses(std::uint16_t* outClassCodes, std::uint16_t maxCount) {
    std::uint16_t foundCount = 0;
    unsigned int index = 0;
    if (!m_pstClassKindInfo->GetTotalClassNum()) {
        return 0;
    }

    do {
        strClassKindInfo* classInfo = m_pstClassKindInfo->GetClassKindInfoByIndex(index);
        if (classInfo && classInfo->wTransferableClasses == m_wClassCode && maxCount > foundCount) {
            outClassCodes[foundCount++] = classInfo->wClassId;
        }
        ++index;
    } while (index < m_pstClassKindInfo->GetTotalClassNum());

    return foundCount;
}

std::uint16_t cltClassSystem::GetMaxHPConstant() {
    const int classLevel = m_pClassKindInfo->bTransferStage;
    if (classLevel == 1) return 4;
    if (classLevel == 2) return 5;
    return 3;
}

std::uint16_t cltClassSystem::GetMaxManaConstant() {
    const int classLevel = m_pClassKindInfo->bTransferStage;
    if (classLevel == 1) return 5;
    if (classLevel == 2) return 6;
    return 4;
}

std::uint16_t cltClassSystem::GetAPowerConstant() {
    const int classLevel = m_pClassKindInfo->bTransferStage;
    if (classLevel == 1) return 150;
    if (classLevel == 2) return 200;
    return 100;
}

std::uint16_t cltClassSystem::GetDPowerConstant() {
    const int classLevel = m_pClassKindInfo->bTransferStage;
    if (classLevel == 1) return 120;
    if (classLevel == 2) return 150;
    return 100;
}

BOOL cltClassSystem::IsFighterClass() {
    strClassKindInfo* rootClass = m_pClassKindInfo;
    for (std::uint16_t parentCode = rootClass->wTransferableClasses; parentCode; parentCode = rootClass->wTransferableClasses) {
        rootClass = m_pstClassKindInfo->GetClassKindInfo(parentCode);
    }
    return rootClass->wClassId == cltClassKindInfo::TranslateKindCode(const_cast<char*>("FIG"));
}

BOOL cltClassSystem::IsArcherClass() {
    strClassKindInfo* rootClass = m_pClassKindInfo;
    for (std::uint16_t parentCode = rootClass->wTransferableClasses; parentCode; parentCode = rootClass->wTransferableClasses) {
        rootClass = m_pstClassKindInfo->GetClassKindInfo(parentCode);
    }
    return rootClass->wClassId == cltClassKindInfo::TranslateKindCode(const_cast<char*>("ARC"));
}

BOOL cltClassSystem::IsMageClass() {
    strClassKindInfo* rootClass = m_pClassKindInfo;
    for (std::uint16_t parentCode = rootClass->wTransferableClasses; parentCode; parentCode = rootClass->wTransferableClasses) {
        rootClass = m_pstClassKindInfo->GetClassKindInfo(parentCode);
    }
    return rootClass->wClassId == cltClassKindInfo::TranslateKindCode(const_cast<char*>("MAG"));
}

BOOL cltClassSystem::IsClericClass() {
    strClassKindInfo* rootClass = m_pClassKindInfo;
    for (std::uint16_t parentCode = rootClass->wTransferableClasses; parentCode; parentCode = rootClass->wTransferableClasses) {
        rootClass = m_pstClassKindInfo->GetClassKindInfo(parentCode);
    }
    return rootClass->wClassId == cltClassKindInfo::TranslateKindCode(const_cast<char*>("CLA"));
}

std::uint8_t cltClassSystem::GetClassLevel() {
    return m_pClassKindInfo->bTransferStage;
}

int cltClassSystem::GetDefaultBuffNum() {
    return static_cast<int>(m_pClassKindInfo->dwBaseBuffUsageCount);
}
