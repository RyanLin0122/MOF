#include "Info/cltSkillKindInfo.h"
#include <cassert>

// 靜態
cltClassKindInfo* cltSkillKindInfo::m_pclClassKindInfo = nullptr;

// ──────────────────────────────────────────────────────────────────────────
// 小工具
// ──────────────────────────────────────────────────────────────────────────
bool cltSkillKindInfo::IsDigits(const char* s) {
    if (!s || !*s) return false;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p)
        if (!std::isdigit(*p)) return false;
    return true;
}
bool cltSkillKindInfo::IsHexLike(const char* s) {
    if (!s || !*s) return false;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p)
        if (!std::isxdigit(*p)) return false;
    return true;
}
bool cltSkillKindInfo::NextTok(char*& out, const char* delims) {
    out = std::strtok(nullptr, delims);
    return out != nullptr;
}

// ──────────────────────────────────────────────────────────────────────────
// 建構/清理
// ──────────────────────────────────────────────────────────────────────────
cltSkillKindInfo::cltSkillKindInfo() {
    std::memset(m_idx, 0, sizeof(m_idx));
}
void cltSkillKindInfo::Free() {
    m_passive.clear(); m_passive.shrink_to_fit();
    m_active.clear();  m_active.shrink_to_fit();
    std::memset(m_idx, 0, sizeof(m_idx));
}

// ──────────────────────────────────────────────────────────────────────────
// 代碼轉換（嚴格依反編譯）
// ──────────────────────────────────────────────────────────────────────────
uint16_t cltSkillKindInfo::TranslateKindCode(const char* s)
{
    if (!s || std::strlen(s) != 6) return 0;
    int ch = std::toupper(static_cast<unsigned char>(s[0]));
    uint16_t hi = 0;
    if (ch == 'A') hi = 0x8000;
    else if (ch == 'P') hi = 0;
    else return 0;
    long v = std::atoi(s + 1);
    if (v < 0 || v >= 0x8000) return 0;
    return static_cast<uint16_t>(hi | static_cast<uint16_t>(v));
}
uint16_t cltSkillKindInfo::TranslateEffectKindCode(const char* s)
{
    if (!s || std::strlen(s) != 5) return 0;
    int base = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    long num = std::atoi(s + 1);
    if (num < 0 || num >= 0x800) return 0;
    return static_cast<uint16_t>(base | (num & 0x7FF));
}
bool cltSkillKindInfo::IsPassiveSkill(uint16_t a1) {
    return a1 >= TranslateKindCode("P00001") && a1 <= TranslateKindCode("P32767");
}
bool cltSkillKindInfo::IsActiveSkill(uint16_t a1) {
    return a1 >= TranslateKindCode("A00001") && a1 <= TranslateKindCode("A32767");
}

