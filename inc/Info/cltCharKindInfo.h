#pragma once
#include <cstdint>
#include <cstddef>

class DCTTextManager;

// ---------------------------------------------------------------------------
// stCharKindInfo — 캐릭터 종류 정보 / 角色種類資料 (sizeof = 0x118 = 280 bytes)
// 還原自 mofclient.c 0x005644D0 (cltCharKindInfo::Initialize, line 292148)。
//
// 對應檔案：text_dump/charkindinfo.txt
//   檔案編碼：cp949 (EUC-KR)；分隔符 "\t\n"；前 3 行為標題列、第 4 行起為資料。
//   每筆資料 47 欄；欄位順序、位移、型別、解析方式皆需與 GT 等價。
//
// 命名規則（每個欄位的註解格式）：
//   韓: <韓文欄位名>      中: <中文欄位名>   解: <解析方式>
//
// 解碼端 (CToolTip / cltQuestSystem / cltDropItemKindInfo / ClientCharacter ...)
// 皆以原始 byte offset (charInfo + N) 訪問，因此 #pragma pack(1) 是強制的。
// ---------------------------------------------------------------------------
#pragma pack(push, 1)
struct stCharKindInfo {
    // ----- col 0 --------------------------------------------------------
    // 韓: monster_ID            中: 角色種類 ID (J-code 解碼後的 16-bit code)
    // 解: cltCharKindInfo::TranslateKindCode("J0001"...) → +0 (WORD)
    uint16_t kindCode;             // +0    (offset 0x000)

    // ----- col 2 --------------------------------------------------------
    // 韓: 캐릭터 이름           中: 角色名稱 (DCTText id, atoi)
    // 解: atoi → +2 (WORD)
    uint16_t nameTextCode;         // +2    (offset 0x002)

    // ----- col 3 --------------------------------------------------------
    // 韓: 몬스터 설명           中: 怪物說明 (DCTText id, atoi)
    // 解: atoi → +4 (WORD)
    uint16_t descTextCode;         // +4    (offset 0x004)

    // ----- col 4 --------------------------------------------------------
    // 韓: 몬스터 도감용         中: 怪物圖鑑用 (主要 J-code，分身 (Clone) 共用一個)
    // 解: TranslateKindCode → +6 (WORD)；當 kindCode==monsterRegistryKind 時為主要 entry
    uint16_t monsterRegistryKind;  // +6    (offset 0x006)

    // ----- col 1 --------------------------------------------------------
    // 韓: 캐릭터이름(기획자용)  中: 企劃用名稱 (UI 不顯示；GetCharID 線性搜尋用)
    // 解: strcpy → +8 (max 64 bytes incl. NUL)
    char     plannerName[64];      // +8..71 (offset 0x008..0x047)

    // ----- col 6 --------------------------------------------------------
    // 韓: 애니메이션 정보 파일  中: 動畫資訊檔 (xxx.txt)
    // 解: strstr(".txt") 檢查 → strcpy → +72 (max 32 bytes)
    //     被 cltMonsterAniInfo::Initialize 與 cltClientCharKindInfo::GetMonsterAniInfo 使用
    char     aniFileName[32];      // +72..103 (offset 0x048..0x067)

    // ----- col 5 --------------------------------------------------------
    // 韓: 몬스터 여부 (Y/N)     中: 是否為怪物
    // 解: toupper(*tok)=='Y' ? flags|=2 : flags|=1   (僅修改低位元組)
    //     bit 0 = isPlayer (cltCharKindInfo::IsPlayerChar)
    //     bit 1 = isMonster (cltCharKindInfo::IsMonsterChar)
    uint32_t flags;                // +104   (offset 0x068)

    // ----- col 7 --------------------------------------------------------
    // 韓: 아이템 그룹 (D-code)  中: 掉落物品群組
    // 解: 若不為 "0" → cltDropItemKindInfo::TranslateKindCode → +108 (WORD)
    uint16_t dropItemKind;         // +108   (offset 0x06C)

