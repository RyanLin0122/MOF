#pragma once

#include <cstdint>

#ifndef __int8
#define __int8 char
#endif
#ifndef __int16
#define __int16 short
#endif
#ifndef __int64
#define __int64 long long
#endif

typedef int BOOL;
typedef unsigned int _DWORD;
typedef unsigned short _WORD;

struct stItemKindInfo;
struct strUsingItemInfo {
    std::uint16_t itemKind = 0;
    std::uint16_t reserved = 0;
    std::uint32_t timerId = 0;
    std::uint32_t value0 = 0;
    std::uint16_t addStr = 0;
    std::uint16_t addDex = 0;
    std::uint16_t addInt = 0;
    std::uint16_t addVit = 0;
};
class cltItemKindInfo;
class cltPandoraKindInfo;
class cltPetKindInfo;
class cltTimerManager;
class cltBaseInventory;
class cltPlayerAbility;
class cltSpecialtySystem;
class cltMakingItemSystem;
class cltUsingSkillSystem;
class cltWorkingPassiveSkillSystem;
class cltEmblemSystem;
class cltSkillSystem;
class cltClassSystem;
class cltSexSystem;
class cltEquipmentSystem;
class cltBasicAppearSystem;
class cltPetSystem;
class cltMyItemSystem;
class cltMarriageSystem;
class cltPartySystem;

class cltUsingItemSystem {
public:
    static void InitializeStaticVariable(cltItemKindInfo* a1, cltPandoraKindInfo* a2, cltPetKindInfo* a3, cltTimerManager* a4,
        int (*a5)(void*), void (*a6)(void*), void (*a7)(int, void*, char*), void (*a8)(void*, std::uint16_t), int (*a9)(),
        unsigned int (*a10)(void*, std::uint16_t, char*), void (*a11)(int, void*, char*, char*), int (*a12)(void*),
        int (*a13)(void*, std::uint16_t), void (*a14)(void*, std::uint16_t), unsigned int (*a15)(void*), void (*a16)(void*),
        unsigned int (*a17)(void*, std::uint16_t), void (*a18)(void*, std::uint16_t, int), void (*a19)(unsigned int, unsigned int),
        void (*a20)(unsigned int, unsigned int), void (*a21)(unsigned int, unsigned int), void (*a22)(unsigned int, unsigned int));

    cltUsingItemSystem();
    ~cltUsingItemSystem() = default;

    void Initialize(void* a2, cltBaseInventory* a3, cltPlayerAbility* a4, cltSpecialtySystem* a5, cltMakingItemSystem* a6, cltUsingSkillSystem* a7, cltWorkingPassiveSkillSystem* a8, cltEmblemSystem* a9, cltSkillSystem* a10, cltClassSystem* a11, cltSexSystem* a12, cltEquipmentSystem* a13, cltBasicAppearSystem* a14, cltPetSystem* a15, cltMyItemSystem* a16, cltMarriageSystem* a17);
    void Free();
    bool IsUseItem(std::uint16_t a2);
    bool IsSpecialUseItem(std::uint16_t a2);
    std::uint8_t GetSpecialItemType(std::uint16_t a2);
    bool CanUseReturnItem(std::uint16_t a2);
    bool CanUseTeleportItem(std::uint16_t a2);
    bool CanMoveTeleportItem(std::uint16_t a2);
    bool CanUseTeleportDragon(std::uint16_t a2);
    bool CanUseTakeShip(std::uint16_t a2);
    BOOL CanUseTownPortalItem(std::uint16_t a2);
    int CanUseItem(std::uint16_t a2, int a3, char* a4, char* a5, cltPartySystem* a6, int a7, unsigned int a8, unsigned int a9, unsigned int a10, unsigned int a11);
    void UseItem(std::uint16_t a2, unsigned int a3);
    std::uint16_t UpdateCoinItem(int a2, int a3, std::uint16_t a4, std::uint8_t* a5);
    std::uint16_t ChangeCoinItem(int a2, std::uint8_t* a3);
    int UseItem(std::uint16_t a2, int a3, unsigned int a4, char* a5, char* a6, char* a7, std::int16_t a8, char a9, std::uint16_t a10, std::uint16_t a11, std::uint16_t a12, std::uint16_t* a13, int* a14, std::uint16_t* a15, int* a16, std::uint16_t* a17, cltPartySystem* a18, unsigned int Seed, int a20, std::uint16_t* a21, std::uint16_t* a22, int* a23, int* a24, std::uint8_t* a25);
    int CanUseSustainedTypeItemWithoutItem(std::uint16_t a2);
    void UseSustainedTypeItemWithoutItem(std::uint16_t a2, unsigned int a3);
    strUsingItemInfo* GetUsingItemInfo();
    strUsingItemInfo* GetUsingItemInfo(unsigned int a2);
    std::int16_t GetTotalStrOfUsingItem();
    std::int16_t GetTotalDexOfUsingItem();
    std::int16_t GetTotalIntOfUsingItem();
    std::int16_t GetTotalVitOfUsingItem();
    static void OnUsingItemInitialize(unsigned int a1, cltUsingItemSystem* a2);
    static void OnUsingItemPoll(unsigned int a1, cltUsingItemSystem* a2);
    static void OnUsingItemCustom(unsigned int a1, cltUsingItemSystem* a2);
    static void OnUsingItemTimeOuted(unsigned int a1, cltUsingItemSystem* a2);
    int IsAlreadyUsedItem(std::uint16_t a2);
    strUsingItemInfo* GetEmptyUsingItemInfo();
    int GetUsingItemNum();
    int IsActiveFastRun();
    int GetMaxHPAdvantage();
    int GetMaxManaAdvantage();
    int GetHitRateAdvantage();
    int GetCriticalHitRateAdvantage();
    int GetMissRateAdvantage();
    int GetAPowerAdvantage();
    int GetDPowerAdvantage();
    stItemKindInfo* IsCosmeticItem(std::uint16_t a2);
    void OnUsingItemTimeOuted(unsigned int a2);
    void SetChangeCoinSucRate(int a2);
    void SetChangeCoinRate(int a2, int a3);

