#include "Info/cltNPCInfo.h"
#include "Text/cltTextFileManager.h"
#include "Text/DCTTextManager.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <windows.h>

// 全域文本檔案管理器 (定義在 cltTextFileManager.cpp)
extern cltTextFileManager g_clTextFileManager;

// GT: mofclient.c:25383
DCTTextManager* cltNPCInfo::m_pclTextManager = nullptr;

// ─── helpers (GT mofclient.c:342909 / 342945 등가) ──────────────────────────
//  · 빈 문자열(첫 바이트 == 0) → true
//  · 매 반복마다 한 번 '+' / '-' 를 건너뜀
//  · 그 외 한 글자라도 isdigit 실패 시 false
bool cltNPCInfo::IsDigit(const char* a1)
{
    if (!a1) return false;
    const char* v1 = a1;
    if (!*v1) return true;
    while (true) {
        if (*v1 == '+' || *v1 == '-') ++v1;
        if (!std::isdigit(static_cast<unsigned char>(*v1))) return false;
        ++v1;
        if (!*v1) return true;
    }
}

//  · 빈 문자열(첫 바이트 == 0) → true
bool cltNPCInfo::IsAlphaNumeric(const char* a1)
{
    if (!a1) return false;
    const char* v1 = a1;
    if (!*v1) return true;
    while (std::isalnum(static_cast<unsigned char>(*v1))) {
        ++v1;
        if (!*v1) return true;
    }
    return false;
}

// ── mofclient.c:317017 (00581500) ──────────────────────────────────────────
void cltNPCInfo::InitializeStaticVariable(DCTTextManager* a1)
{
    m_pclTextManager = a1;
}

// ── mofclient.c:317023 (00581510) ──────────────────────────────────────────
//   GT: *(DWORD*)this = 0;  *((WORD*)this + 2) = 0;
cltNPCInfo::cltNPCInfo()
    : m_pInfo(nullptr)
    , m_wTotalNPCNum(0)
{
}

// ── mofclient.c:317492 (00581EB0) ──────────────────────────────────────────
//   ((toupper(s[0]) + 31) << 11) | atoi(s + 1)；길이 != 5 또는 num >= 0x800 → 0
//   GT: a1 에 대해 nullptr 검사를 하지 않음 (caller 가 non-null 을 보장).
std::uint16_t cltNPCInfo::TranslateKindCode(const char* a1)
{
    if (std::strlen(a1) != 5) return 0;
    int v2 = (std::toupper(static_cast<unsigned char>(*a1)) + 31) << 11;
    std::uint16_t v3 = static_cast<std::uint16_t>(std::atoi(a1 + 1));
    if (v3 < 0x800u)
        return static_cast<std::uint16_t>(v2 | v3);
    return 0;
}

// ── mofclient.c:317510 (00581F00) ──────────────────────────────────────────
void cltNPCInfo::Free()
{
    if (m_pInfo) {
        ::operator delete(m_pInfo);
        m_pInfo = nullptr;
    }
    m_wTotalNPCNum = 0;
}

// ── mofclient.c:317521 (00581F20) ──────────────────────────────────────────
std::uint16_t cltNPCInfo::GetTotalNPCNum() const
{
    return m_wTotalNPCNum;
}

// ── mofclient.c:317527 (00581F30) ──────────────────────────────────────────
//   return *((_WORD*)GetNPCInfoByID(this, npcKind) + 18);  (= byte 36)
std::uint16_t cltNPCInfo::GetShopID(std::uint16_t a2)
{
    return GetNPCInfoByID(a2)->m_wShopID;
}

// ── mofclient.c:317533 (00581F50) ──────────────────────────────────────────
stNPCInfo* cltNPCInfo::GetNPCInfoByIndex(int a2)
{
    if (a2 < 0 || a2 >= static_cast<int>(m_wTotalNPCNum))
        return nullptr;
    return m_pInfo + a2;
}

// ── mofclient.c:317545 (00581F80) ──────────────────────────────────────────
stNPCInfo* cltNPCInfo::GetNPCInfoByID(std::uint16_t a2)
{
    int v2 = 0;
    int v3 = m_wTotalNPCNum;
    if (!static_cast<std::uint16_t>(v3))
        return nullptr;
    auto* base = reinterpret_cast<std::uint16_t*>(m_pInfo);
    auto* i = base;
    while (*i != a2) {
        if (++v2 >= v3) return nullptr;
        i += 250;  // 250 WORDs == 500 bytes == sizeof(stNPCInfo)
    }
    return reinterpret_cast<stNPCInfo*>(&base[250 * v2]);
}

