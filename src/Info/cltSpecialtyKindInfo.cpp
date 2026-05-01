#include "Info/cltSpecialtyKindInfo.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Info/cltItemKindInfo.h"        // IsDigit / IsAlphaNumeric / GetMakingCategory
#include "Info/cltMakingItemKindInfo.h"  // cltMakingItemKindInfo::TranslateKindCode
#include "Info/cltSkillKindInfo.h"       // cltSkillKindInfo::TranslateKindCode / IsPassiveSkill / IsActiveSkill
#include "Text/cltTextFileManager.h"
#include "global.h"

// ──────────────────────────────────────────────────────────────────────────
// 靜態成員（對應 mofclient.c:25423）
// ──────────────────────────────────────────────────────────────────────────
cltSkillKindInfo* cltSpecialtyKindInfo::m_pclSkillKindInfo = nullptr;

// ──────────────────────────────────────────────────────────────────────────
// 建構 / 解構
//   原始反編譯：構造子將 *(this)=0、*(this+1)=0；解構子呼叫 Free。
// ──────────────────────────────────────────────────────────────────────────
cltSpecialtyKindInfo::cltSpecialtyKindInfo()
    : m_table(nullptr)
    , m_count(0)
{}

cltSpecialtyKindInfo::~cltSpecialtyKindInfo() {
    Free();
}

// ──────────────────────────────────────────────────────────────────────────
// (00596A00) cltSpecialtyKindInfo::InitializeStaticVariable
// ──────────────────────────────────────────────────────────────────────────
void cltSpecialtyKindInfo::InitializeStaticVariable(cltSkillKindInfo* a1) {
    cltSpecialtyKindInfo::m_pclSkillKindInfo = a1;
}

