#pragma once
#include <cstdint>

#ifndef BOOL
#define BOOL int
#endif

// 反編譯顯示含有 vtable（出現 vector deleting destructor）
// 這裡提供虛擬解構以對應 vptr 版位（[0]）。
class cltMoneySystem {
public:
    virtual ~cltMoneySystem();

    // 對應：void __thiscall InitiaizeMoney(this, a2)
    // 直接把 a2 寫入金額欄位
    void InitiaizeMoney(int a2);

    // 對應：void __thiscall Free(this)
    // 將金額與鎖定旗標清為 0
    void Free();

    // 對應：BOOL __thiscall CanIncreaseMoney(this, a2)
    // 不在鎖定狀態時，檢查 (money + a2) 是否介於 [0, 2_000_000_000]
    BOOL CanIncreaseMoney(int a2);

    // 對應：void __thiscall IncreaseMoney(this, a2)
    // money += a2；若 >= 2_000_000_000 則飽和為上限
    void IncreaseMoney(int a2);

    // 對應：BOOL __thiscall CanDecreaseMoney(this, a2)
    // 不在鎖定狀態時，僅允許 a2 >= 0 且 a2 <= money
    BOOL CanDecreaseMoney(int a2);

    // 對應：void __thiscall DecreaseMoney(this, a2)
    // money -= a2；若 <= 0 則設為 0
    void DecreaseMoney(int a2);

    // 對應：int __thiscall GetGameMoney(this)
    int GetGameMoney() const;

    // 對應：void __thiscall Lock(this) / Unlock(this) / int __thiscall IsLock(this)
    void Lock();
    void Unlock();
    int  IsLock() const;

private:
    // 版位配置需對應反編譯（[0] vptr 由編譯器自動放置）
    int m_money = 0; // [ +4 ] 金額
    int m_lock = 0; // [ +8 ] 1=鎖定, 0=未鎖
};
