#include "System/cltTutorialSystem.h"

namespace {
constexpr int kMyAccount = 10;
constexpr int kMonsterAccount = 100;
constexpr int kTutorialMapKind = 0x5001;
constexpr std::uint16_t kTutorialDropPotionKind = 0x292E;
constexpr std::uint16_t kTutorialUseItemKind = 10542;

constexpr int kExitLeft = 1420;
constexpr int kExitTop = 488;
constexpr int kExitRight = 1490;
constexpr int kExitBottom = 535;

constexpr int kStartX = 642;
constexpr int kStartY = 507;

constexpr int kMoveNeedX = 240;
constexpr int kMoveNeedY = 140;

constexpr unsigned int kAttackIntervalMs = 0x320;
constexpr unsigned int kUseItemTimerMs = 0x3E8;
constexpr unsigned int kExitMapTimerMs = 0x7D0;

constexpr int kTutorialAddStepIntro = 1;
constexpr int kTutorialAddStepMoveRight = 3;
constexpr int kTutorialAddStepMoveLeft = 4;
constexpr int kTutorialAddStepMoveUp = 5;
constexpr int kTutorialAddStepMoveDown = 6;
constexpr int kTutorialAddStepAttackDone = 8;
constexpr int kTutorialAddStepPickupDone = 10;
constexpr int kTutorialAddStepUseItemDone = 12;
constexpr int kTutorialAddStepExitReady = 15;
} // namespace

cltTutorialSystem* cltTutorialSystem::s_activeTutorial = nullptr;

cltTutorialSystem::cltTutorialSystem() { ResetRuntimeState(); }
cltTutorialSystem::~cltTutorialSystem() {
    if (s_activeTutorial == this) s_activeTutorial = nullptr;
}

int cltTutorialSystem::InitalizeTutorialSystem(std::uint8_t tutorialType) {
    ResetRuntimeState();
    s_activeTutorial = this;

    InitializeInputAndMessageLayer();
    InitializeQuestAndMeritoriousLayer();
    InitializeHelpAndUIState();

    TutorialProfile profile{};
    BuildProfileFromType(tutorialType, profile);

    BuildInitialInventories();
    BuildInitialEquipment(profile);
    BuildInitialWorld(profile);
    ApplyProfileToCharacter(profile);

    tutorialState_ = 0;
    AddTutorialStep(kTutorialAddStepIntro);
    return 1;
}

int cltTutorialSystem::Poll() {
    if (!tutorialActive_) return 0;

    // simulate game tick
    fakeNowTick_ += 100;
    ProcessTimers();

    switch (tutorialState_) {
    case 1:
        MoveCharacterMission(0);
        return ExitTutorialMap();
    case 2:
        MoveCharacterMission(1);
        return ExitTutorialMap();
    case 3:
        MoveCharacterMission(2);
        return ExitTutorialMap();
    case 4:
        MoveCharacterMission(3);
        return ExitTutorialMap();
    case 5:
        AttackMonster();
        return ExitTutorialMap();
    case 6:
        PickUpItem();
        return ExitTutorialMap();
    case 7:
        UseItem();
        return ExitTutorialMap();
    default:
        return ExitTutorialMap();
    }
}

void cltTutorialSystem::AttackMonster() {
    auto* me = GetMyCharacter();
    if (!me) return;

    if (fakeNowTick_ - lastAttackTick_ < kAttackIntervalMs) return;

    SpawnTrainingMonsterIfNeeded();

    const std::uint32_t monAccount = FindNearestAliveMonsterAccount();
    if (!monAccount) return;

    auto* mon = GetCharacterByAccount(monAccount);
    if (!mon || mon->dead) return;

    const int dx = std::abs(me->x - mon->x);
    const int dy = std::abs(me->y - mon->y);
    if (dx > 70 || dy > 20) return;

    lastAttackTick_ = fakeNowTick_;

    int damage = 5 + (std::rand() % 5);
    damage = std::min(damage, monsterHp_);
    IssueAttackOrder(*me, *mon, damage);

    if (monsterHp_ <= 0) {
        SpawnPotionDropIfNeeded();
        AddTutorialStep(kTutorialAddStepAttackDone);
    }
}

void cltTutorialSystem::PickUpItem() {
    auto* me = GetMyCharacter();
    if (!me) return;

    auto* drop = FindPickupCandidate();
    if (!drop) return;

    // emulate gradual move-to-item like original logic
    const int dx = drop->x - me->x;
    const int dy = drop->y - me->y;
    if (std::abs(dx) > 16) me->x += (dx > 0 ? 16 : -16);
    if (std::abs(dy) > 16) me->y += (dy > 0 ? 16 : -16);

    if (std::abs(drop->x - me->x) <= 10 && std::abs(drop->y - me->y) <= 10) {
        PickupCandidate(*drop);
        AddTutorialStep(kTutorialAddStepPickupDone);
    }
}

