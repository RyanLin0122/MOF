#include "Info/cltTitleKindInfo.h"

// =============================================================================
// cltTitleKindInfo —— 호칭 종류 정보 (稱號種類資訊)
// 實作位置對應反編譯：
//     Initialize           mofclient.c:338263
//     Free                 mofclient.c:338441
//     TranslateKindCode    mofclient.c:338452
//     GetTitleKindInfo()   mofclient.c:338470
//     GetTitleKindNum      mofclient.c:338476
//     GetTitleKindInfo(c)  mofclient.c:338482
//     TranslateGetCondition mofclient.c:338503
// =============================================================================

// -----------------------------------------------------------------------------
// Initialize — 反編譯邏輯逐行還原
//
//   1) cltTextFileManager::fopen()
//   2) 連續三次 _fgets 讀取 header（標題行 / 空行 / 欄位名）；任何一次失敗 → return 0
//   3) _fgetpos 記錄資料區起始位置
//   4) 第一遍掃描：每讀到一行 fgets 成功就 ++m_count
//   5) operator new(28 * m_count) + memset 0
//   6) _fsetpos 回到資料起點
//   7) 第一行 fgets：
//        失敗 → goto LABEL_35（v27 = 1，視為「空資料區」成功）
//        成功 → 進入 while(2) 解析迴圈
//   8) 每個 row 依序 strtok 取 7~9 欄；任一失敗或 condition==0 → 跳出 while(2) 失敗
//   9) Row 解析成功 → v7 += 28；下一次 fgets：失敗 → goto LABEL_35（v27 = 1, 全部成功）
// -----------------------------------------------------------------------------
int cltTitleKindInfo::Initialize(char* filename)
{
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    // 反編譯 strcpy(Delimiter, "\t\n") 寫入 4-byte slot；strtok 需 NUL-terminated
    char Delimiter[4] = { '\t', '\n', '\0', '\0' };
    char Buffer[1024];
    std::memset(Buffer, 0, sizeof(Buffer));

    int v27 = 0;  // 成功旗標 (對應反編譯 [esp+14h] [ebp-40Ch])

    // header 三行
    if (std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp))
    {
        fpos_t Position{};
        std::fgetpos(fp, &Position);

        // 第一遍掃描：計算資料行數
        m_count = 0;
        while (std::fgets(Buffer, 1023, fp)) {
            ++m_count;
        }

        // 配置並清 0 (operator new + memset(28*m_count))
        Free();
        m_list = new strTitleKindInfo[m_count]();  // value-init = 全 0

        std::fsetpos(fp, &Position);

        // v7 = 反編譯局部指標，每解析完一筆 +=28
        char* v7 = reinterpret_cast<char*>(m_list);

        if (std::fgets(Buffer, 1023, fp)) {
            for (;;) {
                // ---- col 0 : 호칭_ID (稱號代碼) ----
                char* v8 = std::strtok(Buffer, Delimiter);
                if (!v8) break;
                uint16_t v9 = TranslateKindCode(v8);
                *reinterpret_cast<uint16_t*>(v7 + 0) = v9;
                if (v9 == 0) break;

                // ---- col 1 : 호칭 (한글 표기) — 僅檢查存在，不存值 ----
                if (!std::strtok(nullptr, Delimiter)) break;

                // ---- col 2 : 이름_code (名稱資源代碼, WORD) ----
                char* v10 = std::strtok(nullptr, Delimiter);
                if (!v10) break;
                *reinterpret_cast<uint16_t*>(v7 + 2) =
                    static_cast<uint16_t>(std::atoi(v10));

                // ---- col 3 : lv_min (最低等級, DWORD) ----
                char* v11 = std::strtok(nullptr, Delimiter);
                if (!v11) break;
                *reinterpret_cast<int32_t*>(v7 + 4) = std::atoi(v11);

                // ---- col 4 : lv_max (最高等級, DWORD) ----
                char* v12 = std::strtok(nullptr, Delimiter);
                if (!v12) break;
                *reinterpret_cast<int32_t*>(v7 + 8) = std::atoi(v12);

                // ---- col 5 : condition (取得條件字串 → 列舉) ----
                char* v13 = std::strtok(nullptr, Delimiter);
                if (!v13) break;
                int v14 = TranslateGetCondition(v13);
                *reinterpret_cast<int32_t*>(v7 + 12) = v14;
                if (v14 == 0) break;  // 反編譯：if (v14) ... 否則落到外層 break

                // ---- col 6+ : 依條件解析 param1 / param2 / param3 ----
                bool advance = false;  // 對應反編譯 goto LABEL_34
                switch (v14) {
                case TGC_COMPLETEQUEST: {  // 1 — 퀘스트 완수
                    char* v15 = std::strtok(nullptr, Delimiter);
                    if (v15) {
                        *reinterpret_cast<uint16_t*>(v7 + 16) =
                            cltQuestKindInfo::TranslateKindCode(v15);
                        advance = true;
                    }
                } break;

                case TGC_GETSKILL: {  // 2 — 스킬 습득
                    char* v16 = std::strtok(nullptr, Delimiter);
                    if (v16) {
                        *reinterpret_cast<uint16_t*>(v7 + 16) =
                            cltSkillKindInfo::TranslateKindCode(v16);
                        advance = true;
                    }
                } break;

                case TGC_SETEMBLEM: {  // 3 — 엠블렘 장착
                    char* v17 = std::strtok(nullptr, Delimiter);
                    if (v17) {
                        *reinterpret_cast<uint16_t*>(v7 + 16) =
                            cltEmblemKindInfo::TranslateKindCode(v17);
                        advance = true;
                    }
                } break;

                case TGC_GETITEM: {  // 4 — 아이템 획득 (param1=道具代碼, param2=數量)
                    char* v18 = std::strtok(nullptr, Delimiter);
                    if (v18) {
                        *reinterpret_cast<uint16_t*>(v7 + 16) =
                            cltItemKindInfo::TranslateKindCode(v18);
                        char* v19 = std::strtok(nullptr, Delimiter);
                        if (v19) {
                            *reinterpret_cast<int32_t*>(v7 + 20) = std::atoi(v19);
                            advance = true;
                        }
                    }
                } break;

                case TGC_NONE: {  // 5 — 조건 없음 (僅消耗 2 欄位)
                    if (std::strtok(nullptr, Delimiter)
                     && std::strtok(nullptr, Delimiter))
                        advance = true;
                } break;

                case TGC_KILLMONSTER: {  // 6 — 몬스터 처치
                    char* v20 = std::strtok(nullptr, Delimiter);
                    if (v20) {
                        *reinterpret_cast<uint16_t*>(v7 + 16) =
                            cltCharKindInfo::TranslateKindCode(v20);
                        char* v21 = std::strtok(nullptr, Delimiter);
                        if (v21) {
                            *reinterpret_cast<int32_t*>(v7 + 20) = std::atoi(v21);
                            char* v22 = std::strtok(nullptr, Delimiter);
                            if (v22) {
                                int v23 = std::atoi(v22);
                                bool v24 = (*reinterpret_cast<uint16_t*>(v7 + 16) == 0);
                                *reinterpret_cast<int32_t*>(v7 + 24) = v23;
                                // 反編譯：!v24 || v23 才視為有效 row
                                //   即「param1_code != 0」或「param3 != 0」其一成立
                                if (!v24 || v23 != 0)
                                    advance = true;
                            }
                        }
                    }
                } break;

                default:
                    // 反編譯 default 分支即 LABEL_34（advance）。
                    // 實務上 TranslateGetCondition 只回 {0..6}，且 v14==0 已被前面攔截，
                    // 故 default 在實際資料下不會被命中。
                    advance = true;
                    break;
                }

                if (!advance) break;  // 任一條件解析失敗 → 跳出 while(2)，v27 保持 0

                // LABEL_34 — 前進至下一筆
                v7 += sizeof(strTitleKindInfo);
                if (!std::fgets(Buffer, 1023, fp)) {
                    v27 = 1;  // LABEL_35
                    break;
                }
            }
        }
        else {
            // 空資料區 → 反編譯 LABEL_35：v27 = 1
            v27 = 1;
        }
    }

    g_clTextFileManager.fclose(fp);
    return v27;
}

