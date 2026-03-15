#include "System/CMeritoriousSystem.h"


cltCharKindInfo* CMeritoriousSystem::m_pclCharKindInfo = nullptr;
CExpRewardParser* CMeritoriousSystem::m_pclExpRewardParser = nullptr;
CMeritoriousGradeParser* CMeritoriousSystem::m_pclMeritoriousGradeParser = nullptr;
CMeritoriousRewardParser* CMeritoriousSystem::m_pclMeritoriousRewardParser = nullptr;
CSupplyMeritoriousParser* CMeritoriousSystem::m_pclSupplyMeritoriousParser = nullptr;
CMonsterGroupPerLevel CMeritoriousSystem::m_clMonsterGroupPerLevel;

void CMeritoriousSystem::InitializeStaticVariable(cltCharKindInfo* a1, CSupplyMeritoriousParser* a2,
                                                  CMeritoriousRewardParser* a3, CMeritoriousGradeParser* a4,
                                                  CExpRewardParser* a5) {
    m_pclCharKindInfo = a1;
    m_pclSupplyMeritoriousParser = a2;
    m_pclMeritoriousRewardParser = a3;
    m_pclMeritoriousGradeParser = a4;
    m_pclExpRewardParser = a5;
}

// Free: 只清除怪物擊殺追蹤 map（ground truth 中 Free 僅清空此 map）
void CMeritoriousSystem::Free() {
    monsterKillMap_.clear();
}

