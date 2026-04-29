#pragma once
#include <cstdint>
#include <cstddef>

// stCharKindInfo — 還原自 mofclient.c 0x005644D0 (cltCharKindInfo::Initialize)。
// Layout 由解析器逐欄寫入位移推導；總大小恰為 0x118 bytes。
// 所有現存呼叫端 (CToolTip / cltQuestSystem / cltDropItemKindInfo / ...)
// 皆以原始位移 (charInfo + N) 訪問，因此欄位順序、位移與型別必須完全對齊。
#pragma pack(push, 1)
struct stCharKindInfo {
    uint16_t kindCode;             // +0    J-code → TranslateKindCode
    uint16_t nameTextCode;         // +2    캐릭터 이름 (DCTText id)
    uint16_t descTextCode;         // +4    몬스터 설명 (DCTText id)
    uint16_t monsterRegistryKind;  // +6    몬스터 도감용 J-code
    char     plannerName[64];      // +8..71  기획자용 이름 (strcpy)
    char     aniFileName[32];      // +72..103 애니메이션 정보 파일 (strcpy)
    // aniFileName 真實寬度由「下一個欄位 = flags@104」反推。
    uint32_t flags;                // +104  bit0=isPlayer, bit1=isMonster
    uint16_t dropItemKind;         // +108  아이템 그룹 D-code → cltDropItemKindInfo::TranslateKindCode
    uint16_t width;                // +110  캐릭터 Width
    uint16_t height;               // +112  캐릭터 Height
    uint16_t maxHeight;            // +114  max(height, 중점y)
    uint32_t hp;                   // +116  체력
    uint32_t missHit;              // +120  miss hit 값
    uint32_t normalHit;            // +124  normal hit 값
    uint32_t criticalHit;          // +128  critical hit 값 (천분율)
    uint16_t minAtk;               // +132  최소 공격력
    uint16_t maxAtk;               // +134  최대 공격력
    uint16_t def;                  // +136  방어력
    uint8_t  _pad138[2];           // +138  WORD 對齊空隙 (原始無資料)
    uint32_t exp;                  // +140  exp
    uint8_t  moveSpeedType;        // +144  이동 속도 (GetMoveSpeedType)
    uint8_t  attackSpeedType;      // +145  공격속도 (GetAttackSpeedType)，後段才寫入
    uint8_t  level;                // +146  레벨
    char     atkSound[16];         // +147..162  공격사운드 (strcpy)
    char     deadSound[17];        // +163..179  죽음사운드 (strcpy)
    uint16_t ai;                   // +180  몬스터 AI 1..8 (string→enum)
    uint8_t  remainHpRatio;        // +182  남은 체력(비율)
    uint8_t  triggerProb;          // +183  발동 확률
    uint16_t aggroRangeX;          // +184  선공범위x
    uint16_t aggroRangeY;          // +186  선공범위y
    uint16_t helpRangeX;           // +188  헬프범위x
    uint16_t helpRangeY;           // +190  헬프범위y
    uint8_t  nearLong;             // +192  공격 타입 NEAR=0 / LONG=1
    uint8_t  _pad193;
    uint16_t projectileKind;       // +194  발사체 J-code (LONG 必須非零)
    uint32_t atkRangeX;            // +196  공격 범위X
    uint32_t atkRangeY;            // +200  공격 범위Y
    uint32_t monsterAttr;          // +204  몬스터 속성 (cltAttackAtb::GetAttackTargetAtb)
    uint8_t  mineCheck;            // +208  마인 체크
    uint8_t  isClone;              // +209  클론 여부
    uint8_t  _pad210[2];
    uint32_t bossKind;             // +212  보스 여부 (0..2)
    uint32_t deadDelay;            // +216  죽음 딜레이
    uint8_t  isMerit;              // +220  공적 여부
    uint8_t  continent;            // +221  대륙 구분
    uint8_t  _pad222[2];
    uint32_t macroKind;            // +224  매크로 구분
    uint32_t battleKind;           // +228  대항전 구분
    uint32_t eventKind;            // +232  이벤트 구분
    uint32_t fieldItemBoxKind;     // +236  필드아이템박스 구분
    uint32_t skillCount;           // +240  몬스터 스킬 數量
    uint16_t skills[10];           // +244..263  몬스터 스킬 (S-code, max 10)
    uint16_t aniTotalFrames[8];    // +264..279  InitMonsterAinFrame 後填入
};
#pragma pack(pop)
static_assert(sizeof(stCharKindInfo) == 0x118, "stCharKindInfo size must be 0x118 bytes");
static_assert(offsetof(stCharKindInfo, flags) == 104, "stCharKindInfo::flags offset");
static_assert(offsetof(stCharKindInfo, dropItemKind) == 108, "stCharKindInfo::dropItemKind offset");
static_assert(offsetof(stCharKindInfo, level) == 146, "stCharKindInfo::level offset");
static_assert(offsetof(stCharKindInfo, ai) == 180, "stCharKindInfo::ai offset");
static_assert(offsetof(stCharKindInfo, monsterAttr) == 204, "stCharKindInfo::monsterAttr offset");
static_assert(offsetof(stCharKindInfo, isClone) == 209, "stCharKindInfo::isClone offset");
static_assert(offsetof(stCharKindInfo, bossKind) == 212, "stCharKindInfo::bossKind offset");
static_assert(offsetof(stCharKindInfo, deadDelay) == 216, "stCharKindInfo::deadDelay offset");
static_assert(offsetof(stCharKindInfo, fieldItemBoxKind) == 236, "stCharKindInfo::fieldItemBoxKind offset");
static_assert(offsetof(stCharKindInfo, skills) == 244, "stCharKindInfo::skills offset");
static_assert(offsetof(stCharKindInfo, aniTotalFrames) == 264, "stCharKindInfo::aniTotalFrames offset");

