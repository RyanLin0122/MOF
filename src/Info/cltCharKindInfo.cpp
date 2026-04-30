#include "Info/cltCharKindInfo.h"
#include "Info/cltDropItemKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltMonsterAniInfo.h"
#include "Info/cltSkillKindInfo.h"
#include "Other/cltAttackAtb.h"
#include "Text/DCTTextManager.h"
#include "Text/cltTextFileManager.h"
#include "global.h"

#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cstdio>
#include <windows.h>

// ---------------------------------------------------------------------------
// 協助函式：判斷整段字串是否為 +/- 起頭 + 純十進位數字 (對齊 mofclient.c IsDigit)
// ---------------------------------------------------------------------------
static bool IsDigitString(const char* s)
{
    if (!s) return false;
    if (!*s) return true;
    while (true) {
        if (*s == '+' || *s == '-') ++s;
        if (!std::isdigit(static_cast<unsigned char>(*s))) return false;
        ++s;
        if (!*s) return true;
    }
}

// ---------------------------------------------------------------------------
// Constructor / destructor (aligned with mofclient.c)
// ---------------------------------------------------------------------------
cltCharKindInfo::cltCharKindInfo()
{
    // mofclient.c memsets 0xFFFF slots; 我們配置 0x10000 個指標 slot 與其等價。
    m_ppCharKindTable = new stCharKindInfo*[0x10000];
    std::memset(m_ppCharKindTable, 0, sizeof(stCharKindInfo*) * 0x10000);
    m_pBossList   = nullptr;
    m_nBossCount  = 0;

    // Mirror mofclient.c: g_pcltCharKindInfo = this;
    g_pcltCharKindInfo = this;
}

