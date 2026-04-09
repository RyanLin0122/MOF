#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cstddef>
#include <cstdlib>

#include "global.h"
#include "Info/cltItemKindInfo.h"

// ------------------------------------------------------------
// 總大小必須為 72 bytes
// ------------------------------------------------------------
struct strClassKindInfo
{
    std::uint16_t wClassId;                 // Offset 0,  Size 2

    // 第二欄：職業名稱
    // 依 offset 推回，只能位於 2~7，共 6 bytes
    char          szClassName[6];           // Offset 2,  Size 6

    // 每筆記錄的 one-hot attribute bit
    std::uint64_t qwClassAtb;               // Offset 8,  Size 8

    std::uint16_t wClassNameCode;           // Offset 16, Size 2
    std::uint8_t  bTransferStage;           // Offset 18, Size 1
    std::uint8_t  padding_1[1];             // Offset 19, Size 1

    std::uint16_t wTransferableClasses;     // Offset 20, Size 2
    std::uint8_t  bMinLevel;                // Offset 22, Size 1
    std::uint8_t  padding_2[1];             // Offset 23, Size 1

    std::uint16_t wMinAttack;               // Offset 24, Size 2
    std::uint16_t wMinAgility;              // Offset 26, Size 2
    std::uint16_t wMinHealth;               // Offset 28, Size 2
    std::uint16_t wMinIntelligence;         // Offset 30, Size 2
    std::uint16_t wSwordsmanshipSkill;      // Offset 32, Size 2
    std::uint16_t wMagicSkill;              // Offset 34, Size 2
    std::uint16_t wArcherySkill;            // Offset 36, Size 2
    std::uint16_t wPriestSkill;             // Offset 38, Size 2
    std::uint16_t wRogueSkill;              // Offset 40, Size 2

    std::uint8_t  padding_3[2];             // Offset 42, Size 2

    std::uint32_t dwTargetSearchRange;      // Offset 44, Size 4
    std::uint16_t wCommentCode;             // Offset 48, Size 2
    std::uint16_t wMarkImageBlockId;        // Offset 50, Size 2
    std::uint16_t wItem1CodeOnTransfer;     // Offset 52, Size 2
    std::uint16_t wItem1QuantityOnTransfer; // Offset 54, Size 2
    std::uint16_t wItem2CodeOnTransfer;     // Offset 56, Size 2
    std::uint16_t wItem2QuantityOnTransfer; // Offset 58, Size 2
    std::uint32_t dwBaseBuffUsageCount;     // Offset 60, Size 4
    std::uint32_t dwHPMapRecoveryIncrease;  // Offset 64, Size 4
    std::uint32_t dwManaMapRecoveryIncrease;// Offset 68, Size 4
};

static_assert(sizeof(strClassKindInfo) == 72, "strClassKindInfo size must be 72 bytes");

// ------------------------------------------------------------
// cltClassKindInfo
// ------------------------------------------------------------
class cltClassKindInfo
{
public:
    cltClassKindInfo();
    ~cltClassKindInfo();

    int Initialize(char* fileName);
    void Free();

    static std::uint16_t TranslateKindCode(char* text);

    std::uint16_t GetTotalClassNum();
    strClassKindInfo* GetClassKindInfo(std::uint16_t classId);
    strClassKindInfo* GetClassKindInfoByIndex(unsigned int index);
    strClassKindInfo* GetClassKindInfoByAtb(std::uint64_t atb);

    int GetClassKindsByAtb(std::uint64_t atb, std::uint16_t* outClassIds);

    // 只回傳低 32-bit
    int GetClassAtb(char* text);

private:
    strClassKindInfo* m_pInfo;
    std::uint16_t     m_wTotalClassNum;
};
