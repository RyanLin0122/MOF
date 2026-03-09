#include "cltClassSystem.h"

#include <cstdint>

class cltClassKindInfo {
public:
    static strClassKindInfo* GetClassKindInfo(cltClassKindInfo* self, std::uint16_t classCode);
    static unsigned short GetTotalClassNum(cltClassKindInfo* self);
    static strClassKindInfo* GetClassKindInfoByIndex(cltClassKindInfo* self, unsigned int index);
    static std::uint16_t TranslateKindCode(char* code);
};

class cltItemKindInfo {
public:
    static void* GetItemKindInfo(cltItemKindInfo* self, std::uint16_t itemCode);
};

class cltLevelSystem { public: static std::uint8_t GetLevel(cltLevelSystem* self); };
class cltPlayerAbility {
public:
    static std::uint16_t GetBaseStr(cltPlayerAbility* self);
    static std::uint16_t GetBaseDex(cltPlayerAbility* self);
    static std::uint16_t GetBaseInt(cltPlayerAbility* self);
    static std::uint16_t GetBaseVit(cltPlayerAbility* self);
};
class cltLessonSystem {
public:
    static std::uint16_t GetSwordLessonPt(cltLessonSystem* self);
    static std::uint16_t GetBowLessonPt(cltLessonSystem* self);
    static std::uint16_t GetTheologyLessonPt(cltLessonSystem* self);
    static std::uint16_t GetMagicLessonPt(cltLessonSystem* self);
};
class cltBaseInventory {
public:
    static int CanAddInventoryItems(cltBaseInventory* self, cltItemList* list);
    static void AddInventoryItem(cltBaseInventory* self, cltItemList* list, std::uint8_t* outFlags);
};
class cltEquipmentSystem { public: static int IsEquipedBattleItem(cltEquipmentSystem* self); };
class cltQuickSlotSystem { public: static void OnClassReseted(cltQuickSlotSystem* self); };
class cltUsingSkillSystem { public: static int GetUsingSkillNum(cltUsingSkillSystem* self); };
class cltWorkingPassiveSkillSystem { public: static int GetWorkingSkillNum(cltWorkingPassiveSkillSystem* self); };
class cltSkillSystem { public: static void UpdateValidity(cltSkillSystem* self); };
class cltEmblemSystem { public: static void OnEvent_ChangeClass(cltEmblemSystem* self); };
class cltItemList {
public:
    static void AddItem(int self, std::uint16_t itemCode, std::uint16_t count, int a4, int a5, std::uint16_t a6, int a7);
};

void cltItemList_ctor(cltItemList* self);
void cltItemList_dtor(cltItemList* self);

cltClassKindInfo* cltClassSystem::m_pstClassKindInfo = nullptr;
cltItemKindInfo* cltClassSystem::m_pclItemKindInfo = nullptr;

namespace {
constexpr std::size_t kItemListObjectSize = 10012;
constexpr const char* kFighterCode = "FIG"; // decompile token was `string`
}

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
        m_pClassKindInfo = cltClassKindInfo::GetClassKindInfo(m_pstClassKindInfo, classCode);
    } else {
        m_pClassKindInfo = nullptr;
    }
}

std::uint16_t cltClassSystem::GetClass() { return m_wClassCode; }

