#include "System/CMeritoriousSystem.h"


namespace {
constexpr std::uint16_t kMaxQuestMonster = 16;
constexpr std::uint16_t kMaxGrade = 15;
constexpr unsigned int kGradeUnitPoint = 1000;
constexpr unsigned int kWarBaseExp = 100;
constexpr unsigned int kWarDifficultyExpBonus = 35;
constexpr unsigned int kWarDifficultyPointBonus = 5;
constexpr unsigned int kSupplyBaseLibi = 100;
constexpr unsigned int kSupplyBasePoint = 1;
constexpr std::uint16_t kWarQuestNeedCountByDifficulty[6] = {2, 3, 4, 5, 6, 7};
constexpr std::uint16_t kSupplyItemNeedByKind[16] = {
    0, 4, 6, 8, 10, 12, 14, 16,
    18, 20, 22, 24, 26, 28, 30, 32
};

static std::uint16_t ClampGrade(std::uint16_t grade) {
    return std::clamp<std::uint16_t>(grade, 1, kMaxGrade);
}

static std::uint16_t NormalizeDifficulty(int difficulty) {
    return static_cast<std::uint16_t>(std::clamp(difficulty, 0, 5));
}

static unsigned int SafeMul(unsigned int a, unsigned int b) {
    if (a == 0 || b == 0) return 0;
    if (a > 0xFFFFFFFFu / b) return 0xFFFFFFFFu;
    return a * b;
}

static unsigned int SafeAdd(unsigned int a, unsigned int b) {
    if (a > 0xFFFFFFFFu - b) return 0xFFFFFFFFu;
    return a + b;
}

static std::uint16_t SumItemCount(cltItemList* list, std::uint16_t itemKind) {
    if (!list || !itemKind) return 0;

    std::uint16_t total = 0;
    const std::uint16_t n = list->GetItemsNum();
    for (std::uint16_t i = 0; i < n; ++i) {
        std::uint16_t k = 0;
        std::uint16_t q = 0;
        unsigned int dummy0 = 0;
        std::uint16_t dummy1 = 0;
        std::uint16_t dummy2 = 0;
        if (!list->GetItem(i, &k, &q, &dummy0, &dummy1, &dummy2)) continue;
        if (k != itemKind) continue;
        const unsigned int merged = static_cast<unsigned int>(total) + q;
        total = static_cast<std::uint16_t>(std::min<unsigned int>(merged, 65535));
    }
    return total;
}
}

cltCharKindInfo* CMeritoriousSystem::m_pclCharKindInfo = nullptr;
CExpRewardParser* CMeritoriousSystem::m_pclExpRewardParser = nullptr;
CMeritoriousGradeParser* CMeritoriousSystem::m_pclMeritoriousGradeParser = nullptr;
CMeritoriousRewardParser* CMeritoriousSystem::m_pclMeritoriousRewardParser = nullptr;
CSupplyMeritoriousParser* CMeritoriousSystem::m_pclSupplyMeritoriousParser = nullptr;

void CMeritoriousSystem::InitializeStaticVariable(cltCharKindInfo* a1, CSupplyMeritoriousParser* a2,
                                                  CMeritoriousRewardParser* a3, CMeritoriousGradeParser* a4,
                                                  CExpRewardParser* a5) {
    m_pclCharKindInfo = a1;
    m_pclSupplyMeritoriousParser = a2;
    m_pclMeritoriousRewardParser = a3;
    m_pclMeritoriousGradeParser = a4;
    m_pclExpRewardParser = a5;
}

CMeritoriousSystem::CMeritoriousSystem() { Free(); }
CMeritoriousSystem::~CMeritoriousSystem() = default;

