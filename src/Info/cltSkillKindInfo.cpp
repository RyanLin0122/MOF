#include "Info/cltSkillKindInfo.h"
#include "Info/cltClassKindInfo.h"
#include <cassert>

// 靜態
cltClassKindInfo* cltSkillKindInfo::m_pclClassKindInfo = nullptr;

// ──────────────────────────────────────────────────────────────────────────
// 小工具（與 mofclient.c 內 IsDigit/IsAlphaNumeric 行為一致）
// ──────────────────────────────────────────────────────────────────────────
bool cltSkillKindInfo::IsDigit(const char* s) {
    // 反編譯：空字串回 true；首字可為 '+' 或 '-' 一次；其餘必須全為 0-9
    if (!s) return false;
    if (!*s) return true;
    const char* p = s;
    if (*p == '+' || *p == '-') ++p;
    while (*p) {
        if (!std::isdigit(static_cast<unsigned char>(*p))) return false;
        ++p;
    }
    return true;
}
bool cltSkillKindInfo::IsAlphaNumeric(const char* s) {
    if (!s) return false;
    if (!*s) return true;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p)
        if (!std::isalnum(*p)) return false;
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
    // 其他/USESKILL_SUSTAIN → 6
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
    return rec ? rec->u.active.effectType : 0;
}
int cltSkillKindInfo::GetSkillClass(const char* s)
{
#ifdef _WIN32
    auto ieq = [](const char* a, const char* b) { return _stricmp(a, b) == 0; };
#else
    auto ieq = [](const char* a, const char* b) { return strcasecmp(a, b) == 0; };
#endif
    // SWORD=0, BOW=1, THEOLOGY=2, MAGIC=3, SPECIALTY/其他=4
    if (ieq(s, "SWORD"))     return 0;
    if (ieq(s, "BOW"))       return 1;
    if (ieq(s, "THEOLOGY"))  return 2;
    if (ieq(s, "MAGIC"))     return 3;
    return 4;
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

    // GT: strtok 直接破壞傳入字串（呼叫端傳入即為可改寫的暫存）
    char* tok = std::strtok(s, "|");
    uint64_t mask = 0;
    while (tok) {
        uint16_t cls = cltClassKindInfo::TranslateKindCode(tok);
        strClassKindInfo* info = m_pclClassKindInfo->GetClassKindInfo(cls);
        if (info) {
            // 反編譯以兩個 DWORD 個別 OR 累加；等價於以 QWORD 讀取 +8 後 OR
            const uint64_t* q = reinterpret_cast<const uint64_t*>((const uint8_t*)info + 8);
            mask |= *q;
        }
        tok = std::strtok(nullptr, "|");
    }
    return mask;
}
int cltSkillKindInfo::GetReqWeaponClasses(cltSkillKindInfo* /*self*/, char* s, uint8_t* flags)
{
#ifdef _WIN32
    auto ieq = [](const char* a, const char* b) { return _stricmp(a, b) == 0; };
#else
    auto ieq = [](const char* a, const char* b) { return strcasecmp(a, b) == 0; };
#endif
    if (!s) return 0;
    if (ieq(s, "NONE") || ieq(s, "0")) return 0;

    int cnt = 0;
    // GT: strtok 直接破壞傳入字串（呼叫端傳入即為可改寫的暫存）
    char* tok = std::strtok(s, "|");
    // 反編譯：v4 初始化為 String1（即指標位址）；若 token 不在已知清單中，
    // v4 仍會被當索引使用（這是 GT 的危險行為）。為了安全又對應實作，這裡若
    // token 無法辨識則跳過，與 GT 對於合法資料下行為一致。
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

        if (idx >= 0) { flags[idx] = 1; }
        ++cnt;  // GT 不論 token 是否辨識皆累加 v8
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
    return r ? r->u.active.soundName : nullptr;
}