// ──────────────────────────────────────────────────────────────────────────
// (00596A30) cltSpecialtyKindInfo::Initialize
//   - 跳過 3 行表頭
//   - 紀錄第一筆資料的位置 (fgetpos)，先以空轉計 fgets 數量決定 m_count 大小
//   - 重設位置 (fsetpos) 後逐行解析
//   - 任一欄解析失敗即關閉檔案並回傳 0；正常讀至 EOF 才回傳 1
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::Initialize(char* String2) {
    char  Delimiter[2];
    char  Buffer[1024];
    char  String1[1024];
    fpos_t Position;

    std::memset(Buffer, 0, sizeof(Buffer));
    std::memset(String1, 0, sizeof(String1));
    std::strcpy(Delimiter, "\t\n");

    int v36 = 0;

    FILE* Stream = g_clTextFileManager.fopen(String2);
    if (!Stream) return 0;

    if (std::fgets(Buffer, 1023, Stream)
        && std::fgets(Buffer, 1023, Stream)
        && std::fgets(Buffer, 1023, Stream))
    {
        std::fgetpos(Stream, &Position);

        // GT: for (; fgets(...); ++*((DWORD*)this + 1)) ; — 直接累加 m_count
        for (; std::fgets(Buffer, 1023, Stream); ++m_count)
            ;

        // GT: operator new(156 * count) + memset(0)
        m_table = static_cast<strSpecialtyKindInfo*>(
            ::operator new(sizeof(strSpecialtyKindInfo) * static_cast<size_t>(m_count)));
        std::memset(m_table, 0, sizeof(strSpecialtyKindInfo) * static_cast<size_t>(m_count));

        std::fsetpos(Stream, &Position);

        // 對應 LABEL_36：在第一筆 fgets 失敗（檔案剛好沒有資料行）就直接成功返回
        if (!std::fgets(Buffer, 1023, Stream)) {
            v36 = 1;
            g_clTextFileManager.fclose(Stream);
            return v36;
        }

        int idx = 0;

        // 對應 LABEL_8 主迴圈
        for (;;) {
            strSpecialtyKindInfo* e = (idx < m_count) ? &m_table[idx] : nullptr;
            if (!e) break;  // 安全保險（行數應與 m_count 相符）

            char* tok;

            // [0] 전공 아이디 → wKind
            tok = std::strtok(Buffer, Delimiter);
            if (!tok) break;
            e->wKind = cltSpecialtyKindInfo::TranslateKindCode(tok);

            // [1] 전공명 → 略過
            tok = std::strtok(nullptr, Delimiter);
            if (!tok) break;

            // [2] 전공명 코드 → wTextCode (atoi)
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsDigit(tok)) break;
            e->wTextCode = static_cast<uint16_t>(std::atoi(tok));

            // [3] 전공 설명 코드 → wDescCode (atoi)
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsDigit(tok)) break;
            e->wDescCode = static_cast<uint16_t>(std::atoi(tok));

            // [4] 전공 타입 → byType
            tok = std::strtok(nullptr, Delimiter);
            if (!tok) break;
            e->byType = cltSpecialtyKindInfo::GetSpecialtyType(tok);

            // [5] 이전 전공 아이디 → wRequiredSpecialtyKind (TranslateKindCode)
            tok = std::strtok(nullptr, Delimiter);
            if (!tok) break;
            e->wRequiredSpecialtyKind = cltSpecialtyKindInfo::TranslateKindCode(tok);

            // [6] 요구 전공 포인트 → byRequiredSpecialtyPt (atoi)
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsDigit(tok)) break;
            e->byRequiredSpecialtyPt = static_cast<uint8_t>(std::atoi(tok));

            // [7] 학년 제한 → byRequiredGrade (atoi)
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsDigit(tok)) break;
            e->byRequiredGrade = static_cast<uint8_t>(std::atoi(tok));

            // [8] 레벨 제한 → dwRequiredLevel (atoi → DWORD)
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsDigit(tok)) break;
            e->dwRequiredLevel = static_cast<uint32_t>(std::atoi(tok));

            // [9] 리소스 아이디 → dwResourceID (sscanf "%x")
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsAlphaNumeric(tok)) break;
            std::sscanf(tok, "%x", &e->dwResourceID);

            // [10] 도움 아이디 → wHelpID (atoi)
            tok = std::strtok(nullptr, Delimiter);
            if (!tok || !IsDigit(tok)) break;
            e->wHelpID = static_cast<uint16_t>(std::atoi(tok));

            // [11] 제작 분류 → 暫存於 String1（稍後僅 MAKING 類型才解析）
            tok = std::strtok(nullptr, Delimiter);
            if (!tok) break;
            std::strcpy(String1, tok);

            // [12..36] 제작 품목 1~25 → wMakingItemKinds[25]
            int v26 = 0;
            bool reachedItem25 = false;
            for (;;) {
                char* v28 = std::strtok(nullptr, Delimiter);
                if (!v28) break;
                e->wMakingItemKinds[v26] = cltMakingItemKindInfo::TranslateKindCode(v28);
                ++v26;
                if (v26 >= 25) { reachedItem25 = true; break; }
            }
            if (!reachedItem25) {
                // 對應 goto LABEL_37 → fclose, 回傳 v36 (=0)
                g_clTextFileManager.fclose(Stream);
                return v36;
            }

            // [37..41] 추가 스킬 1~5 → wAcquiredSkillKinds[5]
            int v29 = 0;
            bool reachedSkill5 = false;
            for (;;) {
                char* v31 = std::strtok(nullptr, Delimiter);
                if (!v31) break;
                e->wAcquiredSkillKinds[v29] = cltSkillKindInfo::TranslateKindCode(v31);
                ++v29;
                if (v29 >= 5) { reachedSkill5 = true; break; }
            }
            if (!reachedSkill5) {
                g_clTextFileManager.fclose(Stream);
                return v36;
            }

            // 解析完所有 5 個技能後：若是製作類型，將分類字串展開為 dwMakingCategory[]
            if (e->byType == SPECIALTY_TYPE_MAKING) {
                SetMakingItemCategory(String1, e->dwMakingCategory);
            }

            // 推進到下一筆，並讀取下一行
            ++idx;
            if (!std::fgets(Buffer, 1023, Stream)) {
                // 對應 LABEL_36：EOF 為正常結束
                v36 = 1;
                g_clTextFileManager.fclose(Stream);
                return v36;
            }
        }
    }

    g_clTextFileManager.fclose(Stream);
    return v36;
}