unsigned int cltClassSystem::CanUpgradeClass(
    std::uint16_t classCode,
    int extraItemCount,
    std::uint16_t* extraItemCodes,
    std::uint16_t* extraItemCounts) {
    std::uint8_t temp[kItemListObjectSize] = {};
    auto* itemList = reinterpret_cast<cltItemList*>(temp);
    cltItemList_ctor(itemList);

    strClassKindInfo* kind = cltClassKindInfo::GetClassKindInfo(m_pstClassKindInfo, classCode);
    if (!kind || kind->byNeedLevel > cltLevelSystem::GetLevel(m_pLevelSystem) || kind->wNeedClassCode != GetClass()) {
        cltItemList_dtor(itemList);
        return 6;
    }
    if (kind->wNeedStr > cltPlayerAbility::GetBaseStr(m_pPlayerAbility)) {
        cltItemList_dtor(itemList);
        return 7;
    }
    if (kind->wNeedDex > cltPlayerAbility::GetBaseDex(m_pPlayerAbility)) {
        cltItemList_dtor(itemList);
        return 8;
    }
    if (kind->wNeedInt > cltPlayerAbility::GetBaseInt(m_pPlayerAbility)) {
        cltItemList_dtor(itemList);
        return 9;
    }
    if (kind->wNeedVit > cltPlayerAbility::GetBaseVit(m_pPlayerAbility)) {
        cltItemList_dtor(itemList);
        return 10;
    }
    if (kind->wNeedSwordLesson > cltLessonSystem::GetSwordLessonPt(m_pLessonSystem)) {
        cltItemList_dtor(itemList);
        return 11;
    }
    if (kind->wNeedBowLesson > cltLessonSystem::GetBowLessonPt(m_pLessonSystem)) {
        cltItemList_dtor(itemList);
        return 12;
    }
    if (kind->wNeedTheologyLesson > cltLessonSystem::GetTheologyLessonPt(m_pLessonSystem)) {
        cltItemList_dtor(itemList);
        return 13;
    }
    if (kind->wNeedMagicLesson > cltLessonSystem::GetMagicLessonPt(m_pLessonSystem)) {
        cltItemList_dtor(itemList);
        return 14;
    }

    if (kind->wNeedItemCode1) {
        cltItemList::AddItem(static_cast<int>(reinterpret_cast<std::intptr_t>(itemList)), kind->wNeedItemCode1, kind->wNeedItemCount1, 0, 0, 0xFFFFu, 0);
    }
    if (kind->wNeedItemCode2) {
        cltItemList::AddItem(static_cast<int>(reinterpret_cast<std::intptr_t>(itemList)), kind->wNeedItemCode2, kind->wNeedItemCount2, 0, 0, 0xFFFFu, 0);
    }
    for (int i = 0; i < extraItemCount; ++i) {
        cltItemList::AddItem(static_cast<int>(reinterpret_cast<std::intptr_t>(itemList)), extraItemCodes[i], extraItemCounts[i], 0, 0, 0xFFFFu, 0);
    }

    const int canAdd = cltBaseInventory::CanAddInventoryItems(m_pBaseInventory, itemList);
    cltItemList_dtor(itemList);
    if (canAdd) {
        return 103;
    }
    return 0;
}

void cltClassSystem::UpgradeClass(
    std::uint16_t classCode,
    int a3,
    int extraItemCount,
    std::uint16_t* extraItemCodes,
    std::uint16_t* extraItemCounts,
    std::uint8_t* outFlags,
    cltItemList* outItemList) {
    std::uint8_t temp[kItemListObjectSize] = {};
    auto* localList = reinterpret_cast<cltItemList*>(temp);
    cltItemList_ctor(localList);

    SetClass(classCode);
    strClassKindInfo* kind = cltClassKindInfo::GetClassKindInfo(m_pstClassKindInfo, classCode);
    if (kind) {
        cltItemList* targetList = outItemList ? outItemList : localList;

        if (kind->wNeedItemCode1) {
            cltItemList::AddItem(static_cast<int>(reinterpret_cast<std::intptr_t>(targetList)), kind->wNeedItemCode1, kind->wNeedItemCount1, 0, 0, 0xFFFFu, 0);
        }
        if (kind->wNeedItemCode2) {
            cltItemList::AddItem(static_cast<int>(reinterpret_cast<std::intptr_t>(targetList)), kind->wNeedItemCode2, kind->wNeedItemCount2, 0, 0, 0xFFFFu, 0);
        }

        for (int i = 0; i < extraItemCount; ++i) {
            int opt = 0;
            if ((*reinterpret_cast<std::uint16_t*>(static_cast<std::uint8_t*>(cltItemKindInfo::GetItemKindInfo(m_pclItemKindInfo, extraItemCodes[i])) + 60)) != 0) {
                opt = a3;
            }
            cltItemList::AddItem(static_cast<int>(reinterpret_cast<std::intptr_t>(targetList)), extraItemCodes[i], extraItemCounts[i], opt, 0, 0xFFFFu, 0);
        }

        cltBaseInventory::AddInventoryItem(m_pBaseInventory, targetList, outFlags);
        cltEmblemSystem::OnEvent_ChangeClass(m_pEmblemSystem);
    }

    cltItemList_dtor(localList);
}

