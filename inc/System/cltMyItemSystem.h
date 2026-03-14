#pragma once

#include <array>
#include <cstdint>

class cltMyItemKindInfo;
class cltCoupleRingKindInfo;
class cltQuickSlotSystem;
class cltMarriageSystem;
class CMofMsg;

struct strMyItem {
    std::uint16_t wKind = 0;
    std::uint16_t wPad = 0;
    int nValue0 = 0;
    int nValue1 = 0;
    int nValue2 = 0;
};
static_assert(sizeof(strMyItem) == 16, "strMyItem must be 16 bytes");

class cltMyItemSystem {
public:
    static void InitializeStaticVariable(cltMyItemKindInfo* myItemKindInfo, cltCoupleRingKindInfo* coupleRingKindInfo,
                                         void (*onMyItemTimeouted)(void*, strMyItem*));

    cltMyItemSystem();
    ~cltMyItemSystem();

    void Initialize(void* owner, cltQuickSlotSystem* quickSlotSystem, cltMarriageSystem* marriageSystem, CMofMsg* msg);
    void Free();

    unsigned int CanAddMyItem(std::uint16_t myItemKind);
    void AddMyItem(std::uint16_t myItemKind, int v0, int v1, int v2);
    void DelMyItem(std::uint16_t myItemKind, int v0);
    void DeleteExpiredMyItem();

    int GetExpAdvantage();
    int GetDropMoneyAmountAdvangae();
    int GetSpouseChargeRecallQty();
    static int GetSpouseChargeRecallQty(cltMyItemSystem* self);

    int GetMyItemNum();
    strMyItem* GetMyItem();

private:
    void* m_owner = nullptr;
    cltQuickSlotSystem* m_quickSlotSystem = nullptr;
    cltMarriageSystem* m_marriageSystem = nullptr;
    std::array<strMyItem, 10> m_myItems{};
    int m_myItemCount = 0;

    static cltMyItemKindInfo* m_pclMyItemKindInfo;
    static cltCoupleRingKindInfo* m_pclCoupleRingKindInfo;
    static void (*m_pMyItemFuncPtr_OnMyItemTimeouted)(void*, strMyItem*);
};