cltCharKindInfo::~cltCharKindInfo()
{
    cltCharKindInfo::Free();
    if (m_ppCharKindTable) {
        delete[] m_ppCharKindTable;
        m_ppCharKindTable = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Initialize — mofclient.c 0x005644D0 char kind 表解析器
// ---------------------------------------------------------------------------
// 檔案格式 (charkindinfo.txt, EUC-KR / cp949)：
//   line 1: 標題列          (略)
//   line 2: 註解 / 空白      (略)
//   line 3: 欄位名稱列       (略)
//   line 4..n: 資料列 (47 columns，\t 分隔)
//
// 每一資料列對應一筆 stCharKindInfo (0x118 bytes)，存入 m_ppCharKindTable[kind]。
// 解析過程任一欄位失敗即跳出迴圈、整體仍標記成功 (v110=1) 並進入 InitMonsterAinFrame。
// 這個寬鬆語意完全對齊原始 binary。
// 解析狀態機；對齊 mofclient.c 的 LABEL_130 / LABEL_138 三條收尾路徑。
enum class CharKindParseExit {
    HeaderFail,     // 頭 3 行 fgets 失敗 → return 0，不建 boss list
    ErrorBreak,     // while 內任一欄位驗證失敗 → return 0，不建 boss list
    SkillOverflow,  // 單筆 skill 數量 >= 10 → return 0，不建 boss list (LABEL_138)
    NormalEnd,      // while 內 fgets 抓不到下一行 → return 1 + 建 boss list (LABEL_130)
    NoFirstData     // 4 行 header ok 但第 1 筆 fgets 失敗 → return 1 + 建空 boss list
};

int cltCharKindInfo::Initialize(char* String2)
{
    char  delimiter[3];
    FILE* stream;
    char  buffer[2024];
    char  skillBuf[1024];

    std::memset(buffer, 0, sizeof(buffer));
    std::strcpy(delimiter, "\t\n");

    stream = g_clTextFileManager.fopen(String2);
    if (!stream)
        return 0;

    CharKindParseExit exitWhy = CharKindParseExit::HeaderFail;

    // mofclient.c：先讀 3 行 (title/註解/欄位名稱)。失敗即 HeaderFail。
    if (std::fgets(buffer, 2023, stream) &&
        std::fgets(buffer, 2023, stream) &&
        std::fgets(buffer, 2023, stream))
    {
        // 第 4 個 fgets 是「第一筆資料列」。失敗 → NoFirstData (仍視為成功)。
        if (!std::fgets(buffer, 2023, stream)) {
            exitWhy = CharKindParseExit::NoFirstData;
            goto finalize;
        }
        exitWhy = CharKindParseExit::ErrorBreak;  // 預設：任何 break 均視為錯誤
        while (true) {
            // (col 0) J-code → kindCode
            char* tok = std::strtok(buffer, delimiter);
            if (!tok) break;
            uint16_t kind = TranslateKindCode(tok);
            if (!kind) {
                MessageBoxA(nullptr,
                            "char kind is zero or there is a blank line",
                            "char kind info loading error", 0);
            }

            // 與原始一致：若 slot 已存在就中斷整個解析迴圈。
            if (m_ppCharKindTable[kind])
                break;

            stCharKindInfo* rec = static_cast<stCharKindInfo*>(::operator new(sizeof(stCharKindInfo)));
            std::memset(rec, 0, sizeof(stCharKindInfo));
            m_ppCharKindTable[kind] = rec;
            rec->kindCode = kind;

            // (col 1) 기획자 이름
            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            std::strcpy(rec->plannerName, tok);

            // (col 2) 캐릭터 이름 코드
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->nameTextCode = static_cast<uint16_t>(std::atoi(tok));

            // (col 3) 몬스터 설명 코드
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->descTextCode = static_cast<uint16_t>(std::atoi(tok));

            // (col 4) 몬스터 도감용 J-code
            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            rec->monsterRegistryKind = TranslateKindCode(tok);

            // (col 5) 몬스터 여부 (Y/N) → flags bit 1 (monster) / bit 0 (player)
            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            {
                bool isMonster = (std::toupper(static_cast<unsigned char>(*tok)) == 'Y');
                uint32_t f = rec->flags;
                if (isMonster) f |= 0x2u;   // bit 1
                else           f |= 0x1u;   // bit 0
                rec->flags = f;
            }

            // (col 6) 애니메이션 정보 파일 (xxx.txt)
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !std::strstr(tok, ".txt")) break;
            std::strcpy(rec->aniFileName, tok);

            // (col 7) 아이템 그룹 D-code 或 "0"
            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            if (_stricmp(tok, "0") != 0)
                rec->dropItemKind = cltDropItemKindInfo::TranslateKindCode(tok);

            // (col 8) Width
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->width = static_cast<uint16_t>(std::atoi(tok));

            // (col 9) Height
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->height = static_cast<uint16_t>(std::atoi(tok));

            // (col 10) 中점y → maxHeight = max(height, centerY)
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            {
                uint16_t centerY = static_cast<uint16_t>(std::atoi(tok));
                rec->maxHeight = rec->height;
                if (centerY > rec->height) rec->maxHeight = centerY;
            }

            // (col 11..14) HP / missHit / normalHit / criticalHit
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->hp = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->missHit = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->normalHit = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->criticalHit = static_cast<uint32_t>(std::atoi(tok));

            // (col 15..17) minAtk / maxAtk / def
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->minAtk = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->maxAtk = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->def = static_cast<uint16_t>(std::atoi(tok));

            // (col 18) exp (DWORD)
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->exp = static_cast<uint32_t>(std::atoi(tok));

            // (col 19) 이동 속도 (string)
            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            rec->moveSpeedType = GetMoveSpeedType(tok);

            // (col 20) 레벨
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->level = static_cast<uint8_t>(std::atoi(tok));

            // (col 21) 공격사운드 / (col 22) 죽음사운드
            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            std::strcpy(rec->atkSound, tok);

            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            std::strcpy(rec->deadSound, tok);

            // (col 23) 몬스터 AI (string → 1..8)
            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            if      (_stricmp(tok, "N_C_OFFEN")            == 0) rec->ai = 1;
            else if (_stricmp(tok, "C_OFFEN")              == 0) rec->ai = 2;
            else if (_stricmp(tok, "N_C_OFFEN_REINFORCE")  == 0) rec->ai = 3;
            else if (_stricmp(tok, "C_OFFEN_REINFORCE")    == 0) rec->ai = 4;
            else if (_stricmp(tok, "N_C_OFFEN_R_AWAY")     == 0) rec->ai = 5;
            else if (_stricmp(tok, "C_OFFEN_R_AWAY")       == 0) rec->ai = 6;
            else if (_stricmp(tok, "N_C_OFFEN_RACE")       == 0) rec->ai = 7;
            else if (_stricmp(tok, "C_OFFEN_RACE")         == 0) rec->ai = 8;
            else if (_stricmp(tok, "0")                    == 0) rec->ai = 0;
            else MessageBoxA(nullptr, "ERROR MONSTER AI TYPE", "AI TYPE", 0);

            // (col 24..29) 남은 체력 / 발동확률 / 선공X / 선공Y / 헬프X / 헬프Y
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->remainHpRatio = static_cast<uint8_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->triggerProb = static_cast<uint8_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->aggroRangeX = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->aggroRangeY = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->helpRangeX = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->helpRangeY = static_cast<uint16_t>(std::atoi(tok));

            // (col 30) 몬스터 속성 → cltAttackAtb::GetAttackTargetAtb
            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            rec->monsterAttr = static_cast<uint32_t>(cltAttackAtb::GetAttackTargetAtb(tok));

            // (col 31) 공격속도
            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            rec->attackSpeedType = GetAttackSpeedType(tok);

            // (col 32) 공격 범위X / (col 33) 공격 범위Y
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->atkRangeX = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->atkRangeY = static_cast<uint32_t>(std::atoi(tok));

            // (col 34) 공격 타입 NEAR/LONG
            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            if      (std::strcmp(tok, "NEAR") == 0) rec->nearLong = 0;
            else if (std::strcmp(tok, "LONG") == 0) rec->nearLong = 1;
            else                                     break;

            // (col 35) 발사체 J-code (LONG 必須非零)
            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            rec->projectileKind = TranslateKindCode(tok);
            if (rec->nearLong == 1 && rec->projectileKind == 0)
                break;

            // (col 36) 마인 체크
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->mineCheck = static_cast<uint8_t>(std::atoi(tok));

            // (col 37) 클론 여부
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->isClone = static_cast<uint8_t>(std::atoi(tok));

            // (col 38) 보스 여부 (0..2)；非 0 且 kind == registry 計入 boss 數
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            {
                int b = std::atoi(tok);
                rec->bossKind = static_cast<uint32_t>(b);
                if (b > 2) break;
                if (b && rec->kindCode == rec->monsterRegistryKind)
                    ++m_nBossCount;
            }

            // (col 39) 죽음 딜레이
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->deadDelay = static_cast<uint32_t>(std::atoi(tok));

            // (col 40) 공적 / (col 41) 대륙
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->isMerit = static_cast<uint8_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->continent = static_cast<uint8_t>(std::atoi(tok));

            // (col 42..45) 매크로 / 대항전 / 이벤트 / 필드아이템박스
            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->macroKind = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->battleKind = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->eventKind = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, delimiter);
            if (!tok || !IsDigitString(tok)) break;
            rec->fieldItemBoxKind = static_cast<uint32_t>(std::atoi(tok));

            // (col 46) 몬스터 스킬 ("S0011|S0012|..." 或 "0")，最多 10 個
            tok = std::strtok(nullptr, delimiter);
            if (!tok) break;
            std::strcpy(skillBuf, tok);
            if (std::strcmp(skillBuf, "0") != 0) {
                bool full = false;
                for (char* sk = std::strtok(skillBuf, "|"); sk; sk = std::strtok(nullptr, "|")) {
                    rec->skills[rec->skillCount] = cltSkillKindInfoM::TranslateKindCode(sk);
                    ++rec->skillCount;
                    if (rec->skillCount >= 10) { full = true; break; }
                }
                if (full) {
                    exitWhy = CharKindParseExit::SkillOverflow;
                    goto finalize;   // 對齊原始 LABEL_138 (跳過 boss list)
                }
            }

            if (!std::fgets(buffer, 2023, stream)) {
                exitWhy = CharKindParseExit::NormalEnd;  // LABEL_130
                break;
            }
        }
    }

finalize:
    // boss list 僅在「正常讀完」或「沒有第一筆資料」兩條路徑收尾。
    if (exitWhy == CharKindParseExit::NormalEnd || exitWhy == CharKindParseExit::NoFirstData) {
        if (m_nBossCount) {
            m_pBossList = static_cast<stCharKindInfo**>(
                ::operator new(sizeof(stCharKindInfo*) * m_nBossCount));
            int idx = 0;
            for (int i = 0; i < 0xFFFF; ++i) {
                stCharKindInfo* r = m_ppCharKindTable[i];
                if (r && r->bossKind && r->kindCode == r->monsterRegistryKind) {
                    m_pBossList[idx++] = r;
                }
            }
        }
    }

    g_clTextFileManager.fclose(stream);
    InitMonsterAinFrame();

    // 對齊 mofclient.c v110：HeaderFail/ErrorBreak/SkillOverflow → 0；其他 → 1。
    return (exitWhy == CharKindParseExit::NormalEnd ||
            exitWhy == CharKindParseExit::NoFirstData) ? 1 : 0;
}