void CMeritoriousSystem::Free() {
    questSystem_ = nullptr;
    inventory_ = nullptr;
    specialtySystem_ = nullptr;
    moneySystem_ = nullptr;
    emblemSystem_ = nullptr;

    point_ = 0;
    totalPoint_ = 0;
    grade_ = 1;
    gradePoint_ = 0;

    warQuestPlaying_ = false;
    supplyQuestPlaying_ = 0;
    warQuestDifficulty_ = 0;
    supplyQuestKind_ = 0;

    warQuestMonKinds_.fill(0);
    warQuestMonGoals_.fill(0);
    warQuestMonKills_.fill(0);
    warQuestMonCount_ = 0;
}

int CMeritoriousSystem::Initialize(cltQuestSystem* questSystem, cltBaseInventory* inventory, cltSpecialtySystem* specialty,
                                   cltMoneySystem* moneySystem, cltEmblemSystem* emblemSystem,
                                   unsigned int point, unsigned int totalPoint,
                                   std::uint16_t grade, std::uint16_t gradePoint,
                                   bool warQuestPlaying, char supplyQuestPlaying,
                                   std::uint16_t warQuestMonCount,
                                   const std::uint16_t* warQuestMonKinds,
                                   const std::uint16_t* warQuestMonGoals) {
    Free();

    questSystem_ = questSystem;
    inventory_ = inventory;
    specialtySystem_ = specialty;
    moneySystem_ = moneySystem;
    emblemSystem_ = emblemSystem;

    SetMeritoriousInfo(point, totalPoint, grade, gradePoint,
                       warQuestPlaying, supplyQuestPlaying,
                       warQuestMonCount, warQuestMonKinds, warQuestMonGoals);

    return 1;
}

unsigned int CMeritoriousSystem::StartWarMeritoriousQuest(std::uint16_t mapKind, int difficulty, std::uint16_t* outQuestKinds) {
    if (!CanStartWarMeritoriousQuest()) return 0;

    const auto d = NormalizeDifficulty(difficulty);
    warQuestDifficulty_ = d;
    warQuestPlaying_ = true;

    if (warQuestMonCount_ == 0) {
        warQuestMonCount_ = kWarQuestNeedCountByDifficulty[d];
        for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
            warQuestMonKinds_[i] = static_cast<std::uint16_t>(mapKind + i + 1);
            warQuestMonGoals_[i] = static_cast<std::uint16_t>(3 + d + i);
            warQuestMonKills_[i] = 0;
        }
    } else {
        warQuestMonKills_.fill(0);
    }

    if (outQuestKinds) {
        for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
            outQuestKinds[i] = warQuestMonKinds_[i];
        }
    }
    return 1;
}

unsigned int CMeritoriousSystem::CanStartWarMeritoriousQuest(int minLv, int maxLv, std::uint16_t needClass, std::uint16_t needNation,
                                                             std::uint16_t monsterKind, stMonsterKind*) {
    if (!CanStartWarMeritoriousQuest()) return 0;
    if (!monsterKind) return 0;

    if (minLv > maxLv) return 0;
    if (needClass == 0 || needNation == 0) return 0;

    return 1;
}

int CMeritoriousSystem::CanStartWarMeritoriousQuest() {
    if (warQuestPlaying_) return 0;
    if (supplyQuestPlaying_) return 0;
    return 1;
}

void CMeritoriousSystem::PlayWarMeritoriousQuest(int playing) {
    warQuestPlaying_ = (playing != 0);
}

unsigned int CMeritoriousSystem::CompleteWarMeritoriousQuest(unsigned int seed, std::int64_t* outExp, int* outMoney,
                                                             std::uint16_t* outQuestKinds, unsigned int* outQuestValues) {
    if (outExp) *outExp = 0;
    if (outMoney) *outMoney = 0;

    if (!CanCompleteWarMeritoriousQuest()) return 0;

    const unsigned int rewardExp = GetRewardWarMeritoriousExp();
    const unsigned int rewardPt = GetRewardWarMeritoriousPoint();
    const int rewardMoney = static_cast<int>(SafeMul(rewardPt, 100));

    if (outExp) *outExp = rewardExp;
    if (outMoney) *outMoney = rewardMoney;

    if (outQuestKinds && outQuestValues) {
        for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
            outQuestKinds[i] = warQuestMonKinds_[i];
            outQuestValues[i] = warQuestMonKills_[i];
        }
    }

    IncreaseMeritoriousPoint(static_cast<std::uint16_t>(std::min<unsigned int>(rewardPt, 65535)));
    if (moneySystem_) moneySystem_->IncreaseMoney(rewardMoney);

    InitCompleteWarMeritoriousQuest();
    return seed;
}