// ── mofclient.c:317566 (00581FC0) ──────────────────────────────────────────
//   비교 시작점: *(_DWORD*)this + 2  → byte 2 of entry → m_wBookCode
stNPCInfo* cltNPCInfo::GetNPCInfoByBookID(std::uint16_t a2)
{
    int v2 = 0;
    int v3 = m_wTotalNPCNum;
    if (!static_cast<std::uint16_t>(v3))
        return nullptr;
    auto* base = reinterpret_cast<char*>(m_pInfo);
    auto* i = reinterpret_cast<std::uint16_t*>(base + 2);
    while (*i != a2) {
        if (++v2 >= v3) return nullptr;
        i += 250;
    }
    return reinterpret_cast<stNPCInfo*>(base + 500 * v2);
}

// ── mofclient.c:317587 (00582000) ──────────────────────────────────────────
//   비교 키: byte 0 (NPCKind), 결과: byte 2 (BookCode) of 일치 entry
std::uint16_t cltNPCInfo::GetNPCBookID(std::uint16_t a2)
{
    int v2 = 0;
    int v3 = m_wTotalNPCNum;
    if (!static_cast<std::uint16_t>(v3))
        return 0;
    auto* base = reinterpret_cast<std::uint16_t*>(m_pInfo);
    auto* i = base;
    while (*i != a2) {
        if (++v2 >= v3) return 0;
        i += 250;
    }
    return base[250 * v2 + 1];
}

// ── mofclient.c:317608 (00582050) ──────────────────────────────────────────
//   index a3 의 entry.m_wMapID == a2 ?  (range check 포함)
BOOL cltNPCInfo::GetNPCInSameMapID(std::uint16_t a2, int a3)
{
    if (a3 < 0 || a3 >= static_cast<int>(m_wTotalNPCNum))
        return 0;
    auto* base = reinterpret_cast<char*>(m_pInfo);
    return *reinterpret_cast<std::uint16_t*>(base + 500 * a3 + 12) == a2;
}

// ── mofclient.c:317620 (00582090) ──────────────────────────────────────────
//   같은 map id 를 갖는 npc 갯수 세기
int cltNPCInfo::GetNPCInSameMapID(std::uint16_t a2)
{
    int result = 0;
    int v3 = m_wTotalNPCNum;
    if (!static_cast<std::uint16_t>(v3))
        return 0;
    auto* v4 = reinterpret_cast<std::uint16_t*>(reinterpret_cast<char*>(m_pInfo) + 12);
    do {
        if (*v4 == a2) ++result;
        v4 += 250;
        --v3;
    } while (v3);
    return result;
}

// ── mofclient.c:317644 (005820C0) ──────────────────────────────────────────
//   GT: return *((_WORD*)GetNPCInfoByID(this, a2) + a3 + 100);
//        (a3 + 100) WORD index → byte (a3*2 + 200) = m_wQuestIDs[a3]
std::uint16_t cltNPCInfo::GetQuestID(std::uint16_t a2, std::uint8_t a3)
{
    return GetNPCInfoByID(a2)->m_wQuestIDs[a3];
}

// ── mofclient.c:317650 (005820E0) ──────────────────────────────────────────
//   GT: return *((_WORD*)GetNPCInfoByID(this, a2) + 6);   (= byte 12)
std::uint16_t cltNPCInfo::GetMapID(std::uint16_t a2)
{
    return GetNPCInfoByID(a2)->m_wMapID;
}

// ── mofclient.c:317656 (00582100) ──────────────────────────────────────────
//   각 NPC의 m_wQuestIDs[0..69] 중 a2 와 일치하는 슬롯을 찾음
//   *a3 ← 슬롯 index  /  return ← 해당 NPC 의 KindCode
//   x64 노트: GT 가 v5 = (DWORD)pInfo + 200 처럼 pointer 를 int 로 보관.
//     32-bit 에서는 정상이지만 64-bit 에서는 truncation 이 되므로
//     포인터를 그대로 보관하도록 char* 로 보관한다 (의미는 동일).
std::uint16_t cltNPCInfo::GetNPCIDByQuestID(std::uint16_t a2, int* a3)
{
    int v3 = m_wTotalNPCNum;
    int v4 = 0;
    if (!static_cast<std::uint16_t>(v3))
        return 0;

    auto* base = reinterpret_cast<char*>(m_pInfo);
    char* v5 = base + 200;
    while (true) {
        int v6 = 0;
        auto* v7 = reinterpret_cast<std::uint16_t*>(v5);
        do {
            if (*v7 == a2) {
                *a3 = v6;
                return *reinterpret_cast<std::uint16_t*>(base + 500 * v4);
            }
            ++v6;
            ++v7;
        } while (v6 < 70);
        ++v4;
        v5 += 500;
        if (v4 >= v3) break;
    }
    return 0;
}