void cltCharKindInfo::Free()
{
    // mofclient.c walks the 65535 pointer slots and operator deletes each
    // non-null entry.
    if (m_ppCharKindTable) {
        for (int i = 0; i < 0xFFFF; ++i) {
            if (m_ppCharKindTable[i]) {
                ::operator delete(m_ppCharKindTable[i]);
                m_ppCharKindTable[i] = nullptr;
            }
        }
    }
    if (m_pBossList) {
        ::operator delete(m_pBossList);
        m_pBossList = nullptr;
    }
    m_nBossCount = 0;
    g_pcltCharKindInfo = nullptr;
}

// ---------------------------------------------------------------------------
// Static: TranslateKindCode (5 碼 J/字母 + 數字 → 16-bit code)
// ---------------------------------------------------------------------------
uint16_t cltCharKindInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;
    if (std::strlen(a1) != 5) return 0;

    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    int lo = std::atoi(a1 + 1);
    if (lo < 0x800)
        return static_cast<uint16_t>(hi | lo);

    return 0;
}

// mofclient.c 0x00565570
uint8_t cltCharKindInfo::GetMoveSpeedType(const char* s)
{
    if (!_stricmp(s, "STATIONARY")) return 1;
    if (!_stricmp(s, "VERY SLOW"))  return 2;
    if (!_stricmp(s, "SLOW"))       return 3;
    if (!_stricmp(s, "NORMAL"))     return 4;
    if (!_stricmp(s, "FAST"))       return 5;
    return _stricmp(s, "VERY FAST") != 0 ? 4 : 6;
}

