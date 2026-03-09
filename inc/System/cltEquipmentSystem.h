#pragma once

#include <array>
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
typedef unsigned char _BYTE;
typedef unsigned short _WORD;
typedef unsigned int _DWORD;

struct stItemKindInfo;
struct strInventoryItem;
struct stEquipItemInfo {
    std::uint16_t itemKind = 0;
    std::uint16_t reserved = 0;
    std::uint32_t itemTime = 0;
    std::uint32_t sealedStatus = 0;
};
class cltItemKindInfo;
class cltClassKindInfo;
class cltSexSystem;
class cltLevelSystem;
class cltClassSystem;
class cltPlayerAbility;
class cltBaseInventory;
class cltSkillSystem;
class CPlayerSpirit;
class cltBasicAppearSystem;
class cltItemList;

class cltEquipmentSystem {
public:
    static void InitializeStaticVariable(cltItemKindInfo* a1, cltClassKindInfo* a2);

    cltEquipmentSystem();
    ~cltEquipmentSystem();

    void Initialize(cltSexSystem* a2, cltLevelSystem* a3, cltClassSystem* a4, cltPlayerAbility* a5, cltBaseInventory* a6, cltSkillSystem* a7, CPlayerSpirit* a8, cltBasicAppearSystem* a9, std::uint16_t* const a10, std::uint16_t* const a11, unsigned int* const a12, unsigned int* const a13);
    void Initialize(cltSexSystem* a2, cltLevelSystem* a3, cltClassSystem* a4, cltPlayerAbility* a5, cltBaseInventory* a6, cltSkillSystem* a7, CPlayerSpirit* a8, cltBasicAppearSystem* a9, const stEquipItemInfo* a10, const stEquipItemInfo* a11);
    void Free();
    strInventoryItem* CanEquipItem(int a2, unsigned __int16 a3);
    unsigned int CanEquipItem(unsigned int a2, unsigned int a3, unsigned __int16 a4);
    stItemKindInfo* CanEquipItemByItemKind(int a2, unsigned __int16 a3);
    __int16 EquipItem(unsigned int a2, unsigned int a3, unsigned __int16 a4);
    strInventoryItem* CanUnEquipItem(unsigned int a2, unsigned int a3, unsigned __int16 a4);
    void UnEquipItem(unsigned int a2, unsigned int a3, unsigned __int16 a4);
    unsigned __int16 GetEquipItem(unsigned int a2, unsigned int a3);
    int GetSealedStatus(int a2, int a3);
    unsigned int GetEquipItemTime(unsigned int a2, unsigned int a3);
    unsigned __int16* GetEquipItemList(unsigned int a2);
    int CanUnEquipAllItem(unsigned int a2);
    void UnEquipAllItem(cltItemList* a2, unsigned int a3, unsigned __int8* a4, unsigned __int8* a5);
    void UnEquipAllItem(unsigned int a2, unsigned __int8* a3);
    stItemKindInfo* CanMoveEquipItem(unsigned int a2, unsigned int a3, unsigned int a4);
    void MoveEquipItem(unsigned int a2, unsigned int a3, unsigned int a4);
    unsigned __int16 GetTotalStr();
    unsigned __int16 GetTotalDex();
    unsigned __int16 GetTotalInt();
    unsigned __int16 GetTotalVit();
    void GetAPower(unsigned __int16* a2, unsigned __int16* a3);
    int GetAPowerAdvantage(int a2);
    __int16 GetDPower();
    int GetDPowerAdvatnage(int a2);
    int GetMaxHPAdvantage();
    int GetMaxManaAdvantage();
    int GetSkillAPowerAdvantage();
    int GetHitRate();
    int GetCriticalHitRate();
    int GetMissRate();
    stItemKindInfo* GetEquipablePos(unsigned __int16 a2, unsigned int* a3, unsigned int* a4, unsigned int* a5);
    stItemKindInfo* GetEquipablePosByItemKind(unsigned __int16 a2, unsigned int* a3, unsigned int* a4, unsigned int* a5);
    int GetUnEquipablePos(unsigned int a2, unsigned int a3, unsigned __int16* a4);
    int GetExpAdvantageByEquipItem();
    void UpdateValidity();
    int IsEquipItemValidity(unsigned int a2, unsigned int a3);
    int GetWeaponAttackAtb();
    int IsEquipedFashionFullSet();
    stItemKindInfo* IsEquipedMultiTargetWeapon();
    int IsEquipedBattleItem();
    int IsEffectedEquipKindByFashionFullSet(unsigned int a2);
    int GetMagicResist();
    int GetAutoRecoverHPAdvantage();
    int GetAutoRecoverManaAdavntage();
    int IsFashionAllUnequiped();
    void SetEquipItemsByChangeSex(unsigned __int16 a2, unsigned __int16 a3, unsigned __int16 a4);
    char SetItemSealed(char a2, unsigned __int16 a3, int a4);

    static cltItemKindInfo* m_pclItemKindInfo;
    static cltClassKindInfo* m_pclClassKindInfo;
    static unsigned int* m_dwEquipAtbForFashion;
    static unsigned int* m_dwEquipAtbForBattle;
    static unsigned __int16* m_wFashionFullSetEffectedKinds;

public:
    cltLevelSystem* m_pLevelSystem = nullptr;
    cltClassSystem* m_pClassSystem = nullptr;
    cltSexSystem* m_pSexSystem = nullptr;
    cltPlayerAbility* m_pPlayerAbility = nullptr;
    cltBaseInventory* m_pBaseInventory = nullptr;
    cltSkillSystem* m_pSkillSystem = nullptr;
    CPlayerSpirit* m_pPlayerSpirit = nullptr;
    cltBasicAppearSystem* m_pBasicAppearSystem = nullptr;
    std::array<std::uint16_t, 11> m_fashionItemKinds{};
    std::array<std::uint16_t, 11> m_battleItemKinds{};
    std::array<std::uint32_t, 11> m_fashionValidity{};
    std::array<std::uint32_t, 11> m_battleValidity{};
    std::array<std::uint32_t, 11> m_fashionItemTimes{};
    std::array<std::uint32_t, 11> m_battleItemTimes{};
    std::array<std::uint32_t, 11> m_fashionSealStates{};
    std::array<std::uint32_t, 11> m_battleSealStates{};
};

