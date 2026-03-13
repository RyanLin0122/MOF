#pragma once

#include "System/cltClassSystem.h"

class cltOverMindSystem {
public:
    cltOverMindSystem();
    ~cltOverMindSystem() = default;

    void Initialize(cltClassSystem* classSystem);
    void Free();
    int CanIncreaseOverMindStep();
    void IncreaseOverMindStep();
    int GetOverMindStep();
    int GetOverMindKind();
    int GetClassOverMindKind();
    int CanUseOverMind();
    int UseOverMind();

private:
    cltClassSystem* classSystem_ = nullptr;
    int overMindStep_ = 0;
};
