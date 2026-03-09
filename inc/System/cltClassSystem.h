#pragma once

#include <cstdint>

using BOOL = int;

struct strClassKindInfo {
    std::uint16_t wClassCode;                 // +0
    std::uint8_t pad02[16];                   // +2
    std::uint8_t byClassLevel;                // +18
    std::uint8_t pad13;                       // +19
    std::uint16_t wNeedClassCode;             // +20
    std::uint8_t byNeedLevel;                 // +22
    std::uint8_t pad17;                       // +23
    std::uint16_t wNeedStr;                   // +24
    std::uint16_t wNeedDex;                   // +26
    std::uint16_t wNeedVit;                   // +28
    std::uint16_t wNeedInt;                   // +30
    std::uint16_t wNeedSwordLesson;           // +32
    std::uint16_t wNeedBowLesson;             // +34
    std::uint16_t wNeedMagicLesson;           // +36
    std::uint16_t wNeedTheologyLesson;        // +38
    std::uint8_t pad28[12];                   // +40
    std::uint16_t wNeedItemCode1;             // +52
    std::uint16_t wNeedItemCount1;            // +54
    std::uint16_t wNeedItemCode2;             // +56
    std::uint16_t wNeedItemCount2;            // +58
    std::uint32_t dwDefaultBuffNum;           // +60
};

class cltClassKindInfo;
class cltItemKindInfo;
class cltLevelSystem;
class cltPlayerAbility;
class cltLessonSystem;
class cltBaseInventory;
class cltEquipmentSystem;
class cltQuickSlotSystem;
class cltUsingSkillSystem;
class cltWorkingPassiveSkillSystem;
class cltSkillSystem;
class cltEmblemSystem;
class cltItemList;

class cltClassSystem {
public:
    static void InitializeStaticVariable(cltClassKindInfo* classKindInfo, cltItemKindInfo* itemKindInfo);

    cltClassSystem();

    void Initialize(
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
        std::uint16_t classCode);

    void Free();
    void SetClass(std::uint16_t classCode);
    std::uint16_t GetClass();

    unsigned int CanUpgradeClass(
        std::uint16_t classCode,
        int extraItemCount,
        std::uint16_t* extraItemCodes,
        std::uint16_t* extraItemCounts);

    void UpgradeClass(
        std::uint16_t classCode,
        int a3,
        int extraItemCount,
        std::uint16_t* extraItemCodes,
        std::uint16_t* extraItemCounts,
        std::uint8_t* outFlags,
        cltItemList* outItemList);

    BOOL CanResetClass(std::uint16_t classCode);
    void ResetClass(std::uint16_t classCode);

    std::uint16_t GetUpgradeableClasses(std::uint16_t* outClassCodes, std::uint16_t maxCount);

    std::uint16_t GetMaxHPConstant();
    std::uint16_t GetMaxManaConstant();
    std::uint16_t GetAPowerConstant();
    std::uint16_t GetDPowerConstant();

    BOOL IsFighterClass();
    BOOL IsArcherClass();
    BOOL IsMageClass();
    BOOL IsClericClass();

    std::uint8_t GetClassLevel();
    int GetDefaultBuffNum();

    static cltClassKindInfo* m_pstClassKindInfo;
    static cltItemKindInfo* m_pclItemKindInfo;

private:
    cltPlayerAbility* m_pPlayerAbility = nullptr;
    cltLevelSystem* m_pLevelSystem = nullptr;
    cltLessonSystem* m_pLessonSystem = nullptr;
    cltBaseInventory* m_pBaseInventory = nullptr;
    cltEquipmentSystem* m_pEquipmentSystem = nullptr;
    cltQuickSlotSystem* m_pQuickSlotSystem = nullptr;
    cltUsingSkillSystem* m_pUsingSkillSystem = nullptr;
    cltWorkingPassiveSkillSystem* m_pWorkingPassiveSkillSystem = nullptr;
    cltSkillSystem* m_pSkillSystem = nullptr;
    cltEmblemSystem* m_pEmblemSystem = nullptr;
    std::uint16_t m_wClassCode = 0;
    std::uint16_t m_padClass = 0;
    strClassKindInfo* m_pClassKindInfo = nullptr;
};
