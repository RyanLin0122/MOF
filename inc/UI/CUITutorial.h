#pragma once

#include "UI/CMessageBoxManager.h"

class CUITutorial : public CUIMessageBoxBase {
public:
    CUITutorial();
    virtual ~CUITutorial();

    // Advances the tutorial UI to the given step.
    void AddTutorial(int stepId);

    // Called when the player cancels / exits the tutorial map.
    // Parameters mirror the ground-truth OnCancel call:
    //   a1       : -1 sentinel
    //   a2-a4    : reserved (0)
    //   exitX/Y  : player position at exit
    void OnCancel(int a1, int a2, int a3, int a4, int exitX, int exitY);
};

extern CUITutorial* g_pUITutorial;
