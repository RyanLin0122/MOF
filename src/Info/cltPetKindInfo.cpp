#include "Info/cltPetKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

int cltPetKindInfo::Initialize(char* a2, char* a3) {
    int ok = LoadPetKindInfo(a2);
    if (ok) ok = (LoadPetDyeKindInfo(a3) != 0);
    return ok;
}

void cltPetKindInfo::Free() {
    if (m_petRows) { operator delete(static_cast<void*>(m_petRows)); m_petRows = nullptr; }
    m_petCount = 0;
    if (m_dyeRows) { operator delete(static_cast<void*>(m_dyeRows)); m_dyeRows = nullptr; }
    m_dyeCount = 0;
}

static inline uint32_t ReadHexDword(const char* s) {
    uint32_t v = 0; if (s) std::sscanf(s, "%x", &v); return v;
}

uint16_t cltPetKindInfo::TranslateKindCode(char* s) {
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int num = static_cast<unsigned int>(std::atoi(s + 1));
    if (num < 0x800u) return static_cast<uint16_t>(hi | num);
    return 0;
}

int cltPetKindInfo::LoadPetKindInfo(char* filename) {
    if (!filename) return 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    const char* DELIMS = "\t\n";
    char line[2048] = { 0 };
    int ret = 0, haveLastCol = 1;

    // 跳過表頭三行
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp); return 0;
    }

    // 預掃行數
    fpos_t pos{}; std::fgetpos(fp, &pos);
    m_petCount = 0;
    while (std::fgets(line, sizeof(line), fp)) ++m_petCount;

    // 配置 172 bytes/列
    if (m_petCount > 0) {
        m_petRows = static_cast<strPetKindInfo*>(operator new(sizeof(strPetKindInfo) * m_petCount));
        std::memset(m_petRows, 0, sizeof(strPetKindInfo) * m_petCount);
    }

    std::fsetpos(fp, &pos);

    int idx = 0;
    if (std::fgets(line, sizeof(line), fp)) {
        do {
            char* tok = std::strtok(line, DELIMS);         // pet_ID
            if (!tok) break;
            uint16_t kind = TranslateKindCode(tok);
            if (!std::strtok(nullptr, DELIMS)) break;      // 企劃用名稱（丟棄）

            tok = std::strtok(nullptr, DELIMS);            // 이름(text code)
            if (!tok || !IsDigitStr(tok)) break;
            uint16_t nameText = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 설명(text code)
            if (!tok || !IsDigitStr(tok)) break;
            uint16_t descText = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 前一階寵物ID
            if (!tok) break;
            uint16_t prev = TranslateKindCode(tok);

            tok = std::strtok(nullptr, DELIMS);            // 動作檔 *.txt
            if (!tok || !std::strstr(tok, ".txt")) break;

            strPetKindInfo rec{};                          // 整筆記錄
            rec.kind = kind;
            rec.nameText = nameText;
            rec.descText = descText;
            rec.prevPetKind = prev;
            std::strncpy(rec.animGi, tok, sizeof(rec.animGi) - 1);

            tok = std::strtok(nullptr, DELIMS);            // 염색 수
            if (!tok || !IsDigitStr(tok)) break;
            uint16_t dyeCount = static_cast<uint16_t>(std::atoi(tok));
            rec.dyeCount = dyeCount;

            // 填染色列表（最多 20，常見 4 筆：原/橘/綠/紅）
            for (uint16_t i = 0; i < dyeCount; ++i) {
                tok = std::strtok(nullptr, DELIMS);
                if (!tok) goto done;
                rec.dyeKinds[i] = TranslateKindCode(tok);
            }

            tok = std::strtok(nullptr, DELIMS);            // width
            if (!tok || !IsDigitStr(tok)) break;
            rec.width = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // height
            if (!tok || !IsDigitStr(tok)) break;
            rec.height = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 도트 리소스 ID (hex)
            if (!tok) break;
            rec.dotResHex = ReadHexDword(tok);

            tok = std::strtok(nullptr, DELIMS);            // 블록 ID
            if (!tok || !IsDigitStr(tok)) break;
            rec.dotBlockId = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 펫UI 알 리소스 (hex)
            if (!tok) break;
            rec.eggResHex = ReadHexDword(tok);

            tok = std::strtok(nullptr, DELIMS);            // 알 블록ID
            if (!tok || !IsDigitStr(tok)) break;
            rec.eggBlockId = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 기본 패시브 스킬（代碼）
            if (!tok) break;
            rec.basePassiveSkill = cltPetSkillKindInfo::TranslateKindCode(tok);

            tok = std::strtok(nullptr, DELIMS);            // 펫 분류（字串）
            if (!tok) break;
            if (!std::strcmp(tok, "BASICPET"))   rec.petTypeEnum = 0;
            else if (!std::strcmp(tok, "DRAGON"))     rec.petTypeEnum = 1;
            else if (!std::strcmp(tok, "TINKERBELL")) rec.petTypeEnum = 2;
            else if (!std::strcmp(tok, "PENGUIN"))    rec.petTypeEnum = 3;
            else if (!std::strcmp(tok, "BEARDOLL"))   rec.petTypeEnum = 4;
            else if (!std::strcmp(tok, "DEVIL"))      rec.petTypeEnum = 5;
            else if (!std::strcmp(tok, "BISQUEDOLL")) rec.petTypeEnum = 6;
            else if (!std::strcmp(tok, "MECHANIC"))   rec.petTypeEnum = 7;

            // 跳過「레벨(기획자용)」
            if (!std::strtok(nullptr, DELIMS)) break;

            tok = std::strtok(nullptr, DELIMS);            // LOVE (經驗值)
            if (!tok || !IsDigitStr(tok)) break;
            rec.loveExp = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 포만감
            if (!tok || !IsDigitStr(tok)) break;
            rec.satiety = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 포만감 하락치(1분당)
            if (!tok || !IsDigitStr(tok)) break;
            rec.satietyDropPerMin = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 펫 단계별 기준 y좌표
            if (!tok || !IsDigitStr(tok)) break;
            rec.stageY = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 펫 종류(text code)
            if (!tok) break;
            rec.typeTextCode = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 펫 베이직 스킬 이름(text code)
            if (!tok) break;
            rec.basicSkillNameTxt = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 스킬획득여부
            if (!tok) break;
            rec.skillObtained = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 基本給包數
            if (!tok) break;
            rec.bagInitCount = static_cast<uint8_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 最大擴張包數
            if (!tok) break;
            rec.bagMaxExpand = static_cast<uint8_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 可改名
            if (!tok) break;
            rec.canRename = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 前/後
            if (!tok || !IsDigitStr(tok)) break;
            rec.frontOrBack = static_cast<uint8_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // Y 調整
            if (!tok) break;
            rec.yAdjust = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 放生價格
            if (!tok) break;
            rec.releasePrice = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 放生技能單價
            if (!tok) break;
            rec.releaseSkillUnit = static_cast<uint32_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 停止動畫資源（hex）
            if (!tok) break;
            rec.idleResHex = ReadHexDword(tok);

            tok = std::strtok(nullptr, DELIMS);            // 停止動畫區塊ID
            if (!tok || !IsDigitStr(tok)) break;
            rec.idleBlockId = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, DELIMS);            // 펫 이펙트（代碼或 0）
            if (!tok) break;
            rec.effectKind = TranslateKindCode(tok);       // 若為 "0" 會回傳 0（長度≠5）

            tok = std::strtok(nullptr, DELIMS);            // 펫 요구레벨（可能缺省）
            if (tok) {
                rec.requiredLevel = static_cast<uint16_t>(std::atoi(tok));
            }
            else {
                haveLastCol = 0; // 反編譯：缺少最後欄位會彈錯誤
            }

            // 以現有 kind 回推計算等級（直到 prev==0）
            rec.levelComputed = static_cast<uint16_t>(GetPetLevel(kind));

            if (idx < m_petCount) {
                m_petRows[idx] = rec;
                ++idx;
            }

        } while (std::fgets(line, sizeof(line), fp));
        ret = 1;
    }
    else {
        ret = 1; // 無資料列也當成功
    }