// ──────────────────────────────────────────────────────────────────────────
// 枚舉/對應（嚴格依反編譯）
// ──────────────────────────────────────────────────────────────────────────
int cltSkillKindInfo::GetWorkingType(const char* s)
{
#ifdef _WIN32
    auto ieq = [](const char* a, const char* b) { return _stricmp(a, b) == 0; };
#else
    auto ieq = [](const char* a, const char* b) { return strcasecmp(a, b) == 0; };
#endif
    if (ieq(s, "ATTACK_ONCE"))     return 1;
    if (ieq(s, "ATTACK_SUSTAIN"))  return 2;
    if (ieq(s, "HITTED_ONCE"))     return 3;
    if (ieq(s, "HITTED_SUSTAIN"))  return 4;
    if (ieq(s, "USESKILL_ONCE"))   return 5;
    // 位元技巧的同義：其他→6；"USESKILL_SUSTAIN"→6
    return 6;
}
uint8_t cltSkillKindInfo::GetActiveEffectType(const char* s)
{
#ifdef _WIN32
    auto ieq = [](const char* a, const char* b) { return _stricmp(a, b) == 0; };
#else
    auto ieq = [](const char* a, const char* b) { return strcasecmp(a, b) == 0; };
#endif
    if (ieq(s, "ATTACK_ONE"))   return 1;
    if (ieq(s, "ATTACK_MANY"))  return 2;
    if (ieq(s, "SUSTAIN"))      return 3;
    if (ieq(s, "HEAL_MYSELF"))  return 4;
    if (ieq(s, "HEAL_OTHER"))   return 5;
    if (ieq(s, "HEAL_OTHERS"))  return 6;
    if (ieq(s, "RESURRECT"))    return 7;
    if (ieq(s, "MINE"))         return 8;
    if (ieq(s, "RECALL"))       return 9;
    if (ieq(s, "HEAL_PARTY"))   return 10;
    return ieq(s, "DEBUFF") ? 11 : 0;
}
uint8_t cltSkillKindInfo::GetActiveEffectType(uint16_t code)
{
    stSkillKindInfo* rec = GetSkillKindInfo_A(code);
    return rec ? rec->raw[112] : 0;
}
int cltSkillKindInfo::GetSkillClass(const char* s)
{
#ifdef _WIN32
    auto ieq = [](const char* a, const char* b) { return _stricmp(a, b) == 0; };
#else
    auto ieq = [](const char* a, const char* b) { return strcasecmp(a, b) == 0; };
#endif
    // 反編譯分支：SWORD=0, BOW=1, THEOLOGY=2, MAGIC=3, SPECIALTY=4
    if (ieq(s, "SWORD"))     return 0;
    if (ieq(s, "BOW"))       return 1;  // 反編譯裡的 `string` 即 BOW
    if (ieq(s, "THEOLOGY"))  return 2;
    if (ieq(s, "MAGIC"))     return 3;
    return 4; // SPECIALTY 與其他皆落到此
}

// ──────────────────────────────────────────────────────────────────────────
// 需求解析（職業＆武器）；對齊反編譯介面與語意
// ──────────────────────────────────────────────────────────────────────────
uint64_t cltSkillKindInfo::GetReqClasses(cltSkillKindInfo* /*self*/, char* s)
{
#ifdef _WIN32
    auto ieq = [](const char* a, const char* b) { return _stricmp(a, b) == 0; };
#else
    auto ieq = [](const char* a, const char* b) { return strcasecmp(a, b) == 0; };
#endif
    if (!s) return 0;
    if (ieq(s, "NONE") || ieq(s, "0")) return 0;

    // strtok 會破壞字串，複製一份
    std::string buf = s;
    char* ctx = buf.data();
    char* tok = std::strtok(ctx, "|");
    uint64_t mask = 0;
    while (tok) {
        uint16_t cls = cltClassKindInfo::TranslateKindCode(tok);
        strClassKindInfo* info = cltClassKindInfo::GetClassKindInfo(m_pclClassKindInfo, cls);
        if (info) {
            // 反編譯在某些地方以 DWORD[2] 讀，也以 QWORD 讀
            // 這裡以 QWORD 讀取 +8 位移（等價 *((_QWORD*)info + 1)）
            const uint64_t* q = reinterpret_cast<const uint64_t*>((const uint8_t*)info + 8);
            mask |= *q;
        }
        tok = std::strtok(nullptr, "|");
    }
    return mask;
}
int cltSkillKindInfo::GetReqWeaponClasses(cltSkillKindInfo* /*self*/, char* s, uint8_t* const flags)
{
#ifdef _WIN32
    auto ieq = [](const char* a, const char* b) { return _stricmp(a, b) == 0; };
#else
    auto ieq = [](const char* a, const char* b) { return strcasecmp(a, b) == 0; };
#endif
    if (!s) return 0;
    if (ieq(s, "NONE") || ieq(s, "0")) return 0;

    int cnt = 0;
    std::string buf = s;
    char* ctx = buf.data();
    char* tok = std::strtok(ctx, "|");
    while (tok) {
        int idx = -1;
        if (ieq(tok, "LONG_SWORD"))      idx = 1;
        else if (ieq(tok, "SHORT_SWORD"))idx = 2;
        else if (ieq(tok, "HAMMER"))     idx = 3;
        else if (ieq(tok, "LONG_SWORD_TH")) idx = 4;
        else if (ieq(tok, "TOMAHAWK"))   idx = 5;
        else if (ieq(tok, "SPEAR"))      idx = 6;
        else if (ieq(tok, "STAFF"))      idx = 7;
        else if (ieq(tok, "BOW"))        idx = 8;   // 反編譯中的 `string`
        else if (ieq(tok, "GUN"))        idx = 9;
        else if (ieq(tok, "DUAL_SWORD")) idx = 10;
        else if (ieq(tok, "CROSSBOW"))   idx = 11;
        else if (ieq(tok, "SPELLBOOK"))  idx = 12;
        else if (ieq(tok, "HOLYCROSS"))  idx = 13;
        else if (ieq(tok, "SHIELD"))     idx = 14;

        if (idx >= 0) { flags[idx] = 1; ++cnt; }
        tok = std::strtok(nullptr, "|");
    }
    return cnt;
}

