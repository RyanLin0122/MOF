#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"

// 其他模組（僅需前置宣告）
class cltPetSkillKindInfo { public: static uint16_t TranslateKindCode(char* s); };

// 內部小工具：判斷字串是否為十進位數字
static inline bool IsDigitStr(const char* s) {
    if (!s || !*s) return false;
    const char* p = s;
    if (*p == '-' || *p == '+') ++p;
    for (; *p; ++p) if (*p < '0' || *p > '9') return false;
    return true;
}

/*
  單筆寵物資料（嚴格 172 bytes）
  偏移對照（// 後面中文為對應欄位）：
   +0  WORD  kind               // pet_ID（Pxxxx）
   +2  WORD  nameText           // 펫 이름(text code)
   +4  WORD  descText           // 펫 설명(text code)
   +6  WORD  prevPetKind        // 前一階寵物 ID（Pxxxx）
   +8  char[32] animGi          // 動作檔名（*.txt）
   +40 WORD  dyeCount           // 염색 수
   +42 WORD  dyeKinds[20]       // 每色對應 ID（最多填充到 +81；常見 4 筆：原/橘/綠/紅）
   +82 WORD  width              // 角色寬
   +84 WORD  height             // 角色高(POS_Y)
   +86 WORD  _padA
   +88 DWORD dotResHex          // 도트 리소스 ID（%x）
   +92 WORD  dotBlockId         // 블록 ID
   +94 WORD  _padB
   +96 DWORD eggResHex          // 펫UI 알 리소스（%x）
   +100 WORD eggBlockId         // 블록ID
   +102 WORD basePassiveSkill   // 기본 패시브 스킬（代碼）
   +104 WORD levelComputed      // 由 GetPetLevel 計算的等級
   +106 WORD _padC
   +108 DWORD loveExp           // LOVE(經驗值)
   +112 WORD  satiety           // 포만감
   +114 WORD  _padD
   +116 DWORD satietyDropPerMin // 포만감 하락치(每分鐘)
   +120 WORD  stageY            // 階段基準Y
   +122 WORD  typeTextCode      // 펫 종류(text code)
   +124 WORD  basicSkillNameTxt // 펫 베이직 스킬 이름(text code)
   +126 WORD  _padE
   +128 DWORD skillObtained     // 스킬획득여부（0/1）
   +132 BYTE  bagInitCount      // 基本給予包數
   +133 BYTE  bagMaxExpand      // 最大擴張包數
   +134 WORD  _padF
   +136 DWORD canRename         // 名稱可變（0/1）
   +140 BYTE  frontOrBack       // 前(0)/後(1)
   +141 BYTE  _padG[3]
   +144 DWORD yAdjust           // 位置Y補正
   +148 DWORD idleResHex        // 停止動畫資源（%x）
   +152 WORD  idleBlockId       // 停止動畫區塊ID
   +154 WORD  _padH
   +156 DWORD releasePrice      // 放生價格
   +160 DWORD releaseSkillUnit  // 放生時「技能一個」單價
   +164 BYTE  petTypeEnum       // BASICPET/DRAGON/... 對應 0..7
   +165 BYTE  _padI
   +166 WORD  effectKind        // 펫 이펙트（代碼或 0）
   +168 WORD  requiredLevel     // 펫 요구레벨（可能缺省）
   +170 WORD  _padJ
*/
#pragma pack(push, 1)
struct strPetKindInfo {
    uint16_t kind;
    uint16_t nameText;
    uint16_t descText;
    uint16_t prevPetKind;
    char     animGi[32];
    uint16_t dyeCount;
    uint16_t dyeKinds[20];
    uint16_t width;
    uint16_t height;
    uint16_t _padA;
    uint32_t dotResHex;
    uint16_t dotBlockId;
    uint16_t _padB;
    uint32_t eggResHex;
    uint16_t eggBlockId;
    uint16_t basePassiveSkill;
    uint16_t levelComputed;
    uint16_t _padC;
    uint32_t loveExp;
    uint16_t satiety;
    uint16_t _padD;
    uint32_t satietyDropPerMin;
    uint16_t stageY;
    uint16_t typeTextCode;
    uint16_t basicSkillNameTxt;
    uint16_t _padE;
    uint32_t skillObtained;
    uint8_t  bagInitCount;
    uint8_t  bagMaxExpand;
    uint16_t _padF;
    uint32_t canRename;
    uint8_t  frontOrBack;
    uint8_t  _padG[3];
    uint32_t yAdjust;
    uint32_t idleResHex;
    uint16_t idleBlockId;
    uint16_t _padH;
    uint32_t releasePrice;
    uint32_t releaseSkillUnit;
    uint8_t  petTypeEnum;
    uint8_t  _padI;
    uint16_t effectKind;
    uint16_t requiredLevel;
    uint16_t _padJ;
};
#pragma pack(pop)
static_assert(sizeof(strPetKindInfo) == 172, "strPetKindInfo must be 172 bytes");

// 染色表（每筆 8 bytes）
#pragma pack(push, 1)
struct strPetDyeKindInfo {
    uint16_t dyeCode;   // Dxxxx -> TranslateKindCode
    uint16_t _pad;      // 對齊
    uint32_t dyeIndex;  // 第三欄「염색 인덱스」
};
#pragma pack(pop)
static_assert(sizeof(strPetDyeKindInfo) == 8, "strPetDyeKindInfo must be 8 bytes");

class cltPetKindInfo {
public:
    // a2: pet 主表檔、a3: 染色表檔
    int Initialize(char* a2, char* a3);

    // 釋放兩張表
    void Free();

    // 由 pet_ID 找記錄
    strPetKindInfo* GetPetKindInfo(uint16_t kind);

    // 由「前一階寵物」找出「下一階的寵物」ID（找 +6 == a2 的那筆，回傳其 +0）
    uint16_t GetNextPetKind(uint16_t prevKind);

    // 取「原色」的寵物 ID（等於 dyeKinds[0]，也就是 offset +42）
    uint16_t GetOriginalPetKind(uint16_t kind);

    // 依代碼找染色表
    strPetDyeKindInfo* GetPetDyeKindInfo(uint16_t kind);

    // 由「寵物分流類別」(0..7) 取「펫 종류(text code)」（+122）
    uint16_t GetPetTypeTextCode(uint8_t typeEnum);

    // 代碼轉換：((toupper(s[0]) + 31) << 11) | atoi(s+1)，長度需為 5 且尾四碼 < 0x800
    static uint16_t TranslateKindCode(char* s);

    // 由「此寵物 ID」往前沿著 prevPetKind 追朔計算層級（直到 prev==0）
    int16_t GetPetLevel(uint16_t kind);

    // 判斷兩個寵物是否在同一條升級鏈上
    int IsSamePet(uint16_t kindA, uint16_t kindB);

    // 反編譯就長這樣（把 +156 當 base，加 a3*+160，再 cast 成指標回傳）
    strPetKindInfo* GetPetReleaseCost(uint16_t kind, uint16_t a3);

private:
    int LoadPetKindInfo(char* filename);
    int LoadPetDyeKindInfo(char* filename);

private:
    // 物件版位（與反編譯一致）
    strPetKindInfo* m_petRows = nullptr; // *(_DWORD*)this
    int               m_petCount = 0;       // *((_DWORD*)this + 1)
    strPetDyeKindInfo* m_dyeRows = nullptr; // *((_DWORD*)this + 2)
    int               m_dyeCount = 0;       // *((_DWORD*)this + 3)
};