unsigned int CMeritoriousSystem::GetRewardWarMeritoriousExp() {
    if (!warQuestMonCount_) return 0;

    unsigned int total = 0;
    const unsigned int diffBonus = SafeMul(static_cast<unsigned int>(warQuestDifficulty_), kWarDifficultyExpBonus);
    for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
        const unsigned int base = SafeMul(warQuestMonGoals_[i], kWarBaseExp);
        total = SafeAdd(total, SafeAdd(base, diffBonus));
    }
    return total;
}

unsigned int CMeritoriousSystem::GetRewardWarMeritoriousPoint() {
    if (!warQuestMonCount_) return 0;

    unsigned int total = 0;
    for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
        const unsigned int kill = min(warQuestMonKills_[i], warQuestMonGoals_[i]);
        total = SafeAdd(total, kill);
    }

    const unsigned int diffBonus = SafeMul(static_cast<unsigned int>(warQuestDifficulty_), kWarDifficultyPointBonus);
    return SafeAdd(total, diffBonus);
}

unsigned int CMeritoriousSystem::GetRewardSupplyMeritoriousLibi() {
    const unsigned int count = GetSupplyMeritoriousItemCount();
    return SafeMul(count, kSupplyBaseLibi);
}

unsigned int CMeritoriousSystem::GetRewardSupplyMeritoriousPoint() {
    const unsigned int count = GetSupplyMeritoriousItemCount();
    return SafeMul(count, kSupplyBasePoint);
}

unsigned int CMeritoriousSystem::GetSupplyMeritoriousItemCount() {
    if (!supplyQuestKind_) return 0;
    if (supplyQuestKind_ < std::size(kSupplyItemNeedByKind)) return kSupplyItemNeedByKind[supplyQuestKind_];
    return 3 + (supplyQuestKind_ % 10);
}

void CMeritoriousSystem::SetMeritoriousUpGrade(std::uint16_t grade, std::uint16_t gradePoint) {
    grade_ = ClampGrade(grade);
    gradePoint_ = static_cast<std::uint16_t>(gradePoint % kGradeUnitPoint);
}

unsigned int CMeritoriousSystem::CanCompleteWarMeritoriousQuest() {
    if (!warQuestPlaying_) return 0;
    if (!warQuestMonCount_) return 0;

    for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
        if (warQuestMonKills_[i] < warQuestMonGoals_[i]) return 0;
    }
    return 1;
}

void CMeritoriousSystem::InitCompleteWarMeritoriousQuest() {
    warQuestPlaying_ = false;
    warQuestMonKills_.fill(0);
    warQuestDifficulty_ = 0;
}

std::uint16_t CMeritoriousSystem::GetWarMeritoriousQuestMonsterSize() {
    return warQuestMonCount_;
}

std::uint16_t CMeritoriousSystem::GetWarMeritoriousMonsterKillCount(std::uint16_t kind) {
    for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
        if (warQuestMonKinds_[i] == kind) return warQuestMonKills_[i];
    }
    return 0;
}

std::uint16_t CMeritoriousSystem::GetWarMeritoriousMonsterGoalKillCount(std::uint16_t kind) {
    for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
        if (warQuestMonKinds_[i] == kind) return warQuestMonGoals_[i];
    }
    return 0;
}

