#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include "Text/cltTextFileManager.h"   // 使用你專案中的 cltTextFileManager (g_clTextFileManager)

// ──────────────────────────────────────────────────────────────────────────
// 外部依賴（前置宣告；請連結你現有的定義）
// ──────────────────────────────────────────────────────────────────────────
struct strClassKindInfo;  // 需能被以下 API 取用之結構
class  cltClassKindInfo {
public:
    // 這些函式會在你的專案內提供真實實作
    static uint16_t TranslateKindCode(const char* s);
    static strClassKindInfo* GetClassKindInfo(cltClassKindInfo* self, uint16_t code);
    static int GetClassKindsByAtb(cltClassKindInfo* self, int64_t atbMask, uint16_t outCodes[64]);
};

class cltMineKindInfo { public: static uint16_t TranslateKindCode(const char* s); };
class cltDebuffKindInfo { public: static uint16_t TranslateKindCode(const char* s); };
class cltAttackAtb {
public:
    static int GetAttackAtb(const char* s);
    static int GetAttackTargetAtb(const char* s);
};

// ──────────────────────────────────────────────────────────────────────────
// 技能記錄：嚴格 400 bytes（與反編譯一致）。
// 我們用原始 byte 區塊，並在 .cpp 以位移寫入/讀取。
// 下面提供常用位移對照（韓文欄位名｜中文說明）
// ──────────────────────────────────────────────────────────────────────────
#pragma pack(push,1)
struct stSkillKindInfo {
    // 生的 400 bytes，內部以 offset 操作（與反編譯一致）
    uint8_t raw[400];
};
/*
  主要位移（擷取反編譯實際使用）：
  +0  : uint16  ID（"ID"｜技能ID，A/P+5位）:contentReference[oaicite:2]{index=2} :contentReference[oaicite:3]{index=3}
  +2  : uint16  초기 ID / 初始ID（上一級或初始碼）
  +4  : uint16  이름 코드 / 名稱代碼
  +6  : uint16  （檔案中數值欄；依表對應）
  +8  : uint32  아이콘 그림 ID（十六進位字串）｜圖標ID
  +12 : uint16  아이콘 그림 블록｜圖標區塊
  +16 : uint32  작은 아이콘 그림 ID（十六進位）｜小圖標ID
  +20 : uint16  작은 아이콘 그림 블록｜小圖標區塊
  +24 : uint32 （被動：數值欄）
  +28 : uint32 （被動：數值欄）
  +32 : uint8   SkillLevel（由鏈條反推：等級數，1 起）※初始化後計算
  +36 : uint32 （主動：數值欄）
  +40 : uint32  習得類別遮罩 L（GetReqClasses 低 32）※與 +44 組成 64-bit
  +44 : uint32  習得類別遮罩 H（GetReqClasses 高 32）
  +48 : uint32  스킬 구매 타입 / 技能購買類型（SWORD/BOW/THEOLOGY/MAGIC/SPECIALTY）
  +52 : uint32  習得點數（FIG / ARC / MAG / CLA 對應四欄）
  +56 : uint32
  +60 : uint32
  +64 : uint32
  +68 : uint16  필요 레벨 / 需要等級
  +70 : uint16  이전/상위 스킬 코드 / 前置或上位技能碼
  +72 : uint8[] 使用時所需武器類別 Flag（GetReqWeaponClasses 以 index 設 1，最多用到 14）
  +88 : uint32  使用時所需武器類別的數量（函式回傳）
  +92 : uint16  EffectKind(1)｜效果代碼（5 碼字頭+數字，見 TranslateEffectKindCode）
  +94 : uint16  EffectKind(2)
  +96 : uint16  EffectKind(3)
  +98 : uint16  EffectKind(4)
  +100: uint16  EffectKind(7)
  +102: uint16  EffectKind(8)
  +104: uint16  EffectKind(9)
  +106: uint16  EffectKind(10)
  +108: uint16  EffectKind(5)
  +110: uint16  EffectKind(6)
  +112: uint8   主動技能效果類型（ATTACK_ONE/…，被動時用作數值欄）
  +114: uint16 （共用數值欄）
  +116: uint16 （共用數值欄）
  +120~160、164~196、200~396：依反編譯序列寫入（含攻擊屬性、Mine、Debuff、各項倍率/時間等）
  +148: char[]  사운드 / 使用音效字串（UseSkillSound 直接回傳這個位移）
*/
#pragma pack(pop)
static_assert(sizeof(stSkillKindInfo) == 400, "stSkillKindInfo must be 400 bytes");

// ──────────────────────────────────────────────────────────────────────────
// 主類別
// ──────────────────────────────────────────────────────────────────────────
class cltSkillKindInfo {
public:
    // 反編譯：設定外部 Class 資訊供查詢
    static void InitializeStaticVariable(cltClassKindInfo* p) { m_pclClassKindInfo = p; }

    cltSkillKindInfo();
    ~cltSkillKindInfo() { Free(); }