// ──────────────────────────────────────────────────────────────────────────
// 技能等級計算與標記
// ──────────────────────────────────────────────────────────────────────────
uint16_t cltSkillKindInfo::GetSkillLevel(uint16_t code)
{
    uint16_t level = 1;
    stSkillKindInfo* r = GetSkillKindInfo(code);
    if (!r) return 0;
    // 依「上一級技能碼」(prevSkillCode @ +70) 一路往回追
    uint16_t prev_code = r->prevSkillCode;
    while (prev_code != 0) {
        stSkillKindInfo* prev_rec = GetSkillKindInfo(prev_code);
        if (!prev_rec) break;
        ++level;
        prev_code = prev_rec->prevSkillCode;
    }
    return level;
}
int cltSkillKindInfo::IsLastLevelSkill(uint16_t code)
{
    // 反編譯：先掃 P 表 prevSkillCode，若有等於 code 則 return 0；
    //         否則掃 A 表 prevSkillCode，若有等於 code 則 return 0；皆無則 return 1。
    for (const auto& r : m_passive) {
        if (r.prevSkillCode == code) return 0;
    }
    for (const auto& r : m_active) {
        if (r.prevSkillCode == code) return 0;
    }
    return 1;
}

// ──────────────────────────────────────────────────────────────────────────
// IsCircleSkillKind  (005945E0)
//   GT：result = GetSkillKindInfo_P(this, code);
//       if (result) result = (passive.durationMin != 0);  // 偏移 +256
// ──────────────────────────────────────────────────────────────────────────
int cltSkillKindInfo::IsCircleSkillKind(cltSkillKindInfo* self, uint16_t code)
{
    stSkillKindInfo* result = self->GetSkillKindInfo_P(code);
    if (!result) return 0;
    return result->u.passive.durationMin != 0 ? 1 : 0;
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
            if (r.prevSkillCode == 0) { // 非升級鏈（前置為 0）
                if ((clsMask & r.acquireClassMask) != 0) {
                    out[outN++] = r.skillCode;
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
            if (r.prevSkillCode == 0) {
                if ((clsMask & r.acquireClassMask) != 0) {
                    out[outN++] = r.skillCode;
                }
            }
        }
    }
    return outN;
}

// ──────────────────────────────────────────────────────────────────────────
// 主要初始化（P→A），並建立索引與等級
//   反編譯：先 Initialize_P，再 Initialize_A，再以 skillCode 為 key 建立 m_idx，
//           最後將 skillLevel 填入 GetSkillLevel 結果。
// ──────────────────────────────────────────────────────────────────────────
int cltSkillKindInfo::Initialize(char* pfile, char* afile)
{
    if (!Initialize_P(pfile)) return 0;
    if (!Initialize_A(afile)) return 0;

    // 建立索引：每一筆的 skillCode 放入 m_idx[code]
    for (auto& r : m_passive) {
        if (r.skillCode == 0xFFFF) return 0;
        if (m_idx[r.skillCode])    return 0; // 重複的 Code
        m_idx[r.skillCode] = &r;
    }
    for (auto& r : m_active) {
        if (r.skillCode == 0xFFFF) return 0;
        if (m_idx[r.skillCode])    return 0;
        m_idx[r.skillCode] = &r;
    }

    // 回寫每筆的 skillLevel = 等級（沿鏈條追上去）
    for (auto& r : m_passive) {
        r.skillLevel = static_cast<uint8_t>(GetSkillLevel(r.skillCode));
    }
    for (auto& r : m_active) {
        r.skillLevel = static_cast<uint8_t>(GetSkillLevel(r.skillCode));
    }
    return 1;
}