// ── mofclient.c:317695 (00582160) ──────────────────────────────────────────
//   각 NPC 의 quest table 에서 questID 일치 시 → 해당 NPC 의 m_wNameTextID 반환
//   x64 노트: GT 의 v4 = (DWORD)pInfo + 200 → 64-bit 환경에서는 truncation 발생.
//     포인터를 그대로 char* 로 보관해 동작을 보존.
//   GT: GetNPCInfoByID 의 결과에 nullptr 검사를 하지 않음 (직접 +2 로 dereference).
std::uint16_t cltNPCInfo::GetNPCNameByQuestID(std::uint16_t a2)
{
    int v2 = m_wTotalNPCNum;
    int v3 = 0;
    if (!static_cast<std::uint16_t>(v2))
        return 0;
    auto* base = reinterpret_cast<char*>(m_pInfo);
    char* v4 = base + 200;
    while (true) {
        int v5 = 0;
        auto* v6 = reinterpret_cast<std::uint16_t*>(v4);
        do {
            if (*v6 == a2) {
                std::uint16_t kind = *reinterpret_cast<std::uint16_t*>(base + 500 * v3);
                return GetNPCInfoByID(kind)->m_wNameTextID;
            }
            ++v5;
            ++v6;
        } while (v5 < 70);
        ++v3;
        v4 += 500;
        if (v3 >= v2) break;
    }
    return 0;
}

// ── mofclient.c:317731 (005821D0) ──────────────────────────────────────────
//   비교 시작점: *(_DWORD*)this + 36  → byte 36 of entry → m_wShopID
stNPCInfo* cltNPCInfo::GetNPCInfoByShopID(std::uint16_t a2)
{
    int v2 = 0;
    int v3 = m_wTotalNPCNum;
    if (!static_cast<std::uint16_t>(v3))
        return nullptr;
    auto* base = reinterpret_cast<char*>(m_pInfo);
    auto* i = reinterpret_cast<std::uint16_t*>(base + 36);
    while (*i != a2) {
        if (++v2 >= v3) return nullptr;
        i += 250;
    }
    return reinterpret_cast<stNPCInfo*>(base + 500 * v2);
}

// ── mofclient.c:317752 (00582210) ──────────────────────────────────────────
//   name(text id 4) 비교 → 일치 시 entry 포인터를 stack[20] 에 모음
//   (단, 같은 mapid (offset 12) 가 이미 들어있으면 skip)
//   결과 1개 → 그것 반환
//   다수 → mapid==a3 인 entry 찾고, 다음 index 반환  (없으면 stack[0])
//   x64 노트: GT 가 stack[20] 을 int v13[20] (DWORD) 으로 선언하고 그 안에
//     entry 포인터(*(_DWORD*)this + v4) 를 저장 — 32-bit 에서는 OK 지만
//     64-bit 에서는 truncation 이 발생. 의미는 "entry 포인터의 배열" 이므로
//     포인터를 그대로 보관하도록 char* v13[20] 으로 변경한다.
stNPCInfo* cltNPCInfo::GetNPCInfoByNpcName(char* a2, std::uint16_t a3)
{
    int v4 = 0;       // byte offset 누적 (per entry: 500)
    int v5 = 0;       // 결과 갯수
    int v12 = 0;      // 검사한 entry 갯수
    if (!m_wTotalNPCNum)
        return nullptr;

    char* v13[20] = {};
    char** v11 = v13;
    auto* base = reinterpret_cast<char*>(m_pInfo);

    do {
        std::uint16_t nameTextID = *reinterpret_cast<std::uint16_t*>(base + v4 + 4);
        // GT: strcmp(a2, DCTTextManager::GetText(m_pclTextManager, nameTextID))
        //   GT 不做 nullptr 保護；若 GetText 回傳 NULL 會崩 — 與反編譯一致。
        int cmp = std::strcmp(a2, m_pclTextManager->GetText(nameTextID));
        if (cmp == 0) {
            int v6 = 0;
            if (v5 > 0) {
                char** v7 = v13;
                do {
                    if (*reinterpret_cast<std::uint16_t*>(*v7 + 12)
                        == *reinterpret_cast<std::uint16_t*>(base + v4 + 12))
                        break;
                    ++v6;
                    ++v7;
                } while (v6 < v5);
            }
            if (v6 == v5) {
                ++v5;
                *v11++ = base + v4;
                if (v5 >= 20) break;
            }
        }
        v4 += 500;
        ++v12;
    } while (v12 < m_wTotalNPCNum);

    if (!v5) return nullptr;
    int v9 = 0;
    if (v5 - 1 <= 0)
        return reinterpret_cast<stNPCInfo*>(v13[0]);
    char** i = v13;
    while (*reinterpret_cast<std::uint16_t*>(*i + 12) != a3) {
        if (++v9 >= v5 - 1)
            return reinterpret_cast<stNPCInfo*>(v13[0]);
        ++i;
    }
    return reinterpret_cast<stNPCInfo*>(v13[v9 + 1]);
}