    // ----- col 8 --------------------------------------------------------
    // 韓: 캐릭터 Width          中: 角色寬度 (px)
    // 解: atoi → +110 (WORD)；GT GetCharWidthA 讀此欄位
    uint16_t width;                // +110   (offset 0x06E)

    // ----- col 9 --------------------------------------------------------
    // 韓: 캐릭터 Height         中: 角色高度 (px)
    // 解: atoi → +112 (WORD)；GT GetCharHeight 讀此欄位
    uint16_t height;               // +112   (offset 0x070)

    // ----- col 10 -------------------------------------------------------
    // 韓: 중점y                 中: 中心點 Y (用於 InfoPosY)
    // 解: atoi(centerY) → maxHeight = max(height, centerY) → +114 (WORD)
    //     GT GetCharInfoPosY 讀此欄位
    uint16_t maxHeight;            // +114   (offset 0x072)

    // ----- col 11 -------------------------------------------------------
    // 韓: 체력                  中: 生命值
    // 解: atoi → +116 (DWORD)
    uint32_t hp;                   // +116   (offset 0x074)

    // ----- col 12 -------------------------------------------------------
    // 韓: miss hit 값           中: 完全閃避值 (千分率)
    // 解: atoi → +120 (DWORD)
    uint32_t missHit;              // +120   (offset 0x078)

    // ----- col 13 -------------------------------------------------------
    // 韓: normal hit 값         中: 一般命中值 (千分率)
    // 解: atoi → +124 (DWORD)
    uint32_t normalHit;            // +124   (offset 0x07C)

    // ----- col 14 -------------------------------------------------------
    // 韓: critical hit 값(천분율) 中: 暴擊值 (千分率)
    // 解: atoi → +128 (DWORD)
    uint32_t criticalHit;          // +128   (offset 0x080)

    // ----- col 15 -------------------------------------------------------
    // 韓: 최소 공격력           中: 最小攻擊力
    // 解: atoi → +132 (WORD)
    uint16_t minAtk;               // +132   (offset 0x084)

    // ----- col 16 -------------------------------------------------------
    // 韓: 최대 공격력           中: 最大攻擊力
    // 解: atoi → +134 (WORD)
    uint16_t maxAtk;               // +134   (offset 0x086)

    // ----- col 17 -------------------------------------------------------
    // 韓: 방어력                中: 防禦力
    // 解: atoi → +136 (WORD)
    uint16_t def;                  // +136   (offset 0x088)

    // GT layout 在 +138 留空；exp@+140 為下一個 DWORD-aligned 欄位。
    uint8_t  _pad138[2];           // +138..139 (offset 0x08A..0x08B) padding

    // ----- col 18 -------------------------------------------------------
    // 韓: exp                   中: 經驗值
    // 解: atoi → +140 (DWORD)
    uint32_t exp;                  // +140   (offset 0x08C)

    // ----- col 19 -------------------------------------------------------
    // 韓: 이동 속도             中: 移動速度型別
    // 解: GetMoveSpeedType → +144 (BYTE)
    //     STATIONARY=1 / VERY SLOW=2 / SLOW=3 / NORMAL=4 / FAST=5 / VERY FAST=6
    //     未知 → 4 (NORMAL)；GetMoveSpeedConstantByType 換算為 0/1/3/5/7/10
    uint8_t  moveSpeedType;        // +144   (offset 0x090)

    // ----- col 31 -------------------------------------------------------
    // 韓: 공격속도              中: 攻擊速度型別 (在解析中於後段才寫入)
    // 解: GetAttackSpeedType → +145 (BYTE)
    //     SLOWEST=1 / SLOW=2 / NORMAL=3 / FAST=4 / FASTEST=5
    //     未知 → 4 (low-byte truncation 行為)；GetAttackDelayTimeByAttackSpeedType
    //     換算為 3000/2500/2000/1500/1000 ms
    uint8_t  attackSpeedType;      // +145   (offset 0x091)