// ──────────────────────────────────────────────────────────────────────────
// 索引/存取
// ──────────────────────────────────────────────────────────────────────────
stSkillKindInfo* cltSkillKindInfo::GetSkillKindInfo(uint16_t code) {
    return m_idx[code];
}
stSkillKindInfo* cltSkillKindInfo::GetSkillKindInfo_P(uint16_t code) {
    return IsPassiveSkill(code) ? m_idx[code] : nullptr;
}
stSkillKindInfo* cltSkillKindInfo::GetSkillKindInfo_A(uint16_t code) {
    return IsActiveSkill(code) ? m_idx[code] : nullptr;
}
char* cltSkillKindInfo::UseSkillSound(uint16_t code) {
    stSkillKindInfo* r = GetSkillKindInfo(code);
    return r ? reinterpret_cast<char*>(r->raw + 148) : nullptr;
}

// ──────────────────────────────────────────────────────────────────────────
// 技能等級計算與標記
// ──────────────────────────────────────────────────────────────────────────
uint16_t cltSkillKindInfo::GetSkillLevel(uint16_t code)
{
    uint16_t level = 1;
    stSkillKindInfo* r = GetSkillKindInfo(code);
    if (!r) return 0;
    // 依「上一級技能碼」（+70）一路往回追
    for (uint16_t prev = *reinterpret_cast<uint16_t*>(r->raw + 70);
        prev != 0;
        prev = *reinterpret_cast<uint16_t*>(GetSkillKindInfo(prev)->raw + 70))
    {
        ++level;
    }
    return level;
}
int cltSkillKindInfo::IsLastLevelSkill(uint16_t code)
{
    // 在 P/A 陣列中檢查是否存在「上一級=code」的記錄，若有則非最後一級
    for (const auto& r : m_passive) {
        if (*reinterpret_cast<const uint16_t*>(r.raw + 70) == code) return 0;
    }
    for (const auto& r : m_active) {
        if (*reinterpret_cast<const uint16_t*>(r.raw + 70) == code) return 0;
    }
    return 1;
}