// ──────────────────────────────────────────────────────────────────────────
// 逐檔初始化：被動（p_skillinfo.txt）共 93 欄
// 解析順序與型別嚴格對應 GT (mofclient.c: 005913B0)
// ──────────────────────────────────────────────────────────────────────────
int cltSkillKindInfo::Initialize_P(char* filename)
{
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    char line[4096]; const char* DELIM = "\t\n";
    // 跳過 3 行表頭
    for (int i = 0; i < 3; ++i) {
        if (!std::fgets(line, sizeof(line), fp)) {
            g_clTextFileManager.fclose(fp); return 0;
        }
    }

    // 計算實際資料列數，並一次配置
    fpos_t pos; std::fgetpos(fp, &pos);
    size_t count = 0; while (std::fgets(line, sizeof(line), fp)) ++count;
    m_passive.assign(count, stSkillKindInfo{});
    std::fsetpos(fp, &pos);

    int success = 0;       // GT v183：成功旗標
    size_t idx = 0;
    while (std::fgets(line, sizeof(line), fp)) {
        // GT 在每一行內以一個 while(1) 包裹解析；任一檢查失敗即 break 結束整個函式。
        char* tok = std::strtok(line, DELIM);
        if (!tok) break;

        if (idx >= count) break;
        stSkillKindInfo& R = m_passive[idx];
        std::memset(&R, 0, sizeof(R));
        auto& P = R.u.passive;

        // 1) ID
        R.skillCode = TranslateKindCode(tok);
        if (!R.skillCode || !IsPassiveSkill(R.skillCode)) break;

        // 2) 초기 ID
        if (!NextTok(tok, DELIM)) break;
        R.initialSkillCode = TranslateKindCode(tok);
        if (!R.initialSkillCode) break;

        // 3) 이름（略過）
        if (!NextTok(tok, DELIM)) break;

        // 4) 이름 코드
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        R.nameCode = static_cast<uint16_t>(std::atoi(tok));

        // 5) 圖標 ID (hex)
        if (!NextTok(tok, DELIM) || !IsAlphaNumeric(tok)) break;
        R.iconID = std::strtoul(tok, nullptr, 16);

        // 6) 圖標區塊
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        R.iconBlock = static_cast<uint16_t>(std::atoi(tok));

        // 7) 小圖 ID (hex)
        if (!NextTok(tok, DELIM) || !IsAlphaNumeric(tok)) break;
        R.smallIconID = std::strtoul(tok, nullptr, 16);

        // 8) 小圖區塊
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        R.smallIconBlock = static_cast<uint16_t>(std::atoi(tok));

        // 9) 상위 스킬 코드
        if (!NextTok(tok, DELIM)) break;
        R.prevSkillCode = TranslateKindCode(tok);

        // 10) 스킬 설명
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        R.descCode = static_cast<uint16_t>(std::atoi(tok));

        // 11) 스킬 짧은 설명
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        P.shortDescCode = static_cast<uint16_t>(std::atoi(tok));

        // 12) 스킬 구매 타입
        if (!NextTok(tok, DELIM)) break;
        R.skillBuyClass = GetSkillClass(tok);

        // 13~16) 4 系點數
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; R.needFigPt = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; R.needArcPt = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; R.needMagPt = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; R.needPriPt = std::atoi(tok);

        // 17) 사용시 필요 무기
        std::string needWeapon;
        if (!NextTok(tok, DELIM)) break; needWeapon = tok;

        // 18~24)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; P.hpUpPct = static_cast<uint16_t>(std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; P.mpUpPct = static_cast<uint16_t>(std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; R.attackPowerUp = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; R.defensePowerUp = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; P.hitRate = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; P.critRateUp = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; P.skillAtkPowerPct = std::atoi(tok);

        // 25) 필요 레벨
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        R.reqLevel = static_cast<uint16_t>(std::atoi(tok));

        // 26) 습득 클래스
        std::string acquireClass;
        if (!NextTok(tok, DELIM)) break; acquireClass = tok;

        // 27~42) 16 個 DWORD 欄位（順序與 GT 完全一致）
        // GT 寫入順序：atkAdj 8 系 (164,168,172,176,180,184,188,192) 後接 defAdj 8 系 (132..160)
        uint32_t* const adjFields[] = {
            &P.atkAdjMonsterEtc,    // +164
            &P.atkAdjMonsterStone,  // +168
            &P.atkAdjMonsterPlant,  // +172
            &P.atkAdjMonsterAlien,  // +176
            &P.atkAdjSpirit,        // +180
            &P.atkAdjCurse,         // +184
            &P.atkAdjDevil,         // +188
            &P.atkAdjDragon,        // +192
            &P.defAdjMonsterEtc,    // +132
            &P.defAdjMonsterStone,  // +136
            &P.defAdjMonsterPlant,  // +140
            &P.defAdjMonsterAlien,  // +144
            &P.defAdjSpirit,        // +148
            &P.defAdjCurse,         // +152
            &P.defAdjDevil,         // +156
            &P.defAdjDragon         // +160
        };
        bool ok = true;
        for (uint32_t* dst : adjFields) {
            if (!NextTok(tok, DELIM) || !IsDigit(tok)) { ok = false; break; }
            *dst = std::atoi(tok);
        }
        if (!ok) break;

        // 43) triggerRate (+196)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        P.triggerRate = std::atoi(tok);

        // 44) workingType (+248) = GetWorkingType (字串)
        if (!NextTok(tok, DELIM)) break;
        P.workingType = GetWorkingType(tok);

        // 45) triggerTime (+200)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        P.triggerTime = std::atoi(tok);

        // 46) 連動技能字串
        std::string linkSkillsStr;
        if (!NextTok(tok, DELIM)) break; linkSkillsStr = tok;

        // 47) 攻擊目標屬性 (+252)
        // GT (mofclient.c 330900): *v92 == 48 ? -1 : cltAttackAtb::GetAttackTargetAtb(v92);
        // GT 只判斷字串第一個字元是否為 '0'，並非整串等於 "0"。
        if (!NextTok(tok, DELIM)) break;
        P.triggerAttackTargetAtb = (tok[0] == '0')
            ? -1
            : cltAttackAtb::GetAttackTargetAtb(tok);

        // 48) monsterCritAdj (+396)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        P.monsterCritAdj = std::atoi(tok);

        // 49~83) 35 個 DWORD 欄位（GT 嚴格順序）
        uint32_t* const tailFields[] = {
            &P.durationMin,          // +256
            &P.maxConcurrentCycle,   // +268
            &P.triggerOnHpChange,    // +260
            &P.triggerOnSkillUse,    // +264
            &P.addRecoverPermil,     // +272
            &P.addHpRecoverPermil,   // +276
            &P.addMpRecoverPermil,   // +280
            &P.party2pAtkUp,         // +284
            &P.party2pDefUp,         // +288
            &P.party2pHitUp,         // +292
            &P.party3pAtkUp,         // +296
            &P.party3pDefUp,         // +300
            &P.party3pHitUp,         // +304
            &P.party4pAtkUp,         // +308
            &P.party4pDefUp,         // +312
            &P.party4pHitUp,         // +316
            &P.party5pAtkUp,         // +320
            &P.party5pDefUp,         // +324
            &P.party5pHitUp,         // +328
            &P.hpRecover,            // +332
            &P.mpRecover,            // +336
            &P.atkBlockProb,         // +340
            &P.atkBlockTime,         // +344
            &P.atkFreezeProb,        // +348
            &P.atkFreezeTime,        // +352
            &P.atkStunProb,          // +356
            &P.atkStunTime,          // +360
            &P.attackSpeed,          // +364
            &P.damageReceiveConvert, // +368
            &P.skillAfterAdvantage,  // +372
            &P.monsterDamageReduce,  // +376
            &P.monsterDefAdj,        // +380
            &P.monsterAtkAdj,        // +384
            &P.monsterHitAdj,        // +388
            &P.monsterEvadeAdj       // +392
        };
        for (uint32_t* dst : tailFields) {
            if (!NextTok(tok, DELIM) || !IsDigit(tok)) { ok = false; break; }
            *dst = std::atoi(tok);
        }
        if (!ok) break;

        // 84~93) 10 個 EffectKindCode（GT 順序：1,2,3,4,5,6,7,8,9,10
        //         寫入順序：effectTopUp1, effectTopUp2, effectBotDown1, effectBotDown2,
        //                   effectFlyTop, effectFlyBot, effectHittedTopUp1, effectHittedTopUp2,
        //                   effectHittedBotDown1, effectHittedBotDown2）
        uint16_t* const fxFields[] = {
            &R.effectTopUp1,          // +92
            &R.effectTopUp2,          // +94
            &R.effectBotDown1,        // +96
            &R.effectBotDown2,        // +98
            &R.effectFlyTop,          // +108
            &R.effectFlyBot,          // +110
            &R.effectHittedTopUp1,    // +100
            &R.effectHittedTopUp2,    // +102
            &R.effectHittedBotDown1,  // +104
            &R.effectHittedBotDown2   // +106
        };
        for (uint16_t* dst : fxFields) {
            if (!NextTok(tok, DELIM)) { ok = false; break; }
            *dst = TranslateEffectKindCode(tok);
        }
        if (!ok) break;

        // ── 後處理 ──────────────────────────────────────────────────────
        // 類別遮罩 → acquireClassMask；GT 會原地破壞字串，傳 .data() 即可
        R.acquireClassMask = GetReqClasses(this, acquireClass.data());

        // 武器旗標 → reqWeaponFlags；數量 → reqWeaponCount
        R.reqWeaponCount = GetReqWeaponClasses(this, needWeapon.data(), R.reqWeaponFlags);

        // 連動技能：以 '&' 切分；GT 將 v179（uint16）以 DWORD 寫入，count 在 +244。
        // GT 失敗條件 (mofclient.c 331198~331213)：
        //   (1) TranslateKindCode 回 0 → inner break → 落到 outer break，整列解析失敗
        //   (2) linkSkillCount 已達 10 仍有未處理 token → while 條件不成立 → outer break，失敗
        // 成功路徑：以 strtok 取得 NULL 時 goto LABEL_185 (跳過 outer break)。
        if (linkSkillsStr != "0") {
            char* link_ctx = (char*)linkSkillsStr.c_str();
            char* link_tok = std::strtok(link_ctx, "&");
            if (link_tok) {
                bool link_done = false;
                while (P.linkSkillCount < 10) {
                    uint16_t link_code = TranslateKindCode(link_tok);
                    if (!link_code) break;                 // GT (1)
                    P.linkSkills[P.linkSkillCount++] = link_code;
                    link_tok = std::strtok(nullptr, "&");
                    if (!link_tok) { link_done = true; break; }   // GT goto LABEL_185
                }
                if (!link_done) break;                     // GT outer break (失敗)
            }
            // 第一個 strtok 即為 NULL：直接落到 LABEL_185（成功）
        }

        ++idx;
    }

    if (idx == count) success = 1;
    g_clTextFileManager.fclose(fp);
    return success;
}