// ──────────────────────────────────────────────────────────────────────────
// (00596E70) cltSpecialtyKindInfo::Free
//   GT: if (m_table) { operator delete(m_table); m_table = 0; } m_count = 0;
// ──────────────────────────────────────────────────────────────────────────
void cltSpecialtyKindInfo::Free() {
    if (m_table) {
        ::operator delete(static_cast<void*>(m_table));
        m_table = nullptr;
    }
    m_count = 0;
}

// ──────────────────────────────────────────────────────────────────────────
// (00596EA0) cltSpecialtyKindInfo::GetSpecialtyKindInfo
// ──────────────────────────────────────────────────────────────────────────
strSpecialtyKindInfo* cltSpecialtyKindInfo::GetSpecialtyKindInfo(uint16_t a2) {
    if (m_count <= 0) return nullptr;
    for (int i = 0; i < m_count; ++i) {
        if (m_table[i].wKind == a2) return &m_table[i];
    }
    return nullptr;
}

// ──────────────────────────────────────────────────────────────────────────
// (00596EE0) cltSpecialtyKindInfo::GetSpecialtyList
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::GetSpecialtyList(char a2, uint16_t* a3) {
    switch (a2) {
        case 1: return GetGenericSpecialtyList(a3);
        case 2: return GetMakingSpecialtyList(a3);
        case 3: return GetTransformSpecialtyList(a3);
    }
    return 0;
}

// ──────────────────────────────────────────────────────────────────────────
// (00596F20) cltSpecialtyKindInfo::GetGenericSpecialtyList
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::GetGenericSpecialtyList(uint16_t* a2) {
    int written = 0;
    for (int i = 0; i < m_count; ++i) {
        if (m_table[i].byType == SPECIALTY_TYPE_GENERIC) {
            *a2++ = m_table[i].wKind;
            ++written;
        }
    }
    return written;
}

// ──────────────────────────────────────────────────────────────────────────
// (00596F70) cltSpecialtyKindInfo::GetMakingSpecialtyList
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::GetMakingSpecialtyList(uint16_t* a2) {
    int written = 0;
    for (int i = 0; i < m_count; ++i) {
        if (m_table[i].byType == SPECIALTY_TYPE_MAKING) {
            *a2++ = m_table[i].wKind;
            ++written;
        }
    }
    return written;
}

// ──────────────────────────────────────────────────────────────────────────
// (00596FC0) cltSpecialtyKindInfo::GetTransformSpecialtyList
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::GetTransformSpecialtyList(uint16_t* a2) {
    int written = 0;
    for (int i = 0; i < m_count; ++i) {
        if (m_table[i].byType == SPECIALTY_TYPE_TRANSFORM) {
            *a2++ = m_table[i].wKind;
            ++written;
        }
    }
    return written;
}

// ──────────────────────────────────────────────────────────────────────────
// (00597010) cltSpecialtyKindInfo::IsLastLevelSpecialty
//   反編譯語意：若無任何記錄、或無記錄之 wRequiredSpecialtyKind 等於 a2，回傳 1。
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::IsLastLevelSpecialty(uint16_t a2) {
    if (m_count <= 0) return 1;
    for (int i = 0; i < m_count; ++i) {
        if (m_table[i].wRequiredSpecialtyKind == a2) return 0;
    }
    return 1;
}

// ──────────────────────────────────────────────────────────────────────────
// (00597050) cltSpecialtyKindInfo::IsGenericSpeciatly
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::IsGenericSpeciatly(uint16_t a2) {
    auto* info = GetSpecialtyKindInfo(a2);
    if (!info) return 0;
    return info->byType == SPECIALTY_TYPE_GENERIC ? 1 : 0;
}

