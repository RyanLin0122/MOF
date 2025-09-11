#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

class cltMonsterAniInfo
{
public:
    // 與反編譯建構子一致：整體歸零
    cltMonsterAniInfo() { std::memset(this, 0, sizeof(*this)); }

    // 讀取 bigbad.txt；完全讀畢→回傳 1，格式錯誤→回傳 0
    int Initialize(char* filename);

    // 取得影格資訊：a2=動作(0..7), a3=第幾影格
    // 輸出：a4=資源ID(HEX)，a5=參數1（與反編譯 GetFrameInfo 相同，取第一組 WORD 參數）
    int GetFrameInfo(unsigned int a2, unsigned short a3, unsigned int* a4, unsigned short* a5);

    // 回傳該動作的總影格數
    unsigned short GetTotalFrameNum(unsigned int a2);

    // 方便外部存取（若需要）
    enum { kActionCount = 8, kMaxFrames = 100 };
    enum Action {
        STOP = 0, MOVE = 1, DIE = 2, ATTACK = 3,
        N_HITTED = 4, F_HITTED = 5, E_HITTED = 6, I_HITTED = 7
    };

private:
    // 一組動作的資料（對齊反編譯的配置：DWORD 陣列 + 三組 WORD 陣列）
    struct AniSet {
        uint16_t count;                   // 影格數
        uint32_t frameRes[kMaxFrames];    // 影格資源ID (以 %x 讀入)
        uint16_t paramA[kMaxFrames];      // 參數A（GetFrameInfo 會回傳這組）
        uint16_t paramB[kMaxFrames];      // 參數B
        uint16_t paramC[kMaxFrames];      // 參數C
    };

    // 內部小工具
    static bool IsAlphaNumeric(const char* s);
    static bool ieq(const char* a, const char* b); // 忽略大小寫相等

private:
    AniSet    set_[kActionCount];  // 8 組動作，每組最多 100 影格
public:
    uint16_t  attackKey;           // ATTACK_KEY（原程式寫在整體記憶體尾端的 WORD）
};
