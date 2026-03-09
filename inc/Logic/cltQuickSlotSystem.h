#pragma once

#include <cstdint>

typedef unsigned int _DWORD;
typedef unsigned short _WORD;

struct stItemKindInfo;
struct stSkillKindInfo;
struct strInventoryItemForSort;
class cltItemKindInfo;
class cltSkillKindInfo;
class cltBaseInventory;
class cltMyItemSystem;
class cltQuestSystem;
class CMofMsg;

class cltQuickSlotSystem {
public:
    static void InitializeStaticVariable(cltItemKindInfo* a1, cltSkillKindInfo* a2);

    cltQuickSlotSystem();
    ~cltQuickSlotSystem() = default;

    void Initialize(cltBaseInventory* a2, cltMyItemSystem* a3, cltQuestSystem* a4, CMofMsg* a5);
    std::uint16_t GetTransportSlot();
    void Free();
    stItemKindInfo* AddForItem(unsigned int a2, unsigned int a3, std::uint8_t a4);
    int AddForSkill(unsigned int a2, unsigned int a3, std::uint16_t a4);
    int AddForEmblem(unsigned int a2, unsigned int a3, std::uint16_t a4, std::uint16_t* a5, unsigned int* a6);
    int Del(unsigned int a2, unsigned int a3);
    int Move(unsigned int a2, unsigned int a3, unsigned int a4);
    unsigned int Get(unsigned int a2, unsigned int a3);
    void OnItemMerged(std::uint8_t a2, std::uint8_t a3);
    void OnItemMoved(std::uint8_t a2, std::uint8_t a3);
    void OnItemOuted(std::uint8_t a2, int a3);
    void OnSkillDeleted(std::uint16_t a2);
    void OnSkillAdded(std::uint16_t a2);
    void OnClassReseted();
    void OnItemSorted(strInventoryItemForSort* a2);
    void GetQuickSlotInfoForDBQuery(char* Buffer);
    void OnPremiumQuickSlotEnabled();
    void OnPremiumQuickSlotDisabled();
    int OnRelinkItem(unsigned int a2, int a3);
    void SetChange(int a2, int a3);
    int IsChange();
    void CheckQSLItem();

    static cltItemKindInfo* m_pclItemKindInfo;
    static cltSkillKindInfo* m_pclSkillKindInfo;
};