done:
    if (!haveLastCol) {
#ifdef _WIN32
        // 與原始行為相近：在 Windows 顯示警告
#include <windows.h>
        MessageBoxA(nullptr, "petkindinfo: 最後欄位(펫 요구레벨) 缺失", "Error", MB_OK);
#endif
    }

    g_clTextFileManager.fclose(fp);
    return ret;
}

int cltPetKindInfo::LoadPetDyeKindInfo(char* filename) {
    if (!filename) return 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    const char* DELIMS = "\t\r\n";
    char line[1024] = { 0 };

    // 跳過前 3 行表頭（若不足 3 行，自然會提早結束）
    for (int i = 0; i < 3 && std::fgets(line, sizeof(line), fp); ++i) {}

    // === 預掃：只計「有效資料行」 ===
    fpos_t pos{}; std::fgetpos(fp, &pos);
    int count = 0;
    while (std::fgets(line, sizeof(line), fp)) {
        // 去除前置空白；忽略空行與 // 註解
        char* s = line;
        while (*s == ' ' || *s == '\t') ++s;
        if (!*s || (s[0] == '/' && s[1] == '/')) continue;

        // 拆三欄：Dxxxx、(描述字串略過)、索引整數
        char* c1 = std::strtok(s, DELIMS);
        char* c2 = std::strtok(nullptr, DELIMS); (void)c2; // 企劃用名稱
        char* c3 = std::strtok(nullptr, DELIMS);
        if (!c1 || !c3) continue;
        if (std::strlen(c1) != 5) continue;        // 期望 Dxxxx
        // 第 3 欄要是數字
        const char* p = c3;
        if (*p == '+' || *p == '-') ++p;
        bool allDigit = *p != 0;
        for (; *p; ++p) if (*p < '0' || *p > '9') { allDigit = false; break; }
        if (!allDigit) continue;

        ++count;
    }

    // 配置
    m_dyeCount = count;
    m_dyeRows = (count > 0)
        ? static_cast<strPetDyeKindInfo*>(operator new(sizeof(strPetDyeKindInfo) * count))
        : nullptr;
    if (m_dyeRows) std::memset(m_dyeRows, 0, sizeof(strPetDyeKindInfo) * count);

    // 回到資料起點
    std::fsetpos(fp, &pos);

    // 實際讀入
    int idx = 0;
    while (idx < count && std::fgets(line, sizeof(line), fp)) {
        char* s = line;
        while (*s == ' ' || *s == '\t') ++s;
        if (!*s || (s[0] == '/' && s[1] == '/')) continue;

        char* code = std::strtok(s, DELIMS);
        char* name = std::strtok(nullptr, DELIMS); (void)name;
        char* idxs = std::strtok(nullptr, DELIMS);
        if (!code || !idxs) continue;

        m_dyeRows[idx].dyeCode = TranslateKindCode(code);
        m_dyeRows[idx].dyeIndex = static_cast<uint32_t>(std::atoi(idxs));
        ++idx;
    }

    // 以實際成功讀到的筆數為準（避免尾端空白行造成未填資料）
    m_dyeCount = idx;

    g_clTextFileManager.fclose(fp);
    return 1;
}


