#include "Info/cltMonsterAniInfo.h"

bool cltMonsterAniInfo::IsAlphaNumeric(const char* s) {
    if (!s || !*s) return false;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p)
        if (!std::isalnum(*p)) return false;  // 允許 0-9 a-z A-Z（十六進位字元 OK）
    return true;
}

bool cltMonsterAniInfo::ieq(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (std::tolower((unsigned char)*a) != std::tolower((unsigned char)*b)) return false;
        ++a; ++b;
    }
    return *a == 0 && *b == 0;
}

int cltMonsterAniInfo::Initialize(char* filename)
{
    // 與建構子一致：整體清零
    std::memset(this, 0, sizeof(*this));

    const char* Delim = "\t\n,[]";
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    // 用 heap 取代大型堆疊陣列
    const size_t kBufSize = 10240; // 與原版 fgets 上限一致
    char* line = static_cast<char*>(std::malloc(kBufSize));
    char* temp = static_cast<char*>(std::malloc(kBufSize));
    if (!line || !temp) {
        if (line) std::free(line);
        if (temp) std::free(temp);
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    int ret = 1;           // 預設成功（讀到 EOF 亦回傳 1）
    AniSet* cur = nullptr; // 目前動作區

    // 第一行就 EOF：依原碼邏輯，視為成功
    if (!std::fgets(line, static_cast<int>(kBufSize), fp)) {
        std::free(line);
        std::free(temp);
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    do {
        std::strcpy(temp, line);
        char* head = std::strtok(temp, Delim);
        if (!head) break;

        // 區段辨識（大小寫不敏感）
        if (ieq(head, "STOP"))            cur = &set_[STOP];
        else if (ieq(head, "MOVE"))       cur = &set_[MOVE];
        else if (ieq(head, "DIE"))        cur = &set_[DIE];
        else if (ieq(head, "ATTACK"))     cur = &set_[ATTACK];
        else if (ieq(head, "N_HITTED"))   cur = &set_[N_HITTED];
        else if (ieq(head, "F_HITTED"))   cur = &set_[F_HITTED];
        else if (ieq(head, "E_HITTED"))   cur = &set_[E_HITTED];
        else if (ieq(head, "I_HITTED"))   cur = &set_[I_HITTED];
        else if (ieq(head, "ATTACK_KEY")) {
            char* k = std::strtok(nullptr, Delim);
            if (!k) { ret = 0; break; }
            attackKey = static_cast<uint16_t>(std::atoi(k));
            continue; // 下一行
        }
        else {
            ret = 0;  // 未知標籤 → 視為格式錯誤
            break;
        }

        // 解析該行後續的多組 [HEX, a, b, c]
        char* tok = std::strtok(nullptr, Delim);
        while (tok && IsAlphaNumeric(tok)) {
            if (!cur) { ret = 0; goto CLEANUP; }

            uint16_t& cnt = cur->count;
            if (cnt >= kMaxFrames) {
                // 超過容量上限（100）：停在此行其餘資料
                break;
            }

            // 1) HEX 資源ID
            unsigned int res = 0;
            if (std::sscanf(tok, "%x", &res) != 1) { ret = 0; goto CLEANUP; }
            cur->frameRes[cnt] = res;

            // 2) 參數A
            tok = std::strtok(nullptr, Delim);
            if (!tok) { ret = 0; goto CLEANUP; }
            cur->paramA[cnt] = static_cast<uint16_t>(std::atoi(tok));

            // 3) 參數B
            tok = std::strtok(nullptr, Delim);
            if (!tok) { ret = 0; goto CLEANUP; }
            cur->paramB[cnt] = static_cast<uint16_t>(std::atoi(tok));

            // 4) 參數C
            tok = std::strtok(nullptr, Delim);
            if (!tok) { ret = 0; goto CLEANUP; }
            cur->paramC[cnt] = static_cast<uint16_t>(std::atoi(tok));

            ++cnt;

            // 嘗試讀下一筆 HEX（或行尾）
            tok = std::strtok(nullptr, Delim);
        }

        // 若 tok 非空但不是英數，依原行為視為格式錯誤
        if (tok && !IsAlphaNumeric(tok)) { ret = 0; break; }

    } while (std::fgets(line, static_cast<int>(kBufSize), fp));

CLEANUP:
    std::free(line);
    std::free(temp);
    g_clTextFileManager.fclose(fp);
    return ret;
}

int cltMonsterAniInfo::GetFrameInfo(unsigned int a2, unsigned short a3,
    unsigned int* a4, unsigned short* a5)
{
    if (!a4 || !a5) return 0;
    *a4 = 0; *a5 = 0;

    if (a2 >= kActionCount) return 0;
    const AniSet& s = set_[a2];
    if (a3 >= s.count) return 0;

    *a4 = s.frameRes[a3];    // 對應反編譯：*((DWORD*)this + 251*a2 + a3 + 1)
    *a5 = s.paramA[a3];      // 對應反編譯：*((WORD*)this + 502*a2 + a3 + 202)
    return 1;
}

unsigned short cltMonsterAniInfo::GetTotalFrameNum(unsigned int a2)
{
    return (a2 < kActionCount) ? set_[a2].count : 0;
}
