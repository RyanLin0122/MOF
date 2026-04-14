#pragma once

#include "MiniGame/Ani_Board.h"

class GameImage;

// mofclient.c 還原：BoardMgr — 管理 10 塊「木板」(Ani_Board) 及左右支柱。
// SetBreakBoard 觸發木板依序碎裂，最後一塊時額外播出計時延遲後的通知。
class BoardMgr {
public:
    BoardMgr();
    ~BoardMgr();

    void InitBoardMgr(float x, float yTop);
    int  SetBreakBoard(int count);
    void SetPositionY(float deltaY);
    char Process(float dt);
    void Render();

private:
    Ani_Board   m_boards[10];      // 10 × Ani_Board
    float       m_x;               // 全部木板的水平中心
    float       m_yCurrent;        // 最上方的 Y（可往下捲動）
    float       m_yInitial;        // 初始 Y（捲動的上限）
    int         m_initialBreakCount;
    int         m_breakCount;      // 剩餘要碎的塊數
    bool        m_lastBreakResult; // 最近一次 Ani_Board::Process 的回傳值
    int         m_currentIdx;      // 目前正在處理的木板索引
    bool        m_dwordPad117;     // (對應原始 +468 byte)
    GameImage*  m_pLeftPole;       // 左側支柱
    GameImage*  m_pRightPole;      // 右側支柱
    bool        m_bFinalBreakTriggered; // 最後一塊碎裂後進入延遲階段
    float       m_finalTimer;      // 延遲累計
};