BOOL cltClassSystem::CanResetClass(std::uint16_t classCode) {
    if (cltUsingSkillSystem::GetUsingSkillNum(m_pUsingSkillSystem)) {
        return 0;
    }
    if (cltWorkingPassiveSkillSystem::GetWorkingSkillNum(m_pWorkingPassiveSkillSystem)) {
        return 0;
    }
    if (cltEquipmentSystem::IsEquipedBattleItem(m_pEquipmentSystem) == 1) {
        return 0;
    }
    if (GetClass() == classCode) {
        return 0;
    }

    return classCode == cltClassKindInfo::TranslateKindCode(const_cast<char*>(kFighterCode))
        || classCode == cltClassKindInfo::TranslateKindCode(const_cast<char*>("ARC"))
        || classCode == cltClassKindInfo::TranslateKindCode(const_cast<char*>("MAG"))
        || classCode == cltClassKindInfo::TranslateKindCode(const_cast<char*>("CLA"));
}

void cltClassSystem::ResetClass(std::uint16_t classCode) {
    SetClass(classCode);
    cltQuickSlotSystem::OnClassReseted(m_pQuickSlotSystem);
    cltSkillSystem::UpdateValidity(m_pSkillSystem);
}

std::uint16_t cltClassSystem::GetUpgradeableClasses(std::uint16_t* outClassCodes, std::uint16_t maxCount) {
    std::uint16_t count = 0;
    unsigned int i = 0;
    if (!cltClassKindInfo::GetTotalClassNum(m_pstClassKindInfo)) {
        return 0;
    }

    do {
        auto* info = reinterpret_cast<std::uint16_t*>(cltClassKindInfo::GetClassKindInfoByIndex(m_pstClassKindInfo, i));
        if (info[10] == m_wClassCode && maxCount > count) {
            outClassCodes[count++] = *info;
        }
        ++i;
    } while (i < cltClassKindInfo::GetTotalClassNum(m_pstClassKindInfo));

    return count;
}

std::uint16_t cltClassSystem::GetMaxHPConstant() {
    const int level = m_pClassKindInfo->byClassLevel;
    if (level == 1) return 4;
    if (level == 2) return 5;
    return 3;
}

std::uint16_t cltClassSystem::GetMaxManaConstant() {
    const int level = m_pClassKindInfo->byClassLevel;
    if (level == 1) return 5;
    if (level == 2) return 6;
    return 4;
}

std::uint16_t cltClassSystem::GetAPowerConstant() {
    const int level = m_pClassKindInfo->byClassLevel;
    if (level == 1) return 150;
    if (level == 2) return 200;
    return 100;
}

std::uint16_t cltClassSystem::GetDPowerConstant() {
    const int level = m_pClassKindInfo->byClassLevel;
    if (level == 1) return 120;
    if (level == 2) return 150;
    return 100;
}

BOOL cltClassSystem::IsFighterClass() {
    strClassKindInfo* kind = m_pClassKindInfo;
    for (std::uint16_t root = kind->wNeedClassCode; root; root = kind->wNeedClassCode) {
        kind = cltClassKindInfo::GetClassKindInfo(m_pstClassKindInfo, root);
    }
    return kind->wClassCode == cltClassKindInfo::TranslateKindCode(const_cast<char*>(kFighterCode));
}

BOOL cltClassSystem::IsArcherClass() {
    strClassKindInfo* kind = m_pClassKindInfo;
    for (std::uint16_t root = kind->wNeedClassCode; root; root = kind->wNeedClassCode) {
        kind = cltClassKindInfo::GetClassKindInfo(m_pstClassKindInfo, root);
    }
    return kind->wClassCode == cltClassKindInfo::TranslateKindCode(const_cast<char*>("ARC"));
}

BOOL cltClassSystem::IsMageClass() {
    strClassKindInfo* kind = m_pClassKindInfo;
    for (std::uint16_t root = kind->wNeedClassCode; root; root = kind->wNeedClassCode) {
        kind = cltClassKindInfo::GetClassKindInfo(m_pstClassKindInfo, root);
    }
    return kind->wClassCode == cltClassKindInfo::TranslateKindCode(const_cast<char*>("MAG"));
}

BOOL cltClassSystem::IsClericClass() {
    strClassKindInfo* kind = m_pClassKindInfo;
    for (std::uint16_t root = kind->wNeedClassCode; root; root = kind->wNeedClassCode) {
        kind = cltClassKindInfo::GetClassKindInfo(m_pstClassKindInfo, root);
    }
    return kind->wClassCode == cltClassKindInfo::TranslateKindCode(const_cast<char*>("CLA"));
}

std::uint8_t cltClassSystem::GetClassLevel() { return m_pClassKindInfo->byClassLevel; }

int cltClassSystem::GetDefaultBuffNum() { return static_cast<int>(m_pClassKindInfo->dwDefaultBuffNum); }