    // ----- col 20 -------------------------------------------------------
    // 韓: 레벨                  中: 等級
    // 解: atoi → +146 (BYTE)；GetCharLevel 讀此欄位
    uint8_t  level;                // +146   (offset 0x092)

    // ----- col 21 -------------------------------------------------------
    // 韓: 공격사운드            中: 攻擊音效 (e.g. "D0030"，0=無)
    // 解: strcpy → +147 (16 bytes incl. NUL)；GetAttackSound 回傳此欄位指標
    char     atkSound[16];         // +147..162 (offset 0x093..0x0A2)

    // ----- col 22 -------------------------------------------------------
    // 韓: 죽음사운드            中: 死亡音效 (e.g. "C0001")
    // 解: strcpy → +163 (17 bytes incl. NUL)；GetDeadSound 回傳此欄位指標
    char     deadSound[17];        // +163..179 (offset 0x0A3..0x0B3)

    // ----- col 23 -------------------------------------------------------
    // 韓: 몬스터 AI             中: 怪物 AI 型別 (字串→1..8)
    // 解: stricmp 比對 → +180 (WORD)
    //     N_C_OFFEN=1, C_OFFEN=2, N_C_OFFEN_REINFORCE=3, C_OFFEN_REINFORCE=4,
    //     N_C_OFFEN_R_AWAY=5, C_OFFEN_R_AWAY=6, N_C_OFFEN_RACE=7, C_OFFEN_RACE=8
    //     "0" → 0；其他 → MessageBox + 不寫入 (保持 memset 0)
    uint16_t ai;                   // +180   (offset 0x0B4)

    // ----- col 24 -------------------------------------------------------
    // 韓: 남은 체력(비율)       中: 剩餘 HP 比例 (觸發追擊條件)
    // 解: atoi → +182 (BYTE)
    uint8_t  remainHpRatio;        // +182   (offset 0x0B6)

    // ----- col 25 -------------------------------------------------------
    // 韓: 발동 확률             中: 觸發機率 (%)
    // 解: atoi → +183 (BYTE)
    uint8_t  triggerProb;          // +183   (offset 0x0B7)

    // ----- col 26 -------------------------------------------------------
    // 韓: 선공범위x             中: 主動攻擊範圍 X
    // 解: atoi → +184 (WORD)；GetFirstHitMonsterRangeX 讀此欄位
    uint16_t aggroRangeX;          // +184   (offset 0x0B8)

    // ----- col 27 -------------------------------------------------------
    // 韓: 선공범위y             中: 主動攻擊範圍 Y
    // 解: atoi → +186 (WORD)；GetFirstHitMonsterRangeY 讀此欄位
    uint16_t aggroRangeY;          // +186   (offset 0x0BA)

    // ----- col 28 -------------------------------------------------------
    // 韓: 헬프범위x             中: 求救範圍 X (受擊時呼叫鄰近怪物)
    // 解: atoi → +188 (WORD)；GetMonsterHelpReqRangeX 讀此欄位
    uint16_t helpRangeX;           // +188   (offset 0x0BC)

    // ----- col 29 -------------------------------------------------------
    // 韓: 헬프범위y             中: 求救範圍 Y
    // 解: atoi → +190 (WORD)；GetMonsterHelpReqRangeY 讀此欄位
    uint16_t helpRangeY;           // +190   (offset 0x0BE)

    // ----- col 34 -------------------------------------------------------
    // 韓: 공격 타입             中: 攻擊類型
    // 解: strcmp("NEAR")=0 → 0；strcmp("LONG")=0 → 1；其他 → break (parse error)
    //     直接寫 +192 (BYTE)
    uint8_t  nearLong;             // +192   (offset 0x0C0)

    uint8_t  _pad193;              // +193   (offset 0x0C1) WORD-align padding

    // ----- col 35 -------------------------------------------------------
    // 韓: 발사체                中: 投射物 J-code (LONG 攻擊必須非零，否則 break)
    // 解: TranslateKindCode → +194 (WORD)
    uint16_t projectileKind;       // +194   (offset 0x0C2)

