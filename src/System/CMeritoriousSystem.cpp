#include "System/CMeritoriousSystem.h"


namespace {
constexpr std::uint16_t kMaxQuestMonster = 16;
constexpr std::uint16_t kMaxGrade = 15;
constexpr unsigned int kGradeUnitPoint = 1000;
constexpr unsigned int kWarPointRewardBase = 10;

static std::uint16_t ClampGrade(std::uint16_t grade) {
    return std::clamp<std::uint16_t>(grade, 1, kMaxGrade);
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

static bool GetSupplyInfo(CSupplyMeritoriousParser* parser, std::uint16_t id, CSupplyMeritoriousInfo* out) {
    if (!parser || !out) return false;
    const auto& m = parser->GetMap();
    const auto it = m.find(id);
    if (it == m.end()) return false;
    *out = it->second;
    return true;
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
    warQuestMonKinds_.fill(0);
    warQuestMonGoals_.fill(0);
    warQuestMonKills_.fill(0);

    warQuestMonCount_ = std::clamp<int>(difficulty, 0, kMaxQuestMonster);
    if (outQuestKinds) {
        for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
            warQuestMonKinds_[i] = outQuestKinds[i];
        }
    }
    warQuestPlaying_ = true;
    warQuestDifficulty_ = mapKind;
    return 0;
}

unsigned int CMeritoriousSystem::CanStartWarMeritoriousQuest(int minLv, int maxLv, std::uint16_t needClass, std::uint16_t needNation,
                                                             std::uint16_t monsterKind, stMonsterKind*) {
    if (CanStartWarMeritoriousQuest()) return 1;
    if (!monsterKind) return 1;

    if (minLv > maxLv) return 1;
    if (needClass == 0 || needNation == 0) return 1;

    return 0;
}

int CMeritoriousSystem::CanStartWarMeritoriousQuest() {
    return warQuestPlaying_ ? 1 : 0;
}

void CMeritoriousSystem::PlayWarMeritoriousQuest(int playing) {
    if (!warQuestPlaying_) return;

    const auto kind = static_cast<std::uint16_t>(playing);
    for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
        if (warQuestMonKinds_[i] == kind) {
            ++warQuestMonKills_[i];
            return;
        }
    }
}

unsigned int CMeritoriousSystem::CompleteWarMeritoriousQuest(unsigned int seed, std::int64_t* outExp, int* outMoney,
                                                             std::uint16_t* outQuestKinds, unsigned int* outQuestValues) {
    if (outExp) *outExp = 0;
    if (outMoney) *outMoney = 0;

    if (CanCompleteWarMeritoriousQuest()) return 1;

    const unsigned int rewardExp = GetRewardWarMeritoriousExp();
    const int rewardPt = (emblemSystem_ ? emblemSystem_->GetMeritoriousAdvantage() : 0) + static_cast<int>(kWarPointRewardBase);

    if (outExp) *outExp = rewardExp;
    if (outMoney) *outMoney = rewardPt;

    if (outQuestKinds && outQuestValues) {
        for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
            outQuestKinds[i] = warQuestMonKinds_[i];
            outQuestValues[i] = warQuestMonKills_[i];
        }
    }

    IncreaseMeritoriousPoint(static_cast<std::uint16_t>(std::clamp(rewardPt, 0, 65535)));

    InitCompleteWarMeritoriousQuest();
    if (emblemSystem_) emblemSystem_->OnEvent_CompleteMeritous(seed);
    if (questSystem_) questSystem_->CompleteFunctionQuest(24, outQuestKinds, outQuestValues);
    return 0;
}

unsigned int CMeritoriousSystem::GetRewardWarMeritoriousExp() {
    if (!m_pclExpRewardParser) return 0;
    return m_pclExpRewardParser->GetMeritoriousRewardExp(static_cast<int>(warQuestDifficulty_));
}

unsigned int CMeritoriousSystem::GetRewardWarMeritoriousPoint() {
    return kWarPointRewardBase;
}

unsigned int CMeritoriousSystem::GetRewardSupplyMeritoriousLibi() {
    CSupplyMeritoriousInfo info{};
    if (!GetSupplyInfo(m_pclSupplyMeritoriousParser, supplyQuestKind_, &info)) return 0;
    return info.rewardLibi;
}