// mofclient.c 0x005655E0：
//   v2 = -stricmp(s, "FASTEST");
//   LOBYTE(v2) = -(v2 != 0);  // 任何 stricmp != 0 → 低位元組變 0xFF
//   return v2 + 5;
// 此值最終以 *(BYTE*)(rec+145) 寫入；BYTE 取 (v2+5) 的低位元組：
//   FASTEST     → 0 + 5             = 5
//   未知字串    → (... | 0xFF) + 5  = ...0x04 (低位元組 4)
// 實際 charkindinfo.txt 欄位含 "VERY FAST"/"VERY SLOW" 共 9 筆未知值，
// 之前回 5 與 GT 不符 (差 1)；用「FASTEST=5、其他未匹配=4」正確還原 GT
// byte-truncation 行為。
int cltCharKindInfo::GetAttackSpeedType(const char* s)
{
    if (!_stricmp(s, "SLOWEST")) return 1;
    if (!_stricmp(s, "SLOW"))    return 2;
    if (!_stricmp(s, "NORMAL"))  return 3;
    if (!_stricmp(s, "FAST"))    return 4;
    if (!_stricmp(s, "FASTEST")) return 5;
    return 4;  // 未知字串：低位元組 = (0xFF + 5) & 0xFF = 0x04
}

// mofclient.c 0x005655A0：moveSpeedType (1..6) → 移動常數。
uint8_t cltCharKindInfo::GetMoveSpeedConstantByType(uint8_t a1)
{
    switch (a1) {
        case 1u: return 0;
        case 2u: return 1;
        case 4u: return 5;
        case 5u: return 7;
        case 6u: return 10;
        default: return 3;     // 含 case 3 (SLOW)
    }
}

// mofclient.c 0x00565660：attackSpeedType (1..5) → 攻擊延遲 ms。
unsigned int cltCharKindInfo::GetAttackDelayTimeByAttackSpeedType(uint8_t a1)
{
    switch (a1) {
        case 1u: return 3000;
        case 2u: return 2500;
        case 4u: return 1500;
        case 5u: return 1000;
        default: return 2000;  // 含 case 3 (NORMAL)
    }
}

// ---------------------------------------------------------------------------
// Lookup helpers (align with mofclient.c semantics)
// ---------------------------------------------------------------------------
void* cltCharKindInfo::GetCharKindInfo(uint16_t a2)
{
    if (!m_ppCharKindTable) return nullptr;
    return m_ppCharKindTable[a2];
}

uint16_t cltCharKindInfo::GetRealCharID(uint16_t charKind)
{
    return charKind;
}