    // ----- col 32 -------------------------------------------------------
    // 韓: 공격 범위X            中: 攻擊範圍 X
    // 解: atoi → +196 (DWORD)
    uint32_t atkRangeX;            // +196   (offset 0x0C4)

    // ----- col 33 -------------------------------------------------------
    // 韓: 공격 범위Y            中: 攻擊範圍 Y
    // 解: atoi → +200 (DWORD)
    uint32_t atkRangeY;            // +200   (offset 0x0C8)

    // ----- col 30 -------------------------------------------------------
    // 韓: 몬스터 속성           中: 怪物屬性
    // 解: cltAttackAtb::GetAttackTargetAtb → +204 (DWORD)
    //     MONSTER_ETC=0, MONSTER_FIRE=1, MONSTER_ICE=2, MONSTER_LIGHTNING=3,
    //     BEAST=4, UNDEAD=5, DEMON=6, DRAGON=7；未知 → 0
    uint32_t monsterAttr;          // +204   (offset 0x0CC)

    // ----- col 36 -------------------------------------------------------
    // 韓: 마인 체크             中: 是否為飛行怪 (Mine 檢查) — IDA 命名為 GetFlyMonsterByKind
    // 解: atoi → +208 (BYTE)
    uint8_t  mineCheck;            // +208   (offset 0x0D0)

    // ----- col 37 -------------------------------------------------------
    // 韓: 클론 여부             中: 是否為分身 (Clone)；1=clone (不出現於名稱搜尋)
    // 解: atoi → +209 (BYTE)；GT GetCharID2 跳過 isClone==1 的條目
    uint8_t  isClone;              // +209   (offset 0x0D1)

    uint8_t  _pad210[2];           // +210..211 (offset 0x0D2..0x0D3) DWORD-align padding

    // ----- col 38 -------------------------------------------------------
    // 韓: 보스 여부             中: 是否為 BOSS (0/1/2；>2 視為解析錯誤)
    // 解: atoi → +212 (DWORD)；非 0 且 kindCode==monsterRegistryKind 時 ++bossCount
    //     GT GetBossInfoByKind 讀此欄位（轉型為指標供 non-null 判定）
    uint32_t bossKind;             // +212   (offset 0x0D4)

    // ----- col 39 -------------------------------------------------------
    // 韓: 죽음 딜레이           中: 死亡延遲 (動畫/物件回收 timing)
    // 解: atoi → +216 (DWORD)；GT GetDieDelayAniByKind 讀此欄位
    uint32_t deadDelay;            // +216   (offset 0x0D8)

    // ----- col 40 -------------------------------------------------------
    // 韓: 공적 여부             中: 公績 (continent/livearea filter，1 或 2)
    // 解: atoi → +220 (BYTE)；GT GetMonsterCharKinds 用此欄位過濾
    uint8_t  isMerit;              // +220   (offset 0x0DC)

    // ----- col 41 -------------------------------------------------------
    // 韓: 대륙 구분             中: 大陸區分；GT GetLiveAreaInfo 讀此欄位
    // 解: atoi → +221 (BYTE)
    uint8_t  continent;            // +221   (offset 0x0DD)

    uint8_t  _pad222[2];           // +222..223 (offset 0x0DE..0x0DF) DWORD-align padding

    // ----- col 42 -------------------------------------------------------
    // 韓: 매크로 구분           中: 巨集區分
    // 解: atoi → +224 (DWORD)
    uint32_t macroKind;            // +224   (offset 0x0E0)

    // ----- col 43 -------------------------------------------------------
    // 韓: 대항전 구분           中: 對抗戰區分
    // 解: atoi → +228 (DWORD)
    uint32_t battleKind;           // +228   (offset 0x0E4)

    // ----- col 44 -------------------------------------------------------
    // 韓: 이벤트 구분           中: 事件區分
    // 解: atoi → +232 (DWORD)
    uint32_t eventKind;            // +232   (offset 0x0E8)

