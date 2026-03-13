#pragma once

#include <array>
#include <cstdint>

class CMofMsg;
class cltBaseInventory;
class cltClassSystem;
class cltCoupleRingKindInfo;
class cltItemKindInfo;
class cltItemList;
class cltMyItemSystem;
class cltWeddingHallKindInfo;
struct strCoupleRingKindInfo;

class cltMarriageSystem {
public:
    static void InitializeStaticVariable(cltItemKindInfo* itemKindInfo, cltWeddingHallKindInfo* weddingHallKindInfo,
                                         cltCoupleRingKindInfo* coupleRingKindInfo,
                                         void (*chargedRecallQtyByMyItemFuncPtr)(void*));

    cltMarriageSystem();
    ~cltMarriageSystem();

    void Initialize(void* owner, int now, cltBaseInventory* baseInventory, cltClassSystem* classSystem,
                    cltMyItemSystem* myItemSystem, CMofMsg* msg, int* outCharged);
    void Free();

    std::uint32_t GetSpouseDBAccount();
    char* GetSpouseName();
    std::uint16_t GetCoupleRingKind();
    void SetSpouseName(char* spouseName);

    int CanChangeCoupleRing(std::uint16_t ringKind);
    void ChangeCoupleRing(int now, std::uint16_t ringKind);

    int GetMarriageState();
    int CanPropose();
    int CanBeProposed();
    int CanRecallSpouse();
    void RecallSpouse();

    int GetRemainedRecallQty();
    int GetRemainedRecallQty_MyItem();
    int GetRemainedRecallQty_Total();
    int GetLastChargedRecallQtyTime();

    int IsMarriageState_None();
    int IsMarriageState_Married();

    void Marry(int now, int consumeRing, std::uint32_t spouseDbAccount, char* spouseName,
               std::uint16_t weddingTicketItemKind, std::uint16_t coupleRingItemKind,
               std::uint16_t slotA, std::uint16_t slotB, std::uint8_t* outInventoryChanged,
               cltItemList* outRewardItems, cltItemList* outConsumedItems);

    int IsMarried();
    int CanSeekDivorce(std::uint32_t dbAccount);
    int SeekDivorce(std::uint32_t dbAccount, int time);
    int OnSeekDivorced(std::uint32_t dbAccount, int time);
    int IsSeekDivorce();
    std::uint32_t GetSeekDivorceDBAccount();
    int GetSeekDivorceTime();
    int DidISeekDivorce();
    int CanCancelSeekDivorce(std::uint32_t dbAccount);
    void CancelSeekDivorce();
    void OnCanceledSeekDivorce();
    void OnDivorced();

    int GetCoupleRingExpAdvantage();
    int ChargeRecallQty(int now);
    void OnChargeRecallQtyByMyItem(int chargedQty);

    static cltItemKindInfo* m_pclItemKindInfo;
    static cltWeddingHallKindInfo* m_pclWeddingHallKindInfo;
    static cltCoupleRingKindInfo* m_pclCoupleRingKindInfo;
    static void (*m_pChargedRecallQtyByMyItemFuncPtr)(void*);

private:
    void* m_pOwner = nullptr;
    cltBaseInventory* m_pBaseInventory = nullptr;
    cltClassSystem* m_pClassSystem = nullptr;
    cltMyItemSystem* m_pMyItemSystem = nullptr;

    std::uint32_t m_spouseDBAccount = 0;
    std::array<char, 13> m_spouseName{};
    std::uint16_t m_coupleRingKind = 0;

    int m_marriageState = 0;
    std::uint32_t m_seekDivorceDBAccount = 0;
    int m_seekDivorceTime = 0;

    int m_remainedRecallQty = 0;
    int m_remainedRecallQtyMyItem = 0;
    int m_lastChargedRecallQtyTime = 0;
};
