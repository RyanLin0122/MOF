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
    // 反編譯 (32-bit) 的建構子是 `memset(this + 4, 0, 0x1F60)`，等於「保留
    // vtable，把後面 8 組 AniSet 全部清零」。x64 build 上 vtable 是 8 bytes，
    // 不能直接照搬 +4；改以成員存取清空，offset 由編譯器計算，邏輯等價。
    cltPetAniInfo() {
        std::memset(set_, 0, sizeof(set_));
        attackKey_ = 0;
    }
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
    // 對應 pet_*.txt 一行格式：
    //   <ACTION>\t [HEX, paramA, paramB, paramC] [HEX, paramA, paramB, paramC] ...
    // 區段名（韓/英）：STOP(정지) / MOVE(이동) / DIE(사망) / ATTACK(공격) /
    //                   N_HITTED(일반 피격) / F_HITTED(불 피격) /
    //                   E_HITTED(전기 피격) / I_HITTED(얼음 피격)
    struct AniSet {
        uint16_t count;                 // 韓: 프레임 개수                     中: 影格數(WORD)                                  offset: 0x000
        uint32_t frameRes[kMaxFrames];  // 韓: 프레임 리소스ID (%x)             中: 影格資源ID(HEX, 100 個)                      offset: 0x004 (=4)
        uint16_t paramA[kMaxFrames];    // 韓: 파라미터 A                       中: 參數A(WORD, 100 個)                           offset: 0x194 (=404)
        uint16_t paramB[kMaxFrames];    // 韓: 파라미터 B                       中: 參數B(WORD, 100 個)                           offset: 0x25C (=604)
        uint16_t paramC[kMaxFrames];    // 韓: 파라미터 C                       中: 參數C(WORD, 100 個)                           offset: 0x324 (=804)
    };

    static bool IsAlphaNumeric(const char* s);
    static bool ieq(const char* a, const char* b);

private:
    // 在 32-bit ground truth：set_[0] 從 this+4 開始（vtable=4 bytes）；x64 build
    // 為 this+8。但 GT 的 parser 全程以 byte arithmetic 取址（v5[4*c+4],
    // v5[2*c+404]…），那是「相對 AniSet 起點」的偏移，跟 vtable 大小無關。
    // 因此只要 AniSet 內部欄位 offset 與 GT 一致即可，下方 static_assert 鎖死。
    AniSet   set_[kActionCount];  // 8 組 × 1004 bytes = 8032 bytes
    uint16_t attackKey_ = 0;      // ATTACK_KEY，緊接於資料區之後

    // 鎖死 AniSet 內部 offset：GT parser 依賴 count@0、frameRes@4、
    // paramA@404、paramB@604、paramC@804，sizeof(AniSet)==1004。
    // 任一錯位都會讓 parser 把值寫到錯欄位、後續 GetFrameInfo 拿到亂值。
    static_assert(sizeof(AniSet) == 1004,
                  "AniSet must be 1004 bytes (matches GT stride 1008-4)");
    static_assert(offsetof(AniSet, count)    == 0,   "AniSet::count must be at offset 0");
    static_assert(offsetof(AniSet, frameRes) == 4,   "AniSet::frameRes must be at offset 4 (GT v5[4*c+4])");
    static_assert(offsetof(AniSet, paramA)   == 404, "AniSet::paramA must be at offset 404 (GT v5[2*c+404])");
    static_assert(offsetof(AniSet, paramB)   == 604, "AniSet::paramB must be at offset 604 (GT v5[2*c+604])");
    static_assert(offsetof(AniSet, paramC)   == 804, "AniSet::paramC must be at offset 804 (GT v5[2*c+804])");
};