    int  Initialize(char* passiveFile, char* activeFile);  // 先 P 再 A；並建立索引與等級
    int  Initialize_P(char* filename);                     // 解析 p_skillinfo.txt
    int  Initialize_A(char* filename);                     // 解析 a_skillinfo.txt
    void Free();

    // 依碼取記錄（位移行為對齊：原本為 *((_DWORD*)this + code)）
    stSkillKindInfo* GetSkillKindInfo(uint16_t code);
    stSkillKindInfo* GetSkillKindInfo_P(uint16_t code);
    stSkillKindInfo* GetSkillKindInfo_A(uint16_t code);
    stSkillKindInfo* GetSkillKindInfo_P() { return m_passive.data(); }
    stSkillKindInfo* GetSkillKindInfo_A() { return m_active.data(); }

    // 數量查詢
    uint16_t GetPassiveSkillNum() const { return static_cast<uint16_t>(m_passive.size()); }
    uint16_t GetActiveSkillNum()  const { return static_cast<uint16_t>(m_active.size()); }
    int      GetTotalSkillNum()   const { return GetPassiveSkillNum() + GetActiveSkillNum(); }

    // 工具（與反編譯一致）
    static bool     IsPassiveSkill(uint16_t code);
    static bool     IsActiveSkill(uint16_t code);
    static uint16_t TranslateKindCode(const char* s);        // 'A'/'P' + 5 digits, 15-bit
    static uint16_t TranslateEffectKindCode(const char* s);  // 5 碼：字母 + 數字，(ch+31)<<11 | num<0x800

    static int      GetWorkingType(const char* s);           // ATTACK_ONCE/… → 1..6
    static uint8_t  GetActiveEffectType(const char* s);      // ATTACK_ONE/… → 1..11
    uint8_t         GetActiveEffectType(uint16_t code);      // 讀取記錄 +112 位移

    static int      GetSkillClass(const char* s);            // SWORD/BOW/THEOLOGY/MAGIC/SPECIALTY → 0..4

    // 來自表格欄位的需求解析
    static uint64_t GetReqClasses(cltSkillKindInfo* self, char* classes);        // 回傳 64-bit 遮罩
    static int      GetReqWeaponClasses(cltSkillKindInfo* self, char* weapons,
        uint8_t* const outFlags /* 指向 +72 位移 */);

    // 額外查詢
    char* UseSkillSound(uint16_t code);                    // 回傳 +148 的字串
    uint16_t GetSkillLevel(uint16_t code);                   // 依「上一級鏈」計算等級（至少 1）
    int      IsLastLevelSkill(uint16_t code);                // 反編譯語意

    // 職業鏈判定（等級層級）
    int IsUnderTwoLevelClassSkill(int64_t atbMask);
    int IsTwoLevelClassSkill(int64_t atbMask);
    int IsUnderThreeLevelClassSkill(int64_t atbMask);
    int IsThreeLevelClassSkill(int64_t atbMask);
    int IsFourLevelClassSkill(int64_t atbMask);

    // 依職業取可用技能（沿職業樹向上蒐集；回傳寫入個數）
    int GetClassActiveSkill(uint16_t startClass, uint16_t* outCodes);
    int GetClassPassiveSkill(uint16_t startClass, uint16_t* outCodes);

    // 靜態外掛：職業資料
    static cltClassKindInfo* m_pclClassKindInfo;

private:
    // 工具
    static bool IsDigits(const char* s);           // 十進位數字
    static bool IsHexLike(const char* s);          // 0-9A-Fa-f
    static bool NextTok(char*& out, const char* delims); // strtok 包裝

    // 內部資料
    std::vector<stSkillKindInfo> m_passive;   // P*
    std::vector<stSkillKindInfo> m_active;    // A*
    stSkillKindInfo* m_idx[65536];// 技能碼 → 記錄指標（含 P/A 全部）
};

// 可選：M 版（按反編譯行為，這裡給最小還原）
class cltSkillKindInfoM {
public:
    cltSkillKindInfoM();
    ~cltSkillKindInfoM() { Free(); }

    int  Initialize(char* pfile, char* afile);
    int  Initialize_P(char* filename);
    int  Initialize_A(char* filename) { (void)filename; return 1; }
    void Free();

    static uint16_t TranslateKindCode(const char* s) { return cltSkillKindInfo::TranslateKindCode(s); }
    static int      IsPassiveSkill(uint16_t c) { return cltSkillKindInfo::IsPassiveSkill(c); }
    static int      IsActiveSkill(uint16_t c) { return cltSkillKindInfo::IsActiveSkill(c); }

private:
    struct Rec28 { uint8_t raw[28]; };
    std::vector<Rec28>   m_pList; // 28 bytes
    std::vector<Rec28>   m_aList; // 未使用
    stSkillKindInfo* m_idx[65536]{};
};

extern cltTextFileManager g_clTextFileManager;