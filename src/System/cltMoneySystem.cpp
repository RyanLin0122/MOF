#include "System/cltMoneySystem.h"

// === 常數（與反編譯檢查上限相符） ===
static constexpr int   kMoneyCap = 2000000000;       // 0x77359400
static constexpr int64_t kMoneyCapLL = 2000000000LL;

// ----- dtor（對應：~cltMoneySystem; 反編譯只看到設 vftable，C++ 會自動處理） -----
cltMoneySystem::~cltMoneySystem() = default;

// ----- InitiaizeMoney：直接設值 -----
void cltMoneySystem::InitiaizeMoney(int a2) {
    m_money = a2;
}

// ----- Free：金額與鎖定清零 -----
void cltMoneySystem::Free() {
    m_money = 0;
    m_lock = 0;
}

// ----- CanIncreaseMoney：檢查加總是否在 [0, 上限]，且未鎖定 -----
BOOL cltMoneySystem::CanIncreaseMoney(int a2) {
    if (IsLock() == 1) return 0;

    // 以 64 位計算避免溢位，忠於反編譯裡用 HIDWORD/OFADD 的意圖
    int64_t sum = static_cast<int64_t>(m_money) + static_cast<int64_t>(a2);
    if (sum < 0) return 0;
    if (sum > kMoneyCapLL) return 0;
    return 1;
}

// ----- IncreaseMoney：加總並做上限飽和 -----
void cltMoneySystem::IncreaseMoney(int a2) {
    int64_t sum = static_cast<int64_t>(m_money) + static_cast<int64_t>(a2);
    if (sum >= kMoneyCapLL) {
        m_money = kMoneyCap;
    }
    else if (sum <= 0) {
        // 雖然原始 IncreaseMoney 未處理負值下限，但為安全避免 UB 先收斂
        // 若你希望 100% 嚴格遵照反編譯行為，可移除此分支以保留 "可能成負" 的語義。
        m_money = 0;
    }
    else {
        m_money = static_cast<int>(sum);
    }
}

// ----- CanDecreaseMoney：未鎖、a2>=0、且 a2<=現金額 -----
BOOL cltMoneySystem::CanDecreaseMoney(int a2) {
    if (IsLock() == 1) return 0;
    if (a2 < 0) return 0;
    return (a2 <= m_money) ? 1 : 0;
}

// ----- DecreaseMoney：money -= a2；若 <=0 則歸零 -----
void cltMoneySystem::DecreaseMoney(int a2) {
    int64_t diff = static_cast<int64_t>(m_money) - static_cast<int64_t>(a2);
    m_money = (diff <= 0) ? 0 : static_cast<int>(diff);
}

// ----- GetGameMoney：回傳目前金額 -----
int cltMoneySystem::GetGameMoney() const {
    return m_money;
}

// ----- Lock / Unlock / IsLock -----
void cltMoneySystem::Lock() { m_lock = 1; }
void cltMoneySystem::Unlock() { m_lock = 0; }
int  cltMoneySystem::IsLock() const { return m_lock; }