stCharKindInfo* cltCharKindInfo::GetMonsterNameByKind(unsigned short a2)
{
    // mofclient.c 0x005653E0：return *((WORD*)info + 1) → name text code 的指標還原。
    // 呼叫端通常只判 non-null 並再轉型；維持既有行為。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    if (!info) return nullptr;
    return reinterpret_cast<stCharKindInfo*>(static_cast<uintptr_t>(info->nameTextCode));
}

int cltCharKindInfo::GetCharKindInfoByDropItemKind(uint16_t dropItemKindCode, stCharKindInfo** outChars)
{
    // mofclient.c 0x00565360：遍歷所有 slot，將 dropItemKind == 目標 的記錄寫入 outChars。
    // 注意原始 binary 的回傳值固定為 0；呼叫端依目前 outChars 寫入位置遞推。
    // 我們遵循「呼叫端期望實際數量」的常識，回傳寫入筆數。
    if (!m_ppCharKindTable || !outChars) return 0;
    int count = 0;
    for (int i = 0; i < 0xFFFF; ++i) {
        stCharKindInfo* r = m_ppCharKindTable[i];
        if (r && r->dropItemKind == dropItemKindCode)
            outChars[count++] = r;
    }
    return count;
}

// mofclient.c 0x00565280：對所有 slot 篩選「primary 怪物 entry」(kindCode ==
// monsterRegistryKind & IsMonsterChar)，並依 continent / level / 排除 boss flag
// 篩出符合的 kindCode 寫入 a6，回傳寫入筆數。
//   a2 = 1 → continent==1，否則 continent==2
//   a3..a4 = level 範圍 (含)
//   a5 = 1 → 排除 boss
int cltCharKindInfo::GetMonsterCharKinds(int a2, int a3, int a4, int a5, uint16_t* a6)
{
    if (!m_ppCharKindTable || !a6) return 0;

    int written = 0;
    for (int idx = 0; idx < 0xFFFF; ++idx) {
        stCharKindInfo* r = m_ppCharKindTable[idx];
        if (!r) continue;
        // GT: v8 == v10[3] — 迭代索引 == monsterRegistryKind
        // (slot index 即 kindCode；條件等同 kindCode == monsterRegistryKind)
        if (static_cast<uint16_t>(idx) != r->monsterRegistryKind) continue;
        if (!IsMonsterChar(r->kindCode)) continue;

        // continent 過濾 (GT byte+220 = isMerit；下一格 byte+221 = continent。
        // GT 用的是 +220 (isMerit) 但欄位語義其實是 continent；mofclient.c 反編譯
        // 用 `*((_BYTE *)*v9 + 220)` 與 a2 比較 — 這對應到 stCharKindInfo::isMerit。
        // 為與 GT byte-equal，沿用 isMerit 比較。)
        const uint8_t merit = r->isMerit;
        if (a2 == 1) {
            if (merit != 1) continue;
        } else {
            if (merit != 2) continue;
        }

        const int lv = static_cast<int>(r->level);
        if (lv < a3 || lv > a4) continue;
        if (a5 == 1 && GetBossInfoByKind(r->kindCode)) continue;

        a6[written++] = r->kindCode;
    }
    return written;
}

int cltCharKindInfo::IsMonsterChar(uint16_t kindCode)
{
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(kindCode));
    if (!info) return 0;
    return static_cast<int>((info->flags >> 1) & 1u);
}

int cltCharKindInfo::IsPlayerChar(uint16_t kindCode)
{
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(kindCode));
    if (!info) return 0;
    return static_cast<int>(info->flags & 1u);
}

unsigned char cltCharKindInfo::GetIsClone(uint16_t kindCode)
{
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(kindCode));
    if (!info) return 0;
    return info->isClone;
}

int cltCharKindInfo::GetDieDelayAniByKind(uint16_t kindCode)
{
    // mofclient.c 0x00565860：return *((DWORD*)info + 54) = info+216 (deadDelay)。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(kindCode));
    if (!info) return 0;
    return static_cast<int>(info->deadDelay);
}

char* cltCharKindInfo::GetDeadSound(uint16_t kindCode)
{
    // mofclient.c 0x005656D0：return (char*)info + 163。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(kindCode));
    if (!info) {
        static char s_empty[1] = { '\0' };
        return s_empty;
    }
    return info->deadSound;
}