int CMeritoriousSystem::Initialize(cltQuestSystem* questSystem, cltBaseInventory* inventory,
                                   cltSpecialtySystem* specialty, cltMoneySystem* moneySystem,
                                   cltEmblemSystem* emblemSystem,
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

// StartWarMeritoriousQuest: 對每個 questKind 做 map::insert({kind, 0})（重複則不更新）
// 設定 warQuestPlaying_=true、warQuestMapKind_=mapKind
unsigned int CMeritoriousSystem::StartWarMeritoriousQuest(std::uint16_t mapKind, int count,
                                                          const std::uint16_t* questKinds) {
    const auto n = static_cast<std::uint16_t>(count);
    for (std::uint16_t i = 0; i < n; ++i) {
        monsterKillMap_.insert({questKinds[i], 0});
    }
    warQuestPlaying_ = true;
    warQuestMapKind_ = mapKind;
    return 0;
}

// CanStartWarMeritoriousQuest（6 參）:
// 1. warQuestPlaying_ 為 true → return 1
// 2. VerifyingMonsterKinds 失敗 → return 1
// 3. 成功時對 outMonsterInfo 3 個條目套用 emblem advantage 調整後 return 0
unsigned int CMeritoriousSystem::CanStartWarMeritoriousQuest(int minLv, int maxLv,
                                                              std::uint16_t needClass,
                                                              std::uint16_t needNation,
                                                              std::uint16_t monsterKind,
                                                              stMonsterKind* outMonsterInfo) {
    if (warQuestPlaying_) return 1;
    if (!m_clMonsterGroupPerLevel.VerifyingMonsterKinds(minLv, maxLv, needClass, needNation,
                                                        monsterKind, outMonsterInfo))
        return 1;

    const int adv = emblemSystem_->GetWarMetoriousMonsterKillNumAdvantage();
    for (int i = 0; i < 3; ++i) {
        if (outMonsterInfo[i].kind) {
            int cnt = outMonsterInfo[i].count;
            cnt -= static_cast<std::uint16_t>(cnt) * adv / 1000;
            outMonsterInfo[i].count = static_cast<std::uint16_t>(cnt);
        }
    }
    return 0;
}

// CanStartWarMeritoriousQuest（0 參）: warQuestPlaying_ == 1
int CMeritoriousSystem::CanStartWarMeritoriousQuest() {
    return warQuestPlaying_ ? 1 : 0;
}

// PlayWarMeritoriousQuest:
// 1. GetRealCharID 解析實際怪物 ID
// 2. 條件: warQuestPlaying_ && IsExistWarQuestMonsterKind && !IsQuestMonster
// 3. find-or-create map entry
// 4. 若 CMonsterGroupPerLevel 的外部 kill count > 目前已記錄 → 遞增記錄值
void CMeritoriousSystem::PlayWarMeritoriousQuest(int monsterID) {
    const std::uint16_t kind = m_pclCharKindInfo->GetRealCharID(
        static_cast<std::uint16_t>(monsterID));
    if (!warQuestPlaying_) return;
    if (!IsExistWarQuestMonsterKind(kind)) return;
    if (questSystem_->IsQuestMonster(kind)) return;

    auto it = monsterKillMap_.insert({kind, 0}).first;
    const std::uint16_t externalCount =
        m_clMonsterGroupPerLevel.GetMonsterKillCount(warQuestMapKind_, kind);
    if (externalCount > it->second) {
        ++it->second;
    }
}

// CompleteWarMeritoriousQuest（ground truth 執行順序）:
// 1. 初始化輸出為 0
// 2. CanCompleteWarMeritoriousQuest 檢查
// 3. GetRewardWarMeritoriousExp
// 4. 第一次呼叫 GetMeritoriousAdvantage → IncreaseMeritoriousPoint(adv+10)
// 5. InitCompleteWarMeritoriousQuest
// 6. 設定 *outExp
// 7. 第二次呼叫 GetMeritoriousAdvantage → 設定 *outMoney = adv+10
// 8. OnEvent_CompleteMeritous, CompleteFunctionQuest
unsigned int CMeritoriousSystem::CompleteWarMeritoriousQuest(unsigned int seed, std::int64_t* outExp,
                                                              int* outMoney,
                                                              std::uint16_t* outQuestKinds,
                                                              unsigned int* outQuestValues) {
    if (outExp)   *outExp   = 0;
    if (outMoney) *outMoney = 0;

    if (CanCompleteWarMeritoriousQuest()) return 1;

    const unsigned int rewardExp = GetRewardWarMeritoriousExp();
    const int adv1 = emblemSystem_->GetMeritoriousAdvantage();
    IncreaseMeritoriousPoint(static_cast<std::uint16_t>(adv1 + 10));
    InitCompleteWarMeritoriousQuest();

    if (outExp)   *outExp   = rewardExp;
    if (outMoney) *outMoney = emblemSystem_->GetMeritoriousAdvantage() + 10;

    emblemSystem_->OnEvent_CompleteMeritous(seed);
    questSystem_->CompleteFunctionQuest(24, outQuestKinds, outQuestValues);
    return 0;
}

unsigned int CMeritoriousSystem::GetRewardWarMeritoriousExp() {
    if (!m_pclExpRewardParser) return 0;
    return m_pclExpRewardParser->GetMeritoriousRewardExp(
        static_cast<int>(warQuestMapKind_));
}

unsigned int CMeritoriousSystem::GetRewardWarMeritoriousPoint() {
    return 10;
}

// GetRewardSupplyMeritoriousLibi/Point/ItemCount:
// ground truth 直接呼叫 map::operator[]（key 不存在時插入 default entry）
unsigned int CMeritoriousSystem::GetRewardSupplyMeritoriousLibi() {
    return (*m_pclSupplyMeritoriousParser)[supplyQuestKind_].rewardLibi;
}

unsigned int CMeritoriousSystem::GetRewardSupplyMeritoriousPoint() {
    return (*m_pclSupplyMeritoriousParser)[supplyQuestKind_].rewardPoint;
}

unsigned int CMeritoriousSystem::GetSupplyMeritoriousItemCount() {
    return (*m_pclSupplyMeritoriousParser)[supplyQuestKind_].itemCount;
}

void CMeritoriousSystem::SetMeritoriousUpGrade(std::uint16_t grade, std::uint16_t gradePoint) {
    grade_ = grade;
    specialtySystem_->IncreaseSpecialtyPt(gradePoint);
}

// CanCompleteWarMeritoriousQuest:
// map 為空 → return 0（可完成）
// 對每個 entry: 若 kills < GetMonsterKillCount(mapKind,kind) * emblem_adjustment → return 1（不可完成）
// 全部通過 → return 0
unsigned int CMeritoriousSystem::CanCompleteWarMeritoriousQuest() {
    if (monsterKillMap_.empty()) return 0;

    const int adv = emblemSystem_
        ? emblemSystem_->GetWarMetoriousMonsterKillNumAdvantage()
        : 0;

    for (const auto& [kind, kills] : monsterKillMap_) {
        const int goal = m_clMonsterGroupPerLevel.GetMonsterKillCount(warQuestMapKind_, kind);
        if (kills < goal - goal * adv / 1000) return 1;
    }
    return 0;
}

// InitCompleteWarMeritoriousQuest:
// warQuestMapKind_=0, warQuestPlaying_=false, 清空 map（等同 GT 的 Free 操作）
void CMeritoriousSystem::InitCompleteWarMeritoriousQuest() {
    warQuestMapKind_ = 0;
    warQuestPlaying_ = false;
    monsterKillMap_.clear();
}

std::uint16_t CMeritoriousSystem::GetWarMeritoriousQuestMonsterSize() {
    return static_cast<std::uint16_t>(monsterKillMap_.size());
}

// GetWarMeritoriousMonsterKillCount: 回傳 map 中該 kind 的 value（已記錄擊殺數）
std::uint16_t CMeritoriousSystem::GetWarMeritoriousMonsterKillCount(std::uint16_t kind) {
    if (IsExistWarQuestMonsterKind(kind) != 1) return 0;
    return monsterKillMap_.insert({kind, 0}).first->second;
}

// GetWarMeritoriousMonsterGoalKillCount: 動態從 CMonsterGroupPerLevel 取得目標值，套用 emblem 調整
std::uint16_t CMeritoriousSystem::GetWarMeritoriousMonsterGoalKillCount(std::uint16_t kind) {
    if (IsExistWarQuestMonsterKind(kind) != 1) return 0;
    const int goal = m_clMonsterGroupPerLevel.GetMonsterKillCount(warQuestMapKind_, kind);
    const int adv = emblemSystem_
        ? emblemSystem_->GetWarMetoriousMonsterKillNumAdvantage()
        : 0;
    return static_cast<std::uint16_t>(goal - goal * adv / 1000);
}

// GetWarMeritoriousMonsterKind: 收集 map 所有 key → outKinds，設 *outSize，固定 return 1
int CMeritoriousSystem::GetWarMeritoriousMonsterKind(std::uint16_t* outSize, std::uint16_t* outKinds) {
    if (!warQuestPlaying_) return 0;
    std::uint16_t count = 0;
    for (const auto& [kind, kills] : monsterKillMap_) {
        outKinds[count++] = kind;
    }
    *outSize = count;
    return 1;
}

// IncreaseMeritoriousPoint: point_ 與 totalPoint_ 均增加
void CMeritoriousSystem::IncreaseMeritoriousPoint(std::uint16_t point) {
    point_      += point;
    totalPoint_ += point;
}

// DecreaseMeritoriousPoint: ground truth 只減 totalPoint_（offset 4）
void CMeritoriousSystem::DecreaseMeritoriousPoint(std::uint16_t point) {
    totalPoint_ -= point;
}

int CMeritoriousSystem::IsExistWarQuestMonsterKind(std::uint16_t kind) {
    return monsterKillMap_.count(kind) ? 1 : 0;
}

// CanGiveUpWarMeritorious: warQuestPlaying_ != 1（可以放棄）
int CMeritoriousSystem::CanGiveUpWarMeritorious() {
    return warQuestPlaying_ ? 0 : 1;
}

// GiveUpWarMeritorious: 清除整個戰鬥功績狀態（InitCompleteWarMeritoriousQuest 已清空 map）
void CMeritoriousSystem::GiveUpWarMeritorious() {
    InitCompleteWarMeritoriousQuest();
}

// CanGiveUpSupplyMeritorious: supplyQuestKind_ == 0 → 可放棄
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
    return supplyQuestKind_ != 0 ? 1 : 0;
}

