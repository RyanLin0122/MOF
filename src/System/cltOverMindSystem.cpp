#include "System/cltOverMindSystem.h"

cltOverMindSystem::cltOverMindSystem() = default;

void cltOverMindSystem::Initialize(cltClassSystem* classSystem) {
    overMindStep_ = 0;
    classSystem_ = classSystem;
}

void cltOverMindSystem::Free() {
    overMindStep_ = 0;
    classSystem_ = nullptr;
}

int cltOverMindSystem::CanIncreaseOverMindStep() {
    switch (GetClassOverMindKind()) {
    case 1: return overMindStep_ < 10;
    case 2: return overMindStep_ < 25;
    case 3: return overMindStep_ < 45;
    case 4: return overMindStep_ < 70;
    default: return 0;
    }
}

void cltOverMindSystem::IncreaseOverMindStep() { ++overMindStep_; }
int cltOverMindSystem::GetOverMindStep() { return overMindStep_; }

int cltOverMindSystem::GetOverMindKind() {
    if (overMindStep_ <= 10) return 1;
    if (overMindStep_ > 25) return (overMindStep_ > 45) + 3;
    return 2;
}

int cltOverMindSystem::GetClassOverMindKind() {
    if (!classSystem_) return 0;
    switch (classSystem_->GetClassLevel()) {
    case 1: return 1;
    case 2: return 2;
    case 3: return 3;
    case 4: return 4;
    default: return 0;
    }
}

int cltOverMindSystem::CanUseOverMind() {
    switch (GetOverMindKind()) {
    case 1: return GetOverMindStep() == 10;
    case 2: return GetOverMindStep() == 25;
    case 3: return GetOverMindStep() == 45;
    case 4: return GetOverMindStep() == 70;
    default: return 0;
    }
}

int cltOverMindSystem::UseOverMind() {
    switch (GetOverMindKind()) {
    case 1: overMindStep_ = 0; return 2;
    case 2: overMindStep_ = 10; return 3;
    case 3: overMindStep_ = 25; return 4;
    case 4: overMindStep_ = 45; return 5;
    default: return 1;
    }
}