void* cltCharKindInfo::GetBossInfoByKind(uint16_t kindCode)
{
    // mofclient.c 0x00565A20：return (void*)*((DWORD*)info + 53) = info+212 (bossKind)。
    // 呼叫端只做 non-null 判定；非 boss → 0。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(kindCode));
    if (!info) return nullptr;
    return reinterpret_cast<void*>(static_cast<uintptr_t>(info->bossKind));
}

// ---------------------------------------------------------------------------
// InitMonsterAinFrame — mofclient.c 0x005657F0
// ---------------------------------------------------------------------------
// 對每筆 char-kind，若不是玩家角色，建立暫時的 cltMonsterAniInfo，呼叫
// Initialize(aniFileName) 後讀出 8 個動畫的 total frame，回填到 aniTotalFrames。
void cltCharKindInfo::InitMonsterAinFrame()
{
    if (!m_ppCharKindTable) return;

    for (int i = 0; i < 0xFFFF; ++i) {
        stCharKindInfo* rec = m_ppCharKindTable[i];
        if (!rec) continue;
        if (IsPlayerChar(rec->kindCode)) continue;

        cltMonsterAniInfo ani;
        ani.Initialize(rec->aniFileName);
        for (unsigned int k = 0; k < 8; ++k) {
            rec->aniTotalFrames[k] = ani.GetTotalFrameNum(k);
        }
        // ani 在 scope 結束時自動解構，等價於原始 binary 的 stack-local 物件。
    }
}

// ---------------------------------------------------------------------------
// 補齊 mofclient.c 的 cltCharKindInfo 取值 helpers (0x00565270 .. 0x00565B50)
// 全部都是 GetCharKindInfo + 單欄位讀取。對齊 GT byte-offset 取址。
// ---------------------------------------------------------------------------
stCharKindInfo** cltCharKindInfo::GetCharInfo()
{
    // GT: return (stCharKindInfo**)((char*)this + 4);
    return m_ppCharKindTable;
}

stCharKindInfo* cltCharKindInfo::GetCharKindInfoByStringKindCode(char* a2)
{
    // GT 0x00565330：呼叫的是 cltItemKindInfo::TranslateKindCode (與
    // cltCharKindInfo::TranslateKindCode 演算法相同：5 字元 → 16-bit code)。
    if (!a2) return nullptr;
    uint16_t v3 = cltItemKindInfo::TranslateKindCode(a2);
    if (!v3) return nullptr;
    return static_cast<stCharKindInfo*>(GetCharKindInfo(v3));
}

uint16_t cltCharKindInfo::GetCharWidthA(uint16_t a2)
{
    // GT 0x00565410：return *((WORD*)info + 55) = info+110 (width)。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    return info ? info->width : static_cast<uint16_t>(0);
}

uint16_t cltCharKindInfo::GetCharHeight(uint16_t a2)
{
    // GT 0x00565430：return *((WORD*)info + 56) = info+112 (height)。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    return info ? info->height : static_cast<uint16_t>(0);
}

uint16_t cltCharKindInfo::GetCharInfoPosY(uint16_t a2)
{
    // GT 0x00565450：return *((WORD*)info + 57) = info+114 (maxHeight 中점y)。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    return info ? info->maxHeight : static_cast<uint16_t>(0);
}

uint8_t cltCharKindInfo::GetMoveSpeedConstant(uint16_t a2)
{
    // GT 0x00565470：GetMoveSpeedConstantByType(*((BYTE*)info + 144)) = moveSpeedType。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    if (!info) return 0;
    return GetMoveSpeedConstantByType(info->moveSpeedType);
}

char* cltCharKindInfo::GetAttackSound(uint16_t a2)
{
    // GT 0x005656B0：return (char*)info + 147 = atkSound[]。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    if (!info) {
        static char s_empty[1] = { '\0' };
        return s_empty;
    }
    return info->atkSound;
}

uint16_t cltCharKindInfo::GetMonsterAIAttr(uint16_t a2)
{
    // GT 0x005656F0：return *((WORD*)info + 90) = info+180 (ai)。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    return info ? info->ai : static_cast<uint16_t>(0);
}

uint16_t cltCharKindInfo::GetFirstHitMonsterRangeX(uint16_t a2)
{
    // GT 0x00565720：return *((WORD*)info + 92) = info+184 (aggroRangeX)。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    return info ? info->aggroRangeX : static_cast<uint16_t>(0);
}

