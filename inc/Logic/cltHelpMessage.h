#pragma once

class cltHelpMessage {
public:
    cltHelpMessage() = default;
    ~cltHelpMessage() = default;

    // Disables help messages during tutorial.
    void SetDisable();

    // Shows or hides the help overlay according to helpState.
    void IsShow(int helpState);

    // 繪製相關 (ground truth DrawObject_All 呼叫)
    void PrepareDrawing();
    void Draw();
};

extern cltHelpMessage g_clHelpMessage;