    static cltItemKindInfo* m_pclItemKindInfo;
    static cltPandoraKindInfo* m_pclPandoraKindInfo;
    static cltPetKindInfo* m_pclPetKindInfo;
    static cltTimerManager* m_pclTimerManager;
    static int (*m_pIsPlayerDeadFuncPtr)(void*);
    static void (*m_pResurrectPlayerFuncPtr)(void*);
    static void (*m_pShoutFuncPtr)(int, void*, char*);
    static void (*m_pUseMapUsingItemFuncPtr)(void*, std::uint16_t);
    static int (*m_pIsRelayServerConnectedFuncPtr)();
    static unsigned int (*m_pCanUsePostItItemFuncPtr)(void*, std::uint16_t, char*);
    static void (*m_pPostItFuncPtr)(int, void*, char*, char*);
    static unsigned int (*m_pCanPlayerCureFuncPtr)(void*);
    static int (*m_pCanUseMapUsingItemFuncPtr)(void*, std::uint16_t);
    static void (*m_pPlayerCureFuncPtr)(void*);
    static unsigned int (*m_pCanChangeCoupleRingFuncPtr)(void*, std::uint16_t);
    static void (*m_pChangeCoupleRingFuncPtr)(void*, std::uint16_t, int);
    static int (*m_pCanUseChangeSexItemFuncPtr)(void*);
    static void (*m_pOpenGiftBoxFuncPtr)(void*, std::uint16_t);
    static void (*m_pExternUsingItemInitializeFuncPtr)(unsigned int, unsigned int);
    static void (*m_pExternUsingItemPollFuncPtr)(unsigned int, unsigned int);
    static void (*m_pExternUsingItemCustomFuncPtr)(unsigned int, unsigned int);
    static void (*m_pExternUsingItemTimeOutFuncPtr)(unsigned int, unsigned int);

private:
    void* m_pOwner = nullptr;
    cltBaseInventory* m_pBaseInventory = nullptr;
    cltPlayerAbility* m_pPlayerAbility = nullptr;
    cltSpecialtySystem* m_pSpecialtySystem = nullptr;
    cltMakingItemSystem* m_pMakingItemSystem = nullptr;
    cltUsingSkillSystem* m_pUsingSkillSystem = nullptr;
    cltWorkingPassiveSkillSystem* m_pWorkingPassiveSkillSystem = nullptr;
    cltEmblemSystem* m_pEmblemSystem = nullptr;
    cltSkillSystem* m_pSkillSystem = nullptr;
    cltClassSystem* m_pClassSystem = nullptr;
    cltSexSystem* m_pSexSystem = nullptr;
    cltEquipmentSystem* m_pEquipmentSystem = nullptr;
    cltBasicAppearSystem* m_pBasicAppearSystem = nullptr;
    cltPetSystem* m_pPetSystem = nullptr;
    cltMyItemSystem* m_pMyItemSystem = nullptr;
    cltMarriageSystem* m_pMarriageSystem = nullptr;
    strUsingItemInfo m_usingItems[10]{};
    int m_changeCoinSuccessRate = 0;
    int m_changeCoinRateA = 0;
    int m_changeCoinRateB = 0;
};

