#pragma once

#include <cstdint>

struct cltCharKindInfo;

struct stMonsterKind {
    std::uint16_t kind;
    std::uint16_t count;
};

class CMonsterGroupPerLevel {
public:
    static void InitializeStaticVariable(cltCharKindInfo* charKindInfo);

    CMonsterGroupPerLevel();
    ~CMonsterGroupPerLevel();

    int GetQuestMonsters(int nationKind, int level, stMonsterKind* outMonsters);
    std::uint16_t GetMonsterKillCount(std::uint16_t mapKind, std::uint16_t monsterKind);
    int VerifyingMonsterKinds(int minLv, int maxLv,
                              std::uint16_t needClass, std::uint16_t needNation,
                              std::uint16_t monsterKind, stMonsterKind* outMonsterInfo);

private:
    static cltCharKindInfo* m_pclCharKindInfo;
};
