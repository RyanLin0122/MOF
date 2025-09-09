#include "Info/cltQuestionKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

uint16_t cltQuestionKindInfo::TranslateKindCode(char* s) {
    // ((toupper(s[0]) + 31) << 11) | atoi(s+1)；長度必須為 5 且數值 < 0x800
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const unsigned int num = static_cast<unsigned int>(std::atoi(s + 1));
    if (num < 0x800u) return static_cast<uint16_t>(hi | num);
    return 0;
}

int cltQuestionKindInfo::Initialize(char* filename) {
    if (!filename) return 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    const char* DELIMS = "\t\n";
    char line[1024] = { 0 };

    // 跳過表頭三行（與反編譯一致）
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 紀錄資料起點並先數行確定配置大小
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    m_count = 0;
    while (std::fgets(line, sizeof(line), fp)) ++m_count;

    // 配置（每筆 12 bytes），清 0 以保證未寫入欄位為 0
    m_items = (m_count > 0)
        ? static_cast<strQuestionKindInfo*>(operator new(sizeof(strQuestionKindInfo) * m_count))
        : nullptr;
    if (m_items) std::memset(m_items, 0, sizeof(strQuestionKindInfo) * m_count);

    // 回到資料起點
    std::fsetpos(fp, &pos);

    bool ok = false; // 僅在正常跑到 EOF（或無資料）時置 true

    int idx = 0;
    if (std::fgets(line, sizeof(line), fp)) {
        do {
            // 逐欄以 \t / \n 切割；第 2 欄（韓文「答(企劃用)」）依反編譯流程直接丟棄
            // 第1欄：問題ID -> 轉碼
            char* tok = std::strtok(line, DELIMS);
            if (!tok) break;
            uint16_t qKind = TranslateKindCode(tok);
            if (qKind == 0) break;

            // 第2欄：答(企劃用/韓文) —— 丟棄
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;

            // 第3欄：學年（BYTE）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint8_t grade = static_cast<uint8_t>(std::atoi(tok));

            // 第4欄：試題碼（WORD）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t examCode = static_cast<uint16_t>(std::atoi(tok));

            // 第5欄：縮約說明（WORD，數字代碼）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t shortDesc = static_cast<uint16_t>(std::atoi(tok));

            // 第6欄：答_怪物（"Jxxxx" or "0"）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t ansMonster = cltCharKindInfo::TranslateKindCode(tok); // 長度!=5 時將回 0

            // 第7欄：答_NPC（"Nxxxx" or "0"）
            tok = std::strtok(nullptr, DELIMS);
            if (!tok) break;
            uint16_t ansNpc = cltNPCInfo::TranslateKindCode(tok); // 同上，"0" 會變 0

            if (idx < m_count) {
                strQuestionKindInfo& r = m_items[idx];
                r.qKind = qKind;
                r.grade = grade;
                r._pad = 0;         // 保持結構 12 bytes 對齊
                r.examCode = examCode;
                r.shortDesc = shortDesc;
                r.ansMonsterKind = ansMonster;
                r.ansNpcKind = ansNpc;
                ++idx;
            }

            // 讀下一行；若到 EOF（對應反編譯的 goto LABEL_16）則視為成功
        } while (std::fgets(line, sizeof(line), fp));

        ok = true; // 正常讀到 EOF
    }
    else {
        // 沒有任何資料行也算成功（與反編譯邏輯一致）
        ok = true;
    }

    g_clTextFileManager.fclose(fp);

    // 若成功但實際填入 < 預估，收斂 m_count；若失敗則釋放
    if (ok) {
        if (idx < m_count) m_count = idx;
        return 1;
    }
    else {
        Free();
        return 0;
    }
}

void cltQuestionKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

strQuestionKindInfo* cltQuestionKindInfo::GetQuestionKindInfo(uint16_t code) {
    // 線性掃描（每筆 stride = 6 WORDs = 12 bytes）
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].qKind == code) return &m_items[i];
    }
    return nullptr;
}

int cltQuestionKindInfo::GetQuestions(char grade, uint16_t* outCodes) {
    // 回傳符合學年的題目數量，並將其 qKind 寫入 outCodes（呼叫者需保證緩衝足夠）
    int cnt = 0;
    if (!m_items || m_count <= 0) return 0;
    for (int i = 0; i < m_count; ++i) {
        if (static_cast<char>(m_items[i].grade) == grade) {
            if (outCodes) outCodes[cnt] = m_items[i].qKind;
            ++cnt;
        }
    }
    return cnt;
}

bool cltQuestionKindInfo::IsAnswer_MonsterKind(uint16_t qKindCode) {
    // 反編譯語義：若該題的「答_怪物」欄位非 0 則回 true
    if (auto* p = GetQuestionKindInfo(qKindCode)) {
        return p->ansMonsterKind != 0;
    }
    return false;
}

bool cltQuestionKindInfo::IsAnswer_NpcKind(uint16_t qKindCode) {
    // 反編譯語義：若該題的「答_NPC」欄位非 0 則回 true
    if (auto* p = GetQuestionKindInfo(qKindCode)) {
        return p->ansNpcKind != 0;
    }
    return false;
}
