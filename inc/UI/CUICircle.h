#pragma once

// Minimal stub for the circle (guild) UI window referenced by cltChattingMgr.
// mofclient.c calls CUIManager::GetUIWindow(g_UIMgr, 36) and casts the result
// to CUICircle*.  The underlying window is not yet restored, so the lookup
// returns nullptr; GetCircleName() is null-this-safe so the chat manager's
// strlen() check drops through to the "circle not joined" branch.

#include "UI/CUIBase.h"

class CUICircle : public CUIBase {
public:
    CUICircle();
    ~CUICircle() override;

    // Returns the current circle name, or "" when there is no active circle
    // (including the null-this case used when the UI window is absent).
    const char* GetCircleName();
};