// ──────────────────────────────────────────────────────────────────────────
// (00597080) cltSpecialtyKindInfo::IsMakingItemSpecialty
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::IsMakingItemSpecialty(uint16_t a2) {
    auto* info = GetSpecialtyKindInfo(a2);
    if (!info) return 0;
    return info->byType == SPECIALTY_TYPE_MAKING ? 1 : 0;
}

// ──────────────────────────────────────────────────────────────────────────
// (005970B0) cltSpecialtyKindInfo::IsTransformSpecialty
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::IsTransformSpecialty(uint16_t a2) {
    auto* info = GetSpecialtyKindInfo(a2);
    if (!info) return 0;
    return info->byType == SPECIALTY_TYPE_TRANSFORM ? 1 : 0;
}

// ──────────────────────────────────────────────────────────────────────────
// (005970E0) cltSpecialtyKindInfo::GetSpecialtyType
//   "MAKING" → 2 ; "TRANSFORM" → 3 ; 其他 (含 GENERIC) → 1
// ──────────────────────────────────────────────────────────────────────────
uint8_t cltSpecialtyKindInfo::GetSpecialtyType(char* String1) {
#ifdef _MSC_VER
    if (!_stricmp(String1, "MAKING")) return SPECIALTY_TYPE_MAKING;
    return _stricmp(String1, "TRANSFORM") != 0 ? SPECIALTY_TYPE_GENERIC : SPECIALTY_TYPE_TRANSFORM;
#else
    if (!strcasecmp(String1, "MAKING")) return SPECIALTY_TYPE_MAKING;
    return strcasecmp(String1, "TRANSFORM") != 0 ? SPECIALTY_TYPE_GENERIC : SPECIALTY_TYPE_TRANSFORM;
#endif
}

// ──────────────────────────────────────────────────────────────────────────
// (00597120) cltSpecialtyKindInfo::TranslateKindCode
//   字串長度必須為 5。第 1 字元 toupper 後 +31 左移 11；後 4 位數 atoi (<0x800)
// ──────────────────────────────────────────────────────────────────────────
uint16_t cltSpecialtyKindInfo::TranslateKindCode(char* a1) {
    if (std::strlen(a1) != 5) return 0;
    const int v2 = (std::toupper(static_cast<unsigned char>(*a1)) + 31) << 11;
    const uint16_t v3 = static_cast<uint16_t>(std::atoi(a1 + 1));
    if (v3 < 0x800u) return static_cast<uint16_t>(v2 | v3);
    return 0;
}

