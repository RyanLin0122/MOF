#pragma once
#include <cstdint>
#include <cstdio>
#include "Info/cltMapAreaKindInfo.h"
#include "Info/cltCharKindInfo.h"
#include "global.h"

// ===========================================================================
//  cltExtraRegenMonsterKindInfo  ── 對應檔案：extraregenmonster.txt
//
//  필드아이템박스(Field Item Box) 추가 리젠 몬스터 종류 정보
//  / 額外重生怪物種類資訊（地圖隨機擺放的寶箱怪/事件怪設定）
//
//  反編譯位置：mofclient.c:303359-303607（Initialize / Free / GetByKind /
//  GetByIndex / TranslateKindCode）。
//
//  注意：text_dump_classes.md 將此類別錯誤對應到 opposition.txt；實際
//        對應檔案應為 extraregenmonster.txt（欄位完全吻合）。
// ===========================================================================

// ---------------------------------------------------------------------------
//  stExtraRegenMonster ── 단일 몬스터 슬롯 / 單一怪物項目（8 bytes）
//
//  반편역 (mofclient.c:303474-303491)：
//    v22 = (int *)(v9 + 40);                  // 指向 mob[0].hp
//    *((_WORD *)v22 - 2) = TranslateKindCode  // 寫入 mob[0].charKind = v22-4
//    *v22 = atoi(hp_token);                   // 寫入 mob[0].hp
//    v22 += 2;                                // +8 bytes，跳到下一個 mob
// ---------------------------------------------------------------------------
#pragma pack(push, 1)
struct stExtraRegenMonster {
    uint16_t charKind;  // +0  몬스터 종류코드 / 怪物種類碼  (Jxxxx → cltCharKindInfo::TranslateKindCode)
    uint16_t _pad;      // +2  정렬용 패딩      / 對齊填充    (永遠為 0)
    int32_t  hp;        // +4  지정 HP          / 指定 HP    (atoi)
};
#pragma pack(pop)
static_assert(sizeof(stExtraRegenMonster) == 8, "stExtraRegenMonster must be 8 bytes");

// ---------------------------------------------------------------------------
//  strExtraRegenMonsterKindInfo ── 한 행 데이터 / 單行資料（200 bytes）
//
//  반편역 위치 (mofclient.c:303359-303517)：每行 200 bytes，由 operator new 連續配置。
//  欄位偏移與寫入順序逐字對齊反編譯。每筆資料文件中對應一行（除表頭三行外）。
// ---------------------------------------------------------------------------
#pragma pack(push, 1)
struct strExtraRegenMonsterKindInfo {
    uint16_t kind;           // +0   ID（Fxxxx）           / 主鍵 ID
                             //      반편역：mofclient.c:303427  *(_WORD *)v9 = TranslateKindCode(token)

    uint16_t mapArea[5];     // +2   맵 섹터 ID 배열       / 地圖區域代碼陣列（最多 5 個 Bxxxx）
                             //      반편역：mofclient.c:303500  寫入 v9 + 2 + 2*mapAreaCnt
                             //      解析方式：先把第 3 欄整段 strcpy 到本地 String[]，
                             //      之後以 '|' 拆分，每個 token 經 cltMapAreaKindInfo::TranslateKindCode

    int32_t  mapAreaCnt;     // +12  맵 섹터 수             / 地圖區域已填數
                             //      반편역：寫入「成功經 TranslateKindCode 非零」之數量；
                             //      若拆出的 token 翻譯為 0，slot 仍會寫入 0 但不遞增此計數

    int32_t  initCount;      // +16  초기 리젠 개수         / 初始重生數量（atoi token[3]）
                             //      반편역：mofclient.c:303437  *(_DWORD *)(v9 + 16) = atoi

    int32_t  minRegen;       // +20  min_regen              / 最小再生秒數（atoi token[4]）
                             //      반편역：mofclient.c:303441  *(_DWORD *)(v9 + 20) = atoi

    int32_t  maxRegen;       // +24  max_regen              / 最大再生秒數（atoi token[5]）
                             //      반편역：mofclient.c:303445  *(_DWORD *)(v9 + 24) = atoi