unsigned int CMeritoriousSystem::GetRewardSupplyMeritoriousPoint() {
    CSupplyMeritoriousInfo info{};
    if (!GetSupplyInfo(m_pclSupplyMeritoriousParser, supplyQuestKind_, &info)) return 0;
    return info.rewardPoint;
}

unsigned int CMeritoriousSystem::GetSupplyMeritoriousItemCount() {
    CSupplyMeritoriousInfo info{};
    if (!GetSupplyInfo(m_pclSupplyMeritoriousParser, supplyQuestKind_, &info)) return 0;
    return info.itemCount;
}

void CMeritoriousSystem::SetMeritoriousUpGrade(std::uint16_t grade, std::uint16_t gradePoint) {
    grade_ = grade;
    if (specialtySystem_) specialtySystem_->IncreaseSpecialtyPt(gradePoint);
}

unsigned int CMeritoriousSystem::CanCompleteWarMeritoriousQuest() {
    if (!warQuestMonCount_) return 0;
    for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
        if (warQuestMonGoals_[i] == 0) continue;
        if (warQuestMonGoals_[i] > warQuestMonKills_[i]) return 1;
    }
    return 0;
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
    if (!warQuestPlaying_) return 0;
    if (outSize) *outSize = warQuestMonCount_;

    if (outKinds) {
        for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
            outKinds[i] = warQuestMonKinds_[i];
        }
    }

    return warQuestMonCount_ > 0;
}

void CMeritoriousSystem::IncreaseMeritoriousPoint(std::uint16_t point) {
    point_ += point;
    totalPoint_ += point;
}

void CMeritoriousSystem::DecreaseMeritoriousPoint(std::uint16_t point) {
    point_ -= point;
}

int CMeritoriousSystem::IsExistWarQuestMonsterKind(std::uint16_t kind) {
    for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
        if (warQuestMonKinds_[i] == kind) return 1;
    }
    return 0;
}

int CMeritoriousSystem::CanGiveUpWarMeritorious() {
    return warQuestPlaying_ ? 0 : 1;
}

void CMeritoriousSystem::GiveUpWarMeritorious() {
    InitCompleteWarMeritoriousQuest();
    warQuestMonKinds_.fill(0);
    warQuestMonGoals_.fill(0);
    warQuestMonCount_ = 0;
}

int CMeritoriousSystem::CanGiveUpSupplyMeritorious() {
    return supplyQuestKind_ == 0 ? 1 : 0;
}

void CMeritoriousSystem::GiveUpSupplyMeritorious() {
    InitCompleteSupplyMeritoriousQuest();
}

unsigned int CMeritoriousSystem::StartSupplyMeritoriousQuest(std::uint16_t supplyKind) {
    supplyQuestKind_ = supplyKind;
    return 0;
}

int CMeritoriousSystem::CanStartSupplyMeritoriousQuest() {
    return supplyQuestKind_ != 0;
}

void CMeritoriousSystem::CompleteSupplyMeritoriousQuest(int count, cltItemList* itemList, std::uint8_t* outChangedSlots,
                                                        int* outMoney, std::uint16_t* outQuestKinds, unsigned int* outQuestValues) {
    (void)count;
    CSupplyMeritoriousInfo info{};
    if (!GetSupplyInfo(m_pclSupplyMeritoriousParser, supplyQuestKind_, &info) || !inventory_) return;
    inventory_->DelInventoryItemKind(info.itemKind, info.itemCount, itemList, outChangedSlots);
    IncreaseMeritoriousPoint(info.rewardPoint);
    if (outMoney) *outMoney = static_cast<int>(info.rewardLibi);
    if (moneySystem_) moneySystem_->IncreaseMoney(static_cast<int>(info.rewardLibi));
    InitCompleteSupplyMeritoriousQuest();
    if (questSystem_) questSystem_->CompleteFunctionQuest(25, outQuestKinds, outQuestValues);
}

unsigned int CMeritoriousSystem::CanCompleteSupplyMeritoriousQuest(int count) {
    (void)count;
    if (!supplyQuestKind_) return 1;
    CSupplyMeritoriousInfo info{};
    if (!GetSupplyInfo(m_pclSupplyMeritoriousParser, supplyQuestKind_, &info)) return 1;
    if (!info.itemKind || !info.itemCount) return 1;
    if (!inventory_ || !inventory_->CanDelInventoryItemByKindNQty(info.itemKind, info.itemCount)) return 1;
    if (!info.rewardLibi || (moneySystem_ && moneySystem_->CanIncreaseMoney(static_cast<int>(info.rewardLibi)))) return 0;
    return 106;
}