strPetKindInfo* cltPetKindInfo::GetPetKindInfo(uint16_t a2) {
    int n = m_petCount, i = 0;
    if (n <= 0) return nullptr;
    strPetKindInfo* p = m_petRows;
    for (; i < n; ++i, ++p) {
        if (p->kind == a2) return p;
    }
    return nullptr;
}

uint16_t cltPetKindInfo::GetNextPetKind(uint16_t a2) {
    int n = m_petCount, i = 0;
    if (n <= 0) return 0;
    uint8_t* base = reinterpret_cast<uint8_t*>(m_petRows);
    // 從每筆的 +6（prevPetKind）找等於 a2 的那筆，回傳其 +0（kind）
    for (; i < n; ++i) {
        uint16_t* prevPtr = reinterpret_cast<uint16_t*>(base + i * 172 + 6);
        if (*prevPtr == a2) {
            return *reinterpret_cast<uint16_t*>(base + i * 172 + 0);
        }
    }
    return 0;
}

uint16_t cltPetKindInfo::GetOriginalPetKind(uint16_t a2) {
    strPetKindInfo* r = GetPetKindInfo(a2);
    return r ? r->dyeKinds[0] : 0; // offset +42
}

strPetDyeKindInfo* cltPetKindInfo::GetPetDyeKindInfo(uint16_t a2)
{
    int v2 = m_dyeCount;              // *((_DWORD*)this + 3)
    int v3 = 0;
    if (v2 <= 0) return nullptr;

    // v4 指向整張染色表的開頭，以 WORD* 看待；每筆 4 WORD (= 8 bytes)
    uint16_t* v4 = reinterpret_cast<uint16_t*>(m_dyeRows);   // *((_DWORD*)this + 2)
    uint16_t* i = v4;

    // 尋找首欄 (dyeCode) 等於 a2 的那筆
    for (; *i != a2; i += 4) {       // i += 4 WORD → 前進 8 bytes
        if (++v3 >= v2)
            return nullptr;
    }

    // 回傳該筆資料的位址：&v4[4 * v3] 轉回結構
    return reinterpret_cast<strPetDyeKindInfo*>(v4 + 4 * v3);
}