class cltCharKindInfo {
public:
    cltCharKindInfo();
    virtual ~cltCharKindInfo();

    // cltCharKindInfo::Initialize / Free are virtual in the binary
    // (vftable slots 1 and 2). cltClientCharKindInfo overrides Initialize.
    virtual int Initialize(char* String2);
    virtual void Free();

    // 5 碼字串 → 16-bit 代碼；失敗回傳 0
    static uint16_t TranslateKindCode(char* a1);

    // 字串 → 速度型別 enum (對應 mofclient.c 0x00565570 / 0x005655E0)
    static uint8_t GetMoveSpeedType(const char* s);
    static uint8_t GetAttackSpeedType(const char* s);

    // 保持與 IDA 還原呼叫面一致；具體資料版型由原始客戶端定義。
    void* GetCharKindInfo(uint16_t kindCode);
    uint16_t GetRealCharID(uint16_t charKind);

    // Returns monster name/info block for the given kind code.
    stCharKindInfo* GetMonsterNameByKind(unsigned short kind);

    // Returns all char kind infos that reference the given drop item kind code.
    // outChars must point to an array of at least 65535 stCharKindInfo* elements.
    // Returns the number of entries written.
    int GetCharKindInfoByDropItemKind(uint16_t dropItemKindCode, stCharKindInfo** outChars);

    // Retrieves monster char kinds within level range [minLv, maxLv] matching nation type.
    int GetMonsterCharKinds(int a2, int a3, int a4, int a5, uint16_t* a6);

    // Checks whether a given kind code represents a monster character.
    int IsMonsterChar(uint16_t kindCode);

    // mofclient.c 292838: bit 0 of DWORD+26 (offset 104) in stCharKindInfo.
    int IsPlayerChar(uint16_t kindCode);

    // mofclient.c 0x00565830：依 kind code 判定是否為「分身」(Clone) 角色。
    // 讀 stCharKindInfo +209 byte。
    unsigned char GetIsClone(uint16_t kindCode);

    // mofclient.c 0x00565860：return *((DWORD*)info + 54) = info+216 (deadDelay)。
    // 反編譯把回傳值還原為 stCharKindInfo*；呼叫端只做 non-null 判定。
    int  GetDieDelayAniByKind(uint16_t kindCode);
    char* GetDeadSound(uint16_t kindCode);

    // Returns boss info for the given kind, or nullptr if not a boss.
    void* GetBossInfoByKind(uint16_t kindCode);

    // mofclient.c 0x005657F0：載入 charkindinfo.txt 全表後，逐筆載入怪物動畫
    // 並回填 aniTotalFrames[8]。對玩家角色不執行。
    void InitMonsterAinFrame();

protected:
    // Storage layout logically equivalent to mofclient.c:
    //   - offset this+4:         65535 stCharKindInfo* slots
    //   - offset this+0x40000:   boss-list buffer pointer (stCharKindInfo**)
    //   - offset this+0x40004:   boss count
    // In this reconstruction the 65535-slot array is heap-allocated to avoid
    // bloating the BSS, and the boss list is a separate allocation.
    stCharKindInfo** m_ppCharKindTable;   // 65536 slots (0..0xFFFF)
    stCharKindInfo** m_pBossList;         // 動態長度 = m_nBossCount
    int              m_nBossCount;
};

// mofclient.c 唯一的全域：g_pcltCharKindInfo (定義於 src/global.cpp)。
// 由 cltCharKindInfo::cltCharKindInfo() 將 this 寫入；Free() 清為 0。
// 實際 instance 是 ClientCharacterManager 內嵌的 cltClientCharKindInfo。
