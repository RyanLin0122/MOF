#pragma once
#include <cstdint>
#include <cstddef>   // offsetof
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"

// ---------------------------------------------------------------------------
// cltMonsterAniInfo
// 韓: 몬스터/캐릭터 애니메이션 정보 (charkindinfo.txt 第 9 欄之 .txt 解析器)
// 中: 怪物／角色動畫資料解析器（解析 player.txt、Auger.txt 等共 ~349 個檔）
//
// Ground truth: mofclient.c
//   - Initialize       (0057FC30) at line 315685
//   - GetFrameInfo     (0057FDF0) at line 315808
//   - GetTotalFrameNum (0057FE80) at line 315827
//   - 呼叫端 InitMonsterAinFrame (00565A70) line 293288：alloca 8036 bytes，
//     證明 sizeof(cltMonsterAniInfo) == 8036 且「沒有 vtable」（與 cltPetAniInfo
//     不同——後者帶 vtable）。
//
// 檔案格式（一行一個動作；每行 0..N 組 [HEX,WORD,WORD,WORD]）：
//     STOP        [00000001,6,0,0][00000001,6,0,0]...
//     MOVE        [...]
//     DIE         [...]
//     ATTACK      [...]
//     N_HITTED    [...]
//     F_HITTED    [...]
//     E_HITTED    [...]
//     I_HITTED    [...]
//     ATTACK_KEY  6
// 區段名（韓/中）：
//   STOP(정지/停止) MOVE(이동/移動) DIE(사망/死亡) ATTACK(공격/攻擊)
//   N_HITTED(일반 피격/普通受擊) F_HITTED(불 피격/火屬受擊)
//   E_HITTED(전기 피격/電屬受擊) I_HITTED(얼음 피격/冰屬受擊)
// 分隔符 (Delim)：'\t' '\n' ',' '[' ']'
// ---------------------------------------------------------------------------
class cltMonsterAniInfo
{
public:
    // 反編譯建構子等同把整體記憶體歸零（v17 也歸零）。沒有 vtable，因此可以
    // 直接 memset(this, 0, sizeof(*this))。
    cltMonsterAniInfo() { std::memset(this, 0, sizeof(*this)); }

    // 解析動畫檔；成功 → 1，任何格式錯誤或 fopen 失敗 → 0
    int Initialize(char* filename);

    // 取得影格資訊
    //   a2 = 動作 (0..7)
    //   a3 = 第幾影格 (0..count-1)
    //   a4 = 輸出資源ID（HEX；對應 GT *((DWORD*)this + 251*a2 + a3 + 1)）
    //   a5 = 輸出參數A（WORD；對應 GT *((WORD*)this + 502*a2 + a3 + 202)）
    int GetFrameInfo(unsigned int a2, unsigned short a3,
                     unsigned int* a4, unsigned short* a5);

    // 該動作的總影格數（WORD；對應 GT *((WORD*)this + 502*a2)）
    unsigned short GetTotalFrameNum(unsigned int a2);

    // 動作上限：8 個區段，每個區段最多 100 影格
    enum { kActionCount = 8, kMaxFrames = 100 };
    enum Action {
        STOP     = 0,  // 韓: 정지       中: 停止
        MOVE     = 1,  // 韓: 이동       中: 移動
        DIE      = 2,  // 韓: 사망       中: 死亡
        ATTACK   = 3,  // 韓: 공격       中: 攻擊
        N_HITTED = 4,  // 韓: 일반 피격  中: 普通屬性受擊
        F_HITTED = 5,  // 韓: 불 피격    中: 火屬性受擊
        E_HITTED = 6,  // 韓: 전기 피격  中: 電屬性受擊
        I_HITTED = 7   // 韓: 얼음 피격  中: 冰屬性受擊
    };

private:
    // -----------------------------------------------------------------------
    // 一組動作的記憶體佈局——必須與 GT byte arithmetic 完全吻合：
    //   *(WORD*) v4               → count
    //   (char*)v4 + 4*count + 4   → frameRes[count]
    //   (WORD*) v4 + count + 202  → paramA[count]   (=byte 404 + 2*count)
    //   (WORD*) v4 + count + 302  → paramB[count]   (=byte 604 + 2*count)
    //   (WORD*) v4 + count + 402  → paramC[count]   (=byte 804 + 2*count)
    // 總尺寸固定 1004 bytes，被 GT 在切換區段時用作步進量
    //   (this+0, +1004, +2008, +3012, +4016, +5020, +6024, +7028)。
    // -----------------------------------------------------------------------
    struct AniSet {
        // 韓: 프레임 개수            中: 已登錄影格數                  offset: 0x000 (=0)
        uint16_t count;

