#include "MiniGame/cltValidMiniGameScore.h"

cltValidMiniGameScore::cltValidMiniGameScore() = default;

cltValidMiniGameScore::~cltValidMiniGameScore() = default;

int cltValidMiniGameScore::IsValidScore(std::uint8_t lessonType, std::uint32_t score)
{
    // mofclient.c 還原：每種小遊戲訓練類型對應不同的合法分數上限/下限。
    switch (lessonType)
    {
        case 0x0Au: // cltMini_Sword（劍術 1）
            if (score > 0x64u) return 0;
            break;
        case 0x0Bu: // cltMini_Sword_2（劍術 2）
            if (score > 0x32u) return 0;
            break;
        case 0x14u: // cltMini_Exorcist（神學 1）
            if (score > 0x320u) return 0;
            break;
        case 0x15u: // cltMini_Exorcist_2（神學 2）
        case 0x1Eu: // cltMini_Bow_2（弓箭 2）
            // mofclient.c 0x5A0040: `if ( a2 >= 0xA ) goto LABEL_16(=valid)`
            // 亦即這兩種小遊戲要求分數至少為 10 才算合法，<10 視為無效。
            if (score < 0x0Au) return 0;
            break;
        case 0x1Fu: // cltMini_Magic（魔法 1）
            if (score > 0x96u) return 0;
            break;
        case 0x28u: // cltMini_Magic_2（魔法 2）
            if (score > 0x78u) return 0;
            break;
        case 0x29u: // 其他
            if (score > 0x46u) return 0;
            break;
        default:
            break;
    }
    return 1;
}