// ── mofclient.c:317815 (00582330) ──────────────────────────────────────────
//   "QUEST|SHOPETC|..." 를 '|' 로 분할; 알 수 없는 토큰을 만나면 (CHEER 도 아닌)
//   *return*  → 이후 토큰은 모두 무시됨 (GT 의 quirk 그대로 보존)
//   인자: String1 ─ in/out 임시 1024-byte 문자열 버퍼 (strtok 가 NUL 삽입함)
//        a3      ─ stNPCInfo::m_byNPCType (54-byte) 를 가리킴
void cltNPCInfo::GetNPCType(char* String1, std::uint8_t* a3)
{
    char Delimiter[2] = "|";
    if (_stricmp(String1, "NONE") == 0) return;

    for (char* i = std::strtok(String1, Delimiter); i; i = std::strtok(nullptr, Delimiter)) {
        int v4;
        if      (!_stricmp(i, "QUEST"))                   v4 = NPCTYPE_QUEST;
        else if (!_stricmp(i, "SHOPWEAPON"))              v4 = NPCTYPE_SHOPWEAPON;
        else if (!_stricmp(i, "SHOPDEFENSE"))             v4 = NPCTYPE_SHOPDEFENSE;
        else if (!_stricmp(i, "SHOPJEWEL"))               v4 = NPCTYPE_SHOPJEWEL;
        else if (!_stricmp(i, "SHOPONETIME"))             v4 = NPCTYPE_SHOPONETIME;
        else if (!_stricmp(i, "SHOPMAKING_ONETIME"))      v4 = NPCTYPE_SHOPMAKING_ONETIME;
        else if (!_stricmp(i, "SHOPINCHANT_STONE"))       v4 = NPCTYPE_SHOPINCHANT_STONE;
        else if (!_stricmp(i, "SHOPINDEMNITY"))           v4 = NPCTYPE_SHOPINDEMNITY;
        else if (!_stricmp(i, "SHOPETC"))                 v4 = NPCTYPE_SHOPETC;
        else if (!_stricmp(i, "JOBCHANGE"))               v4 = NPCTYPE_JOBCHANGE;
        else if (!_stricmp(i, "SWORD"))                   v4 = NPCTYPE_SWORD;
        else if (!_stricmp(i, "BOW"))                     v4 = NPCTYPE_BOW;
        else if (!_stricmp(i, "MAGIC"))                   v4 = NPCTYPE_MAGIC;
        else if (!_stricmp(i, "THEOLOGY"))                v4 = NPCTYPE_THEOLOGY;
        else if (!_stricmp(i, "HELP"))                    v4 = NPCTYPE_HELP;
        else if (!_stricmp(i, "LESSONSET"))               v4 = NPCTYPE_LESSONSET;
        else if (!_stricmp(i, "SALESAGENCY"))             v4 = NPCTYPE_SALESAGENCY;
        else if (!_stricmp(i, "STORAGE"))                 v4 = NPCTYPE_STORAGE;
        else if (!_stricmp(i, "TELEDRAGON"))              v4 = NPCTYPE_TELEDRAGON;
        else if (!_stricmp(i, "STORY"))                   v4 = NPCTYPE_STORY;
        else if (!_stricmp(i, "TEST"))                    v4 = NPCTYPE_TEST;
        else if (!_stricmp(i, "SPECIALTY"))               v4 = NPCTYPE_SPECIALTY;
        else if (!_stricmp(i, "CIRCLEQUEST"))             v4 = NPCTYPE_CIRCLEQUEST;
        else if (!_stricmp(i, "BOARD"))                   v4 = NPCTYPE_BOARD;
        else if (!_stricmp(i, "WISHBOARD"))               v4 = NPCTYPE_WISHBOARD;
        else if (!_stricmp(i, "SHIP"))                    v4 = NPCTYPE_SHIP;
        else if (!_stricmp(i, "ENCHANT"))                 v4 = NPCTYPE_ENCHANT;
        else if (!_stricmp(i, "QUIZ"))                    v4 = NPCTYPE_QUIZ;
        else if (!_stricmp(i, "ETC"))                     v4 = NPCTYPE_ETC;
        else if (!_stricmp(i, "WAR"))                     v4 = NPCTYPE_WAR;
        else if (!_stricmp(i, "SUPPLY"))                  v4 = NPCTYPE_SUPPLY;
        else if (!_stricmp(i, "EMBLEM"))                  v4 = NPCTYPE_EMBLEM;
        else if (!_stricmp(i, "TEST_INSTANT"))            v4 = NPCTYPE_TEST_INSTANT;
        else if (!_stricmp(i, "INDUN"))                   v4 = NPCTYPE_INDUN;
        else if (!_stricmp(i, "CIRCLEINDUN"))             v4 = NPCTYPE_CIRCLEINDUN;
        else if (!_stricmp(i, "PETSHOP"))                 v4 = NPCTYPE_PETSHOP;
        else if (!_stricmp(i, "PETNURSERY"))              v4 = NPCTYPE_PETNURSERY;
        else if (!_stricmp(i, "PETSELL"))                 v4 = NPCTYPE_PETSELL;
        else if (!_stricmp(i, "PETSELLINFO"))             v4 = NPCTYPE_PETSELLINFO;
        else if (!_stricmp(i, "PETSEARCH"))               v4 = NPCTYPE_PETSEARCH;
        else if (!_stricmp(i, "PVPITEM"))                 v4 = NPCTYPE_PVPITEM;
        else if (!_stricmp(i, "EXPANDCIRCLEMEMBERS"))     v4 = NPCTYPE_EXPANDCIRCLEMEMBERS;
        else if (!_stricmp(i, "DIVORCE"))                 v4 = NPCTYPE_DIVORCE;
        else if (!_stricmp(i, "MARRY_PETITION"))          v4 = NPCTYPE_MARRY_PETITION;
        else if (!_stricmp(i, "MARRY_SHOP"))              v4 = NPCTYPE_MARRY_SHOP;
        else if (!_stricmp(i, "WEDDINGBOARD"))            v4 = NPCTYPE_WEDDINGBOARD;
        else if (!_stricmp(i, "TELELIBI"))                v4 = NPCTYPE_TELELIBI;
        else if (!_stricmp(i, "TELESKYCASTLE"))           v4 = NPCTYPE_TELESKYCASTLE;
        else if (!_stricmp(i, "TELEBLUESTAR"))            v4 = NPCTYPE_TELEBLUESTAR;
        else if (!_stricmp(i, "SHOP_COIN"))               v4 = NPCTYPE_SHOP_COIN;
        else {
            // GT: 인식 불가 토큰 (CHEER 가 아닌) → 함수 전체 종료 (이후 토큰 무시)
            if (_stricmp(i, "CHEER")) return;
            v4 = NPCTYPE_CHEER;
        }
        a3[v4] = 1;
    }
}