// CompleteSupplyMeritoriousQuest: 使用 operator[]（與 GT 的 map::operator[] 行為一致）
void CMeritoriousSystem::CompleteSupplyMeritoriousQuest(int count, cltItemList* itemList,
                                                         std::uint8_t* outChangedSlots,
                                                         int* outMoney,
                                                         std::uint16_t* outQuestKinds,
                                                         unsigned int* outQuestValues) {
    const std::uint16_t itemKind  = (*m_pclSupplyMeritoriousParser)[supplyQuestKind_].itemKind;
    const std::uint16_t itemCount = (*m_pclSupplyMeritoriousParser)[supplyQuestKind_].itemCount;
    inventory_->DelInventoryItemKind(itemKind, itemCount, itemList, outChangedSlots);

    const std::uint16_t rewardPoint = (*m_pclSupplyMeritoriousParser)[supplyQuestKind_].rewardPoint;
    const std::uint32_t rewardLibi  = (*m_pclSupplyMeritoriousParser)[supplyQuestKind_].rewardLibi;

    IncreaseMeritoriousPoint(rewardPoint);
    if (outMoney) *outMoney = static_cast<int>(rewardLibi);
    moneySystem_->IncreaseMoney(static_cast<int>(rewardLibi));
    InitCompleteSupplyMeritoriousQuest();
    questSystem_->CompleteFunctionQuest(25, outQuestKinds, outQuestValues);
}