// -----------------------------------------------------------------------------
// Free — mofclient.c:338441
// -----------------------------------------------------------------------------
void cltTitleKindInfo::Free()
{
    if (m_list) {
        operator delete(m_list);
        m_list = nullptr;
    }
    m_count = 0;
}

// -----------------------------------------------------------------------------
// TranslateKindCode — mofclient.c:338452
//   strlen(s)==5; result = ((toupper(s[0])+31) << 11) | atoi(s+1) (atoi<0x800)
// -----------------------------------------------------------------------------
uint16_t cltTitleKindInfo::TranslateKindCode(const char* s)
{
    if (!s || std::strlen(s) != 5) return 0;
    int v2 = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    uint16_t v3 = static_cast<uint16_t>(std::atoi(s + 1));
    if (v3 < 0x800u) return static_cast<uint16_t>(v2 | v3);
    return 0;
}

// -----------------------------------------------------------------------------
// GetTitleKindInfo(code) — mofclient.c:338482
//   線性查找 title_code 等於 code 的記錄；m_count<=0 或找不到回 nullptr。
// -----------------------------------------------------------------------------
strTitleKindInfo* cltTitleKindInfo::GetTitleKindInfo(uint16_t code)
{
    if (m_count <= 0) return nullptr;
    for (int i = 0; i < m_count; ++i) {
        if (m_list[i].title_code == code)
            return &m_list[i];
    }
    return nullptr;
}

// -----------------------------------------------------------------------------
// TranslateGetCondition — mofclient.c:338503
//   COMPLETEQUEST=1, GETSKILL=2, SETEMBLEM=3, GETITEM=4, KILLMONSTER=6, NONE=5
//   其餘字串：v2 = -1 → LOBYTE = 0xFB → v2 = 0xFFFFFFFB → v2 + 5 在 32-bit 下溢回 0
// -----------------------------------------------------------------------------
int cltTitleKindInfo::TranslateGetCondition(const char* s)
{
    if (!s) return 0;
#ifdef _WIN32
    auto ieq = [](const char* a, const char* b) { return _stricmp(a, b) == 0; };
#else
    auto ieq = [](const char* a, const char* b) { return strcasecmp(a, b) == 0; };
#endif
    if (ieq(s, "COMPLETEQUEST")) return 1;
    if (ieq(s, "GETSKILL"))      return 2;
    if (ieq(s, "SETEMBLEM"))     return 3;
    if (ieq(s, "GETITEM"))       return 4;
    if (ieq(s, "KILLMONSTER"))   return 6;
    if (ieq(s, "NONE"))          return 5;
    return 0;  // 反編譯 32-bit 算式溢位後即為 0
}
