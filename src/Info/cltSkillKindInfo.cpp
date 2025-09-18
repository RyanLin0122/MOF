#include "Info/cltSkillKindInfo.h"
#include "Info/cltClassKindInfo.h"
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
    if (ieq(s, "BOW"))       return 1;
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
        strClassKindInfo* info = m_pclClassKindInfo->GetClassKindInfo(cls);
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
        else if (ieq(tok, "BOW"))        idx = 8;
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
    uint16_t prev_code = *reinterpret_cast<uint16_t*>(r->raw + 70);
    while (prev_code != 0) {
        stSkillKindInfo* prev_rec = GetSkillKindInfo(prev_code);
        if (!prev_rec) break;
        ++level;
        prev_code = *reinterpret_cast<uint16_t*>(prev_rec->raw + 70);
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
    uint16_t list[64]; int n = m_pclClassKindInfo->GetClassKindsByAtb(atb, list);
    for (int i = 0; i < n; i++) if (auto* c = m_pclClassKindInfo->GetClassKindInfo(list[i])) if (_classLevel(c) <= 1) return 1;
    return 0;
}
int cltSkillKindInfo::IsTwoLevelClassSkill(int64_t atb) {
    uint16_t list[64]; int n = m_pclClassKindInfo->GetClassKindsByAtb(atb, list);
    for (int i = 0; i < n; i++) if (auto* c = m_pclClassKindInfo->GetClassKindInfo(list[i])) if (_classLevel(c) == 2) return 1;
    return 0;
}
int cltSkillKindInfo::IsUnderThreeLevelClassSkill(int64_t atb) {
    uint16_t list[64]; int n = m_pclClassKindInfo->GetClassKindsByAtb(atb, list);
    for (int i = 0; i < n; i++) if (auto* c = m_pclClassKindInfo->GetClassKindInfo(list[i])) if (_classLevel(c) <= 2) return 1;
    return 0;
}
int cltSkillKindInfo::IsThreeLevelClassSkill(int64_t atb) {
    uint16_t list[64]; int n = m_pclClassKindInfo->GetClassKindsByAtb(atb, list);
    for (int i = 0; i < n; i++) if (auto* c = m_pclClassKindInfo->GetClassKindInfo(list[i])) if (_classLevel(c) == 3) return 1;
    return 0;
}
int cltSkillKindInfo::IsFourLevelClassSkill(int64_t atb) {
    uint16_t list[64]; int n = m_pclClassKindInfo->GetClassKindsByAtb(atb, list);
    for (int i = 0; i < n; i++) if (auto* c = m_pclClassKindInfo->GetClassKindInfo(list[i])) if (_classLevel(c) == 4) return 1;
    return 0;
}

