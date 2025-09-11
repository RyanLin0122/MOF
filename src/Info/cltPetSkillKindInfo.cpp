#include "Info/cltPetSkillKindInfo.h"

bool cltPetSkillKindInfo::IsDigitString(const char* s)
{
    if (!s || !*s) return false;
    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p; ++p)
        if (!std::isdigit(*p)) return false;
    return true;
}

int cltPetSkillKindInfo::ToHexU32(const char* s, uint32_t& out)
{
    unsigned int tmp = 0;
    if (!s) return 0;
    if (std::sscanf(s, "%x", &tmp) != 1) return 0;
    out = static_cast<uint32_t>(tmp);
    return 1;
}

uint16_t cltPetSkillKindInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;
    if (std::strlen(a1) != 6) return 0;
    int c = std::toupper(static_cast<unsigned char>(a1[0]));
    uint16_t hi = (c == 'A') ? 0x8000u : (c == 'P' ? 0x0000u : 0xFFFFu);
    if (hi == 0xFFFFu) return 0;
    uint16_t lo = static_cast<uint16_t>(std::atoi(a1 + 1));
    return (lo < 0x8000u) ? (hi | lo) : 0;
}

uint16_t cltPetSkillKindInfo::TranslateEffectKindCode(char* a2)
{
    if (!a2) return 0;
    if (std::strlen(a2) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(a2[0])) + 31) << 11;
    int lo = std::atoi(a2 + 1);
    if (lo < 0x800) return static_cast<uint16_t>(hi | lo);
    return 0;
}

int cltPetSkillKindInfo::Initialize(char* filename)
{
    Free();

    const char* Delim = "\t\n";
    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    int ok = 0;                 // 成功旗標（對齊反編譯 v53）
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
    while (std::fgets(line, sizeof(line), fp)) ++count_;

    if (count_ <= 0) { g_clTextFileManager.fclose(fp); return 0; }

    table_ = static_cast<strPetSkillKindInfo*>(std::malloc(sizeof(strPetSkillKindInfo) * count_));
    if (!table_) { g_clTextFileManager.fclose(fp); return 0; }
    std::memset(table_, 0, sizeof(strPetSkillKindInfo) * count_);

    // 回到資料起點
    std::fsetpos(fp, &pos);

    int idx = 0;
    while (idx < count_ && std::fgets(line, sizeof(line), fp)) {
        strPetSkillKindInfo& rec = table_[idx];
        std::memset(&rec, 0, sizeof(rec));

        // 1) 스킬 ID
        char* tok = std::strtok(line, Delim);
        if (!tok) goto FAIL;
        rec.SkillID = TranslateKindCode(tok);
        if (!rec.SkillID) goto FAIL;

        // 2) 스킬 이름(기획자용) → 僅跳過
        if (!std::strtok(nullptr, Delim)) goto FAIL;

        // 3) 스킬명 text ID
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.NameTextID = static_cast<uint16_t>(std::atoi(tok));

        // 4) 스킬설명 text ID
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.DescTextID = static_cast<uint16_t>(std::atoi(tok));

        // 5) 아이콘 파일명(HEX)
        tok = std::strtok(nullptr, Delim);
        if (!ToHexU32(tok, rec.IconResHex)) goto FAIL;

        // 6) 블록명
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.BlockNameID = static_cast<uint16_t>(std::atoi(tok));

        // 7) 필요레벨
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.RequiredLevel = static_cast<uint16_t>(std::atoi(tok));

        // 8) 스킬공격력 상승
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.SkillAtkAdd = static_cast<uint32_t>(std::atoi(tok));

        // 9~13) 攻擊/防禦/命中/HP自回/MP自回(皆整數)
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.AtkPermille = static_cast<uint32_t>(std::atoi(tok));

        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.DefPermille = static_cast<uint32_t>(std::atoi(tok));

        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.AccPermille = static_cast<uint32_t>(std::atoi(tok));

        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.HpAutoPermille = static_cast<uint32_t>(std::atoi(tok));

        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.MpAutoPermille = static_cast<uint32_t>(std::atoi(tok));

        // 14) 아이템 드랍율 상승
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.DropRatePermille = static_cast<uint32_t>(std::atoi(tok));

        // 15~18) STR / VIT / DEX / INT
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.STR = static_cast<uint32_t>(std::atoi(tok));

        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.VIT = static_cast<uint32_t>(std::atoi(tok));  // 反編譯：先寫 +60

        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.DEX = static_cast<uint32_t>(std::atoi(tok));  // 反編譯：後寫 +56

        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.INT = static_cast<uint32_t>(std::atoi(tok));

        // 19) 공격 속도
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.AttackSpeed = static_cast<uint32_t>(std::atoi(tok));

        // 20) 줍기（WORD）
        tok = std::strtok(nullptr, Delim);
        if (!tok || !IsDigitString(tok)) goto FAIL;
        rec.Pickup = static_cast<uint16_t>(std::atoi(tok));

        // 21) 시전자-바닥（效果KindCode 5碼）
        tok = std::strtok(nullptr, Delim);
        if (!tok) goto FAIL;
        rec.EffectCasterGround = TranslateEffectKindCode(tok);

        // 22) 시전자-위（效果KindCode 5碼）
        tok = std::strtok(nullptr, Delim);
        if (!tok) goto FAIL;
        rec.EffectCasterUp = TranslateEffectKindCode(tok);

        ++idx;
        continue;

    FAIL:
        std::fclose(fp);
        Free();
        return 0;
    }

    ok = 1;       // 成功
    std::fclose(fp);
    count_ = idx; // 實際成功筆數
    return ok;
}

void cltPetSkillKindInfo::Free()
{
    if (table_) { std::free(table_); table_ = nullptr; }
    count_ = 0;
}

strPetSkillKindInfo* cltPetSkillKindInfo::GetPetSkillKindInfo(uint16_t a2)
{
    if (!table_ || count_ <= 0) return nullptr;
    // 反編譯：以 WORD 指針逐筆掃描，步距 36 WORD（=72 bytes）
    for (int i = 0; i < count_; ++i) {
        if (table_[i].SkillID == a2) return &table_[i];
    }
    return nullptr;
}