uint16_t cltPetKindInfo::GetPetTypeTextCode(uint8_t a2) {
    int n = m_petCount, i = 0;
    if (n <= 0) return 0;
    uint8_t* base = reinterpret_cast<uint8_t*>(m_petRows);
    for (; i < n; ++i) {
        uint8_t typeEnum = *(base + i * 172 + 164);
        if (typeEnum == a2) {
            return *reinterpret_cast<uint16_t*>(base + i * 172 + 122);
        }
    }
    return 0;
}

int16_t cltPetKindInfo::GetPetLevel(uint16_t a2) {
    int16_t lv = 1;
    strPetKindInfo* r = GetPetKindInfo(a2);
    if (!r) return 0;
    // 反編譯意義：沿著 +6(prevPetKind) 一直往前追到 0
    uint16_t prev = r->prevPetKind;
    while (prev) {
        ++lv;
        // 移到「前一筆」的 +6；原始反編譯用指標算術（每筆 86 WORD）
        strPetKindInfo* rp = GetPetKindInfo(prev);
        if (!rp) break;
        prev = rp->prevPetKind;
    }
    return lv;
}

int cltPetKindInfo::IsSamePet(uint16_t a2, uint16_t a3) {
    strPetKindInfo* A = GetPetKindInfo(a2);
    if (!A) return 0;
    strPetKindInfo* B = GetPetKindInfo(a3);
    if (!B) return 0;
    if (A == B) return 1;
    // 沿 A 的前溯鏈一路找是否會碰到 B
    while (A) {
        if (A == B) return 1;
        if (!A->prevPetKind) break;
        A = GetPetKindInfo(A->prevPetKind);
    }
    return 0;
}

// 忠實照反編譯：把 +156 視為 base、+160 視為 stride，回傳 (base + a3*stride) 的位址
strPetKindInfo* cltPetKindInfo::GetPetReleaseCost(uint16_t a2, uint16_t a3) {
    strPetKindInfo* r = GetPetKindInfo(a2);
    if (!r) return nullptr;
    uintptr_t base = static_cast<uintptr_t>(r->releasePrice);
    uintptr_t step = static_cast<uintptr_t>(r->releaseSkillUnit);
    return reinterpret_cast<strPetKindInfo*>(base + a3 * step);
}