// ── mofclient.c:317034 (00581530) ──────────────────────────────────────────
//   npclist.txt parser.  파일 포맷:
//     line 1..3 : header (skip)
//     line 4..N : NPC 한 줄에 116 fields
//   알고리즘:
//     1) header 3 lines 읽기
//     2) fgetpos → 한 번 끝까지 읽으며 row count 계산
//     3) operator new(500 * count); memset 0
//     4) fsetpos 복귀 → row 단위 파싱
//   글로벌 quest 중복 검출:
//     · v86[1024] (스택) 에 모든 행의 모든 비-0 quest id 누적
//     · 새 quest id 가 v86[0..v76-1] 에 이미 존재 시 MessageBox + abort (return 0)
//     · v76 는 모든 행에 걸쳐 누적 (NOT reset per row)
//   token 파싱 실패 (strtok=NULL) 시 break / goto LABEL_86 → return 0
//   마지막 row 의 70 quest 까지 파싱 후 추가 fgets 가 EOF 면 return 1
int cltNPCInfo::Initialize(char* String2)
{
    char Delimiter[2 + 1];
    char* String;       // 임시 (현 토큰 holder)
    char* v74;          // 쓰기 포인터 (offset 40 → 46 → 340 → 200 등)
    int   v76;          // 누적 비-0 quest 갯수 (across all rows)
    int   v77;          // 결과 (1=성공, 0=실패)
    char  Text[16];
    char  String1[1024];
    std::uint16_t v86[1024];
    char  Buffer[10240];
    std::fpos_t Position{};

    std::strcpy(Delimiter, "\t\n");
    std::memset(Buffer, 0, sizeof(Buffer));
    std::memset(String1, 0, sizeof(String1));
    v77 = 0;
    v76 = 0;
    // GT 는 v86 을 초기화하지 않음 (스택 garbage). 알고리즘은 v86[v76] 를 먼저 쓰고 v76 을 늘리므로
    // [0..v76-1] 범위만 읽히어 garbage 가 노출되지 않음 → 추가 memset 불필요 (strict GT).

    std::FILE* Stream = g_clTextFileManager.fopen(String2);
    if (!Stream) return 0;

    // header 3 줄
    if (std::fgets(Buffer, 10239, Stream)
        && std::fgets(Buffer, 10239, Stream)
        && std::fgets(Buffer, 10239, Stream))
    {
        std::fgetpos(Stream, &Position);
        for (; std::fgets(Buffer, 10239, Stream); ++m_wTotalNPCNum) { }

        m_pInfo = static_cast<stNPCInfo*>(::operator new(500 * m_wTotalNPCNum));
        // GT: memset(v5, 0, 4 * ((500 * (unsigned int)v6) >> 2));
        //   → 500 은 4 의 배수이므로 결과적으로 전체 0 클리어
        std::memset(m_pInfo, 0, 500 * static_cast<std::uint32_t>(m_wTotalNPCNum));

        std::fsetpos(Stream, &Position);

        auto* v7 = reinterpret_cast<char*>(m_pInfo);

        // GT: 첫 데이터 행 fgets 가 실패 (= header 까지만 있고 데이터 0 행) 한 경우
        //   v77 = 1 (성공) 으로 LABEL_85 → LABEL_86 진행. 누락 시 빈 npclist 가 실패로 잡힘.
        if (!std::fgets(Buffer, 10239, Stream)) {
            v77 = 1;
            goto Lfail;
        }
        while (true)
        {
            // ── 1. NPC ID (offset 0, TranslateKindCode) ───────────────
            char* v8 = std::strtok(Buffer, Delimiter);
                if (!v8) goto Lfail;
                *reinterpret_cast<std::uint16_t*>(v7) = TranslateKindCode(v8);

                // ── 2. NPC분류 / 도감 ID (offset 2) ───────────────────────
                char* v9 = std::strtok(nullptr, Delimiter);
                if (!v9) goto Lfail;
                *reinterpret_cast<std::uint16_t*>(v7 + 2) = TranslateKindCode(v9);

                // ── 3. 이름(기획자용) (skip; non-null 만 검사) ─────────────
                if (!std::strtok(nullptr, Delimiter)) goto Lfail;

                // ── 4. NPC 이름 텍스트 ID (offset 4, digit) ───────────────
                char* v10 = std::strtok(nullptr, Delimiter);
                if (!v10 || !IsDigit(v10)) goto Lfail;
                *reinterpret_cast<std::uint16_t*>(v7 + 4) = static_cast<std::uint16_t>(std::atoi(v10));

                // ── 5. NPC 분류 텍스트 ID (offset 6, digit) ───────────────
                char* v12 = std::strtok(nullptr, Delimiter);
                if (!v12 || !IsDigit(v12)) goto Lfail;
                *reinterpret_cast<std::uint16_t*>(v7 + 6) = static_cast<std::uint16_t>(std::atoi(v12));

                // ── 6. NPC 분류 (offset 8, digit) ─────────────────────────
                char* v14 = std::strtok(nullptr, Delimiter);
                if (!v14 || !IsDigit(v14)) goto Lfail;
                *reinterpret_cast<std::uint16_t*>(v7 + 8) = static_cast<std::uint16_t>(std::atoi(v14));

                // ── 7. NPC 위치 정보 (offset 10, digit) ───────────────────
                char* v16 = std::strtok(nullptr, Delimiter);
                if (!v16 || !IsDigit(v16)) goto Lfail;
                *reinterpret_cast<std::uint16_t*>(v7 + 10) = static_cast<std::uint16_t>(std::atoi(v16));

                // ── 8. 맵 아이디 (offset 12, TranslateKindCode) ───────────
                char* v18 = std::strtok(nullptr, Delimiter);
                if (!v18) goto Lfail;
                *reinterpret_cast<std::uint16_t*>(v7 + 12) = TranslateKindCode(v18);

                // ── 9..12. x / y / Width / Height (DWORD digit, off 20/24/28/32) ─
                char* v19 = std::strtok(nullptr, Delimiter);
                if (!v19 || !IsDigit(v19)) goto Lfail;
                *reinterpret_cast<std::uint32_t*>(v7 + 20) = static_cast<std::uint32_t>(std::atoi(v19));
                char* v21 = std::strtok(nullptr, Delimiter);
                if (!v21 || !IsDigit(v21)) goto Lfail;
                *reinterpret_cast<std::uint32_t*>(v7 + 24) = static_cast<std::uint32_t>(std::atoi(v21));
                char* v23 = std::strtok(nullptr, Delimiter);
                if (!v23 || !IsDigit(v23)) goto Lfail;
                *reinterpret_cast<std::uint32_t*>(v7 + 28) = static_cast<std::uint32_t>(std::atoi(v23));
                char* v25 = std::strtok(nullptr, Delimiter);
                if (!v25 || !IsDigit(v25)) goto Lfail;
                *reinterpret_cast<std::uint32_t*>(v7 + 32) = static_cast<std::uint32_t>(std::atoi(v25));

                // ── 13. 상점 id (offset 36, TranslateKindCode) ───────────
                char* v27 = std::strtok(nullptr, Delimiter);
                if (!v27) goto Lfail;
                *reinterpret_cast<std::uint16_t*>(v7 + 36) = TranslateKindCode(v27);

                // ── 14. 인사말 (offset 38, digit) ─────────────────────────
                char* v28 = std::strtok(nullptr, Delimiter);
                if (!v28 || !IsDigit(v28)) goto Lfail;
                *reinterpret_cast<std::uint16_t*>(v7 + 38) = static_cast<std::uint16_t>(std::atoi(v28));

                // ── 15..17. 말풍선1/2/3 (3 WORD digit, off 40/42/44) ─────
                {
                    int cnt = 0;
                    v74 = v7 + 40;
                    while (true) {
                        char* v31 = std::strtok(nullptr, Delimiter);
                        if (!v31 || !IsDigit(v31)) goto Lfail;
                        *reinterpret_cast<std::uint16_t*>(v74) = static_cast<std::uint16_t>(std::atoi(v31));
                        v74 += 2;
                        ++cnt;
                        if (cnt >= 3) break;
                    }
                }

                // ── 18..20. 페이스_1/2/3 (3 strings × 16 bytes, off 46/62/78) ─
                {
                    char* v34 = std::strtok(nullptr, Delimiter);
                    if (!v34) goto Lfail;
                    std::strcpy(v7 + 46, v34);
                    char* v35 = std::strtok(nullptr, Delimiter);
                    if (!v35) goto Lfail;
                    std::strcpy(v7 + 62, v35);
                    char* v36 = std::strtok(nullptr, Delimiter);
                    if (!v36) goto Lfail;
                    std::strcpy(v7 + 78, v36);
                }

                // ── 21. NPC 타입 문자열 (지역 String1[1024] 로 복사) ──────
                {
                    char* v37 = std::strtok(nullptr, Delimiter);
                    if (!v37) goto Lfail;
                    std::strcpy(String1, v37);
                }

                // ── 22. ResourceID (DWORD hex, off 148) ──────────────────
                {
                    char* v38 = std::strtok(nullptr, Delimiter);
                    if (!v38 || !IsAlphaNumeric(v38)) goto Lfail;
                    std::sscanf(v38, "%x", reinterpret_cast<unsigned int*>(v7 + 148));
                }

                // ── 23. 포탈 아이콘 (WORD digit, off 152) ─────────────────
                {
                    char* v40 = std::strtok(nullptr, Delimiter);
                    if (!v40 || !IsDigit(v40)) goto Lfail;
                    *reinterpret_cast<std::uint16_t*>(v7 + 152) = static_cast<std::uint16_t>(std::atoi(v40));
                }

                // ── 24. 일러스트 ResourceID (DWORD hex, off 156) ─────────
                {
                    char* v42 = std::strtok(nullptr, Delimiter);
                    if (!v42 || !IsAlphaNumeric(v42)) goto Lfail;
                    std::sscanf(v42, "%x", reinterpret_cast<unsigned int*>(v7 + 156));
                }

                // ── 25. 결혼 일러스트 리소스 (DWORD hex, off 160) ────────
                {
                    char* v44 = std::strtok(nullptr, Delimiter);
                    if (!v44 || !IsAlphaNumeric(v44)) goto Lfail;
                    std::sscanf(v44, "%x", reinterpret_cast<unsigned int*>(v7 + 160));
                }

                // ── 26. 결혼 일러스트 파일 ID (WORD digit, off 164) ──────
                {
                    char* v46 = std::strtok(nullptr, Delimiter);
                    if (!v46 || !IsDigit(v46)) goto Lfail;
                    *reinterpret_cast<std::uint16_t*>(v7 + 164) = static_cast<std::uint16_t>(std::atoi(v46));
                }

                // ── 27. 좌우 방향 L=1 / R=0  (DWORD, off 168) ────────────
                {
                    char* v48 = std::strtok(nullptr, Delimiter);
                    if (!v48) goto Lfail;
                    if (std::toupper(static_cast<unsigned char>(*v48)) == 'L') {
                        *reinterpret_cast<std::uint32_t*>(v7 + 168) = 1;
                    } else if (std::toupper(static_cast<unsigned char>(*v48)) == 'R') {
                        *reinterpret_cast<std::uint32_t*>(v7 + 168) = 0;
                    } else {
                        goto Lfail;  // GT: break (parse abort)
                    }
                }

                // ── 28. npc 컷 T=1 / 그 외=0  (DWORD, off 172) ───────────
                {
                    char* v50 = std::strtok(nullptr, Delimiter);
                    if (!v50) goto Lfail;
                    *reinterpret_cast<std::uint32_t*>(v7 + 172) =
                        (std::toupper(static_cast<unsigned char>(*v50)) == 'T') ? 1u : 0u;
                }

                // ── 29..33. 메시지 5종 (WORD digit, off 176/178/180/182/184) ─
                {
                    char* v51 = std::strtok(nullptr, Delimiter);
                    if (!v51 || !IsDigit(v51)) goto Lfail;
                    *reinterpret_cast<std::uint16_t*>(v7 + 176) = static_cast<std::uint16_t>(std::atoi(v51));
                    char* v53 = std::strtok(nullptr, Delimiter);
                    if (!v53 || !IsDigit(v53)) goto Lfail;
                    *reinterpret_cast<std::uint16_t*>(v7 + 178) = static_cast<std::uint16_t>(std::atoi(v53));
                    char* v55 = std::strtok(nullptr, Delimiter);
                    if (!v55 || !IsDigit(v55)) goto Lfail;
                    *reinterpret_cast<std::uint16_t*>(v7 + 180) = static_cast<std::uint16_t>(std::atoi(v55));
                    char* v57 = std::strtok(nullptr, Delimiter);
                    if (!v57 || !IsDigit(v57)) goto Lfail;
                    *reinterpret_cast<std::uint16_t*>(v7 + 182) = static_cast<std::uint16_t>(std::atoi(v57));
                    char* v59 = std::strtok(nullptr, Delimiter);
                    if (!v59 || !IsDigit(v59)) goto Lfail;
                    *reinterpret_cast<std::uint16_t*>(v7 + 184) = static_cast<std::uint16_t>(std::atoi(v59));
                }

                // ── 34..36. 소환 시점 / 테스트 필드 / 노소환 (DWORD digit, off 188/192/196) ─
                //   GT:  IsDigit 미검사, atoi 만 호출
                {
                    char* v61 = std::strtok(nullptr, Delimiter);
                    if (!v61) goto Lfail;
                    *reinterpret_cast<std::uint32_t*>(v7 + 188) = static_cast<std::uint32_t>(std::atoi(v61));
                    char* v62 = std::strtok(nullptr, Delimiter);
                    if (!v62) goto Lfail;
                    *reinterpret_cast<std::uint32_t*>(v7 + 192) = static_cast<std::uint32_t>(std::atoi(v62));
                    char* v63 = std::strtok(nullptr, Delimiter);
                    if (!v63) goto Lfail;
                    *reinterpret_cast<std::uint32_t*>(v7 + 196) = static_cast<std::uint32_t>(std::atoi(v63));
                }

                // ── 37..46. 확장_페이스1..10 (10 strings × 16 bytes, off 340..) ──
                //   GT: 10 보다 적게 토큰이 오면 outer break → return 0
                {
                    int cnt = 0;
                    v74 = v7 + 340;
                    while (true) {
                        char* v64 = std::strtok(nullptr, Delimiter);
                        if (!v64) goto Lfail;
                        std::strcpy(v74, v64);
                        v74 += 16;
                        ++cnt;
                        if (cnt >= 10) break;
                    }
                }

                // ── 47..116. 퀘스트01..70 (70 WORD TranslateKindCode, off 200..338) ─
                //   글로벌 중복 검출 + 메시지 박스 + abort
                {
                    int cnt = 0;
                    v74 = v7 + 200;
                    while (true) {
                        char* v67 = std::strtok(nullptr, Delimiter);
                        if (!v67) goto Lfail;

                        std::uint16_t v68 = TranslateKindCode(v67);
                        *reinterpret_cast<std::uint16_t*>(v74) = v68;
                        std::uint16_t v69 = static_cast<std::uint16_t>(v76);
                        v86[v69] = v68;

                        if (v68) {
                            int v70 = 0;
                            if (v69 > 0) {
                                std::uint16_t* v71 = v86;
                                while (!*v71 || *v71 != v68) {
                                    ++v70;
                                    ++v71;
                                    if (v70 >= v69) goto Lnotdup;
                                }
                                wsprintfA(Text, "%i",
                                    *reinterpret_cast<std::uint16_t*>(v7 + 2 * cnt + 200));
                                MessageBoxA(nullptr, Text, "quest is duplicated", 0);
                                goto Lfail;
                            }
                        Lnotdup:
                            ++v76;
                        }
                        v74 += 2;
                        ++cnt;
                        if (cnt >= 70) {
                            // 70 quest 완료 → NPC 타입 처리 및 다음 row
                            GetNPCType(String1, reinterpret_cast<std::uint8_t*>(v7 + 94));
                            v7 += 500;
                            if (std::fgets(Buffer, 10239, Stream)) {
                                goto Lnextrow;  // 위쪽 while 의 새 iteration 으로
                            }
                            v77 = 1;
                            goto Lfail;  // 정상 종료 (v77 == 1)
                        }
                    }
                }

            Lnextrow:;
        }
    }

Lfail:
    g_clTextFileManager.fclose(Stream);
    return v77;
}