        // 韓: 프레임 리소스 ID (%x)   中: 影格資源ID (HEX，texture+sprite)  offset: 0x004 (=4)
        uint32_t frameRes[kMaxFrames];

        // 韓: 파라미터 A             中: 參數A（GetFrameInfo 回傳此欄）  offset: 0x194 (=404)
        uint16_t paramA[kMaxFrames];

        // 韓: 파라미터 B             中: 參數B                            offset: 0x25C (=604)
        uint16_t paramB[kMaxFrames];

        // 韓: 파라미터 C             中: 參數C                            offset: 0x324 (=804)
        uint16_t paramC[kMaxFrames];
        // sizeof(AniSet) == 1004 (=0x3EC)
    };

    // 內部小工具
    static bool IsAlphaNumeric(const char* s);  // 與 mofclient.c:342945 行為等價
    static bool ieq(const char* a, const char* b);  // __stricmp == 0 ?

private:
    // 韓: 8 개의 동작 데이터 ; 中: 8 組動作資料                            offset: 0x0000 (=0)
    // 8 × sizeof(AniSet)=1004 = 8032 bytes
    AniSet    set_[kActionCount];

public:
    // 韓: ATTACK_KEY (공격 판정 프레임)  中: 攻擊判定影格 (整體尾端 WORD)   offset: 0x1F60 (=8032)
    // GT: *((WORD*)this + 4016) = atoi(...)    (4016*2 == 8032)
    uint16_t  attackKey;
    // sizeof(cltMonsterAniInfo) == 8036（含 2 bytes 對齊 padding）
    // 對應 mofclient.c:293299 的 alloca char v11[8036]，鎖死整體大小。

    // === 鎖死 AniSet 內部 offset，與 GT byte arithmetic 對齊 ===
    static_assert(sizeof(AniSet) == 1004,
                  "AniSet must be 1004 bytes (GT step between sections)");
    static_assert(offsetof(AniSet, count)    == 0,
                  "AniSet::count must be at offset 0 (GT *(WORD*)v4)");
    static_assert(offsetof(AniSet, frameRes) == 4,
                  "AniSet::frameRes must be at offset 4 (GT (char*)v4 + 4*c + 4)");
    static_assert(offsetof(AniSet, paramA)   == 404,
                  "AniSet::paramA must be at offset 404 (GT (WORD*)v4 + c + 202)");
    static_assert(offsetof(AniSet, paramB)   == 604,
                  "AniSet::paramB must be at offset 604 (GT (WORD*)v4 + c + 302)");
    static_assert(offsetof(AniSet, paramC)   == 804,
                  "AniSet::paramC must be at offset 804 (GT (WORD*)v4 + c + 402)");
};

// === 鎖死整體 layout ===
// (a) attackKey 必須位於 8032，對應 GT *((WORD*)this + 4016)
// (b) 整體 sizeof 必須是 8036，對應 GT 呼叫端 alloca v11[8036]
static_assert(offsetof(cltMonsterAniInfo, attackKey) == 8032,
              "attackKey must be at offset 8032 (GT *((WORD*)this + 4016))");
static_assert(sizeof(cltMonsterAniInfo) == 8036,
              "sizeof(cltMonsterAniInfo) must be 8036 (mofclient.c:293299 alloca v11[8036])");