int CMeritoriousSystem::GetWarMeritoriousMonsterKind(std::uint16_t* outSize, std::uint16_t* outKinds) {
    if (outSize) *outSize = warQuestMonCount_;

    if (outKinds) {
        for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
            outKinds[i] = warQuestMonKinds_[i];
        }
    }

    return warQuestMonCount_ > 0;
}

void CMeritoriousSystem::IncreaseMeritoriousPoint(std::uint16_t point) {
    point_ = SafeAdd(point_, point);
    totalPoint_ = SafeAdd(totalPoint_, point);

    std::uint16_t g = grade_;
    std::uint16_t gp = gradePoint_;
    CalcMeritoriousGrade(&g, &gp);
    grade_ = g;
    gradePoint_ = gp;
}

void CMeritoriousSystem::DecreaseMeritoriousPoint(std::uint16_t point) {
    if (point_ > point) point_ -= point;
    else point_ = 0;

    std::uint16_t g = grade_;
    std::uint16_t gp = gradePoint_;
    CalcMeritoriousGrade(&g, &gp);
    grade_ = g;
    gradePoint_ = gp;
}

int CMeritoriousSystem::IsExistWarQuestMonsterKind(std::uint16_t kind) {
    for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
        if (warQuestMonKinds_[i] == kind) return 1;
    }
    return 0;
}

int CMeritoriousSystem::CanGiveUpWarMeritorious() {
    return warQuestPlaying_ ? 1 : 0;
}

void CMeritoriousSystem::GiveUpWarMeritorious() {
    InitCompleteWarMeritoriousQuest();
    warQuestMonKinds_.fill(0);
    warQuestMonGoals_.fill(0);
    warQuestMonCount_ = 0;
}

int CMeritoriousSystem::CanGiveUpSupplyMeritorious() {
    return supplyQuestPlaying_ ? 1 : 0;
}

void CMeritoriousSystem::GiveUpSupplyMeritorious() {
    InitCompleteSupplyMeritoriousQuest();
}

unsigned int CMeritoriousSystem::StartSupplyMeritoriousQuest(std::uint16_t supplyKind) {
    if (!CanStartSupplyMeritoriousQuest()) return 0;

    supplyQuestKind_ = supplyKind;
    supplyQuestPlaying_ = 1;
    return 1;
}

int CMeritoriousSystem::CanStartSupplyMeritoriousQuest() {
    if (supplyQuestPlaying_) return 0;
    if (warQuestPlaying_) return 0;
    return 1;
}

void CMeritoriousSystem::CompleteSupplyMeritoriousQuest(int count, cltItemList* itemList, std::uint8_t* outChangedSlots,
                                                        int* outMoney, std::uint16_t* outQuestKinds, unsigned int* outQuestValues) {
    if (outMoney) *outMoney = 0;
    if (!CanCompleteSupplyMeritoriousQuest(count)) return;

    const std::uint16_t needKind = supplyQuestKind_;
    const std::uint16_t got = SumItemCount(itemList, needKind);
    const std::uint16_t need = static_cast<std::uint16_t>(GetSupplyMeritoriousItemCount());
    if (got < need) return;

    const unsigned int libi = GetRewardSupplyMeritoriousLibi();
    const unsigned int point = GetRewardSupplyMeritoriousPoint();

    if (outMoney) *outMoney = static_cast<int>(libi);
    if (outQuestKinds) outQuestKinds[0] = needKind;
    if (outQuestValues) outQuestValues[0] = need;

    if (moneySystem_) moneySystem_->IncreaseMoney(static_cast<int>(libi));
    IncreaseMeritoriousPoint(static_cast<std::uint16_t>(std::min<unsigned int>(point, 65535)));

    if (outChangedSlots) {
        for (int i = 0; i < 16; ++i) outChangedSlots[i] = static_cast<std::uint8_t>(i);
    }

    InitCompleteSupplyMeritoriousQuest();
}