// CanCompleteSupplyMeritoriousQuest: 使用 operator[]（GT 行為）
unsigned int CMeritoriousSystem::CanCompleteSupplyMeritoriousQuest(int count) {
    if (!supplyQuestKind_) return 1;

    const std::uint16_t itemKind  = (*m_pclSupplyMeritoriousParser)[supplyQuestKind_].itemKind;
    if (!itemKind) return 1;

    const std::uint16_t itemCount = (*m_pclSupplyMeritoriousParser)[supplyQuestKind_].itemCount;
    if (!itemCount) return 1;

    if (!inventory_->CanDelInventoryItemByKindNQty(itemKind, itemCount)) return 1;

    const std::uint32_t rewardLibi = (*m_pclSupplyMeritoriousParser)[supplyQuestKind_].rewardLibi;
    if (!rewardLibi || moneySystem_->CanIncreaseMoney(static_cast<int>(rewardLibi))) return 0;
    return 106;
}

void CMeritoriousSystem::InitCompleteSupplyMeritoriousQuest() {
    supplyQuestKind_ = 0;
}

// RewardMeritoriousItem（2 參）: GT 不檢查 AddInventoryItem 回傳值，直接 DecreaseMeritoriousPoint
int CMeritoriousSystem::RewardMeritoriousItem(std::uint16_t itemKind, unsigned int qty) {
    const int result = CanRewardMeritoriousItem(itemKind);
    if (result) return result;

    const std::uint16_t requirePoint = m_pclMeritoriousRewardParser
        ? static_cast<std::uint16_t>(
              m_pclMeritoriousRewardParser->GetMeritoriousRewardItemRequirePoint(itemKind))
        : 0;

    strInventoryItem it{};
    it.itemKind = itemKind;
    it.itemQty  = 1;
    inventory_->AddInventoryItem(&it, nullptr, nullptr);
    DecreaseMeritoriousPoint(requirePoint);
    return 0;
}

