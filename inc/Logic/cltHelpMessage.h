#pragma once

class cltHelpMessage {
public:
    cltHelpMessage() = default;
    ~cltHelpMessage() = default;

    // Disables help messages during tutorial.
    void SetDisable();

    // Shows or hides the help overlay according to helpState.
    void IsShow(int helpState);
};

extern cltHelpMessage g_clHelpMessage;
