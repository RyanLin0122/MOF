#include "Info/cltMapUseItemInfoKindInfo.h"

// mofclient.c:312108 — cltMapUseItemInfoKindInfo::TranslateKindCode
// GT 內 v3 為 unsigned __int16，因此 _atoi 結果先被 16 bit 截短再與 0x800 比較。
uint16_t cltMapUseItemInfoKindInfo::TranslateKindCode(char* a1)
{
    if (std::strlen(a1) != 5) return 0;
    const int v2 = (std::toupper(static_cast<unsigned char>(*a1)) + 31) << 11;
    const uint16_t v3 = static_cast<uint16_t>(std::atoi(a1 + 1));   // GT: v3 = (uint16) _atoi(...)
    if (v3 < 0x800u) return static_cast<uint16_t>(v2 | v3);
    return 0;
}

bool cltMapUseItemInfoKindInfo::IsDigitString(const char* s)
{
    if (!s || !*s) return false;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p)
        if (!std::isdigit(*p)) return false;
    return true;
}

// mofclient.c:311860 — cltMapUseItemInfoKindInfo::Initialize("mapuseiteminfo.txt")
//
// GT 控制流摘要：
//   1) 設 v41 = 0；count_ = 0；fopen；若失敗 → 直接 return v41(=0)。
//   2) 連續三次 fgets 跳過表頭；任一失敗 → 跳到 fclose / return 0。
//   3) fgetpos → 預掃 fgets 直到 EOF 以決定 count_。
//   4) 若 count_==0 → return 1（GT 在此會洩漏 fp，本 port 改為先 fclose 後 return）。
//   5) operator new(228 * count_) + memset 0；fsetpos 回到資料起點。
//   6) 若首次 fgets 為 NULL（無資料列）→ goto LABEL_40：v41=1 後 fclose / return 1。
//   7) 進入 LABEL_10 解析迴圈：
//        - 第 1 欄 ID 由 TranslateKindCode 寫入 +0；之後依 GT 偏移逐欄解析。
//        - 任一欄位的 strtok 為 NULL → 透過 if-巢狀 fall-through 退出整個迴圈，
//          結束後 fclose / return v41(=0)。
//        - 第 25 欄 EaFileName(strcpy) 寫入 +100；第 26 欄 SubItemCount 寫入 +96。
//        - 第 27..31 欄 SubItemID[0..4] 依序寫入 +4..+12；任一欄 strtok 為 NULL
//          會 break 內層 while → 整體 return 0（與 GT 完全一致）。
//        - 一列 5 個 SubItem 都成功後 v9 += 228、fgets 下一列：
//            * 若 fgets 成功 → 重回 LABEL_10 繼續解析下一列。
//            * 若 fgets 為 NULL → goto LABEL_40 → v41=1 → fclose / return 1。
//
// 檔案欄位 (mapuseiteminfo.txt) 與結構體偏移對照（GT verbatim）：
//   col[ 1] ID                                  -> +0   (u16, TranslateKindCode)
//   col[ 2] 기획자(아이템 이름)                  -> *DISCARDED*（僅檢查非空）
//   col[ 3] 기후명 Text_ID                       -> +2   (u16, atoi)
//   col[ 4] 설명text                             -> +14  (u16, atoi)
//   col[ 5] 지속시간(ms)                         -> +16  (u32, atoi)
//   col[ 6] 유닛숫자                             -> +32  (u32, atoi)
//   col[ 7] 리소스 아이디 (HEX)                  -> +20  (u32, sscanf "%x")
//   col[ 8] 시작 블록                            -> +36  (u32, atoi)
//   col[ 9] 최대 블록수                          -> +40  (u32, atoi)
//   col[10] 애니메이션 시작 블록 아이디          -> +44  (u32, atoi)
//   col[11] 애니메이션 속도                      -> +48  (u32, atoi)
//   col[12] 공중 속도                            -> +52  (u32, atoi)
//   col[13] 아이템 아이콘 리소스 아이디 (HEX)    -> +28  (u32, sscanf "%x")
//   col[14] 블록아이디                           -> +56  (u32, atoi)
//   col[15] 기상아이콘 리소스 아이디 (HEX)       -> +24  (u32, sscanf "%x")
//   col[16] 블록 아이디(二)                      -> +60  (u32, atoi)
//   col[17] 패턴                                 -> +64  (u32, atoi)
//   col[18] HP 회복량(맵내)                      -> +68  (u32, atoi)
//   col[19] MP 회복량(맵내)                      -> +72  (u32, atoi)
//   col[20] 회피율(천분율)                       -> +76  (u32, atoi)
//   col[21] 명중률(천분율)                       -> +80  (u32, atoi)
//   col[22] 크리티컬 확률(천분율)                -> +84  (u32, atoi)
//   col[23] 펫 애정도 상승률(천분율)             -> +88  (u32, atoi)
//   col[24] EaIndex                              -> +92  (u32, atoi)
//   col[25] EaFileName(미리보기)                 -> +100 (char[128], strcpy 不檢長)
//   col[26] 서브아이템갯수                       -> +96  (u32, atoi)
//   col[27..31] 서브아이템ID_1..5                -> +4..+12 (5 × u16, TranslateKindCode)
int cltMapUseItemInfoKindInfo::Initialize(char* filename)
{
    const char* DELIMS = "\t\n";
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    char line[1024] = { 0 };

    // 跳過 3 行表頭（標題列 / 注釋列 / 欄位名列）。
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 預掃資料列數。
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    count_ = 0;
    while (std::fgets(line, sizeof(line), fp)) ++count_;

    // GT: if (!count_) return 1; （GT 在此洩漏 fp，本 port 先 fclose）
    if (count_ == 0) {
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    // operator new + memset 全歸零（GT verbatim：未寫入欄位保持 0）。
    table_ = static_cast<strMapUseItemInfoKindInfo*>(
                operator new(sizeof(strMapUseItemInfoKindInfo) * static_cast<size_t>(count_)));
    std::memset(table_, 0, sizeof(strMapUseItemInfoKindInfo) * static_cast<size_t>(count_));

    std::fsetpos(fp, &pos);

    int v41 = 0;          // 對應 GT v41：函式回傳值（0 失敗、1 成功）
    int idx = 0;
    char* v9 = reinterpret_cast<char*>(table_);  // 對應 GT v9：游標指向當前 record 起點

    // GT: if (!_fgets(...)) goto LABEL_40; → v41=1; fclose; return 1
    if (!std::fgets(line, sizeof(line), fp)) {
        v41 = 1;
        g_clTextFileManager.fclose(fp);
        return v41;
    }

    // GT: LABEL_10 解析迴圈
    for (;;) {
        // [1] ID（M****）→ +0
        char* tok = std::strtok(line, DELIMS);
        if (!tok) break;
        if (idx >= count_) break;  // 防呆：理論上 idx 永遠 < count_
        *reinterpret_cast<uint16_t*>(v9 + 0) = TranslateKindCode(tok);

        // [2] 기획자(아이템 이름) — 僅檢查非空（內容丟棄）
        if (!std::strtok(nullptr, DELIMS)) break;

        // [3] 기후명 Text_ID → +2 (u16, atoi)
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint16_t*>(v9 + 2) = static_cast<uint16_t>(std::atoi(tok));

        // [4] 설명text → +14 (u16, atoi)
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint16_t*>(v9 + 14) = static_cast<uint16_t>(std::atoi(tok));

        // [5] 지속시간(초:밀리세컨) → +16
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 16) = static_cast<uint32_t>(std::atoi(tok));

        // [6] 유닛숫자 → +32
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 32) = static_cast<uint32_t>(std::atoi(tok));

        // [7] 리소스 아이디 (HEX) → +20  GT: sscanf("%x", v9 + 20)
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        std::sscanf(tok, "%x", reinterpret_cast<unsigned int*>(v9 + 20));

        // [8] 시작 블록 → +36
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 36) = static_cast<uint32_t>(std::atoi(tok));

        // [9] 최대 블록수 → +40
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 40) = static_cast<uint32_t>(std::atoi(tok));

        // [10] 애니메이션 시작 블록 아이디 → +44
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 44) = static_cast<uint32_t>(std::atoi(tok));

        // [11] 애니메이션 속도 → +48
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 48) = static_cast<uint32_t>(std::atoi(tok));

        // [12] 공중 속도(떨어지는 속도) → +52
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 52) = static_cast<uint32_t>(std::atoi(tok));

        // [13] 아이템 아이콘 리소스 아이디 (HEX) → +28
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        std::sscanf(tok, "%x", reinterpret_cast<unsigned int*>(v9 + 28));

        // [14] 블록아이디 → +56
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 56) = static_cast<uint32_t>(std::atoi(tok));

        // [15] 기상아이콘 리소스 아이디 (HEX) → +24
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        std::sscanf(tok, "%x", reinterpret_cast<unsigned int*>(v9 + 24));

        // [16] 블록 아이디(二) → +60
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 60) = static_cast<uint32_t>(std::atoi(tok));

        // [17] 패턴 → +64
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 64) = static_cast<uint32_t>(std::atoi(tok));

        // [18] HP 회복량(맵내) → +68
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 68) = static_cast<uint32_t>(std::atoi(tok));

        // [19] MP 회복량(맵내) → +72
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 72) = static_cast<uint32_t>(std::atoi(tok));

        // [20] 회피율(맵내)(천분율) → +76
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 76) = static_cast<uint32_t>(std::atoi(tok));

        // [21] 명중률(맵내)(천분율) → +80
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 80) = static_cast<uint32_t>(std::atoi(tok));

        // [22] 크리티컬 확률(맵내)(천분율) → +84
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 84) = static_cast<uint32_t>(std::atoi(tok));

        // [23] 펫 애정도 상승률(맵내)(천분율) → +88
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 88) = static_cast<uint32_t>(std::atoi(tok));

        // [24] EaIndex → +92
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 92) = static_cast<uint32_t>(std::atoi(tok));

        // [25] EaFileName(미리보기) → +100  GT: strcpy(v9 + 100, tok) — 不檢查長度
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        std::strcpy(v9 + 100, tok);

        // [26] 서브아이템갯수 → +96  注意：GT 在 strcpy(EaFileName) 之後才寫 SubItemCount
        tok = std::strtok(nullptr, DELIMS);
        if (!tok) break;
        *reinterpret_cast<uint32_t*>(v9 + 96) = static_cast<uint32_t>(std::atoi(tok));

        // [27..31] 서브아이템ID_1..5 → +4..+12
        // GT 內層: v35=0; v36=(_WORD*)(v9+4);
        //          while(1) { tok=strtok; if(!tok) break;
        //                     *v36 = TranslateKindCode(tok); ++v35; ++v36;
        //                     if (v35>=5) { v9+=228; fgets...; goto LABEL_10/40 } }
        // 任一個 SubItemID strtok 為 NULL（少於 5 個欄位）→ 整體 return 0。
        bool got_all_five = false;
        {
            uint16_t* v36 = reinterpret_cast<uint16_t*>(v9 + 4);
            int v35 = 0;
            while (true) {
                char* v37 = std::strtok(nullptr, DELIMS);
                if (!v37) break;          // GT: break → fall-through → return 0
                *v36 = TranslateKindCode(v37);
                ++v35;
                ++v36;
                if (v35 >= 5) { got_all_five = true; break; }
            }
        }
        if (!got_all_five) break;

        ++idx;
        v9 += 228;

        // GT: if (!_fgets) goto LABEL_40 (v41=1); else continue (re-enter LABEL_10)
        if (!std::fgets(line, sizeof(line), fp)) {
            v41 = 1;
            break;
        }
    }

    g_clTextFileManager.fclose(fp);

    if (v41 == 1) {
        return 1;
    }

    // GT 在解析失敗時並未 Free，但釋放配置可避免 reload/重入泄漏；
    // 與 cltWeddingHallKindInfo / cltEnchantKindInfo 等 sibling 的保守做法一致。
    Free();
    return 0;
}

// mofclient.c:312097
void cltMapUseItemInfoKindInfo::Free()
{
    if (table_) {
        operator delete(static_cast<void*>(table_));
        table_ = nullptr;
    }
    count_ = 0;
}

// mofclient.c:312126 — 線性掃描 ID (+0)，stride 114 WORDs (=228 bytes)
strMapUseItemInfoKindInfo* cltMapUseItemInfoKindInfo::GetMapUseItemInfoKindInfo(uint16_t a2)
{
    if (count_ <= 0) return nullptr;
    for (int i = 0; i < count_; ++i) {
        if (table_[i].ID == a2) return &table_[i];
    }
    return nullptr;
}
