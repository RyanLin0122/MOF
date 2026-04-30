#include "Info/cltMineKindInfo.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>


// 對應 mofclient.c:342909 — 空字串視為合法 (回 1)，可接受一個前置 '+'/'-'，
// 其餘必須全部為十進位數字。
static int IsDigit(const char* a1)
{
    if (!a1 || !*a1) return 1;
    const unsigned char* p = reinterpret_cast<const unsigned char*>(a1);
    if (*p == '+' || *p == '-') ++p;
    while (*p) {
        if (!std::isdigit(*p)) return 0;
        ++p;
    }
    return 1;
}

// 對應 mofclient.c:342945 — 空字串視為合法 (回 1)，每個字元須通過 isalnum。
static int IsAlphaNumeric(const char* a1)
{
    if (!a1 || !*a1) return 1;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(a1); *p; ++p) {
        if (!std::isalnum(*p)) return 0;
    }
    return 1;
}

cltMineKindInfo::cltMineKindInfo()
    : m_count(0), m_list(nullptr)
{}

// 反編譯：mofclient.c:315277-315498
//   1) 跳過前 3 行（標題 / 空行 / 欄位列）
//   2) fgetpos 後逐行 fgets 數筆數，operator new(56 * count) + memset
//   3) fsetpos 回到資料起點，逐行 strtok 解析；任何欄位失敗即回 0，
//      唯有最後一次 fgets 取到 NULL 才將回傳值設為 1。
int cltMineKindInfo::Initialize(const char* path)
{
    char delim[] = "\t\n";
    int  ok = 0;

    FILE* fp = g_clTextFileManager.fopen((char*)path);
    if (!fp) return 0;

    char buf[1024];
    if (!std::fgets(buf, 1023, fp) ||
        !std::fgets(buf, 1023, fp) ||
        !std::fgets(buf, 1023, fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    fpos_t pos{};
    std::fgetpos(fp, &pos);
    m_count = 0;
    while (std::fgets(buf, 1023, fp)) {
        ++m_count;
    }

    // 反編譯就算 m_count == 0 也照樣 operator new(0) + memset(0)；保持等價。
    void* mem = ::operator new(static_cast<size_t>(56 * m_count));
    m_list = reinterpret_cast<strMineKindInfo*>(mem);
    if (m_count > 0) {
        std::memset(m_list, 0, static_cast<size_t>(56 * m_count));
    }

    std::fsetpos(fp, &pos);

    // 第一筆資料先讀進來（反編譯把 fgets 放在 while(1) 之外）；若沒有任何資料行
    // 則直接走「成功 EOF」分支。
    if (!std::fgets(buf, 1023, fp)) {
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    int idxBytes = 0; // 反編譯以 v7 += 56 推進
    bool fail = false;

    while (true) {
        strMineKindInfo* rec = reinterpret_cast<strMineKindInfo*>(
            reinterpret_cast<unsigned char*>(m_list) + idxBytes);

        // [0] M코드 → kind
        char* t = std::strtok(buf, delim);
        if (!t) { fail = true; break; }
        rec->kind = TranslateKindCode(t);

        // [1] 지뢰 명 — 反編譯只 strtok 取出檢查非 null，不存。
        if (!std::strtok(nullptr, delim)) { fail = true; break; }

        // [2] 스킬 아이디 → skill
        t = std::strtok(nullptr, delim);
        if (!t) { fail = true; break; }
        rec->skill = TranslateKindCodeToSkill(t);

        // [3] 공격력 (+0x04)
        t = std::strtok(nullptr, delim);
        if (!t || !IsDigit(t)) { fail = true; break; }
        rec->attack = static_cast<std::uint32_t>(std::atoi(t));

        // [4] 공격범위 (+0x08)
        t = std::strtok(nullptr, delim);
        if (!t || !IsDigit(t)) { fail = true; break; }
        rec->attackRange = static_cast<std::uint32_t>(std::atoi(t));

        // [5] 인식 범위 (+0x0C)
        t = std::strtok(nullptr, delim);
        if (!t || !IsDigit(t)) { fail = true; break; }
        rec->detectRange = static_cast<std::uint32_t>(std::atoi(t));

        // [6] 봉쇄율 (+0x10)，必須 < 1000
        t = std::strtok(nullptr, delim);
        if (!t || !IsDigit(t)) { fail = true; break; }
        rec->blockRatePermille = static_cast<std::uint32_t>(std::atoi(t));
        if (rec->blockRatePermille >= 1000) { fail = true; break; }

        // [7] 봉쇄 지속 시간 (+0x14)
        t = std::strtok(nullptr, delim);
        if (!t || !IsDigit(t)) { fail = true; break; }
        rec->blockDurationMs = static_cast<std::uint32_t>(std::atoi(t));

        // [8] 마인 소멸시간 (+0x28) — 反編譯這裡跳到 +0x28 寫入。
        t = std::strtok(nullptr, delim);
        if (!t || !IsDigit(t)) { fail = true; break; }
        rec->disappearMs = static_cast<std::uint32_t>(std::atoi(t));

        // [9] 기절확률 (+0x18)，必須 < 1000
        t = std::strtok(nullptr, delim);
        if (!t || !IsDigit(t)) { fail = true; break; }
        rec->stunRatePermille = static_cast<std::uint32_t>(std::atoi(t));
        if (rec->stunRatePermille >= 1000) { fail = true; break; }

        // [10] 기절 지속시간 (+0x1C)
        t = std::strtok(nullptr, delim);
        if (!t || !IsDigit(t)) { fail = true; break; }
        rec->stunDurationMs = static_cast<std::uint32_t>(std::atoi(t));

        // [11] 빙결확률 (+0x20)，必須 < 1000
        t = std::strtok(nullptr, delim);
        if (!t || !IsDigit(t)) { fail = true; break; }
        rec->freezeRatePermille = static_cast<std::uint32_t>(std::atoi(t));
        if (rec->freezeRatePermille >= 1000) { fail = true; break; }

        // [12] 빙결 지속시간 (+0x24)
        t = std::strtok(nullptr, delim);
        if (!t || !IsDigit(t)) { fail = true; break; }
        rec->freezeDurationMs = static_cast<std::uint32_t>(std::atoi(t));

        // [13] 탐색 방벙 (+0x2C, byte)
        t = std::strtok(nullptr, delim);
        if (!t || !IsDigit(t)) { fail = true; break; }
        rec->airGround = static_cast<std::uint8_t>(std::atoi(t));

        // [14] 공격 타입 (+0x2D, byte) — "FIX"=1 / "MOVING"=2
        t = std::strtok(nullptr, delim);
        if (!t) { fail = true; break; }
        if (std::strcmp(t, "FIX") == 0)         rec->moveType = 1;
        else if (std::strcmp(t, "MOVING") == 0) rec->moveType = 2;
        else { fail = true; break; }

        // [15] 공격 숫자 (+0x2E, byte) — "ONE"=1 / "MULTI"=2
        t = std::strtok(nullptr, delim);
        if (!t) { fail = true; break; }
        if (std::strcmp(t, "ONE") == 0)         rec->attackCountType = 1;
        else if (std::strcmp(t, "MULTI") == 0)  rec->attackCountType = 2;
        else { fail = true; break; }

        // [16] 리소스ID (+0x30, dword) — sscanf("%x")
        t = std::strtok(nullptr, delim);
        if (!t || !IsAlphaNumeric(t)) { fail = true; break; }
        std::sscanf(t, "%x", &rec->resourceIdHex);

        // [17] 토탈플레임 (+0x34, word) — 反編譯只 atoi，不做 IsDigit 檢查
        t = std::strtok(nullptr, delim);
        if (!t) { fail = true; break; }
        rec->totalFlame = static_cast<std::uint16_t>(std::atoi(t));

        // 下一筆：先進前 56 bytes，再 fgets 取下一行；EOF 表示成功。
        idxBytes += 56;
        if (!std::fgets(buf, 1023, fp)) {
            ok = 1;
            break;
        }
    }

    g_clTextFileManager.fclose(fp);
    return fail ? 0 : ok;
}

void cltMineKindInfo::Free()
{
    if (m_list) {
        ::operator delete(m_list);
        m_list = nullptr;
    }
    m_count = 0;
}

// 反編譯：mofclient.c:315513 — 以 56 bytes 為步距線性掃描。
strMineKindInfo* cltMineKindInfo::GetMineKindInfo(std::uint16_t code)
{
    if (m_count <= 0 || !m_list) return nullptr;
    for (int i = 0; i < m_count; ++i) {
        strMineKindInfo* rec = reinterpret_cast<strMineKindInfo*>(
            reinterpret_cast<unsigned char*>(m_list) + 56 * i);
        if (rec->kind == code) return rec;
    }
    return nullptr;
}

// 反編譯：mofclient.c:315534 — 長度必須 5；((toupper(s[0])+31)<<11) | atoi(s+1)，
// 若 atoi 結果 >= 0x800 則回 0。
std::uint16_t cltMineKindInfo::TranslateKindCode(const char* a1)
{
    if (!a1 || std::strlen(a1) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    unsigned v = static_cast<unsigned>(std::atoi(a1 + 1));
    if (v < 0x800u) return static_cast<std::uint16_t>(hi | v);
    return 0;
}

// 反編譯：mofclient.c:315552 — 長度必須 6；'A' → 0x8000, 'P' → 0x0000；
// 若 atoi 結果 >= 0x8000 則回 0。
std::uint16_t cltMineKindInfo::TranslateKindCodeToSkill(const char* a1)
{
    if (!a1 || std::strlen(a1) != 6) return 0;
    int c = std::toupper(static_cast<unsigned char>(a1[0]));
    std::uint16_t hi = 0;
    if (c == 'A')      hi = 0x8000;
    else if (c == 'P') hi = 0x0000;
    else return 0;

    unsigned v = static_cast<unsigned>(std::atoi(a1 + 1));
    if (v < 0x8000u) return static_cast<std::uint16_t>(hi | v);
    return 0;
}
