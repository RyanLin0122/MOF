#include "Logic/CMonsterGroupPerLevel.h"
#include "Info/cltCharKindInfo.h"

cltCharKindInfo* CMonsterGroupPerLevel::m_pclCharKindInfo = nullptr;

void CMonsterGroupPerLevel::InitializeStaticVariable(cltCharKindInfo* charKindInfo)
{
    m_pclCharKindInfo = charKindInfo;
}

CMonsterGroupPerLevel::CMonsterGroupPerLevel() {}
CMonsterGroupPerLevel::~CMonsterGroupPerLevel() {}

// stub：回傳固定目標擊殺數 0，待資料表建立後替換。
std::uint16_t CMonsterGroupPerLevel::GetMonsterKillCount(std::uint16_t /*mapKind*/,
                                                          std::uint16_t /*monsterKind*/)
{
    return 0;
}

// stub：無法驗證，固定回傳 0（驗證失敗）。
int CMonsterGroupPerLevel::VerifyingMonsterKinds(int /*minLv*/, int /*maxLv*/,
                                                  std::uint16_t /*needClass*/,
                                                  std::uint16_t /*needNation*/,
                                                  std::uint16_t /*monsterKind*/,
                                                  stMonsterKind* /*outMonsterInfo*/)
{
    return 0;
}

// stub：無怪物資料，固定回傳 0 個。
int CMonsterGroupPerLevel::GetQuestMonsters(int /*nationKind*/, int /*level*/,
                                             stMonsterKind* /*outMonsters*/)
{
    return 0;
}