uint16_t cltCharKindInfo::GetFirstHitMonsterRangeY(uint16_t a2)
{
    // GT 0x00565750：return *((WORD*)info + 93) = info+186 (aggroRangeY)。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    return info ? info->aggroRangeY : static_cast<uint16_t>(0);
}

uint16_t cltCharKindInfo::GetMonsterHelpReqRangeX(uint16_t a2)
{
    // GT 0x00565780：return *((WORD*)info + 94) = info+188 (helpRangeX)。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    return info ? info->helpRangeX : static_cast<uint16_t>(0);
}

uint16_t cltCharKindInfo::GetMonsterHelpReqRangeY(uint16_t a2)
{
    // GT 0x005657B0：return *((WORD*)info + 95) = info+190 (helpRangeY)。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    return info ? info->helpRangeY : static_cast<uint16_t>(0);
}

uint8_t cltCharKindInfo::GetFlyMonsterByKind(uint16_t a2)
{
    // GT 0x005657E0：return *((BYTE*)info + 208) = mineCheck。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    return info ? info->mineCheck : static_cast<uint8_t>(0);
}

uint16_t cltCharKindInfo::GetCharID(char* String1)
{
    // GT 0x00565880：線性搜尋表，比對 plannerName(strcmp)；找到回 slot index (kindCode)。
    if (!m_ppCharKindTable || !String1) return 0;
    for (int i = 0; i < 0xFFFF; ++i) {
        stCharKindInfo* rec = m_ppCharKindTable[i];
        if (rec && _stricmp(String1, rec->plannerName) == 0)
            return static_cast<uint16_t>(i);
    }
    return 0;
}

uint16_t cltCharKindInfo::GetCharID2(DCTTextManager* a2, char* a3)
{
    // GT 0x005658D0：線性搜尋；對每個非 clone (isClone != 1) 的 entry，
    // 透過 nameTextCode 查 DCTText，比對與 a3 字串相等者回傳 slot index。
    if (!m_ppCharKindTable || !a2 || !a3) return 0;
    for (int i = 0; i < 0xFFFF; ++i) {
        stCharKindInfo* rec = m_ppCharKindTable[i];
        if (!rec) continue;
        char* name = a2->GetText(rec->nameTextCode);
        if (!name) continue;
        if (rec->isClone != 1 && std::strcmp(a3, name) == 0)
            return static_cast<uint16_t>(i);
    }
    return 0;
}

int cltCharKindInfo::GetAllBossKinds(stCharKindInfo*** a2)
{
    // GT 0x00565960：*a2 = boss list head；回傳 m_nBossCount。
    if (a2) *a2 = m_pBossList;
    return m_nBossCount;
}

uint8_t cltCharKindInfo::GetLiveAreaInfo(uint16_t a2)
{
    // GT 0x00565980：return *((BYTE*)info + 221) = continent。
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    return info ? info->continent : static_cast<uint8_t>(0);
}

int cltCharKindInfo::GetCharKinds(char* a1, uint16_t* a2)
{
    // GT 0x005659B0：以 '|' 切割 5 碼字串並 TranslateKindCode；遇到無法翻譯時
    // 回 0 (GT 一旦 v5==0 即 break 並 return 0；正常結束時 return 累計筆數)。
    if (!a1 || !a2) return 0;

    char buf[1024];
    std::strncpy(buf, a1, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    int count = 0;
    char* tok = std::strtok(buf, "|");
    if (!tok) return 0;
    while (tok) {
        uint16_t v = TranslateKindCode(tok);
        if (!v) return 0;     // GT: 失敗 → 回 0 (放棄目前計數)
        a2[count++] = v;
        tok = std::strtok(nullptr, "|");
        if (!tok) return count;
    }
    return count;
}

uint16_t cltCharKindInfo::GetAniTotalFrame(uint16_t a2, int a3)
{
    // GT 0x00565B50：a3 >= 8 → 0；否則回 stCharKindInfo::aniTotalFrames[a3]。
    if (a3 >= 8 || a3 < 0) return 0;
    stCharKindInfo* info = static_cast<stCharKindInfo*>(GetCharKindInfo(a2));
    return info ? info->aniTotalFrames[a3] : static_cast<uint16_t>(0);
}

// 不再保留 cltCharKindInfo g_clCharKindInfo 全域實例。
// mofclient.c 並無此符號；唯一 instance 是 ClientCharacterManager 持有的
// cltClientCharKindInfo（建構時將自身寫入 g_pcltCharKindInfo）。
