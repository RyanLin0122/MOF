#include "Info/cltTransformKindInfo.h"
#include <cstdlib>

// 原型建構子：與反編譯碼相同，僅回傳 this（不清零）。
cltTransformKindInfo::cltTransformKindInfo() {}

// 簡易數字字串檢查：需全部為 0-9（允許單一 '0'）
static inline bool IsDigitStr(const char* s) {
    if (!s || !*s) return false;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (!std::isdigit(*p)) return false;
    }
    return true;
}

// T/F 欄位以首字母判斷（與 _toupper(*tok) == 'T' 同義）
static inline int TF_to_int(const char* tok) {
    return (tok && std::toupper(static_cast<unsigned char>(tok[0])) == 'T') ? 1 : 0;
}

int cltTransformKindInfo::LoadTransformKindInfo(char* path) {
    FILE* fp = g_clTextFileManager.fopen(path);
    if (!fp) return 0;

    int result = 0; // 只有「正常讀到 EOF」才會設為 1（對齊反編譯的 v27）

    char line[1024];
    const char* DELIM = "\t\n";

    // 跳過前三行（註解/表頭）
    if (std::fgets(line, 1023, fp) &&
        std::fgets(line, 1023, fp) &&
        std::fgets(line, 1023, fp)) {

        // 第四行開始真正資料
        if (std::fgets(line, 1023, fp)) {
            // v5 = (DWORD*)((char*)this + 8) 對應到 records_[0] 的 offset 8 位置
            // 這裡以索引 i 對應同一個位移關係與步幅 52 bytes。
            int i = 0;

            while (true) {
                // 欄位 1：變身코드（以字串映射成 kind 整數）
                char* tok = std::strtok(line, DELIM);
                if (!tok) break;

                int kind = GetTransformKind(tok);
                // *((WORD*)v5 - 4) = kind → 即寫入當前紀錄 offset 0
                if (i >= 17) break; // 防呆：不可超出 17 筆
                records_[i].kind = static_cast<uint16_t>(kind);

                // 欄位 2：變身ID（略過）
                if (!std::strtok(nullptr, DELIM)) break;

                // 欄位 3：變身名稱（略過）
                if (!std::strtok(nullptr, DELIM)) break;

                // 欄位 4：可攻擊（T/F）→ *(v5 - 1)
                char* t4 = std::strtok(nullptr, DELIM);
                if (!t4) break;
                records_[i].canAttack = TF_to_int(t4);

                // 欄位 5：可被攻擊（T/F）→ *v5
                char* t5 = std::strtok(nullptr, DELIM);
                if (!t5) break;
                records_[i].canBeAttacked = TF_to_int(t5);

                // 欄位 6：可移動（T/F）→ v5[1]
                char* t6 = std::strtok(nullptr, DELIM);
                if (!t6) break;
                records_[i].canMove = TF_to_int(t6);

                // 欄位 7：隱身術（T/F）→ v5[10]
                char* t7 = std::strtok(nullptr, DELIM);
                if (!t7) break;
                records_[i].isStealth = TF_to_int(t7);

                // 欄位 8：附近角色 HP 增加值 → v5[2]
                char* t8 = std::strtok(nullptr, DELIM);
                if (!t8 || !IsDigitStr(t8)) break;
                records_[i].nearCharHPInc = std::atoi(t8);

                // 欄位 9：附近角色 MP 增加值 → v5[3]
                char* t9 = std::strtok(nullptr, DELIM);
                if (!t9 || !IsDigitStr(t9)) break;
                records_[i].nearCharMPInc = std::atoi(t9);

                // 欄位 10：附近隊友 HP 增加值 → v5[4]
                char* t10 = std::strtok(nullptr, DELIM);
                if (!t10 || !IsDigitStr(t10)) break;
                records_[i].nearPartyHPInc = std::atoi(t10);

                // 欄位 11：附近隊友 MP 增加值 → v5[5]
                char* t11 = std::strtok(nullptr, DELIM);
                if (!t11 || !IsDigitStr(t11)) break;
                records_[i].nearPartyMPInc = std::atoi(t11);

                // 欄位 12：影響範圍（左右）→ v5[6]
                char* t12 = std::strtok(nullptr, DELIM);
                if (!t12 || !IsDigitStr(t12)) break;
                records_[i].influenceRangeLR = std::atoi(t12);

                // 欄位 13：作用間隔 → v5[7]
                char* t13 = std::strtok(nullptr, DELIM);
                if (!t13 || !IsDigitStr(t13)) break;
                records_[i].influenceInterval = std::atoi(t13);

                // 欄位 14：變身怪物代碼 → strcpy((char*)(v5+8), tok)
                char* t14 = std::strtok(nullptr, DELIM);
                if (!t14) break;
                // 原碼使用 strcpy，會覆寫到 0x28 起始；為避免覆蓋 0x30 的 isStealth，限制長度 7。
                std::strncpy(records_[i].monsterCode, t14, sizeof(records_[i].monsterCode) - 1);
                records_[i].monsterCode[sizeof(records_[i].monsterCode) - 1] = '\0';

                // 下一列（等價於 v5 += 13 → 前進 52 bytes）
                ++i;

                // 若無下一行（EOF）→ 正常結束（v27 = 1）
                if (!std::fgets(line, 1023, fp)) {
                    result = 1;
                    break;
                }
            }
        }
        else {
            // 第 4 行就讀不到 → 原碼會把 v27 設為 1（視為完成）
            result = 1;
        }
    }

    g_clTextFileManager.fclose(fp);
    return result;
}

int cltTransformKindInfo::GetTransformKind(char* a2) {
    if (!std::strcmp("TRFM_KIND_STONE", a2))            return 1;
    if (!std::strcmp("TRFM_KIND_CHO", a2))              return 2;
    if (!std::strcmp("TRFM_KIND_HP_TREE", a2))          return 3;
    if (!std::strcmp("TRFM_KIND_MANA_TREE", a2))        return 4;
    if (!std::strcmp("TRFM_KIND_HAMMERORGE", a2))       return 5;
    if (!std::strcmp("TRFM_KIND_REDWOLF", a2))          return 6;
    if (!std::strcmp("TRFM_KIND_BLACKWYVERN", a2))      return 7;
    if (!std::strcmp("TRFM_KIND_LIFEAURALV1", a2))      return 8;
    if (!std::strcmp("TRFM_KIND_LIFEAURALV2", a2))      return 9;
    if (!std::strcmp("TRFM_KIND_LIFEAURALV3", a2))      return 10;
    if (!std::strcmp("TRFM_KIND_LIFEAURALV4", a2))      return 11;
    if (!std::strcmp("TRFM_KIND_LIFEAURALV5", a2))      return 12;
    if (!std::strcmp("TRFM_KIND_DESBRO", a2))           return 13;
    if (!std::strcmp("TRFM_KIND_PANTER", a2))           return 14;
    if (!std::strcmp("TRFM_KIND_MON_MIRA", a2))         return 15;
    if (!std::strcmp("TRFM_KIND_SPINE_TORTOISE", a2))   return 16;
    if (!std::strcmp("TRFM_KIND_HIDINGSELF", a2))       return 17;
    return 0;
}

strTransformKindInfo* cltTransformKindInfo::GetTransfromKindInfo(uint16_t key) {
    // 與反編譯碼一致：從 this 起始（offset 0 的 kind）每 +52 掃描，最多 17 筆
    for (int idx = 0; idx < 17; ++idx) {
        if (records_[idx].kind == key) {
            return &records_[idx];
        }
    }
    return nullptr;
}