void CMeritoriousSystem::InitCompleteSupplyMeritoriousQuest() {
    supplyQuestKind_ = 0;
}

int CMeritoriousSystem::RewardMeritoriousItem(std::uint16_t itemKind, unsigned int qty) {
    const int can = CanRewardMeritoriousItem(itemKind);
    if (can != 0) return can;
    if (!inventory_) return 1;

    strInventoryItem it{};
    it.itemKind = itemKind;
    it.itemQty = static_cast<std::uint16_t>(std::clamp<unsigned int>(qty, 1, 1));

    const auto requirePoint = static_cast<std::uint16_t>(
        m_pclMeritoriousRewardParser
            ? m_pclMeritoriousRewardParser->GetMeritoriousRewardItemRequirePoint(itemKind)
            : 0);

    const int ret = inventory_->AddInventoryItem(&it, nullptr, nullptr);
    if (ret == 0) {
        DecreaseMeritoriousPoint(requirePoint);
        return 0;
    }

    return ret;
}

int CMeritoriousSystem::RewardMeritoriousItem(std::uint16_t itemKind, unsigned int* outPos, std::uint8_t* outChangedSlots) {
    const int can = CanRewardMeritoriousItem(itemKind);
    if (can != 0) return can;
    if (!inventory_) return 1;

    strInventoryItem it{};
    it.itemKind = itemKind;
    it.itemQty = 1;

    std::uint16_t pos = 0;
    const auto requirePoint = static_cast<std::uint16_t>(
        m_pclMeritoriousRewardParser
            ? m_pclMeritoriousRewardParser->GetMeritoriousRewardItemRequirePoint(itemKind)
            : 0);

    const int ret = inventory_->AddInventoryItem(&it, outChangedSlots, &pos);
    if (outPos) *outPos = pos;
    if (ret == 0) {
        DecreaseMeritoriousPoint(requirePoint);
        return 0;
    }
    return ret;
}

int CMeritoriousSystem::CanRewardMeritoriousItem(std::uint16_t itemKind) {
    if (!inventory_) return 1;
    const auto requirePoint = m_pclMeritoriousRewardParser
                                  ? m_pclMeritoriousRewardParser->GetMeritoriousRewardItemRequirePoint(itemKind)
                                  : 0;
    if (point_ < requirePoint) return 107;
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
    warQuestPlaying_ = warQuestPlaying;
    warQuestDifficulty_ = gradePoint;
    grade_ = static_cast<std::uint8_t>(supplyQuestPlaying);
    supplyQuestKind_ = grade;
    gradePoint_ = 0;
    supplyQuestPlaying_ = 0;

    warQuestMonCount_ = std::min<std::uint16_t>(warQuestMonCount, kMaxQuestMonster);
    warQuestMonKinds_.fill(0);
    warQuestMonGoals_.fill(0);
    warQuestMonKills_.fill(0);

    for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
        SetWarMeritoriousQuest(warQuestMonKinds ? warQuestMonKinds[i] : 0,
                               warQuestMonGoals ? warQuestMonGoals[i] : 0);
    }

}

void CMeritoriousSystem::SetWarMeritoriousQuest(int playing, int difficulty) {
    const auto kind = static_cast<std::uint16_t>(playing);
    const auto goal = static_cast<std::uint16_t>(max(difficulty, 0));
    if (kind == 0) return;

    for (std::uint16_t i = 0; i < warQuestMonCount_; ++i) {
        if (warQuestMonKinds_[i] == kind) {
            if (goal > warQuestMonGoals_[i]) warQuestMonGoals_[i] = goal;
            return;
        }
        if (warQuestMonKinds_[i] == 0) {
            warQuestMonKinds_[i] = kind;
            warQuestMonGoals_[i] = goal;
            warQuestMonKills_[i] = 0;
            return;
        }
    }
}

int CMeritoriousSystem::CalcMeritoriousGrade(std::uint16_t* outGrade, std::uint16_t* outGradePoint) {
    if (!m_pclMeritoriousGradeParser) return 0;
    if (m_pclMeritoriousGradeParser->CalcMeritoriousGrade(point_, grade_, outGrade, outGradePoint) != 1) return 0;
    if (outGrade) grade_ = *outGrade;
    if (specialtySystem_ && outGradePoint) specialtySystem_->IncreaseSpecialtyPt(*outGradePoint);
    return 1;
}
