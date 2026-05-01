#include "Info/cltMonsterAniInfo.h"

// ---------------------------------------------------------------------------
// IsAlphaNumeric  —  byte-for-byte equivalent to mofclient.c:342945
//   - 空字串 "" 視為 1 (TRUE)（GT 語意）
//   - 任一非 alnum 字元 → 0 (FALSE)
// ---------------------------------------------------------------------------
bool cltMonsterAniInfo::IsAlphaNumeric(const char* s) {
    if (!s) return false;
    if (!*s) return true;              // GT: 空字串回傳 1
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p) {
        if (!std::isalnum(*p)) return false;
    }
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

// ---------------------------------------------------------------------------
// Initialize
// 對齊 mofclient.c:315685 的控制流：
//   v17 = 0;                                  // 預設失敗
//   if (fopen 失敗)            return 0;
//   if (第一行 fgets 失敗)     goto LABEL_32 → return 1;   // 空檔案 = 視為成功
//   while (1) {
//       head = strtok(line);
//       if (head == NULL)              break;          // 空行 → return 0
//       依 head 設定 cur (區段指標) 或 解析 ATTACK_KEY；未知 head → break;
//       v8 = strtok(NULL);
//       if (v8) {
//           while (IsAlphaNumeric(v8)) {
//               sscanf %x → frameRes[cnt]
//               atoi → paramA[cnt]    (任一 token == NULL → break inner)
//               atoi → paramB[cnt]
//               atoi → paramC[cnt]    cnt++;
//               v8 = strtok(NULL);
//               if (!v8) goto LABEL_30;            // 行尾乾淨：跳過下面 break
//           }
//           break;                                  // inner 退出 == 失敗 → return 0
//       }
// LABEL_30:
//       if (!fgets(next))   goto LABEL_32 → return 1;  // 唯一的 success path
//   }
//   return 0;                                     // v17 沒被改過
// ---------------------------------------------------------------------------
int cltMonsterAniInfo::Initialize(char* filename)
{
    // 與 GT 建構子（亦即本類別的建構子）一致：整體歸零
    std::memset(this, 0, sizeof(*this));

    const char* Delim = "\t\n,[]";
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    // GT 用兩個 10240-byte stack buffers，現代 x64 配置改 heap 以避免吃光 stack。
    // 大小與 fgets 上限與 GT 完全一致 (10239 + NUL)。
    const size_t kBufSize = 10240;
    char* line = static_cast<char*>(std::malloc(kBufSize));
    char* temp = static_cast<char*>(std::malloc(kBufSize));
    if (!line || !temp) {
        if (line) std::free(line);
        if (temp) std::free(temp);
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    int rc = 0;            // 對應 GT v17：預設 0，僅在 LABEL_32 路徑改為 1
    AniSet* cur = nullptr; // 對應 GT v4：當前動作區段指標

    // 第一行 fgets 失敗 → LABEL_32 → 回傳 1
    if (!std::fgets(line, static_cast<int>(kBufSize), fp)) {
        std::free(line);
        std::free(temp);
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    for (;;)
    {
        std::strcpy(temp, line);
        char* head = std::strtok(temp, Delim);
        if (!head) break;                          // GT: !v5 → break (return 0)

        // 區段辨識（__stricmp，大小寫不敏感）
        if      (ieq(head, "STOP"))     cur = &set_[STOP];
        else if (ieq(head, "MOVE"))     cur = &set_[MOVE];
        else if (ieq(head, "DIE"))      cur = &set_[DIE];
        else if (ieq(head, "ATTACK"))   cur = &set_[ATTACK];
        else if (ieq(head, "N_HITTED")) cur = &set_[N_HITTED];
        else if (ieq(head, "F_HITTED")) cur = &set_[F_HITTED];
        else if (ieq(head, "E_HITTED")) cur = &set_[E_HITTED];
        else if (ieq(head, "I_HITTED")) cur = &set_[I_HITTED];
        else if (ieq(head, "ATTACK_KEY")) {
            // GT: 解析數值後「不 continue」、繼續往下讀 v8 並判斷
            char* k = std::strtok(nullptr, Delim);
            if (!k) break;                         // GT: !v7 → break
            attackKey = static_cast<uint16_t>(std::atoi(k));
            // ↓ 不 continue：與 GT 一樣，落到下面的 v8 = strtok 路徑
        }
        else {
            break;                                 // GT: 未知 head → break
        }

        char* tok = std::strtok(nullptr, Delim);   // GT: v8
        if (tok) {
            bool clean_eol = false;                // 是否安全結束此行（goto LABEL_30）
            while (IsAlphaNumeric(tok)) {
                if (!cur) goto cleanup;                // 防呆：未指派區段就有資料
                uint16_t& cnt = cur->count;
                if (cnt >= kMaxFrames) {
                    // GT 沒有上限檢查（會寫越界）；現代 build 加防呆，邏輯等價於
                    // 「真實檔不會超過 100」這個前提。退到「此行尾」並繼續下一行。
                    goto next_line;
                }

                // 1) 影格資源 ID（%x）
                unsigned int res = 0;
                std::sscanf(tok, "%x", &res);      // GT: sscanf 回傳值未檢查
                cur->frameRes[cnt] = res;

                // 2) paramA
                tok = std::strtok(nullptr, Delim);
                if (!tok) goto cleanup;                // GT: !v9 → inner break → outer break
                cur->paramA[cnt] = static_cast<uint16_t>(std::atoi(tok));

                // 3) paramB
                tok = std::strtok(nullptr, Delim);
                if (!tok) goto cleanup;
                cur->paramB[cnt] = static_cast<uint16_t>(std::atoi(tok));

                // 4) paramC
                tok = std::strtok(nullptr, Delim);
                if (!tok) goto cleanup;
                cur->paramC[cnt] = static_cast<uint16_t>(std::atoi(tok));

                ++cnt;                              // 對應 GT 的 (count)++

                tok = std::strtok(nullptr, Delim); // GT: 下一個 v8
                if (!tok) { clean_eol = true; break; }   // GT: goto LABEL_30
            }
            if (!clean_eol) goto cleanup;              // GT: inner 退出 == outer break
        }
        // ↑ if (v8) 為 false 也視為「正常空尾」，落到 NEXT_LINE 讀下一行
next_line:
        if (!std::fgets(line, static_cast<int>(kBufSize), fp)) {
            rc = 1;                                 // GT: LABEL_32 → return 1
            break;
        }
    }

cleanup:
    std::free(line);
    std::free(temp);
    g_clTextFileManager.fclose(fp);
    return rc;
}

// ---------------------------------------------------------------------------
// GetFrameInfo  —  byte-arithmetic 對齊 mofclient.c:315808
//   if (a3 >= *((WORD*)this + 502*a2))     return 0;   // 即 set_[a2].count
//   *a4 = *((DWORD*)this + 251*a2 + a3 + 1);            // 即 set_[a2].frameRes[a3]
//   *a5 = *((WORD*)this  + 502*a2 + a3 + 202);          // 即 set_[a2].paramA[a3]
// ---------------------------------------------------------------------------
int cltMonsterAniInfo::GetFrameInfo(unsigned int a2, unsigned short a3,
    unsigned int* a4, unsigned short* a5)
{
    if (!a4 || !a5) return 0;
    *a4 = 0; *a5 = 0;
    if (a2 >= kActionCount) return 0;

    const AniSet& s = set_[a2];
    if (a3 >= s.count) return 0;

    *a4 = s.frameRes[a3];
    *a5 = s.paramA[a3];
    return 1;
}

// ---------------------------------------------------------------------------
// GetTotalFrameNum  —  對齊 mofclient.c:315827
//   if (a2 < 8) return *((WORD*)this + 502*a2);   // 即 set_[a2].count
//   else        return 0;
// ---------------------------------------------------------------------------
unsigned short cltMonsterAniInfo::GetTotalFrameNum(unsigned int a2)
{
    return (a2 < kActionCount) ? set_[a2].count : 0;
}