// ──────────────────────────────────────────────────────────────────────────
// 職業鏈條類別判定（等級層，依 strClassKindInfo 偏移使用）
// ──────────────────────────────────────────────────────────────────────────
static inline uint8_t _classLevel(const strClassKindInfo* info) {
    // 反編譯：*(((uint8_t*)info)+18)
    return *reinterpret_cast<const uint8_t*>((const uint8_t*)info + 18);
}
static inline uint16_t _classNext(const strClassKindInfo* info) {
    // 反編譯：*(((uint16_t*)info)+10) → +20
    return *reinterpret_cast<const uint16_t*>((const uint8_t*)info + 20);
}
int cltSkillKindInfo::IsUnderTwoLevelClassSkill(int64_t atb) {
    uint16_t list[64]; int n = cltClassKindInfo::GetClassKindsByAtb(m_pclClassKindInfo, atb, list);
    for (int i = 0; i < n; i++) if (auto* c = cltClassKindInfo::GetClassKindInfo(m_pclClassKindInfo, list[i])) if (_classLevel(c) <= 1) return 1;
    return 0;
}
int cltSkillKindInfo::IsTwoLevelClassSkill(int64_t atb) {
    uint16_t list[64]; int n = cltClassKindInfo::GetClassKindsByAtb(m_pclClassKindInfo, atb, list);
    for (int i = 0; i < n; i++) if (auto* c = cltClassKindInfo::GetClassKindInfo(m_pclClassKindInfo, list[i])) if (_classLevel(c) == 2) return 1;
    return 0;
}
int cltSkillKindInfo::IsUnderThreeLevelClassSkill(int64_t atb) {
    uint16_t list[64]; int n = cltClassKindInfo::GetClassKindsByAtb(m_pclClassKindInfo, atb, list);
    for (int i = 0; i < n; i++) if (auto* c = cltClassKindInfo::GetClassKindInfo(m_pclClassKindInfo, list[i])) if (_classLevel(c) <= 2) return 1;
    return 0;
}
int cltSkillKindInfo::IsThreeLevelClassSkill(int64_t atb) {
    uint16_t list[64]; int n = cltClassKindInfo::GetClassKindsByAtb(m_pclClassKindInfo, atb, list);
    for (int i = 0; i < n; i++) if (auto* c = cltClassKindInfo::GetClassKindInfo(m_pclClassKindInfo, list[i])) if (_classLevel(c) == 3) return 1;
    return 0;
}
int cltSkillKindInfo::IsFourLevelClassSkill(int64_t atb) {
    uint16_t list[64]; int n = cltClassKindInfo::GetClassKindsByAtb(m_pclClassKindInfo, atb, list);
    for (int i = 0; i < n; i++) if (auto* c = cltClassKindInfo::GetClassKindInfo(m_pclClassKindInfo, list[i])) if (_classLevel(c) == 4) return 1;
    return 0;
}

// 依職業取可用主/被動技能（沿職業樹往上）
int cltSkillKindInfo::GetClassActiveSkill(uint16_t startClass, uint16_t* out)
{
    int outN = 0;
    for (auto* cur = cltClassKindInfo::GetClassKindInfo(m_pclClassKindInfo, startClass);
        cur;
        cur = cltClassKindInfo::GetClassKindInfo(m_pclClassKindInfo, _classNext(cur)))
    {
        // 取 class 遮罩（QWORD，在 +8）
        uint64_t clsMask = *reinterpret_cast<const uint64_t*>((const uint8_t*)cur + 8);
        for (const auto& r : m_active) {
            if (*reinterpret_cast<const uint16_t*>(r.raw + 70) == 0) { // 非升級鏈（前置為 0）
                uint64_t skillMask = *reinterpret_cast<const uint64_t*>(r.raw + 40);
                if ((clsMask & skillMask) != 0) {
                    out[outN++] = *reinterpret_cast<const uint16_t*>(r.raw + 0);
                }
            }
        }
    }
    return outN;
}
int cltSkillKindInfo::GetClassPassiveSkill(uint16_t startClass, uint16_t* out)
{
    int outN = 0;
    for (auto* cur = cltClassKindInfo::GetClassKindInfo(m_pclClassKindInfo, startClass);
        cur;
        cur = cltClassKindInfo::GetClassKindInfo(m_pclClassKindInfo, _classNext(cur)))
    {
        uint64_t clsMask = *reinterpret_cast<const uint64_t*>((const uint8_t*)cur + 8);
        for (const auto& r : m_passive) {
            if (*reinterpret_cast<const uint16_t*>(r.raw + 70) == 0) {
                uint64_t skillMask = *reinterpret_cast<const uint64_t*>(r.raw + 40);
                if ((clsMask & skillMask) != 0) {
                    out[outN++] = *reinterpret_cast<const uint16_t*>(r.raw + 0);
                }
            }
        }
    }
    return outN;
}

