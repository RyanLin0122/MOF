#include "Info/cltPortalInfo.h"

int cltPortalInfo::Initialize(char* filename)
{
    Free();

    const char* Delim = "\t\n";
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    int ok = 0;              // v36：只有完整跑到 EOF/空行時設 1
    char line[1024] = { 0 };

    // 跳過三行表頭
    if (!std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp) ||
        !std::fgets(line, sizeof(line), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 記錄位置並計數
    fpos_t pos{};
    std::fgetpos(fp, &pos);
    count_ = 0;
    while (std::fgets(line, sizeof(line), fp)) ++count_;

    // 配置表（每筆 40 bytes）；對齊反編譯：count_==0 仍呼叫 operator new
    table_ = static_cast<stPortalInfo*>(::operator new(sizeof(stPortalInfo) * count_));
    if (count_ > 0) {
        std::memset(table_, 0, sizeof(stPortalInfo) * count_);
    }

    // 回到資料起點
    std::fsetpos(fp, &pos);

    // 逐行解析
    int idx = 0;
    if (!std::fgets(line, sizeof(line), fp)) {
        // 無資料列也算成功
        g_clTextFileManager.fclose(fp);
        return 1;
    }

    while (true) {
        // 反編譯：if (!strcmp(&Buffer, "")) → 空行哨兵，視為正常結束
        if (line[0] == '\0') { ok = 1; break; }

        stPortalInfo& rec = table_[idx];

        // 1) Portal ID
        char* tok = std::strtok(line, Delim);
        if (!tok) { ok = 0; break; }
        rec.PortalID = TranslateKindCode(tok);

        // 2) MapID_1
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; break; }
        rec.MapID_1 = TranslateKindCode(tok);

        // 3) 맵 이름（跳過）
        if (!std::strtok(nullptr, Delim)) { ok = 0; break; }

        // 4) PosX_1
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { ok = 0; break; }
        rec.PosX_1 = std::atoi(tok);

        // 5) PosY_1
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { ok = 0; break; }
        rec.PosY_1 = std::atoi(tok);

        // 6) Portal type_1（WORD）
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { ok = 0; break; }
        rec.PortalType_1 = static_cast<uint16_t>(std::atoi(tok));

        // 7) 도착 맵 ID（MapID_2）
        tok = std::strtok(nullptr, Delim);
        if (!tok) { ok = 0; break; }
        rec.MapID_2 = TranslateKindCode(tok);

        // 反編譯：若 MapID_1 == MapID_2 則「中斷」（視為格式不符）
        if (rec.MapID_1 == rec.MapID_2) { ok = 0; break; }

        // 8) 도착 맵 이름（跳過）
        if (!std::strtok(nullptr, Delim)) { ok = 0; break; }

        // 9) PosX_2
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { ok = 0; break; }
        rec.PosX_2 = std::atoi(tok);

        // 10) PosY_2
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { ok = 0; break; }
        rec.PosY_2 = std::atoi(tok);

        // 11) 이미지 파일（%x）
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsAlphaNumeric(tok)) { ok = 0; break; }
        std::sscanf(tok, "%x", &rec.ImageFileHex);

        // 12) 블럭이미지（WORD）
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { ok = 0; break; }
        rec.BlockImage = static_cast<uint16_t>(std::atoi(tok));

        // 13) 중간이미지(리소스ID)（%x，可含 0x 前綴）
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsAlphaNumeric(tok)) { ok = 0; break; }
        std::sscanf(tok, "%x", &rec.MiddleImageHex);

        // 14) 레벨 제한（BYTE）
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) { ok = 0; break; }
        rec.LevelLimit = static_cast<uint8_t>(std::atoi(tok));

        // 下一列
        ++idx;
        if (!std::fgets(line, sizeof(line), fp)) { ok = 1; break; }
    }

    // 反編譯：失敗路徑只 fclose，不呼叫 Free()，保留半填 buffer
    g_clTextFileManager.fclose(fp);
    return ok;
}

void cltPortalInfo::Free()
{
    if (table_) {
        ::operator delete(table_);
        table_ = nullptr;
    }
    count_ = 0;
}

stPortalInfo* cltPortalInfo::GetPortalInfoByID(uint16_t a2)
{
    if (!table_ || count_ <= 0) return nullptr;
    // 以 20 WORD（=40 bytes）步距線性掃描
    for (int i = 0; i < count_; ++i) {
        if (table_[i].PortalID == a2) return &table_[i];
    }
    return nullptr;
}

int cltPortalInfo::GetPortalCntInMap(uint16_t a2, int* a3)
{
    // 回傳值為個數；同時把索引逐一寫到 a3[]
    int cnt = 0;
    if (table_ && count_ > 0) {
        for (int i = 0; i < count_; ++i) {
            if (table_[i].MapID_1 == a2) {
                if (a3) a3[cnt] = i; // 反編譯把索引寫入遞增指標
                ++cnt;
            }
        }
    }
    return cnt;
}