// RewardMeritoriousItem（3 參）: *outPos = requirePoint（GT 語義，非庫存位置）
// AddInventoryItem 第 4 參傳 nullptr（GT 傳 0，不取位置）
// GT 不檢查 AddInventoryItem 回傳值
int CMeritoriousSystem::RewardMeritoriousItem(std::uint16_t itemKind, unsigned int* outPos,
                                               std::uint8_t* outChangedSlots) {
    const int result = CanRewardMeritoriousItem(itemKind);
    if (result) return result;

    const std::uint16_t requirePoint = m_pclMeritoriousRewardParser
        ? static_cast<std::uint16_t>(
              m_pclMeritoriousRewardParser->GetMeritoriousRewardItemRequirePoint(itemKind))
        : 0;

    if (outPos) *outPos = requirePoint;  // GT: *outPos = requirePoint（不是庫存位置）

    strInventoryItem it{};
    it.itemKind = itemKind;
    it.itemQty  = 1;
    inventory_->AddInventoryItem(&it, outChangedSlots, nullptr);  // outPos 傳 nullptr，與 GT 一致
    DecreaseMeritoriousPoint(requirePoint);
    return 0;
}

// CanRewardMeritoriousItem: 檢查 totalPoint_（GT 用 offset 4 = totalPoint_）
int CMeritoriousSystem::CanRewardMeritoriousItem(std::uint16_t itemKind) {
    const unsigned int requirePoint = m_pclMeritoriousRewardParser
        ? m_pclMeritoriousRewardParser->GetMeritoriousRewardItemRequirePoint(itemKind)
        : 0;
    if (totalPoint_ < requirePoint) return 107;
    return inventory_->CanAddInventoryItem(itemKind, 1);
}

// SetMeritoriousInfo: 嚴格對齊 GT 欄位賦值順序，不額外歸零其他欄位
// GT offset mapping:
//   point_         ← point  (DWORD offset 0)
//   totalPoint_    ← totalPoint (DWORD offset 4)
//   warQuestPlaying_ ← warQuestPlaying (BYTE offset 32)
//   warQuestMapKind_ ← gradePoint (WORD offset 10)
//   grade_         ← (uint8_t)supplyQuestPlaying (WORD offset 12)
//   supplyQuestKind_ ← grade (WORD offset 36)
void CMeritoriousSystem::SetMeritoriousInfo(unsigned int point, unsigned int totalPoint,
                                             std::uint16_t grade, std::uint16_t gradePoint,
                                             bool warQuestPlaying, char supplyQuestPlaying,
                                             std::uint16_t warQuestMonCount,
                                             const std::uint16_t* warQuestMonKinds,
                                             const std::uint16_t* warQuestMonGoals) {
    point_            = point;
    totalPoint_       = totalPoint;
    warQuestPlaying_  = warQuestPlaying;
    warQuestMapKind_  = gradePoint;
    grade_            = static_cast<std::uint8_t>(supplyQuestPlaying);
    supplyQuestKind_  = grade;

    if (warQuestMonCount) {
        for (std::uint16_t i = 0; i < warQuestMonCount; ++i) {
            const int kind = warQuestMonKinds ? warQuestMonKinds[i] : 0;
            const int goal = warQuestMonGoals ? warQuestMonGoals[i] : 0;
            SetWarMeritoriousQuest(kind, goal);
        }
    }
}

// SetWarMeritoriousQuest: 標準 map::insert（key 已存在時不更新）
// GT 使用 std::map::insert 語義，不做「若目標更高則更新」的邏輯
void CMeritoriousSystem::SetWarMeritoriousQuest(int kind, int killCount) {
    const auto k = static_cast<std::uint16_t>(kind);
    const auto v = static_cast<std::uint16_t>(killCount < 0 ? 0 : killCount);
    monsterKillMap_.insert({k, v});
}

// CalcMeritoriousGrade: 傳入 (uint16_t)point_（GT 用 low WORD of point_）
int CMeritoriousSystem::CalcMeritoriousGrade(std::uint16_t* outGrade, std::uint16_t* outGradePoint) {
    if (!m_pclMeritoriousGradeParser) return 0;
    if (m_pclMeritoriousGradeParser->CalcMeritoriousGrade(
            static_cast<std::uint16_t>(point_), grade_, outGrade, outGradePoint) != 1)
        return 0;
    grade_ = *outGrade;
    specialtySystem_->IncreaseSpecialtyPt(*outGradePoint);
    return 1;
}