// ──────────────────────────────────────────────────────────────────────────
// 主要初始化（P→A），並建立索引與等級
// ──────────────────────────────────────────────────────────────────────────
int cltSkillKindInfo::Initialize(char* pfile, char* afile)
{
    if (!Initialize_P(pfile)) return 0;
    if (!Initialize_A(afile)) return 0;

    // 檢查並建立索引：每一筆的 +0（技能碼）放入 m_idx[code]
    for (const auto& r : m_passive) {
        uint16_t code = *reinterpret_cast<const uint16_t*>(r.raw + 0);
        if (code == 0xFFFF) return 0;
        if (m_idx[code])     return 0;
        m_idx[code] = const_cast<stSkillKindInfo*>(&r);
    }
    for (const auto& r : m_active) {
        uint16_t code = *reinterpret_cast<const uint16_t*>(r.raw + 0);
        if (code == 0xFFFF) return 0;
        if (m_idx[code])     return 0;
        m_idx[code] = const_cast<stSkillKindInfo*>(&r);
    }

    // 回寫每筆的 +32 = 等級（沿鏈條追上去）
    for (auto& r : m_passive) {
        uint16_t code = *reinterpret_cast<uint16_t*>(r.raw + 0);
        r.raw[32] = static_cast<uint8_t>(GetSkillLevel(code));
    }
    for (auto& r : m_active) {
        uint16_t code = *reinterpret_cast<uint16_t*>(r.raw + 0);
        r.raw[32] = static_cast<uint8_t>(GetSkillLevel(code));
    }
    return 1;
}

