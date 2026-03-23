#include "Logic/CMonsterGroupPerLevel.h"
#include "Info/cltCharKindInfo.h"
#include <cstdint>
#include <cstdlib>

cltCharKindInfo* CMonsterGroupPerLevel::m_pclCharKindInfo = nullptr;

void CMonsterGroupPerLevel::InitializeStaticVariable(cltCharKindInfo* charKindInfo)
{
    m_pclCharKindInfo = charKindInfo;
}

CMonsterGroupPerLevel::CMonsterGroupPerLevel() {}
CMonsterGroupPerLevel::~CMonsterGroupPerLevel() {}

// stub：回傳固定目標擊殺數 0，待資料表建立後替換。
int CMonsterGroupPerLevel::GetQuestMonsters(int nationKind, int level, stMonsterKind* outMonsters)
{
    uint16_t monsterKinds[65536];

    int result = m_pclCharKindInfo->GetMonsterCharKinds(nationKind, level + 1, level + 7, 1, monsterKinds);
    if (result > 0)
    {
        uint16_t* pKind = monsterKinds;
        int count = result;
        do
        {
            uint16_t kind = *pKind;
            outMonsters->kind = kind;
            outMonsters->count = GetMonsterKillCount(level, kind);
            ++pKind;
            outMonsters = reinterpret_cast<stMonsterKind*>(reinterpret_cast<char*>(outMonsters) + 4);
            --count;
        } while (count);
    }
    return result;
}

std::uint16_t CMonsterGroupPerLevel::GetMonsterKillCount(std::uint16_t level, std::uint16_t monsterKind)
{
    void* charInfo = m_pclCharKindInfo->GetCharKindInfo(monsterKind);
    if (!charInfo)
        return 0;

    int diff = *((unsigned char*)charInfo + 146) - level;
    if (diff <= 1)
        return 190;

    switch (diff)
    {
    case 2: return 160;
    case 3: return 130;
    case 4: return 100;
    case 5: return 70;
    }

    // diff == 6 → 50, diff > 6 → 30
    int v6 = -(diff != 6);
    v6 = v6 & 0xEC;
    return static_cast<std::uint16_t>(v6 + 50);
}

// stub：無怪物資料，固定回傳 0 個。
int CMonsterGroupPerLevel::VerifyingMonsterKinds(int a2, int a3,
    std::uint16_t a4, std::uint16_t a5,
    std::uint16_t a6, stMonsterKind* a7)
{
    if (!a4 || !a5 || !a6)
        return 0;

    if (a4 == a5 || a5 == a6 || a4 == a6)
        return 0;

    if (a3 < 0 || a3 >= 120)
        return 0;

    for (int i = 0; i < 3; ++i)
    {
        // Assign monster kind to output slot
        if (i == 0)
            a7->kind = a4;
        else if (i == 1)
            reinterpret_cast<uint16_t*>(a7)[2] = a5;
        else
            reinterpret_cast<uint16_t*>(a7)[4] = a6;

        // Read back the kind we just stored
        uint16_t currentKind = reinterpret_cast<uint16_t*>(a7)[2 * i];

        void* info = m_pclCharKindInfo->GetCharKindInfo(currentKind);
        if (!info)
            return 0;

        // Check that kind matches real char ID: word[0] == word[3]
        uint16_t* infoWords = reinterpret_cast<uint16_t*>(info);
        if (infoWords[0] != infoWords[3])
            return 0;

        // Check nation/class flag at byte offset 220
        char flag = *((char*)info + 220);
        if (a2 == 1)
        {
            if (flag != 1)
                return 0;
        }
        else
        {
            if (flag != 2)
                return 0;
        }

        // Check level at byte offset 146 is within [a3+1, a3+7]
        int monsterLevel = *((unsigned char*)info + 146);
        if (monsterLevel < a3 + 1 || monsterLevel > a3 + 7)
            return 0;

        // Check special flag at DWORD offset 53 (byte 212) is 0
        if (reinterpret_cast<uint32_t*>(info)[53])
            return 0;

        // Set kill count
        reinterpret_cast<uint16_t*>(a7)[2 * i + 1] = GetMonsterKillCount(
            a3, reinterpret_cast<uint16_t*>(a7)[2 * i]);
    }

    return 1;
}