    int32_t  interval;       // +28  interval               / 步進秒數（atoi token[6]，最小 1）
                             //      반편역：mofclient.c:303449-303455
                             //         先寫入原值，再 clamp(>=1) 後重寫；
                             //         若 (max_regen - min_regen) % interval != 0
                             //         反編譯會直接放棄整個剩餘解析（不再前進、不再讀下一行）。

    int32_t  type;           // +32  TYPE                   / 生成類型
                             //      반편역：mofclient.c:303462-303473（_stricmp 比對）：
                             //         "RANDOM" → 1
                             //         "ONCE"   → 2
                             //         其他      → 0  (含 "FIX"、空字串、"RANDOM "(尾隨空格))

    stExtraRegenMonster mob[20]; // +36  몬스터 슬롯 배열   / 怪物項目陣列（每個 8 bytes，共 160 bytes）
                                 //      반편역：v22 從 v9+40 開始（指向 mob[0].hp），
                                 //         每寫一組 (charKind+hp) 之後 v22+=2 (8 bytes)。
                                 //      終止條件：(a) charkind token 翻譯為 0、
                                 //                 (b) i 達到 20、(c) token 已耗盡。

    int32_t  mobCnt;         // +196 몬스터 수              / 已填怪物數
                             //      반편역：每寫入一組成功的 (charKind, hp) 後 ++*(v9+196)。
                             //      mob[i].charKind=0 之後不再遞增（即使 slot 已寫入 0）。
};
#pragma pack(pop)
static_assert(sizeof(strExtraRegenMonsterKindInfo) == 200,
    "strExtraRegenMonsterKindInfo must be 200 bytes");

// ---------------------------------------------------------------------------
//  cltExtraRegenMonsterKindInfo ── 載入器 / 載入容器類別
//
//  반편역 데이터 멤버（mofclient.c 觀察）：
//    *(_DWORD *)this        ── m_items（連續陣列起點，operator new 配置）
//    *((_DWORD *)this + 1)  ── m_count（=== 預掃資料行數）
//
//  반편역 메서드：
//    Initialize                     ── mofclient.c:303359
//    Free                           ── mofclient.c:303542
//    GetExtraRegenMonsterNum        ── mofclient.c:303553
//    GetExtraRegenMonsterKindInfo   ── mofclient.c:303559（線性搜尋）
//    GetExtraRegenMonsterKindInfoByIndex ── mofclient.c:303580
//    TranslateKindCode (static)     ── mofclient.c:303592（與 CharKindInfo /
//                                       MapAreaKindInfo 同 body）
// ---------------------------------------------------------------------------
class cltExtraRegenMonsterKindInfo {
public:
    cltExtraRegenMonsterKindInfo() : m_items(nullptr), m_count(0) {}
    ~cltExtraRegenMonsterKindInfo() { Free(); }

    // 반편역：int __thiscall Initialize(this, char* filename)
    int  Initialize(char* filename);

    // 반편역：void __thiscall Free(this)
    void Free();

    // 반편역：int __thiscall GetExtraRegenMonsterNum(this)
    //   ※ 回傳預掃行數，非實際成功解析筆數。
    int  GetExtraRegenMonsterNum() const { return m_count; }

    // 반편역：str* __thiscall GetExtraRegenMonsterKindInfo(this, uint16_t kind)
    strExtraRegenMonsterKindInfo* GetExtraRegenMonsterKindInfo(uint16_t kind);

    // 반편역：str* __thiscall GetExtraRegenMonsterKindInfoByIndex(this, int idx)
    strExtraRegenMonsterKindInfo* GetExtraRegenMonsterKindInfoByIndex(int idx);

    // 반편역：static uint16_t __cdecl TranslateKindCode(char* s)
    //   장 5 글자, 첫 글자 알파벳 + 4 자리 십진수, atoi(s+1) < 0x800.
    static uint16_t TranslateKindCode(char* s);

    // 便利存取
    inline const strExtraRegenMonsterKindInfo* data() const { return m_items; }
    inline int size() const { return m_count; }

private:
    strExtraRegenMonsterKindInfo* m_items; // +0  연속 배열, 200 bytes/筆
    int                           m_count; // +4  파일의 자료 행 수
};
