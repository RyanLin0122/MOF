#pragma once

#include <cstdint>
#include <windows.h>

class cltMoF_BaseMiniGame;

// mofclient.c 還原：cltMoF_MiniGame_Mgr — 小遊戲管理器。
// 根據角色當前課程選擇，建立對應的小遊戲物件（Sword/Bow/Magic/Exorcist 等），
// 管理 Poll/PrepareDrawing/Draw 生命週期，並轉發伺服器回覆給底層遊戲物件。
class cltMoF_MiniGame_Mgr {
public:
    cltMoF_MiniGame_Mgr();
    ~cltMoF_MiniGame_Mgr();

    void InitMiniGame();
    int  Poll();
    void PrepareDrawing();
    void Draw();
    void EndMiniGame();
    BOOL IsActive();
    void SetMiniGameResult(int result, unsigned int seed);
    void SetRankingData(std::uint8_t index, char* name, std::uint8_t classKind, int score);
    void SetReceiveRankingData(std::uint8_t count);
    void SetMyRanking(int rank);
    void InvalidScore();

public:
    std::uint16_t          m_gameKind;    // +0: WORD
    cltMoF_BaseMiniGame*   m_pMiniGame;   // +4: DWORD[1] (32-bit) — 當前小遊戲物件
};
