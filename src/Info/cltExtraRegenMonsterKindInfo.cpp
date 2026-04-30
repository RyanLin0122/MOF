#include "Info/cltExtraRegenMonsterKindInfo.h"
#include <cstring>
#include <cstdlib>
#include <cctype>

// 反編譯位置 mofclient.c:303592（與 cltCharKindInfo / cltMapAreaKindInfo 完全一致）
// 5 글자, 첫 글자 알파벳 + 4 자리 십진수, atoi(s+1) < 0x800.
uint16_t cltExtraRegenMonsterKindInfo::TranslateKindCode(char* s) {
    if (!s) return 0;
    if (std::strlen(s) != 5) return 0;
    const int hi = (std::toupper(static_cast<unsigned char>(s[0])) + 31) << 11;
    const uint16_t num = static_cast<uint16_t>(std::atoi(s + 1));
    if (num < 0x800u) return static_cast<uint16_t>(hi | num);
    return 0;
}

// ---------------------------------------------------------------------------
// cltExtraRegenMonsterKindInfo::Initialize  ── mofclient.c:303359-303539
//
// 解析 extraregenmonster.txt（EUC-KR）。流程逐字對齊反編譯：
//
//   1. fgets x3 跳過表頭。
//   2. 預掃行數，存入 m_count。若 m_count == 0：直接回傳 1。
//   3. operator new(200 * m_count) + memset 0。
//   4. fsetpos 回資料起點。
//   5. fgets 第一筆資料行。失敗（EOF）→ v31 = 1，return 1。
//   6. 對每行（goto LABEL_10 重入點）：
//        [0] 토큰 -> kind @ +0  (TranslateKindCode '\t\n')
//        [1] 토큰 -> 丟棄
//        [2] 토큰 -> strcpy 到本地 String[1024]，留待後續 '|' 切分
//        [3] 토큰 -> initCount @ +16
//        [4] 토큰 -> minRegen  @ +20
//        [5] 토큰 -> maxRegen  @ +24
//        [6] 토큰 -> interval  @ +28，clamp >=1 後再寫一次。
//                    若 (max-min) % interval != 0：直接跳出整個解析（不前進、
//                    不再讀下一行）—— 反編譯之 v31 維持 0。
//        [7] 토큰 -> type @ +32：stricmp "RANDOM"==1, "ONCE"==2, 否則 0。
//        kind/HP loop（最多 20 組）：
//            kind 토큰 NULL  → 跳出整個解析（無 v31 設定）
//            ck != 0：HP 토큰 NULL → 跳出整個解析
//                     寫 mob[i].hp、mobCnt++、若 i+1<20 continue。
//            ck == 0 或 i==20：跳到 map area 區塊（不再讀更多 kind/HP）。
//        map area block：strtok(String, "|")
//            首次 strtok NULL → 跳出整個解析
//            while mapAreaCnt < 5：
//              寫 mapArea[mapAreaCnt]（即使是 0 也照寫）
//              若剛寫入是 0 → break（不遞增）→ 跳出整個解析
//              mapAreaCnt++
//              strtok(NULL, "|") NULL → 前進到下一筆 (v9 += 200)，
//                  fgets 下一行，成功則 goto LABEL_10；失敗則 v31 = 1。
//            mapAreaCnt == 5 而 strtok 仍非 NULL → 跳出整個解析（不前進）
//   7. fclose, return v31。
//
// 重要的反編譯細節：
//   * v31 預設為 0；只有 (a) m_count==0 早退、(b) 首次 fgets 失敗（無資料）、
//     (c) 在 map area 成功路徑後續 fgets 失敗（乾淨 EOF）三種情況回傳 1。
//   * m_count 一開始即定為「預掃行數」，全程不再修改；因此 GetExtraRegenMonsterNum
//     回傳的是檔案行數（含未完整解析的尾段，全部已 memset 為 0）。
//   * 半解析行（中途 break）的部分欄位會留在原地的 m_items[v9]，因為反編譯
//     是直接寫入 v9 指向的記憶體，沒有額外的「commit」步驟。
//   * 同一行若已寫入 5 個有效地區，反編譯不會再讀下一行 → 整個檔案後續行
//     都被靜默丟棄。本實作完全保留此「bug-for-bug」行為。
// ---------------------------------------------------------------------------
int cltExtraRegenMonsterKindInfo::Initialize(char* filename) {
    if (!filename) return 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    char Delimiter[4] = { '\t', '\n', '\0', '\0' };
    int  v31 = 0;
    char Buffer[1024];
    std::memset(Buffer, 0, sizeof(Buffer));
    char String[1024];   // 本地暫存第 3 欄（map_secter_ID 列表）
    std::memset(String, 0, sizeof(String));

    // 跳過表頭 3 行 —— 任一失敗就直接 fclose 回傳 v31 = 0。
    if (std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp)
        && std::fgets(Buffer, 1023, fp))
    {
        fpos_t Position{};
        std::fgetpos(fp, &Position);

        // 預掃資料行數，寫入 m_count。
        m_count = 0;
        while (std::fgets(Buffer, 1023, fp)) ++m_count;

        if (m_count == 0) {
            g_clTextFileManager.fclose(fp);
            return 1;
        }

        m_items = static_cast<strExtraRegenMonsterKindInfo*>(
            operator new(sizeof(strExtraRegenMonsterKindInfo) * static_cast<size_t>(m_count)));
        std::memset(m_items, 0,
            sizeof(strExtraRegenMonsterKindInfo) * static_cast<size_t>(m_count));

        std::fsetpos(fp, &Position);

        // v9 對齊反編譯：當前資料行寫入點。
        char* v9 = reinterpret_cast<char*>(m_items);

        if (!std::fgets(Buffer, 1023, fp)) {
            // LABEL_36：第一行 fgets 即失敗 → 乾淨 EOF。
            v31 = 1;
            g_clTextFileManager.fclose(fp);
            return v31;
        }

        // ----- LABEL_10 重入式逐行解析 -----
        for (;;) {
            // [0] ID
            char* v10 = std::strtok(Buffer, Delimiter);
            if (!v10) goto end_parse;
            *reinterpret_cast<uint16_t*>(v9 + 0) = TranslateKindCode(v10);

            // [1] map_secter_ID(기획자용) —— 丟棄
            if (!std::strtok(nullptr, Delimiter)) goto end_parse;

            // [2] map_secter_ID（"|" 分隔列表，先存到 String[]）
            char* v11 = std::strtok(nullptr, Delimiter);
            if (!v11) goto end_parse;
            std::strcpy(String, v11);

            // [3] 초기 리젠 개수 @ +16
            char* v12 = std::strtok(nullptr, Delimiter);
            if (!v12) goto end_parse;
            *reinterpret_cast<int32_t*>(v9 + 16) = std::atoi(v12);

            // [4] min_regen @ +20
            char* v13 = std::strtok(nullptr, Delimiter);
            if (!v13) goto end_parse;
            *reinterpret_cast<int32_t*>(v9 + 20) = std::atoi(v13);

            // [5] max_regen @ +24
            char* v14 = std::strtok(nullptr, Delimiter);
            if (!v14) goto end_parse;
            *reinterpret_cast<int32_t*>(v9 + 24) = std::atoi(v14);

            // [6] interval @ +28（先寫原值、再 clamp >=1 寫回；隨後檢查整除）
            char* v15 = std::strtok(nullptr, Delimiter);
            if (!v15) goto end_parse;
            int v16 = std::atoi(v15);
            *reinterpret_cast<int32_t*>(v9 + 28) = v16;
            int v17 = (v16 < 1) ? 1 : v16;
            int diff = *reinterpret_cast<int32_t*>(v9 + 24)
                     - *reinterpret_cast<int32_t*>(v9 + 20);
            *reinterpret_cast<int32_t*>(v9 + 28) = v17;
            if ((diff % v17) != 0) goto end_parse;

            // [7] TYPE @ +32
            char* v19 = std::strtok(nullptr, Delimiter);
            if (!v19) goto end_parse;
            if (!::_stricmp(v19, "RANDOM"))
                *reinterpret_cast<int32_t*>(v9 + 32) = 1;
            else if (!::_stricmp(v19, "ONCE"))
                *reinterpret_cast<int32_t*>(v9 + 32) = 2;
            else
                *reinterpret_cast<int32_t*>(v9 + 32) = 0;

            // ----- kind/HP loop -----
            int  v21 = 0;
            int* v22 = reinterpret_cast<int*>(v9 + 40);   // == &mob[0].hp
            bool advance_to_next_row = false;
            for (;;) {
                char* v23 = std::strtok(nullptr, Delimiter);
                if (!v23) goto end_parse;

                uint16_t v24 = cltCharKindInfo::TranslateKindCode(v23);
                // 即使 v24 == 0 也照寫（mofclient.c 寫入後才檢查）。
                *(reinterpret_cast<uint16_t*>(v22) - 2) = v24;

                if (v24) {
                    char* v25 = std::strtok(nullptr, Delimiter);
                    if (!v25) goto end_parse;
                    *v22 = std::atoi(v25);
                    ++v21;
                    v22 += 2;                                     // +8 bytes，跳到下一個 mob 的 hp
                    ++*reinterpret_cast<int32_t*>(v9 + 196);      // mobCnt++
                    if (v21 < 20) continue;
                }

                // ----- map area block（v24 == 0 或 v21 == 20 才到此）-----
                {
                    char* v26 = std::strtok(String, "|");
                    if (!v26) goto end_parse;

                    while (*reinterpret_cast<int32_t*>(v9 + 12) < 5) {
                        // 寫入 mapArea[mapAreaCnt]（即使 TranslateKindCode 回傳 0 也照寫）
                        *reinterpret_cast<uint16_t*>(
                            v9 + 2 * *reinterpret_cast<int32_t*>(v9 + 12) + 2)
                            = cltMapAreaKindInfo::TranslateKindCode(v26);

                        int v27 = *reinterpret_cast<int32_t*>(v9 + 12) + 1;
                        // 檢查剛寫入是否為 0：是 → 不遞增 mapAreaCnt，整個解析中止。
                        if (!*reinterpret_cast<uint16_t*>(v9 + 2 * v27)) goto end_parse;
                        *reinterpret_cast<int32_t*>(v9 + 12) = v27;

                        v26 = std::strtok(nullptr, "|");
                        if (!v26) {
                            // 成功路徑：前進到下一筆，讀下一行。
                            v9 += 200;
                            if (!std::fgets(Buffer, 1023, fp)) {
                                v31 = 1;
                                goto end_parse;
                            }
                            advance_to_next_row = true;
                            break;
                        }
                    }
                    // 若 mapAreaCnt 達到 5 但 strtok 仍非 NULL：直接中止（不前進）。
                    if (!advance_to_next_row) goto end_parse;
                }
                break;  // 跳出 kind/HP loop，外層 for 重新開始解析新行
            }
        }

    end_parse:
        ;  // 落到 fclose
    }

    g_clTextFileManager.fclose(fp);
    return v31;
}

void cltExtraRegenMonsterKindInfo::Free() {
    if (m_items) {
        operator delete(static_cast<void*>(m_items));
        m_items = nullptr;
    }
    m_count = 0;
}

// 反編譯 mofclient.c:303559 — 線性搜尋（步距 100 WORD = 200 bytes）。
strExtraRegenMonsterKindInfo*
cltExtraRegenMonsterKindInfo::GetExtraRegenMonsterKindInfo(uint16_t a2) {
    for (int i = 0; i < m_count; ++i) {
        if (m_items[i].kind == a2) return &m_items[i];
    }
    return nullptr;
}

// 反編譯 mofclient.c:303580
strExtraRegenMonsterKindInfo*
cltExtraRegenMonsterKindInfo::GetExtraRegenMonsterKindInfoByIndex(int a2) {
    if (a2 < 0 || a2 >= m_count) return nullptr;
    return &m_items[a2];
}
