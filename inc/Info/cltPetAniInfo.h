#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

class cltPetAniInfo
{
public:
    // 建構：與反編譯相同，vtable + 後續資料清零
    cltPetAniInfo() { std::memset(reinterpret_cast<char*>(this) + 4, 0, 0x1F60); }
    virtual ~cltPetAniInfo() { /* 僅保留 vtable，與反編譯一致 */ }

    // 讀取 pet_*.txt；成功回傳 1，錯誤回傳 0
    int Initialize(char* filename);

    // 取得影格資訊：a2=動作(0..7), a3=影格索引
    // 輸出：a4=資源ID(HEX)，a5=參數A（對齊反編譯）
    int GetFrameInfo(unsigned int a2, unsigned short a3,
        unsigned int* a4, unsigned short* a5);

    // 回傳該動作的總影格數
    unsigned short GetTotalFrameNum(unsigned int a2);

    // 動作定義與上限
    enum { kActionCount = 8, kMaxFrames = 100 };
    enum Action {
        STOP = 0, MOVE = 1, DIE = 2, ATTACK = 3,
        N_HITTED = 4, F_HITTED = 5, E_HITTED = 6, I_HITTED = 7
    };

    // 供需要時讀取 ATTACK_KEY
    inline uint16_t GetAttackKey() const { return attackKey_; }

private:
    // 一組動作的佈局（與反編譯：每組 1004 bytes）
    struct AniSet {
        uint16_t count;                 // 影格數（WORD）
        uint32_t frameRes[kMaxFrames];  // 影格資源ID（HEX，以 %x 解析）
        uint16_t paramA[kMaxFrames];    // 參數A（WORD）
        uint16_t paramB[kMaxFrames];    // 參數B（WORD）
        uint16_t paramC[kMaxFrames];    // 參數C（WORD）
    };

    static bool IsAlphaNumeric(const char* s);
    static bool ieq(const char* a, const char* b);

private:
    // 注意：在有 vtable 的類別中，set_[0] 會從物件的 +4 偏移開始
    AniSet   set_[kActionCount];  // 8 組 × 1004 bytes = 0x1F60
    uint16_t attackKey_ = 0;      // ATTACK_KEY（存於資料區之後，與反編譯偏移相符）
};