// ──────────────────────────────────────────────────────────────────────────
// 逐檔初始化：主動（a_skillinfo.txt）共 89 欄
// 解析順序與型別嚴格對應 GT (mofclient.c: 005927C0)
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

    int success = 0;
    size_t idx = 0;
    while (std::fgets(line, sizeof(line), fp)) {
        char* tok = std::strtok(line, DELIM);
        if (!tok) break;

        if (idx >= count) break;
        stSkillKindInfo& R = m_active[idx];
        std::memset(&R, 0, sizeof(R));
        auto& A = R.u.active;

        // 1) ID
        R.skillCode = TranslateKindCode(tok);
        if (!R.skillCode || !IsActiveSkill(R.skillCode)) break;

        // 2) 초기 ID
        if (!NextTok(tok, DELIM)) break;
        R.initialSkillCode = TranslateKindCode(tok);
        if (!R.initialSkillCode) break;

        // 3) 이름（略過）
        if (!NextTok(tok, DELIM)) break;

        // 4) 이름 코드
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        R.nameCode = static_cast<uint16_t>(std::atoi(tok));

        // 5) 아이콘 그림 ID (hex)
        if (!NextTok(tok, DELIM) || !IsAlphaNumeric(tok)) break;
        R.iconID = std::strtoul(tok, nullptr, 16);

        // 6) 아이콘 그림 블록
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        R.iconBlock = static_cast<uint16_t>(std::atoi(tok));

        // 7) 작은 아이콘 그림 ID (hex)
        if (!NextTok(tok, DELIM) || !IsAlphaNumeric(tok)) break;
        R.smallIconID = std::strtoul(tok, nullptr, 16);

        // 8) 작은 아이콘 그림 블록
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        R.smallIconBlock = static_cast<uint16_t>(std::atoi(tok));

        // 9) 스킬 타입 (BYTE) → effectType
        if (!NextTok(tok, DELIM)) break;
        A.effectType = GetActiveEffectType(tok);

        // 10) 이전 스킬 코드
        if (!NextTok(tok, DELIM)) break;
        R.prevSkillCode = TranslateKindCode(tok);

        // 11) 스킬설명코드
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        R.descCode = static_cast<uint16_t>(std::atoi(tok));

        // 12~13) HP/MP 소모 (WORD)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        A.hpCost = static_cast<uint16_t>(std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        A.mpCost = static_cast<uint16_t>(std::atoi(tok));

        // 14~15) 공격력/방어력 상승 (DWORD)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; R.attackPowerUp  = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; R.defensePowerUp = std::atoi(tok);

        // 16~19) normalHitWeight/critHitWeight/durationTime/repeatTime (WORD each)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.normalHitWeight = static_cast<uint16_t>(std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.critHitWeight   = static_cast<uint16_t>(std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.durationTime    = static_cast<uint16_t>(std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.repeatTime      = static_cast<uint16_t>(std::atoi(tok));

        // 20~22) remainHpAfterUse/remainMpAfterUse/missHitWeight (DWORD each)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.remainHpAfterUse = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.remainMpAfterUse = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.missHitWeight    = std::atoi(tok);

        // 23~24) castTime/valueColumn (DWORD)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.castTime    = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.valueColumn = std::atoi(tok);

        // 25) 재사용시간 → cooltime
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        R.cooltime = std::atoi(tok);

        // 26) 습득 클래스
        std::string acquireClass;
        if (!NextTok(tok, DELIM)) break; acquireClass = tok;

        // 27) 스킬 구매 타입
        if (!NextTok(tok, DELIM)) break;
        R.skillBuyClass = GetSkillClass(tok);

        // 28~31) 4 系點數
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; R.needFigPt = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; R.needArcPt = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; R.needMagPt = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; R.needPriPt = std::atoi(tok);

        // 32) 사용시 필요 무기
        std::string needWeapon;
        if (!NextTok(tok, DELIM)) break; needWeapon = tok;

        // 33) 필요 레벨
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        R.reqLevel = static_cast<uint16_t>(std::atoi(tok));

        // 34) 사운드 (寫入 +148, 16 byte 字串)
        // GT (mofclient.c 331644)：strcpy((char *)(v7 + 148), v67);
        // GT 直接 strcpy 不檢查長度；資料檔 sound 欄位固定為短字串 (例 "S0001")
        // 故沒有實際溢位風險。為與 GT 嚴格一致此處沿用 strcpy。
        if (!NextTok(tok, DELIM)) break;
        std::strcpy(A.soundName, tok);

        // 35) 원거리 여부 ("LONG"=1, 其他=0)
        if (!NextTok(tok, DELIM)) break;
        A.isLong = (std::strcmp(tok, "LONG") == 0) ? 1 : 0;

        // 36~37) 타겟범위 x/y
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        A.targetRangeX = static_cast<uint16_t>(std::atoi(tok));
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        A.targetRangeY = static_cast<uint16_t>(std::atoi(tok));

        // 38) 시전 애니: NORMAL=0, SPELL=1, PRAY=2, TRIPLE=3, DOUBLE=4
        if (!NextTok(tok, DELIM)) break;
        if      (std::strcmp(tok, "NORMAL") == 0) A.castAnim = 0;
        else if (std::strcmp(tok, "SPELL")  == 0) A.castAnim = 1;
        else if (std::strcmp(tok, "PRAY")   == 0) A.castAnim = 2;
        else if (std::strcmp(tok, "TRIPLE") == 0) A.castAnim = 3;
        else if (std::strcmp(tok, "DOUBLE") == 0) A.castAnim = 4;
        else                                       A.castAnim = 0;

        // 39) 맞는 속성: NONE=0, NORMAL=1, FIRE=2, ELEC=3, ICE=4
        if (!NextTok(tok, DELIM)) break;
        if      (std::strcmp(tok, "NONE")   == 0) A.hitAtb = 0;
        else if (std::strcmp(tok, "NORMAL") == 0) A.hitAtb = 1;
        else if (std::strcmp(tok, "FIRE")   == 0) A.hitAtb = 2;
        else if (std::strcmp(tok, "ELEC")   == 0) A.hitAtb = 3;
        else if (std::strcmp(tok, "ICE")    == 0) A.hitAtb = 4;
        else                                       A.hitAtb = 0;

        // 40) 공격 속성
        if (!NextTok(tok, DELIM)) break;
        A.attackAtb = cltAttackAtb::GetAttackAtb(tok);

        // 41) 파티 공격속성: "0"→(flag=0); 其他→(flag=1, atb=GetAttackAtb)
        if (!NextTok(tok, DELIM)) break;
        if (std::strcmp(tok, "0") == 0) {
            A.partyAtkAtbFlag = 0;
        } else {
            A.partyAtkAtbFlag = 1;
            A.partyAtkAtb = cltAttackAtb::GetAttackAtb(tok);
        }

        // 42~47) 기절/봉쇄/결빙 確率/時間（DWORD x6）
        {
            uint32_t* const stateFields[] = {
                &A.atkStunProb,    // +176
                &A.atkStunTime,    // +180
                &A.atkBlockProb,   // +184
                &A.atkBlockTime,   // +188
                &A.atkFreezeProb,  // +192
                &A.atkFreezeTime   // +196
            };
            bool ok = true;
            for (uint32_t* dst : stateFields) {
                if (!NextTok(tok, DELIM) || !IsDigit(tok)) { ok = false; break; }
                *dst = std::atoi(tok);
            }
            if (!ok) break;
        }

        // 48) mine 아이디 (cltMineKindInfo::TranslateKindCode)
        if (!NextTok(tok, DELIM)) break;
        A.mineKindCode = cltMineKindInfo::TranslateKindCode(tok);

        // 49~50) fastRun / castCount
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.fastRun   = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.castCount = std::atoi(tok);

        // 51~53) value1/value2/value3 (WORD x3) — GT 是「最多 3 次」迴圈寫 _WORD
        {
            uint16_t* const valFields[] = { &A.value1, &A.value2, &A.value3 };
            bool ok = true;
            for (uint16_t* dst : valFields) {
                if (!NextTok(tok, DELIM) || !IsDigit(tok)) { ok = false; break; }
                *dst = static_cast<uint16_t>(std::atoi(tok));
            }
            if (!ok) break;
        }

        // 54~57) spirit/agility/stamina/intelligence (DWORD x4)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.spiritBonus       = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.agilityBonus      = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.staminaBonus      = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.intelligenceBonus = std::atoi(tok);

        // 58~60) atkUseProb/atkUseEff/atkSpeedActive (DWORD x3)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.atkUseProb      = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.atkUseEff       = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.atkSpeedActive  = std::atoi(tok);

        // 61) partyAtkPower
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        A.partyAtkPower = std::atoi(tok);

        // 62~63) GT 順序：先 partySpirit (+236) 後 partyAgility (+232)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.partySpirit  = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.partyAgility = std::atoi(tok);

        // 64) partyStamina
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        A.partyStamina = std::atoi(tok);

        // 65) damageReceiveConvert (+272)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        A.damageReceiveConvert = std::atoi(tok);

        // 66) 略過一欄（GT: 이펙트 단계 之前的欄）
        if (!NextTok(tok, DELIM)) break;

        // 67) atkLineFx (WORD)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        A.atkLineFx = static_cast<uint16_t>(std::atoi(tok));

        // 68~69) triggerProbDamage / triggerProbCurse
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.triggerProbDamage = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.triggerProbCurse  = std::atoi(tok);

        // 70) debuffStage (BYTE) — GT 是 *(_BYTE *)(v7 + 292)
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break;
        A.debuffStage = static_cast<uint8_t>(std::atoi(tok));

        // 71~72) debuffRideTime / effectStage
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.debuffRideTime = std::atoi(tok);
        if (!NextTok(tok, DELIM) || !IsDigit(tok)) break; A.effectStage    = std::atoi(tok);

        // 73) debuffKindCode (WORD via cltDebuffKindInfo::TranslateKindCode) — GT 無 IsDigit 檢查
        if (!NextTok(tok, DELIM)) break;
        A.debuffKindCode = cltDebuffKindInfo::TranslateKindCode(tok);

        // 74~76) expGainPermil/partyExpGainPermil/weaponAtkMode — GT 無 IsDigit 檢查
        if (!NextTok(tok, DELIM)) break; A.expGainPermil      = std::atoi(tok);
        if (!NextTok(tok, DELIM)) break; A.partyExpGainPermil = std::atoi(tok);
        if (!NextTok(tok, DELIM)) break; A.weaponAtkMode      = std::atoi(tok);

        // 77) weaponAtkMultiFlag ("MULTI" → 1, 其他 → 0)
        if (!NextTok(tok, DELIM)) break;
        A.weaponAtkMultiFlag = (std::strcmp(tok, "MULTI") == 0) ? 1 : 0;

        // 78) centeredOnMonster
        if (!NextTok(tok, DELIM)) break;
        A.centeredOnMonster = std::atoi(tok);

        // 79~89) 11 個 EffectKindCode；GT 寫入順序：
        //   effectTopUp1, effectTopUp2, effectBotDown1, effectBotDown2, effectProjectile,
        //   effectFlyTop, effectFlyBot, effectHittedTopUp1, effectHittedTopUp2,
        //   effectHittedBotDown1, effectHittedBotDown2
        {
            uint16_t* const fxFields[] = {
                &R.effectTopUp1,           // +92
                &R.effectTopUp2,           // +94
                &R.effectBotDown1,         // +96
                &R.effectBotDown2,         // +98
                &A.effectProjectile,       // +328
                &R.effectFlyTop,           // +108
                &R.effectFlyBot,           // +110
                &R.effectHittedTopUp1,     // +100
                &R.effectHittedTopUp2,     // +102
                &R.effectHittedBotDown1,   // +104
                &R.effectHittedBotDown2    // +106
            };
            bool ok = true;
            for (uint16_t* dst : fxFields) {
                if (!NextTok(tok, DELIM)) { ok = false; break; }
                *dst = TranslateEffectKindCode(tok);
            }
            if (!ok) break;
        }

        // ── 後處理 ──────────────────────────────────────────────────────
        R.acquireClassMask = GetReqClasses(this, acquireClass.data());
        R.reqWeaponCount   = GetReqWeaponClasses(this, needWeapon.data(), R.reqWeaponFlags);

        ++idx;
    }

    if (idx == count) success = 1;
    g_clTextFileManager.fclose(fp);
    return success;
}