void cltTutorialSystem::UseItem() {
    if (!waitingUseItemResult_) return;

    auto* me = GetMyCharacter();
    if (!me) return;

    // tutorial expects HP to recover after potion usage
    if (me->hp > 25) {
        waitingUseItemResult_ = 0;
        timerId_ = CreateTimer(kUseItemTimerMs, TutorialTimer::Callback::EndUseItem);
    }
}

int cltTutorialSystem::ExitTutorialMap() {
    auto* me = GetMyCharacter();
    if (!me) return 0;

    if (!IsInsideExitPortal(*me)) return 0;

    if (timerId_) {
        ReleaseTimer(timerId_);
        timerId_ = 0;
    }

    characters_.clear();
    drops_.clear();
    inventory_[0] = {};
    inventory_[1] = {};

    tutorialActive_ = 0;
    if (s_activeTutorial == this) s_activeTutorial = nullptr;
    return 1;
}

void cltTutorialSystem::SendTutorialMsg(std::uint8_t msgType) {
    switch (msgType) {
    case 2:
    case 0x0D:
        ++tutorialState_;
        break;
    case 7:
        SpawnTrainingMonsterIfNeeded();
        ++tutorialState_;
        break;
    case 0x0B:
        // add a potion item to inventory (ground-truth uses AddInventoryItem)
        for (auto& slot : inventory_) {
            if (slot.itemKind == 0) {
                slot.itemKind = kTutorialDropPotionKind;
                slot.itemQty = 1;
                break;
            }
        }
        ++tutorialState_;
        break;
    case 0x0E:
        timerId_ = CreateTimer(kExitMapTimerMs, TutorialTimer::Callback::StartExitMap);
        break;
    default:
        break;
    }
}

void cltTutorialSystem::MoveCharacterMission(std::uint8_t missionType) {
    auto* me = GetMyCharacter();
    if (!me) return;

    const int step = ShouldAdvanceMissionByMovement(missionType);
    if (step <= 0) return;

    AddTutorialStep(step);
}

void cltTutorialSystem::OnTimer_EndUseItem() {
    if (!s_activeTutorial) return;
    s_activeTutorial->AddTutorialStep(kTutorialAddStepUseItemDone);
}

void cltTutorialSystem::OnTimer_StartExitMap() {
    if (!s_activeTutorial) return;
    ++s_activeTutorial->tutorialState_;
    s_activeTutorial->AddTutorialStep(kTutorialAddStepExitReady);
}

void cltTutorialSystem::ResetRuntimeState() {
    playerPtr_ = 0;
    timerId_ = 0;
    startX_ = 0.0f;
    startY_ = 0.0f;
    tutorialState_ = 0;
    monsterHp_ = 35;
    lastAttackTick_ = 0;
    waitingUseItemResult_ = 0;
    mapKind_ = kTutorialMapKind;
    helpWasVisible_ = 0;
    tutorialActive_ = 0;

    equipPrimary_.fill({});
    equipSecondary_.fill({});
    inventory_.fill({});
    characters_.clear();
    drops_.clear();

    timers_.fill({});
    timerSerial_ = 0;
    fakeNowTick_ = 0;

    ResetProfileCaches();
}

void cltTutorialSystem::ResetProfileCaches() {
    // placeholder for future decomp-aligned cache invalidation,
    // intentionally explicit to mirror large init routine phases.
}