// ──────────────────────────────────────────────────────────────────────────
// (00597170) cltSpecialtyKindInfo::SetMakingItemCategory
//   字串為 "0" (大小寫不敏感) 時不處理；否則以 "|" 切分後逐一寫入 a3。
// ──────────────────────────────────────────────────────────────────────────
void cltSpecialtyKindInfo::SetMakingItemCategory(char* String1, uint32_t* a3) {
    char Delimiter[2];
    std::strcpy(Delimiter, "|");

#ifdef _MSC_VER
    if (_stricmp(String1, "0") != 0) {
#else
    if (strcasecmp(String1, "0") != 0) {
#endif
        for (char* i = std::strtok(String1, Delimiter); i; i = std::strtok(nullptr, Delimiter)) {
            *a3++ = static_cast<uint32_t>(cltItemKindInfo::GetMakingCategory(i));
        }
    }
}

// ──────────────────────────────────────────────────────────────────────────
// (005971D0) cltSpecialtyKindInfo::IsSpecialtySkillKind
//   遍歷所有記錄的 wAcquiredSkillKinds[5]，若任一格 == skillKind 則 1。
//   原始邏輯：若 *v6 == 0 則該筆停止往後檢查（提前退出）。
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::IsSpecialtySkillKind(uint16_t a2) {
    for (int i = 0; i < m_count; ++i) {
        const uint16_t* v6 = m_table[i].wAcquiredSkillKinds;
        for (int j = 0; j < 5; ++j) {
            if (!v6[j]) break;
            if (v6[j] == a2) return 1;
        }
    }
    return 0;
}

// ──────────────────────────────────────────────────────────────────────────
// (00597230) cltSpecialtyKindInfo::GetSpecialtyKindBySkillKind
//   同上掃描；中時回傳該筆的 wKind。
// ──────────────────────────────────────────────────────────────────────────
uint16_t cltSpecialtyKindInfo::GetSpecialtyKindBySkillKind(uint16_t a2) {
    for (int i = 0; i < m_count; ++i) {
        const uint16_t* v6 = m_table[i].wAcquiredSkillKinds;
        for (int j = 0; j < 5; ++j) {
            if (!v6[j]) break;
            if (v6[j] == a2) return m_table[i].wKind;
        }
    }
    return 0;
}

// ──────────────────────────────────────────────────────────────────────────
// (00597290) cltSpecialtyKindInfo::IsPassiveSpeciaty
//   取記錄第一個附加技能 (wAcquiredSkillKinds[0])，呼叫 cltSkillKindInfo::IsPassiveSkill。
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::IsPassiveSpeciaty(uint16_t a2) {
    auto* info = GetSpecialtyKindInfo(a2);
    if (!info) return 0;
    const uint16_t v3 = info->wAcquiredSkillKinds[0];
    if (!v3) return 0;
    return cltSkillKindInfo::IsPassiveSkill(v3) ? 1 : 0;
}

// ──────────────────────────────────────────────────────────────────────────
// (005972D0) cltSpecialtyKindInfo::IsActiveSpeciaty
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::IsActiveSpeciaty(uint16_t a2) {
    auto* info = GetSpecialtyKindInfo(a2);
    if (!info) return 0;
    const uint16_t v3 = info->wAcquiredSkillKinds[0];
    if (!v3) return 0;
    return cltSkillKindInfo::IsActiveSkill(v3) ? 1 : 0;
}

// ──────────────────────────────────────────────────────────────────────────
// (00597310) cltSpecialtyKindInfo::IsCircleSpecialty
//   GT：result = cltSkillKindInfo::IsCircleSkillKind(m_pclSkillKindInfo, v3)
//   直接呼叫 cltSkillKindInfo::IsCircleSkillKind，不額外做 m_pclSkillKindInfo
//   null 檢查（與 GT 完全一致）。
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::IsCircleSpecialty(uint16_t a2) {
    auto* info = GetSpecialtyKindInfo(a2);
    if (!info) return 0;
    const uint16_t v3 = info->wAcquiredSkillKinds[0];
    if (!v3) return 0;
    return cltSkillKindInfo::IsCircleSkillKind(m_pclSkillKindInfo, v3) ? 1 : 0;
}

// ──────────────────────────────────────────────────────────────────────────
// (00597340) cltSpecialtyKindInfo::CanAddToQuickSlot
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::CanAddToQuickSlot(uint16_t a2) {
    return IsActiveSpeciaty(a2) != 0 ? 1 : 0;
}

// ──────────────────────────────────────────────────────────────────────────
// (00597360) cltSpecialtyKindInfo::GetSpecialtyTotalPoint
//   GT：先累加再判斷，do { v3 += v4[+10]; v4 = Get(v4[+8]); } while(v4);
//   若 a2 無效（v4=NULL），GT 會在第一次累加時對 NULL 做位移存取（崩潰），
//   本還原版完全比照 GT，不在迴圈頂端做額外的 null 檢查。
// ──────────────────────────────────────────────────────────────────────────
int cltSpecialtyKindInfo::GetSpecialtyTotalPoint(uint16_t a2) {
    int v3 = 0;
    strSpecialtyKindInfo* v4 = GetSpecialtyKindInfo(a2);
    do {
        v3 += v4->byRequiredSpecialtyPt;
        v4 = GetSpecialtyKindInfo(v4->wRequiredSpecialtyKind);
    } while (v4);
    return v3;
}