// ==========================================================================
//
//   cltSkillKindInfoM 的實作（M 版；A 檔不解析）
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
    if (!Initialize_P(pfile)) return 0;
    if (!Initialize_A(afile)) return 0;

    for (auto& rec : m_pList) {
        if (rec.skillCode == 0xFFFF) return 0;
        if (m_idx[rec.skillCode]) return 0;
        m_idx[rec.skillCode] = reinterpret_cast<stSkillKindInfo*>(&rec);
    }
    for (auto& rec : m_aList) {
        if (rec.skillCode == 0xFFFF) return 0;
        if (m_idx[rec.skillCode]) return 0;
        m_idx[rec.skillCode] = reinterpret_cast<stSkillKindInfo*>(&rec);
    }
    return 1;
}

int cltSkillKindInfoM::Initialize_P(char* filename)
{
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    char line[4096]; const char* DELIM = "\t\n";
    for (int i = 0; i < 3; ++i) {
        if (!std::fgets(line, sizeof(line), fp)) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }
    }

    fpos_t pos; std::fgetpos(fp, &pos);
    size_t count = 0; while (std::fgets(line, sizeof(line), fp)) ++count;
    m_pList.assign(count, stSkillKindInfoMRec{});
    std::fsetpos(fp, &pos);

    size_t cur = 0;
    while (cur < count && std::fgets(line, sizeof(line), fp)) {
        stSkillKindInfoMRec& R = m_pList[cur];
        std::memset(&R, 0, sizeof(R));

        char* tok = std::strtok(line, DELIM);
        if (!tok) continue;

        R.skillCode = cltSkillKindInfo::TranslateKindCode(tok);
        if (!R.skillCode || !cltSkillKindInfo::IsPassiveSkill(R.skillCode)) {
            g_clTextFileManager.fclose(fp);
            return 0;
        }

        if (!cltSkillKindInfo::NextTok(tok, DELIM)) break;
        if (!cltSkillKindInfo::NextTok(tok, DELIM)) break;

        if (!cltSkillKindInfo::NextTok(tok, DELIM) || !cltSkillKindInfo::IsDigit(tok)) break;
        R.nameCode = std::atoi(tok);

        if (!cltSkillKindInfo::NextTok(tok, DELIM) || !cltSkillKindInfo::IsDigit(tok)) break;
        R.iconID = std::atoi(tok);

        if (!cltSkillKindInfo::NextTok(tok, DELIM) || !cltSkillKindInfo::IsDigit(tok)) break;
        R.iconBlock = std::atoi(tok);

        if (!cltSkillKindInfo::NextTok(tok, DELIM) || !cltSkillKindInfo::IsDigit(tok)) break;
        R.smallIconID = std::atoi(tok);

        if (!cltSkillKindInfo::NextTok(tok, DELIM) || !cltSkillKindInfo::IsDigit(tok)) break;
        R.smallIconBlock = std::atoi(tok);

        if (!cltSkillKindInfo::NextTok(tok, DELIM) || !cltSkillKindInfo::IsDigit(tok)) break;
        R.prevField = std::atoi(tok);

        ++cur;
    }

    g_clTextFileManager.fclose(fp);
    return cur == count ? 1 : 0;
}