void cltTutorialSystem::BuildProfileFromType(std::uint8_t tutorialType, TutorialProfile& outProfile) {
    // decompiled binary uses table blocks with stride 96 bytes.
    // emulate with deterministic local table for repeatable behavior.
    static const std::array<TutorialProfile, 4> kProfiles = {{
        {0, 1, 0, 0x5001, 1001, kTutorialMapKind, "TutorialWarrior",
         {1001, 1002, 1003, 0, 0, 0, 0, 0, 0, 0, 0},
         {2001, 2002, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
        {1, 2, 1, 0x5002, 1002, kTutorialMapKind, "TutorialArcher",
         {1101, 1102, 1103, 0, 0, 0, 0, 0, 0, 0, 0},
         {2101, 2102, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
        {2, 3, 0, 0x5003, 1003, kTutorialMapKind, "TutorialMage",
         {1201, 1202, 1203, 0, 0, 0, 0, 0, 0, 0, 0},
         {2201, 2202, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
        {3, 4, 1, 0x5004, 1004, kTutorialMapKind, "TutorialPriest",
         {1301, 1302, 1303, 0, 0, 0, 0, 0, 0, 0, 0},
         {2301, 2302, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    }};

    outProfile = kProfiles[tutorialType % kProfiles.size()];
}

void cltTutorialSystem::ApplyProfileToCharacter(const TutorialProfile& profile) {
    auto* me = GetMyCharacter();
    if (!me) return;

    me->mapKind = profile.mapKind;
    me->classKind = profile.classKind;
    me->name = profile.charName;
    me->x = kStartX;
    me->y = kStartY;
    me->hp = 100;
    me->maxHp = 100;
    me->attackPower = monsterHp_;

    startX_ = static_cast<float>(me->x);
    startY_ = static_cast<float>(me->y);
}

void cltTutorialSystem::BuildInitialInventories() {
    // keep explicit and verbose to mirror the original sequence.
    for (auto& slot : inventory_) {
        slot.itemKind = 0;
        slot.itemQty = 0;
        slot.value0 = 0;
        slot.value1 = 0;
    }

    inventory_[0].itemKind = 16396; // starter consumable-like item in ground-truth path
    inventory_[0].itemQty = 1;
    inventory_[1].itemKind = 0;
    inventory_[1].itemQty = 0;
}

void cltTutorialSystem::BuildInitialEquipment(const TutorialProfile& profile) {
    for (std::size_t i = 0; i < equipPrimary_.size(); ++i) {
        equipPrimary_[i].itemKind = profile.equipPrimary[i];
        equipPrimary_[i].itemQty = profile.equipPrimary[i] ? 1 : 0;
    }

    for (std::size_t i = 0; i < equipSecondary_.size(); ++i) {
        equipSecondary_[i].itemKind = profile.equipSecondary[i];
        equipSecondary_[i].itemQty = profile.equipSecondary[i] ? 1 : 0;
    }
}

void cltTutorialSystem::BuildInitialWorld(const TutorialProfile& profile) {
    characters_.clear();

    TutorialCharacter me{};
    me.account = kMyAccount;
    me.x = kStartX;
    me.y = kStartY;
    me.mapKind = profile.mapKind;
    me.hp = 100;
    me.maxHp = 100;
    me.attackPower = 25;
    me.classKind = profile.classKind;
    me.isMonster = 0;
    me.dead = 0;
    me.name = profile.charName;
    characters_.push_back(me);

    playerPtr_ = me.account;
    monsterHp_ = 35;
    tutorialActive_ = 1;
}

void cltTutorialSystem::InitializeInputAndMessageLayer() {
    // In original code this resets key map, saves key setting,
    // creates system message box and pushes 안내 text 8208.
    // Here we preserve the side effects as deterministic state changes.
    PushSystemMessage("Tutorial initialized");
}

void cltTutorialSystem::InitializeQuestAndMeritoriousLayer() {
    // Ground-truth explicitly frees quest and meritorious systems before tutorial.
    // We mimic by resetting local mission-driving state.
    tutorialState_ = 0;
    waitingUseItemResult_ = 0;
}

void cltTutorialSystem::InitializeHelpAndUIState() {
    // Ground-truth disables help overlay during tutorial.
    helpWasVisible_ = 1;
}

void cltTutorialSystem::AddTutorialStep(int stepId) {
    (void)stepId; // step id kept for parity with CUITutorial::AddTutorial.
    ++tutorialState_;
}

void cltTutorialSystem::PushSystemMessage(const std::string& msg) {
    (void)msg;
    // keep function for behavior parity without hard dependency on global UI subsystems.
}

cltTutorialSystem::TutorialCharacter* cltTutorialSystem::GetMyCharacter() {
    return GetCharacterByAccount(playerPtr_);
}

cltTutorialSystem::TutorialCharacter* cltTutorialSystem::GetCharacterByAccount(std::uint32_t account) {
    for (auto& c : characters_) {
        if (static_cast<std::uint32_t>(c.account) == account) return &c;
    }
    return nullptr;
}

std::uint32_t cltTutorialSystem::FindNearestAliveMonsterAccount() const {
    const TutorialCharacter* me = nullptr;
    for (const auto& c : characters_) {
        if (c.account == playerPtr_) {
            me = &c;
            break;
        }
    }
    if (!me) return 0;

    std::uint32_t best = 0;
    int bestDist = 0x7fffffff;
    for (const auto& c : characters_) {
        if (!c.isMonster || c.dead) continue;
        const int dx = std::abs(c.x - me->x);
        const int dy = std::abs(c.y - me->y);
        const int dist = dx + dy;
        if (dist < bestDist) {
            bestDist = dist;
            best = static_cast<std::uint32_t>(c.account);
        }
    }
    return best;
}

void cltTutorialSystem::IssueAttackOrder(TutorialCharacter& me, TutorialCharacter& target, int damage) {
    me.attackPower = std::max(0, me.attackPower - damage);
    monsterHp_ = std::max(0, monsterHp_ - damage);
    target.hp = monsterHp_;
    if (target.hp <= 0) {
        target.dead = 1;
    }
}

void cltTutorialSystem::SpawnTrainingMonsterIfNeeded() {
    for (const auto& c : characters_) {
        if (c.isMonster && !c.dead) return;
    }

    TutorialCharacter mon{};
    mon.account = kMonsterAccount;
    mon.x = 1000;
    mon.y = 510;
    mon.mapKind = mapKind_;
    mon.hp = monsterHp_;
    mon.maxHp = monsterHp_;
    mon.attackPower = 0;
    mon.classKind = 0x4801;
    mon.isMonster = 1;
    mon.dead = 0;
    mon.name = "Tutorial Monster";
    characters_.push_back(mon);
}

void cltTutorialSystem::SpawnPotionDropIfNeeded() {
    for (const auto& d : drops_) {
        if (!d.picked && d.dropId == 100) return;
    }

    TutorialItemDrop drop{};
    drop.owner = static_cast<std::uint32_t>(kMyAccount);
    drop.dropId = 100;
    drop.itemKind = kTutorialDropPotionKind;
    drop.itemQty = 1;
    drop.x = 1000;
    drop.y = 510;
    drop.picked = 0;
    drops_.push_back(drop);
}

cltTutorialSystem::TutorialItemDrop* cltTutorialSystem::FindPickupCandidate() {
    for (auto& d : drops_) {
        if (!d.picked && d.owner == static_cast<std::uint32_t>(kMyAccount)) return &d;
    }
    return nullptr;
}

void cltTutorialSystem::PickupCandidate(TutorialItemDrop& drop) {
    drop.picked = 1;

    for (auto& slot : inventory_) {
        if (slot.itemKind == 0) {
            slot.itemKind = kTutorialUseItemKind;
            slot.itemQty = 1;
            break;
        }
    }

    waitingUseItemResult_ = 1;

    auto* me = GetMyCharacter();
    if (me) {
        me->hp = std::min(me->maxHp, me->hp + 30);
    }

    PushSystemMessage("Picked tutorial item");
}

void cltTutorialSystem::ProcessTimers() {
    for (auto& timer : timers_) {
        if (!timer.enabled) continue;
        if (fakeNowTick_ < timer.dueMs) continue;

        const auto cb = timer.callback;
        timer.enabled = 0;
        timer.callback = TutorialTimer::Callback::None;

        if (cb == TutorialTimer::Callback::EndUseItem) {
            OnTimer_EndUseItem();
        } else if (cb == TutorialTimer::Callback::StartExitMap) {
            OnTimer_StartExitMap();
        }
    }
}

unsigned int cltTutorialSystem::CreateTimer(unsigned int dueMs, TutorialTimer::Callback callback) {
    for (auto& timer : timers_) {
        if (timer.enabled) continue;
        timer.enabled = 1;
        timer.id = ++timerSerial_;
        timer.dueMs = fakeNowTick_ + dueMs;
        timer.callback = callback;
        return timer.id;
    }
    return 0;
}

void cltTutorialSystem::ReleaseTimer(unsigned int timerId) {
    for (auto& timer : timers_) {
        if (!timer.enabled || timer.id != timerId) continue;
        timer.enabled = 0;
        timer.callback = TutorialTimer::Callback::None;
        return;
    }
}

bool cltTutorialSystem::IsInsideExitPortal(const TutorialCharacter& me) const {
    return me.x >= kExitLeft && me.x <= kExitRight && me.y >= kExitTop && me.y <= kExitBottom;
}

int cltTutorialSystem::ShouldAdvanceMissionByMovement(std::uint8_t missionType) const {
    const TutorialCharacter* me = nullptr;
    for (const auto& c : characters_) {
        if (c.account == playerPtr_) {
            me = &c;
            break;
        }
    }
    if (!me) return 0;

    switch (missionType) {
    case 0:
        if (startX_ - static_cast<float>(me->x) > static_cast<float>(kMoveNeedX)) return kTutorialAddStepMoveLeft;
        break;
    case 1:
        if (static_cast<float>(me->x) - startX_ > static_cast<float>(kMoveNeedX)) return kTutorialAddStepMoveRight;
        break;
    case 2:
        if (startY_ - static_cast<float>(me->y) > static_cast<float>(kMoveNeedY)) return kTutorialAddStepMoveUp;
        break;
    case 3:
        if (static_cast<float>(me->y) - startY_ > static_cast<float>(kMoveNeedY)) return kTutorialAddStepMoveDown;
        break;
    default:
        break;
    }
    return 0;
}
