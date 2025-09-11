#include "Info/cltMapUseItemInfoKindInfo.h"

static inline int to_hex_u32(const char* s, uint32_t& out)
{
    unsigned int tmp = 0;
    if (!s) return 0;
    if (std::sscanf(s, "%x", &tmp) != 1) return 0;
    out = static_cast<uint32_t>(tmp);
    return 1;
}

bool cltMapUseItemInfoKindInfo::IsDigitString(const char* s)
{
    if (!s || !*s) return false;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p)
        if (!std::isdigit(*p)) return false;
    return true;
}

uint16_t cltMapUseItemInfoKindInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;
    if (std::strlen(a1) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    int lo = std::atoi(a1 + 1);
    if (lo < 0x800) return static_cast<uint16_t>(hi | lo);
    return 0;
}

int cltMapUseItemInfoKindInfo::Initialize(char* filename)
{
    Free();

    const char* Delim = "\t\n";
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    int ok = 0;              // 與反編譯的 v41 同語意：最後成功時設為 1
    char line[1024] = { 0 };

    // 跳過前三行表頭/註解
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 預先計算資料行數
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    count_ = 0;
    while (std::fgets(line, sizeof(line), fp)) ++count_;

    // 無資料亦視為成功（與反編譯一致）
    if (count_ == 0) {
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    table_ = static_cast<strMapUseItemInfoKindInfo*>(std::malloc(sizeof(strMapUseItemInfoKindInfo) * count_));
    if (!table_) { g_clTextFileManager.fclose(fp); return 0; }
    std::memset(table_, 0, sizeof(strMapUseItemInfoKindInfo) * count_);

    // 回到資料起點
    std::fsetpos(fp, &pos);

    int idx = 0;
    while (idx < count_ && std::fgets(line, sizeof(line), fp)) {
        strMapUseItemInfoKindInfo& rec = table_[idx];
        std::memset(&rec, 0, sizeof(rec));

        // 1) ID
        char* tok = std::strtok(line, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.ID = TranslateKindCode(tok);

        // 2) 기획자(아이템 이름)（僅檢查存在、不存入）
        if (!std::strtok(nullptr, Delim)) { ok = 0; goto FAIL; }

        // 3) 기후명 Text_ID
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.ClimateNameTextID = static_cast<uint16_t>(std::atoi(tok));

        // 4) 설명text
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.DescTextID = static_cast<uint16_t>(std::atoi(tok));

        // 5) 지속시간(ms)
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.DurationMS = static_cast<uint32_t>(std::atoi(tok));

        // 6) 유닛숫자
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.UnitCount = static_cast<uint32_t>(std::atoi(tok));

        // 7) 리소스 아이디 (HEX)
        tok = std::strtok(nullptr, Delim);
        if (!to_hex_u32(tok, rec.ResourceID)) { ok = 0; goto FAIL; }

        // 8) 시작 블록
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.StartBlock = static_cast<uint32_t>(std::atoi(tok));

        // 9) 최대 블록수
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.MaxBlocks = static_cast<uint32_t>(std::atoi(tok));

        // 10) 애니메이션 시작 블록 아이디
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.AnimStartBlockID = static_cast<uint32_t>(std::atoi(tok));

        // 11) 애니메이션 속도
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.AnimSpeed = static_cast<uint32_t>(std::atoi(tok));

        // 12) 공중 속도(떨어지는 속도)
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.AirSpeed = static_cast<uint32_t>(std::atoi(tok));

        // 13) 아이템 아이콘 리소스 아이디 (HEX)
        tok = std::strtok(nullptr, Delim);
        if (!to_hex_u32(tok, rec.ItemIconResID)) { ok = 0; goto FAIL; }

        // 14) 블록아이디
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.BlockID1 = static_cast<uint32_t>(std::atoi(tok));

        // 15) 기상아이콘 리소스 아이디 (HEX)
        tok = std::strtok(nullptr, Delim);
        if (!to_hex_u32(tok, rec.WeatherIconResID)) { ok = 0; goto FAIL; }

        // 16) 블록 아이디(二)
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.BlockID2 = static_cast<uint32_t>(std::atoi(tok));

        // 17) 패턴
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.Pattern = static_cast<uint32_t>(std::atoi(tok));

        // 18) HP 회복량(맵내)
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.HPRecov = static_cast<uint32_t>(std::atoi(tok));

        // 19) MP 회복량(맵내)
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.MPRecov = static_cast<uint32_t>(std::atoi(tok));

        // 20) 회피율(천분율)
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.EvasionPermille = static_cast<uint32_t>(std::atoi(tok));

        // 21) 명중률(천분율)
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.AccuracyPermille = static_cast<uint32_t>(std::atoi(tok));

        // 22) 크리티컬 확률(천분율)
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.CriticalPermille = static_cast<uint32_t>(std::atoi(tok));

        // 23) 펫 애정도 상승률(천분율)
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.PetAffinityPermille = static_cast<uint32_t>(std::atoi(tok));

        // 24) EaIndex
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.EaIndex = static_cast<uint32_t>(std::atoi(tok));

        // 25) EaFileName(미리보기)
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        std::strcpy(rec.EaFileName, tok);  // 與反編譯一致（不做長度檢查）

        // 26) 서브아이템갯수
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; goto FAIL; }
        rec.SubItemCount = static_cast<uint32_t>(std::atoi(tok));

        // 27~31) 서브아이템ID_1..5（皆為 5 碼代碼；若為 "0" 會轉為 0）
        for (int i = 0; i < 5; ++i) {
            tok = std::strtok(nullptr, Delim);
            if (!tok) break;
            rec.SubItemID[i] = TranslateKindCode(tok);
        }

        ++idx;
        continue;

    FAIL:
        // 任一筆解析失敗 → 保持 ok=0，釋放並結束
        std::fclose(fp);
        Free();
        return 0;
    }

    // 讀到 EOF（最後一次 _fgets 失敗）才算成功
    ok = 1;
    std::fclose(fp);
    // 若實際讀入筆數少於預估行數，維持行為穩定：更新 count_
    count_ = idx;
    return ok;
}

void cltMapUseItemInfoKindInfo::Free()
{
    if (table_) {
        std::free(table_);
        table_ = nullptr;
    }
    count_ = 0;
}

strMapUseItemInfoKindInfo* cltMapUseItemInfoKindInfo::GetMapUseItemInfoKindInfo(uint16_t a2)
{
    if (!table_ || count_ <= 0) return nullptr;
    // 反編譯為：以 WORD 指針逐筆掃描，步距 114 WORDs (即 228 bytes)
    for (int i = 0; i < count_; ++i) {
        if (table_[i].ID == a2) return &table_[i];
    }
    return nullptr;
}