// ──────────────────────────────────────────────────────────────────────────
// 逐檔初始化：被動（p_skillinfo.txt）
// ──────────────────────────────────────────────────────────────────────────
int cltSkillKindInfo::Initialize_P(char* filename)
{
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    char line[4096]; const char* DELIM = "\t\n";
    // Skip 3 header lines
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp); return 0;
    }

    // Count
    fpos_t pos; std::fgetpos(fp, &pos);
    size_t count = 0; while (std::fgets(line, sizeof(line), fp)) ++count;
    m_passive.assign(count, stSkillKindInfo{}); std::memset(m_passive.data(), 0, count * sizeof(stSkillKindInfo));
    std::fsetpos(fp, &pos);

    size_t idx = 0;
    while (std::fgets(line, sizeof(line), fp)) {
        char* tok = std::strtok(line, DELIM);
        if (!tok) break;

        stSkillKindInfo& R = m_passive[idx];
        // 1) ID
        uint16_t code = TranslateKindCode(tok); *reinterpret_cast<uint16_t*>(R.raw + 0) = code;
        if (!code || !IsPassiveSkill(code)) { g_clTextFileManager.fclose(fp); return 0; }

        // 2) 초기ID
        if (!NextTok(tok, DELIM)) break;
        uint16_t initCode = TranslateKindCode(tok); *reinterpret_cast<uint16_t*>(R.raw + 2) = initCode; if (!initCode) break;

        // 3) 이름（略）
        if (!NextTok(tok, DELIM)) break;

        // 4) 이름 코드
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break;
        *reinterpret_cast<uint16_t*>(R.raw + 4) = static_cast<uint16_t>(std::atoi(tok));

        // 5) 圖標ID（十六進位）
        if (!NextTok(tok, DELIM) || !IsHexLike(tok)) break;
        { unsigned int v = 0; std::sscanf(tok, "%x", &v); *reinterpret_cast<uint32_t*>(R.raw + 8) = v; }

        // 6) 圖標區塊
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break;
        *reinterpret_cast<uint16_t*>(R.raw + 12) = static_cast<uint16_t>(std::atoi(tok));

        // 7) 小圖ID（十六進位）
        if (!NextTok(tok, DELIM) || !IsHexLike(tok)) break;
        { unsigned int v = 0; std::sscanf(tok, "%x", &v); *reinterpret_cast<uint32_t*>(R.raw + 16) = v; }

        // 8) 小圖區塊
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break;
        *reinterpret_cast<uint16_t*>(R.raw + 20) = static_cast<uint16_t>(std::atoi(tok));

        // 9) 상위 스킬 코드
        if (!NextTok(tok, DELIM)) break;
        *reinterpret_cast<uint16_t*>(R.raw + 70) = TranslateKindCode(tok);

        // 10) 스킬 설명（略）
        if (!NextTok(tok, DELIM)) break;

        // 11) 스킬 짧은 설명（略）
        if (!NextTok(tok, DELIM)) break;

        // 12) 스킬 구매 타입
        if (!NextTok(tok, DELIM)) break;
        *reinterpret_cast<uint32_t*>(R.raw + 48) = GetSkillClass(tok);

        // 13~16) 四系點數
        for (int ofs = 52, k = 0; k < 4; ++k, ofs += 4) {
            if (!NextTok(tok, DELIM) || !IsDigits(tok)) break;
            *reinterpret_cast<uint32_t*>(R.raw + ofs) = std::atoi(tok);
        }

        // 17) 使用時必要武器
        std::string needWeapon;
        if (!NextTok(tok, DELIM)) break;
        needWeapon = tok;

        // 18~21) HP/MP/攻防百分比系列與/或數值（依表；原碼對應 +24,+28,+120,+124,+128）
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; *reinterpret_cast<uint32_t*>(R.raw + 24) = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; *reinterpret_cast<uint32_t*>(R.raw + 28) = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; *reinterpret_cast<uint32_t*>(R.raw + 120) = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; *reinterpret_cast<uint32_t*>(R.raw + 124) = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; *reinterpret_cast<uint32_t*>(R.raw + 128) = std::atoi(tok);

        // 22) 필요 레벨
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break;
        *reinterpret_cast<uint16_t*>(R.raw + 68) = static_cast<uint16_t>(std::atoi(tok));

        // 23) 습득 클래스（保留字串，稍後算遮罩）
        std::string acquireClass;
        if (!NextTok(tok, DELIM)) break;
        acquireClass = tok;

        // 24~31) 一長串被動數值欄（+164,168,172,176,180,184,188,192）
        const int ofsA[] = { 164,168,172,176,180,184,188,192 };
        for (int of : ofsA) { if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; *reinterpret_cast<uint32_t*>(R.raw + of) = std::atoi(tok); }

        // 32~39) +132..+160
        const int ofsB[] = { 132,136,140,144,148,152,156,160 };
        for (int of : ofsB) { if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; *reinterpret_cast<uint32_t*>(R.raw + of) = std::atoi(tok); }

        // 40) +196
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break;
        *reinterpret_cast<uint32_t*>(R.raw + 196) = std::atoi(tok);

        // 41) 工作型態（+248）
        if (!NextTok(tok, DELIM)) break;
        *reinterpret_cast<uint32_t*>(R.raw + 248) = GetWorkingType(tok);

        // 42) +200
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break;
        *reinterpret_cast<uint32_t*>(R.raw + 200) = std::atoi(tok);

        // 43) 字串（保存，供後續判斷攻擊目標屬性）
        std::string aux;
        if (!NextTok(tok, DELIM)) break;
        aux = tok;

        // 44) 攻擊目標屬性（+252）：若 "0" → -1，否則以 AttackTargetAtb 對應
        if (!NextTok(tok, DELIM)) break;
        *reinterpret_cast<uint32_t*>(R.raw + 252) = (tok[0] == '0' && tok[1] == '\0') ? -1 : cltAttackAtb::GetAttackTargetAtb(tok);

        // 45) +396
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break;
        *reinterpret_cast<uint32_t*>(R.raw + 396) = std::atoi(tok);

        // 46~64) 一系列 +256..+392（見反編譯序列；逐個數字欄）
        const int ofsC[] = {
            256,268,260,264,272,276,280,284,288,292,296,300,304,308,312,316,320,324,328,108,110,100,102,104,106,92,94,96,98
        };
        for (int of : ofsC) {
            if (!NextTok(tok, DELIM)) break;
            if (of == 304) { // Debuff 代碼
                *reinterpret_cast<uint16_t*>(R.raw + 304) = cltDebuffKindInfo::TranslateKindCode(tok);
            }
            else if (of == 328 || of == 108 || of == 110 || of == 100 || of == 102 || of == 104 || of == 106 || of == 92 || of == 94 || of == 96 || of == 98) {
                *reinterpret_cast<uint16_t*>(R.raw + of) = cltSkillKindInfo::TranslateEffectKindCode(tok);
            }
            else if (of == 292) {
                *reinterpret_cast<uint8_t*>(R.raw + 292) = static_cast<uint8_t>(std::atoi(tok));
            }
            else {
                if (!IsDigits(tok)) break;
                *reinterpret_cast<uint32_t*>(R.raw + of) = std::atoi(tok);
            }
        }

        // Mine ID (+200) 在這個版本反編譯於前面出現；若你有更精準欄位，可再對齊

        // 類別遮罩（+40,+