// 依職業取可用主/被動技能（沿職業樹往上）
int cltSkillKindInfo::GetClassActiveSkill(uint16_t startClass, uint16_t* out)
{
    int outN = 0;
    for (auto* cur = m_pclClassKindInfo->GetClassKindInfo(startClass);
        cur;
        cur = m_pclClassKindInfo->GetClassKindInfo(_classNext(cur)))
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
    for (auto* cur = m_pclClassKindInfo->GetClassKindInfo(startClass);
        cur;
        cur = m_pclClassKindInfo->GetClassKindInfo(_classNext(cur)))
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
    for (auto& r : m_passive) {
        uint16_t code = *reinterpret_cast<const uint16_t*>(r.raw + 0);
        if (code == 0xFFFF) return 0;
        if (m_idx[code])     return 0; // 重複的 Code
        m_idx[code] = &r;
    }
    for (auto& r : m_active) {
        uint16_t code = *reinterpret_cast<const uint16_t*>(r.raw + 0);
        if (code == 0xFFFF) return 0;
        if (m_idx[code])     return 0; // 重複的 Code
        m_idx[code] = &r;
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
    for (int i = 0; i < 3; ++i) {
        if (!std::fgets(line, sizeof(line), fp)) {
            g_clTextFileManager.fclose(fp); return 0;
        }
    }

    fpos_t pos; std::fgetpos(fp, &pos);
    size_t count = 0; while (std::fgets(line, sizeof(line), fp)) ++count;
    m_passive.assign(count, stSkillKindInfo{});
    std::fsetpos(fp, &pos);

    size_t idx = 0;
    while (std::fgets(line, sizeof(line), fp)) {
        char* tok = std::strtok(line, DELIM);
        if (!tok) continue; // 空行

        stSkillKindInfo& R = m_passive[idx];
        std::memset(R.raw, 0, sizeof(R.raw)); //確保每次都是乾淨的

        auto W_U16 = [&](int offset, uint16_t val) { *reinterpret_cast<uint16_t*>(R.raw + offset) = val; };
        auto W_U32 = [&](int offset, uint32_t val) { *reinterpret_cast<uint32_t*>(R.raw + offset) = val; };

        // 1) ID
        uint16_t code = TranslateKindCode(tok); W_U16(0, code);
        if (!code || !IsPassiveSkill(code)) { g_clTextFileManager.fclose(fp); return 0; }

        // 2) 초기ID
        if (!NextTok(tok, DELIM)) break; W_U16(2, TranslateKindCode(tok));

        // 3) 이름（略）
        if (!NextTok(tok, DELIM)) break;

        // 4) 이름 코드
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(4, std::atoi(tok));

        // 5) 圖標ID
        if (!NextTok(tok, DELIM) || !IsHexLike(tok)) break; W_U32(8, std::strtoul(tok, nullptr, 16));

        // 6) 圖標區塊
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(12, std::atoi(tok));

        // 7) 小圖ID
        if (!NextTok(tok, DELIM) || !IsHexLike(tok)) break; W_U32(16, std::strtoul(tok, nullptr, 16));

        // 8) 小圖區塊
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(20, std::atoi(tok));

        // 9) 상위 스킬 코드
        if (!NextTok(tok, DELIM)) break; W_U16(70, TranslateKindCode(tok));

        // 10) 스킬 설명
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(6, std::atoi(tok)); // 根據反編譯碼，這裡是說明碼，對應到+6

        // 11) 스킬 짧은 설명
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(112, std::atoi(tok)); // 根據反編譯碼，這裡是短說明碼，對應到+112

        // 12) 스킬 구매 타입
        if (!NextTok(tok, DELIM)) break; W_U32(48, GetSkillClass(tok));

        // 13~16) 四系點數
        for (int ofs = 52; ofs <= 64; ofs += 4) {
            if (!NextTok(tok, DELIM) || !IsDigits(tok)) goto parse_error;
            W_U32(ofs, std::atoi(tok));
        }

        // 17) 使用時必要武器
        std::string needWeapon;
        if (!NextTok(tok, DELIM)) break; needWeapon = tok;

        // 18~21)
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(114, std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(116, std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U32(24, std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U32(28, std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U32(120, std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U32(124, std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U32(128, std::atoi(tok));

        // 22) 필요 레벨
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(68, std::atoi(tok));

        // 23) 습득 클래스
        std::string acquireClass;
        if (!NextTok(tok, DELIM)) break; acquireClass = tok;

        // 24~39)
        const int ofsA[] = { 164,168,172,176,180,184,188,192, 132,136,140,144,148,152,156,160 };
        for (int of : ofsA) { if (!NextTok(tok, DELIM) || !IsDigits(tok)) goto parse_error; W_U32(of, std::atoi(tok)); }

        // 40)
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U32(196, std::atoi(tok));

        // 41) 工作型態
        if (!NextTok(tok, DELIM)) break; W_U32(248, GetWorkingType(tok));

        // 42)
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U32(200, std::atoi(tok));

        // 43) 連結技能字串
        std::string linkSkills;
        if (!NextTok(tok, DELIM)) break; linkSkills = tok;

        // 44) 攻擊目標屬性
        if (!NextTok(tok, DELIM)) break;
        *reinterpret_cast<int32_t*>(R.raw + 252) = (tok[0] == '0' && tok[1] == '\0') ? -1 : cltAttackAtb::GetAttackTargetAtb(tok);

        // 45) +396
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U32(396, std::atoi(tok));

        // 46~... 剩餘的數值欄
        const int ofsC[] = {
             256, 268, 260, 264, 272, 276, 280, 284, 288, 292, 296, 300, 304, 308, 312, 316, 320, 324, 328,
             332, 336, 340, 344, 348, 352, 356, 360, 364, 368, 372, 376, 380, 384, 388, 392
        };
        for (int of : ofsC) { if (!NextTok(tok, DELIM) || !IsDigits(tok)) goto parse_error; W_U32(of, std::atoi(tok)); }

        // EffectKindCodes
        const int ofsD[] = { 92, 94, 96, 98, 108, 110, 100, 102, 104, 106 };
        for (int of : ofsD) { if (!NextTok(tok, DELIM)) goto parse_error; W_U16(of, TranslateEffectKindCode(tok)); }

        // --- 後處理 ---
        // 類別遮罩
        uint64_t reqClassMask = GetReqClasses(this, (char*)acquireClass.c_str());
        *reinterpret_cast<uint64_t*>(R.raw + 40) = reqClassMask;

        // 武器遮罩
        int weaponCount = GetReqWeaponClasses(this, (char*)needWeapon.c_str(), R.raw + 72);
        W_U32(88, weaponCount);

        // 連結技能
        if (linkSkills != "0") {
            char* link_ctx = (char*)linkSkills.c_str();
            char* link_tok = std::strtok(link_ctx, "&");
            uint32_t& link_count = *reinterpret_cast<uint32_t*>(R.raw + 244);
            link_count = 0;
            while (link_tok && link_count < 10) {
                uint16_t link_code = TranslateKindCode(link_tok);
                if (link_code) {
                    *reinterpret_cast<uint16_t*>(R.raw + 204 + link_count * 4) = link_code;
                    link_count++;
                }
                link_tok = std::strtok(nullptr, "&");
            }
        }

        idx++;
    }

parse_error:
    g_clTextFileManager.fclose(fp);
    return idx == count; // 如果解析的行數等於總行數，視為成功
}

// ──────────────────────────────────────────────────────────────────────────
// 逐檔初始化：主動（a_skillinfo.txt）
// ──────────────────────────────────────────────────────────────────────────
int cltSkillKindInfo::Initialize_A(char* filename)
{
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    char line[4096]; const char* DELIM = "\t\n";
    for (int i = 0; i < 3; ++i) {
        if (!std::fgets(line, sizeof(line), fp)) {
            g_clTextFileManager.fclose(fp); return 0;
        }
    }

    fpos_t pos; std::fgetpos(fp, &pos);
    size_t count = 0; while (std::fgets(line, sizeof(line), fp)) ++count;
    m_active.assign(count, stSkillKindInfo{});
    std::fsetpos(fp, &pos);

    size_t idx = 0;
    while (std::fgets(line, sizeof(line), fp)) {
        char* tok = std::strtok(line, DELIM);
        if (!tok) continue;

        stSkillKindInfo& R = m_active[idx];
        std::memset(R.raw, 0, sizeof(R.raw));

        auto W_U8 = [&](int offset, uint8_t val) { R.raw[offset] = val; };
        auto W_U16 = [&](int offset, uint16_t val) { *reinterpret_cast<uint16_t*>(R.raw + offset) = val; };
        auto W_U32 = [&](int offset, uint32_t val) { *reinterpret_cast<uint32_t*>(R.raw + offset) = val; };

        // 1-8) 基本資訊
        uint16_t code = TranslateKindCode(tok); W_U16(0, code);
        if (!code || !IsActiveSkill(code)) { g_clTextFileManager.fclose(fp); return 0; }
        if (!NextTok(tok, DELIM)) break; W_U16(2, TranslateKindCode(tok));
        if (!NextTok(tok, DELIM)) break; // 이름
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(4, std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsHexLike(tok)) break; W_U32(8, std::strtoul(tok, nullptr, 16));
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(12, std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsHexLike(tok)) break; W_U32(16, std::strtoul(tok, nullptr, 16));
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(20, std::atoi(tok));

        // 9) 스킬 타입 (ActiveEffectType)
        if (!NextTok(tok, DELIM)) break; W_U8(112, GetActiveEffectType(tok));

        // 10) 이전 스킬 코드
        if (!NextTok(tok, DELIM)) break; W_U16(70, TranslateKindCode(tok));

        // 11) 스킬설명코드
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(6, std::atoi(tok));

        // 12-24) 大量數值欄
        const int ofsA[] = { 114, 116, 24, 28, 132, 134, 136, 138, 120, 124, 128, 140, 144, 36 };
        for (int of : ofsA) { if (!NextTok(tok, DELIM) || !IsDigits(tok)) goto a_parse_error; W_U32(of, std::atoi(tok)); }

        // 25) 습득 클래스
        std::string acquireClass;
        if (!NextTok(tok, DELIM)) break; acquireClass = tok;

        // 26) 스킬 구매 타입
        if (!NextTok(tok, DELIM)) break; W_U32(48, GetSkillClass(tok));

        // 27-30) 四系點數
        for (int ofs = 52; ofs <= 64; ofs += 4) {
            if (!NextTok(tok, DELIM) || !IsDigits(tok)) goto a_parse_error;
            W_U32(ofs, std::atoi(tok));
        }

        // 31) 使用時必要武器
        std::string needWeapon;
        if (!NextTok(tok, DELIM)) break; needWeapon = tok;

        // 32) 필요 레벨
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(68, std::atoi(tok));

        // 33) 사운드
        if (!NextTok(tok, DELIM)) break; std::strncpy(reinterpret_cast<char*>(R.raw + 148), tok, 15); R.raw[148 + 15] = '\0';

        // 34) 원거리 여부
        if (!NextTok(tok, DELIM)) break; if (std::strcmp(tok, "LONG") == 0) W_U8(164, 1); else W_U8(164, 0);

        // 35-37)
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(166, std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U16(168, std::atoi(tok));

        // 38) 시전 애니
        if (!NextTok(tok, DELIM)) break;
        {
            const char* anim = tok;
            if (strcmp(anim, "NORMAL") == 0)      W_U8(170, 0);
            else if (strcmp(anim, "SPELL") == 0)  W_U8(170, 1);
            else if (strcmp(anim, "PRAY") == 0)   W_U8(170, 2);
            else if (strcmp(anim, "TRIPLE") == 0) W_U8(170, 3);
            else if (strcmp(anim, "DOUBLE") == 0) W_U8(170, 4);
        }

        // 39) 맞는 속성 (element_tag)
        if (!NextTok(tok, DELIM)) break;
        {
            const char* elem = tok;
            if (strcmp(elem, "NONE") == 0)         W_U8(171, 0);
            else if (strcmp(elem, "NORMAL") == 0)  W_U8(171, 1);
            else if (strcmp(elem, "FIRE") == 0)    W_U8(171, 2);
            else if (strcmp(elem, "ELEC") == 0)    W_U8(171, 3);
            else if (strcmp(elem, "ICE") == 0)     W_U8(171, 4);
        }

        // 40) 공격 속성
        if (!NextTok(tok, DELIM)) break; W_U32(172, cltAttackAtb::GetAttackAtb(tok));

        // 41) 파티 공격속성
        if (!NextTok(tok, DELIM)) break;
        if (tok[0] == '0' && tok[1] == '\0') {
            W_U32(244, 0);
        }
        else {
            W_U32(244, 1);
            W_U32(248, cltAttackAtb::GetAttackAtb(tok));
        }

        // 42-47) 기절/봉쇄/결빙 확률 및 시간
        const int ofsB[] = { 176, 180, 184, 188, 192, 196 };
        for (int of : ofsB) { if (!NextTok(tok, DELIM) || !IsDigits(tok)) goto a_parse_error; W_U32(of, std::atoi(tok)); }

        // 48) mine 아이디
        if (!NextTok(tok, DELIM)) break; W_U16(200, cltMineKindInfo::TranslateKindCode(tok));

        // 49-70) 大量數值欄
        const int ofsC[] = { 204, 208, 252, 256, 260, 264, 268, 272, 280, 284, 288, 292, 296, 300, 212, 216, 220, 224, 228, 232, 236, 240 };
        for (int of : ofsC) { if (!NextTok(tok, DELIM) || !IsDigits(tok)) goto a_parse_error; W_U32(of, std::atoi(tok)); }

        // 71-77)
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; *reinterpret_cast<int32_t*>(R.raw + 276) = std::atoi(tok);
        if (!NextTok(tok, DELIM)) break; // 이펙트 단계
        if (!NextTok(tok, DELIM)) break; W_U16(300, std::atoi(tok));
        if (!NextTok(tok, DELIM)) break; W_U16(302, std::atoi(tok));
        if (!NextTok(tok, DELIM)) break; W_U16(304, cltDebuffKindInfo::TranslateKindCode(tok));
        if (!NextTok(tok, DELIM)) break; W_U32(308, std::atoi(tok));
        if (!NextTok(tok, DELIM)) break; W_U32(312, std::atoi(tok));
        if (!NextTok(tok, DELIM)) break; W_U32(316, std::atoi(tok));

        // 78) 무기 공격방식
        if (!NextTok(tok, DELIM)) break; W_U32(320, (strcmp(tok, "MULTI") == 0));

        // 79) 몬스터를 중심으로 적용되는 스킬
        if (!NextTok(tok, DELIM) || !IsDigits(tok)) break; W_U32(324, std::atoi(tok));

        // 80-90) EffectKindCodes
        const int ofsD[] = { 92, 94, 96, 98, 328, 108, 110, 100, 102, 104, 106 };
        for (int of : ofsD) { if (!NextTok(tok, DELIM)) goto a_parse_error; W_U16(of, TranslateEffectKindCode(tok)); }

        // --- 後處理 ---
        uint64_t reqClassMask = GetReqClasses(this, (char*)acquireClass.c_str());
        *reinterpret_cast<uint64_t*>(R.raw + 40) = reqClassMask;

        int weaponCount = GetReqWeaponClasses(this, (char*)needWeapon.c_str(), R.raw + 72);
        W_U32(88, weaponCount);

        idx++;
    }

a_parse_error:
    g_clTextFileManager.fclose(fp);
    return idx == count;
}

// ==========================================================================
//
//   cltSkillKindInfoM 的實作
//
// ==========================================================================

cltSkillKindInfoM::cltSkillKindInfoM() {
    // m_pList, m_aList 由 std::vector 自動初始化
    // m_idx 已透過成員初始化列表清為 nullptr
}

void cltSkillKindInfoM::Free() {
    m_pList.clear(); m_pList.shrink_to_fit();
    m_aList.clear(); m_aList.shrink_to_fit();
    std::memset(m_idx, 0, sizeof(m_idx));
}

int cltSkillKindInfoM::Initialize(char* pfile, char* afile)
{
    if (!Initialize_P(pfile)) {
        return 0;
    }
    if (!Initialize_A(afile)) { // 雖然 A 檔不做事，但仍依反編譯流程呼叫
        return 0;
    }

    // 建立被動技能索引
    for (auto& rec : m_pList) {
        // 取得技能碼
        uint16_t code = *reinterpret_cast<uint16_t*>(rec.raw);
        if (code == 0xFFFF) {
            return 0; // 錯誤碼
        }
        // 檢查是否重複
        if (m_idx[code]) {
            return 0; // 重複的 ID
        }
        // 建立索引，將 Rec28* 轉型為 stSkillKindInfo* 以符合 m_idx 宣告
        m_idx[code] = reinterpret_cast<stSkillKindInfo*>(&rec);
    }

    // 建立主動技能索引（雖然 M 版沒讀 A 檔，但反編譯碼有此迴圈）
    for (auto& rec : m_aList) {
        uint16_t code = *reinterpret_cast<uint16_t*>(rec.raw);
        if (code == 0xFFFF) {
            return 0;
        }
        if (m_idx[code]) {
            return 0;
        }
        m_idx[code] = reinterpret_cast<stSkillKindInfo*>(&rec);
    }

    return 1;
}

int cltSkillKindInfoM::Initialize_P(char* filename)
{
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) {
        return 0;
    }

    char line[4096];
    const char* DELIM = "\t\n";

    // 跳過 3 行標頭
    for (int i = 0; i < 3; ++i) {
        if (!std::fgets(line, sizeof(line), fp)) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }
    }

    // 預先計算行數以分配記憶體
    fpos_t pos;
    std::fgetpos(fp, &pos);
    size_t count = 0;
    while (std::fgets(line, sizeof(line), fp)) {
        count++;
    }
    m_pList.assign(count, Rec28{});
    std::fsetpos(fp, &pos);

    // 逐行解析
    size_t current_idx = 0;
    while (current_idx < count && std::fgets(line, sizeof(line), fp)) {
        Rec28& R = m_pList[current_idx];
        std::memset(R.raw, 0, sizeof(R.raw));

        // 1) ID
        char* tok = std::strtok(line, DELIM);
        if (!tok) continue; // 空行

        uint16_t code = TranslateKindCode(tok);
        *reinterpret_cast<uint16_t*>(R.raw + 0) = code;
        if (!code || !IsPassiveSkill(code)) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }

        // 2) 초기 ID (反編譯碼中跳過此欄)
        if (!cltSkillKindInfo::NextTok(tok, DELIM)) break;

        // 3) 이름 (反編譯碼中跳過此欄)
        if (!cltSkillKindInfo::NextTok(tok, DELIM)) break;

        // 4) 이름 코드
        if (!cltSkillKindInfo::NextTok(tok, DELIM) || !cltSkillKindInfo::IsDigits(tok)) break;
        *reinterpret_cast<uint32_t*>(R.raw + 4) = std::atoi(tok);

        // 5) 아이콘 그림 ID (反編譯碼當作純數字解析)
        if (!cltSkillKindInfo::NextTok(tok, DELIM) || !cltSkillKindInfo::IsDigits(tok)) break;
        *reinterpret_cast<uint32_t*>(R.raw + 8) = std::atoi(tok);

        // 6) 아이콘 그림 블록
        if (!cltSkillKindInfo::NextTok(tok, DELIM) || !cltSkillKindInfo::IsDigits(tok)) break;
        *reinterpret_cast<uint32_t*>(R.raw + 12) = std::atoi(tok);

        // 7) 작은 아이콘 그림 ID
        if (!cltSkillKindInfo::NextTok(tok, DELIM) || !cltSkillKindInfo::IsDigits(tok)) break;
        *reinterpret_cast<uint32_t*>(R.raw + 16) = std::atoi(tok);

        // 8) 작은 아이콘 그림 블록
        if (!cltSkillKindInfo::NextTok(tok, DELIM) || !cltSkillKindInfo::IsDigits(tok)) break;
        *reinterpret_cast<uint32_t*>(R.raw + 20) = std::atoi(tok);

        // 9) 상위 스킬 코드 (反編譯碼當作純數字解析)
        if (!cltSkillKindInfo::NextTok(tok, DELIM) || !cltSkillKindInfo::IsDigits(tok)) break;
        *reinterpret_cast<uint32_t*>(R.raw + 24) = std::atoi(tok);

        current_idx++;
    }

    g_clTextFileManager.fclose(fp);

    // 如果解析的行數與計數不符，視為失敗
    if (current_idx != count) {
        return 0;
    }

    return 1;
}