unsigned int CMeritoriousSystem::CanCompleteSupplyMeritoriousQuest(int count) {
    if (!supplyQuestPlaying_) return 0;
    return count >= static_cast<int>(GetSupplyMeritoriousItemCount()) ? 1u : 0u;
}

void CMeritoriousSystem::InitCompleteSupplyMeritoriousQuest() {
    supplyQuestPlaying_ = 0;
    supplyQuestKind_ = 0;
}

int CMeritoriousSystem::RewardMeritoriousItem(std::uint16_t itemKind, unsigned int qty) {
    if (CanRewardMeritoriousItem(itemKind) != 0) return 0;
    if (!inventory_) return 0;

    strInventoryItem it{};
    it.itemKind = itemKind;
    it.itemQty = static_cast<std::uint16_t>(std::clamp<unsigned int>(qty, 1, 65535));

    return inventory_->AddInventoryItem(&it, nullptr, nullptr) == 0;
}

int CMeritoriousSystem::RewardMeritoriousItem(std::uint16_t itemKind, unsigned int* outPos, std::uint8_t* outChangedSlots) {
    if (CanRewardMeritoriousItem(itemKind) != 0) return 0;
    if (!inventory_) return 0;

    strInventoryItem it{};
    it.itemKind = itemKind;
    it.itemQty = 1;

    std::uint16_t pos = 0;
    const int ret = inventory_->AddInventoryItem(&it, outChangedSlots, &pos);
    if (outPos) *outPos = pos;
    return ret == 0;
}

int CMeritoriousSystem::CanRewardMeritoriousItem(std::uint16_t itemKind) {
    if (!inventory_) return 1;
    if (!itemKind) return 1;
    return inventory_->CanAddInventoryItem(itemKind, 1);
}

void CMeritoriousSystem::SetMeritoriousInfo(unsigned int point, unsigned int totalPoint,
                                            std::uint16_t grade, std::uint16_t gradePoint,
                                            bool warQuestPlaying, char supplyQuestPlaying,
                                            std::uint16_t warQuestMonCount,
                                            const std::uint16_t* warQuestMonKinds,
                                            const std::uint16_t* warQuestMonGoals) {
    point_ = point;
    totalPoint_ = totalPoint;
    grade_ = ClampGrade(grade);
    gradePoint_ = static_cast<std::uint16_t>(gradePoint % kGradeUnitPoint);
    warQuestPlaying_ = warQuestPlaying;
    supplyQuestPlaying_ = supplyQuestPlaying;

    warQuestMonCount_ = std::min<std::uint16_t>(warQuestMonCount, kMaxQuestMonster);
    warQuestMonKinds_.fill(0);
    warQuestMonGoals_.fill(0);
    warQuestMonKills_.fill(0);

    for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
        warQuestMonKinds_[i] = warQuestMonKinds ? warQuestMonKinds[i] : 0;
        warQuestMonGoals_[i] = warQuestMonGoals ? warQuestMonGoals[i] : 0;
    }

    std::uint16_t g = grade_;
    std::uint16_t gp = gradePoint_;
    CalcMeritoriousGrade(&g, &gp);
    grade_ = g;
    gradePoint_ = gp;
}

void CMeritoriousSystem::SetWarMeritoriousQuest(int playing, int difficulty) {
    warQuestPlaying_ = (playing != 0);
    warQuestDifficulty_ = NormalizeDifficulty(difficulty);
    if (!warQuestPlaying_) {
        warQuestMonKills_.fill(0);
    }
}

int CMeritoriousSystem::CalcMeritoriousGrade(std::uint16_t* outGrade, std::uint16_t* outGradePoint) {
    unsigned int value = point_;
    std::uint16_t g = 1;
    while (value >= kGradeUnitPoint && g < kMaxGrade) {
        value -= kGradeUnitPoint;
        ++g;
    }

    if (outGrade) *outGrade = g;
    if (outGradePoint) *outGradePoint = static_cast<std::uint16_t>(value);
    return g;
}