    // ----- col 45 -------------------------------------------------------
    // 韓: 필드아이템박스 구분   中: 場上寶箱區分
    // 解: atoi → +236 (DWORD)
    //     cltClientCharKindInfo::IsFieldItemBox 直接讀此 DWORD 並轉型為指標
    //     供 non-null 判定 (即此值非 0 表為寶箱)
    uint32_t fieldItemBoxKind;     // +236   (offset 0x0EC)

    // ----- 解析狀態 ------------------------------------------------------
    // 韓: 몬스터 스킬 個數      中: 怪物技能計數 (0..10)
    // 解: 在解析 col 46 時，每寫入一個 skill 即 ++skillCount；達到 10 時
    //     `goto LABEL_138` (skill overflow → return 0)
    uint32_t skillCount;           // +240   (offset 0x0F0)

    // ----- col 46 -------------------------------------------------------
    // 韓: 몬스터 스킬           中: 怪物技能列表 ("S0011|S0012|..." 或 "0")
    // 解: 若不為 "0"：以 '|' 切割，每個 token → cltSkillKindInfoM::TranslateKindCode
    //     寫入 skills[skillCount++]；最多 10 個。
    uint16_t skills[10];           // +244..263 (offset 0x0F4..0x107)

    // ----- 解析後填入 ----------------------------------------------------
    // 韓: 애니메이션 총 프레임  中: 8 個動作的總影格數
    // 解: cltCharKindInfo::InitMonsterAinFrame 對非玩家 entry 載入 aniFileName，
    //     讀取 cltMonsterAniInfo::GetTotalFrameNum(0..7) 填入此陣列。
    //     GT GetAniTotalFrame 讀此欄位（a3 索引 0..7，>=8 回 0）
    //     index 0=STOP, 1=MOVE, 2=DIE, 3=ATTACK,
    //           4=N_HITTED, 5=F_HITTED, 6=E_HITTED, 7=I_HITTED
    uint16_t aniTotalFrames[8];    // +264..279 (offset 0x108..0x117)
};
#pragma pack(pop)
// ---------------------------------------------------------------------------
// 結構大小與關鍵欄位的位移驗證 — 必須與 mofclient.c 反編譯之 byte arithmetic
// (例如 *((WORD*)info + 92) = info+184 = aggroRangeX) 完全吻合。
// ---------------------------------------------------------------------------
static_assert(sizeof(stCharKindInfo) == 0x118, "stCharKindInfo size must be 0x118 bytes");
static_assert(offsetof(stCharKindInfo, kindCode)            ==   0, "+0   kindCode");
static_assert(offsetof(stCharKindInfo, nameTextCode)        ==   2, "+2   nameTextCode");
static_assert(offsetof(stCharKindInfo, descTextCode)        ==   4, "+4   descTextCode");
static_assert(offsetof(stCharKindInfo, monsterRegistryKind) ==   6, "+6   monsterRegistryKind");
static_assert(offsetof(stCharKindInfo, plannerName)         ==   8, "+8   plannerName");
static_assert(offsetof(stCharKindInfo, aniFileName)         ==  72, "+72  aniFileName");
static_assert(offsetof(stCharKindInfo, flags)               == 104, "+104 flags");
static_assert(offsetof(stCharKindInfo, dropItemKind)        == 108, "+108 dropItemKind");
static_assert(offsetof(stCharKindInfo, width)               == 110, "+110 width");
static_assert(offsetof(stCharKindInfo, height)              == 112, "+112 height");
static_assert(offsetof(stCharKindInfo, maxHeight)           == 114, "+114 maxHeight");
static_assert(offsetof(stCharKindInfo, hp)                  == 116, "+116 hp");
static_assert(offsetof(stCharKindInfo, missHit)             == 120, "+120 missHit");
static_assert(offsetof(stCharKindInfo, normalHit)           == 124, "+124 normalHit");
static_assert(offsetof(stCharKindInfo, criticalHit)         == 128, "+128 criticalHit");
static_assert(offsetof(stCharKindInfo, minAtk)              == 132, "+132 minAtk");
static_assert(offsetof(stCharKindInfo, maxAtk)              == 134, "+134 maxAtk");
static_assert(offsetof(stCharKindInfo, def)                 == 136, "+136 def");
static_assert(offsetof(stCharKindInfo, exp)                 == 140, "+140 exp");
static_assert(offsetof(stCharKindInfo, moveSpeedType)       == 144, "+144 moveSpeedType");
static_assert(offsetof(stCharKindInfo, attackSpeedType)     == 145, "+145 attackSpeedType");
static_assert(offsetof(stCharKindInfo, level)               == 146, "+146 level");
static_assert(offsetof(stCharKindInfo, atkSound)            == 147, "+147 atkSound");
static_assert(offsetof(stCharKindInfo, deadSound)           == 163, "+163 deadSound");
static_assert(offsetof(stCharKindInfo, ai)                  == 180, "+180 ai");
static_assert(offsetof(stCharKindInfo, remainHpRatio)       == 182, "+182 remainHpRatio");
static_assert(offsetof(stCharKindInfo, triggerProb)         == 183, "+183 triggerProb");
static_assert(offsetof(stCharKindInfo, aggroRangeX)         == 184, "+184 aggroRangeX");
static_assert(offsetof(stCharKindInfo, aggroRangeY)         == 186, "+186 aggroRangeY");
static_assert(offsetof(stCharKindInfo, helpRangeX)          == 188, "+188 helpRangeX");
static_assert(offsetof(stCharKindInfo, helpRangeY)          == 190, "+190 helpRangeY");
static_assert(offsetof(stCharKindInfo, nearLong)            == 192, "+192 nearLong");
static_assert(offsetof(stCharKindInfo, projectileKind)      == 194, "+194 projectileKind");
static_assert(offsetof(stCharKindInfo, atkRangeX)           == 196, "+196 atkRangeX");
static_assert(offsetof(stCharKindInfo, atkRangeY)           == 200, "+200 atkRangeY");
static_assert(offsetof(stCharKindInfo, monsterAttr)         == 204, "+204 monsterAttr");
static_assert(offsetof(stCharKindInfo, mineCheck)           == 208, "+208 mineCheck");
static_assert(offsetof(stCharKindInfo, isClone)             == 209, "+209 isClone");
static_assert(offsetof(stCharKindInfo, bossKind)            == 212, "+212 bossKind");
static_assert(offsetof(stCharKindInfo, deadDelay)           == 216, "+216 deadDelay");
static_assert(offsetof(stCharKindInfo, isMerit)             == 220, "+220 isMerit");
static_assert(offsetof(stCharKindInfo, continent)           == 221, "+221 continent");
static_assert(offsetof(stCharKindInfo, macroKind)           == 224, "+224 macroKind");
static_assert(offsetof(stCharKindInfo, battleKind)          == 228, "+228 battleKind");
static_assert(offsetof(stCharKindInfo, eventKind)           == 232, "+232 eventKind");
static_assert(offsetof(stCharKindInfo, fieldItemBoxKind)    == 236, "+236 fieldItemBoxKind");
static_assert(offsetof(stCharKindInfo, skillCount)          == 240, "+240 skillCount");
static_assert(offsetof(stCharKindInfo, skills)              == 244, "+244 skills");
static_assert(offsetof(stCharKindInfo, aniTotalFrames)      == 264, "+264 aniTotalFrames");

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
    // GT 0x005655E0 回傳 int (FASTEST=5、SLOWEST=1、SLOW=2、NORMAL=3、FAST=4)。
    static int     GetAttackSpeedType(const char* s);

    // GT 0x005655A0 / 0x00565660：speedType → 移動 / 攻擊延遲常數。
    static uint8_t      GetMoveSpeedConstantByType(uint8_t a1);
    static unsigned int GetAttackDelayTimeByAttackSpeedType(uint8_t a1);

    // 保持與 IDA 還原呼叫面一致；具體資料版型由原始客戶端定義。
    void* GetCharKindInfo(uint16_t kindCode);
    uint16_t GetRealCharID(uint16_t charKind);

    // GT 0x00565270：return (stCharKindInfo**)((char*)this + 4) — table head ptr。
    stCharKindInfo** GetCharInfo();

    // GT 0x00565330：5 碼字串 → cltItemKindInfo::TranslateKindCode → table lookup。
    stCharKindInfo* GetCharKindInfoByStringKindCode(char* a2);

    // GT 0x00565410..0x00565470：char kind 個別欄位 helper。
    uint16_t GetCharWidthA(uint16_t kindCode);
    uint16_t GetCharHeight(uint16_t kindCode);
    uint16_t GetCharInfoPosY(uint16_t kindCode);
    uint8_t  GetMoveSpeedConstant(uint16_t kindCode);

    // GT 0x005656B0 / 0x005656D0：return (stCharKindInfo*)((char*)info + 147 / 163)。
    // 呼叫端只當作 char* 拿來播放音效；回 char* 即可。
    char* GetAttackSound(uint16_t kindCode);
    char* GetDeadSound(uint16_t kindCode);

    // Returns monster name/info block for the given kind code.
    stCharKindInfo* GetMonsterNameByKind(unsigned short kind);

    // GT 0x005656F0 .. 0x005657B0：怪物 AI / 範圍欄位讀取 helpers。
    uint16_t GetMonsterAIAttr(uint16_t kindCode);
    uint16_t GetFirstHitMonsterRangeX(uint16_t kindCode);
    uint16_t GetFirstHitMonsterRangeY(uint16_t kindCode);
    uint16_t GetMonsterHelpReqRangeX(uint16_t kindCode);
    uint16_t GetMonsterHelpReqRangeY(uint16_t kindCode);

    // GT 0x005657E0：byte+208 = mineCheck，但 IDA 命名為 GetFlyMonsterByKind。
    uint8_t GetFlyMonsterByKind(uint16_t kindCode);

    // Returns all char kind infos that reference the given drop item kind code.
    // outChars must point to an array of at least 65535 stCharKindInfo* elements.
    // Returns the number of entries written.
    int GetCharKindInfoByDropItemKind(uint16_t dropItemKindCode, stCharKindInfo** outChars);

    // Retrieves monster char kinds within level range [minLv, maxLv] matching nation type.
    // a2 = continent (1 or 2), a3..a4 = level range, a5 = excludeBosses (1)。
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

    // Returns boss info for the given kind, or nullptr if not a boss.
    void* GetBossInfoByKind(uint16_t kindCode);

    // GT 0x00565880：根據 plannerName(strcmp) 線性搜尋第一個對應的 kind index。
    uint16_t GetCharID(char* String1);
    // GT 0x005658D0：根據 DCTText 字串 (透過 nameTextCode 查表) 比對 + 跳過 isClone。
    uint16_t GetCharID2(DCTTextManager* a2, char* a3);

    // GT 0x00565960：取出 boss list / boss count。
    int GetAllBossKinds(stCharKindInfo*** a2);

    // GT 0x00565980：return *((BYTE*)info + 221) = continent (LiveArea)。
    uint8_t GetLiveAreaInfo(uint16_t kindCode);

    // GT 0x005659B0：'|' 分隔 5 碼 J-code 字串 → uint16_t 陣列。
    static int GetCharKinds(char* a1, uint16_t* a2);

    // mofclient.c 0x005657F0：載入 charkindinfo.txt 全表後，逐筆載入怪物動畫
    // 並回填 aniTotalFrames[8]。對玩家角色不執行。
    void InitMonsterAinFrame();

    // GT 0x00565B50：讀 stCharKindInfo::aniTotalFrames[a3]。
    uint16_t GetAniTotalFrame(uint16_t kindCode, int a3);

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
