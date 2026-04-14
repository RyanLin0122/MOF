#pragma once

#include <cstdint>

// mofclient.c 還原：cltValidMiniGameScore 僅有一個虛擬解構子與一個靜態
// IsValidScore(lessonType, score)，依照小遊戲類型上限/下限驗證分數。
class cltValidMiniGameScore {
public:
    cltValidMiniGameScore();
    virtual ~cltValidMiniGameScore();

    // Ground truth: __cdecl 靜態方法。返回 1 表示分數合法，0 表示不合法。
    static int IsValidScore(std::uint8_t lessonType, std::uint32_t score);
};
