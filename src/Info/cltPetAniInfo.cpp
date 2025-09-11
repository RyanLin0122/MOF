#include "Info/cltPetAniInfo.h"

static inline unsigned int hex_to_u32(const char* s, bool& ok) {
    unsigned int v = 0;
    ok = (s && std::sscanf(s, "%x", &v) == 1);
    return v;
}

bool cltPetAniInfo::IsAlphaNumeric(const char* s) {
    if (!s || !*s) return false;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p)
        if (!std::isalnum(*p)) return false;  // 只允許 0-9 a-z A-Z（十六進位字元OK）
    return true;
}

bool cltPetAniInfo::ieq(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (std::tolower((unsigned char)*a) != std::tolower((unsigned char)*b)) return false;
        ++a; ++b;
    }
    return *a == 0 && *b == 0;
}

int cltPetAniInfo::Initialize(char* filename)
{
    // 與反編譯建構子同：資料區清空（保留 vtable）
    std::memset(reinterpret_cast<char*>(this) + 4, 0, 0x1F60);
    attackKey_ = 0;

    const char* Delim = "\t\n,[]";
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    // 用 heap 取代大型堆疊陣列
    const size_t kBuf = 10240;
    char* line = static_cast<char*>(std::malloc(kBuf));
    char* tmp = static_cast<char*>(std::malloc(kBuf));
    if (!line || !tmp) {
        if (line) std::free(line);
        if (tmp)  std::free(tmp);
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    int ok = 0;              // 與反編譯 v16/v2：正常讀到 EOF 會成為 1
    AniSet* cur = nullptr;   // 目前動作區

    // 第一行
    if (!std::fgets(line, (int)kBuf, fp)) {
        // 空檔案：反編譯會直接關檔並回傳 0；維持一致
        std::free(line); std::free(tmp); g_clTextFileManager.fclose(fp);
        return 0;
    }

    do {
        std::strcpy(tmp, line);
        char* head = std::strtok(tmp, Delim);
        if (!head) break;

        // 區段辨識
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
            if (!k) { ok = 0; goto FAIL; }
            attackKey_ = static_cast<uint16_t>(std::atoi(k));
            // 這行結束，繼續下一行
            ok = 1;
            continue;
        }
        else {
            // 未知標籤 → 與反編譯一致：視為失敗
            ok = 0; goto FAIL;
        }

        // 解析該行後續的多組 [HEX, a, b, c]
        char* tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 1; continue; } // 該行沒有 frame，也算正常行

        while (IsAlphaNumeric(tok)) {
            if (!cur) { ok = 0; goto FAIL; }

            uint16_t& cnt = cur->count;
            if (cnt >= kMaxFrames) break; // 超出上限：與安全一致，忽略後續

            bool hexok = false;
            cur->frameRes[cnt] = hex_to_u32(tok, hexok);
            if (!hexok) { ok = 0; goto FAIL; }

            tok = std::strtok(nullptr, Delim);
            if (!tok) { ok = 0; goto FAIL; }
            cur->paramA[cnt] = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, Delim);
            if (!tok) { ok = 0; goto FAIL; }
            cur->paramB[cnt] = static_cast<uint16_t>(std::atoi(tok));

            tok = std::strtok(nullptr, Delim);
            if (!tok) { ok = 0; goto FAIL; }
            cur->paramC[cnt] = static_cast<uint16_t>(std::atoi(tok));

            ++cnt;

            // 下一筆（可能是下一個 HEX 或行尾）
            tok = std::strtok(nullptr, Delim);
            if (!tok) { ok = 1; break; } // 正常行尾
        }

        // 若 tok 存在但不是英數，模仿反編譯：視為格式錯誤
        if (tok && !IsAlphaNumeric(tok)) { ok = 0; goto FAIL; }

    } while (std::fgets(line, (int)kBuf, fp));

    ok = 1;

FAIL:
    std::free(line);
    std::free(tmp);
    g_clTextFileManager.fclose(fp);
    return ok;
}

int cltPetAniInfo::GetFrameInfo(unsigned int a2, unsigned short a3,
    unsigned int* a4, unsigned short* a5)
{
    if (!a4 || !a5) return 0;
    *a4 = 0; *a5 = 0;

    if (a2 >= kActionCount) return 0;
    const AniSet& s = set_[a2];
    if (a3 >= s.count) return 0;

    // 對齊反編譯：取該影格的資源ID與參數A
    *a4 = s.frameRes[a3];
    *a5 = s.paramA[a3];
    return 1;
}

unsigned short cltPetAniInfo::GetTotalFrameNum(unsigned int a2)
{
    return (a2 < kActionCount) ? set_[a2].count : 0;
}
