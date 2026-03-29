#pragma once

// 最小宣告，供 CControlChatPrint 使用
// 完整實作待其他翻譯單元還原
class cltChattingMgr
{
public:
    cltChattingMgr();
    ~cltChattingMgr();

    void SetChatBlock(int x, int y);
    void DrawBlockBox();
};
