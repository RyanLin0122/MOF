#include "Logic/CSpiritSpeech.h"

#include <cstdlib>
#include <cstring>

#include "global.h"
#include "Logic/SpiritSpeechParser.h"
#include "System/cltLevelSystem.h"
#include "System/cltQuestSystem.h"
#include "System/cltLessonSystem.h"
#include "System/cltSkillSystem.h"
#include "System/cltPlayerAbility.h"
#include "System/cltPetSystem.h"
#include "Info/cltPetKindInfo.h"

CSpiritSpeech::CSpiritSpeech()
    : m_checkFuncs{}
    , m_wMapId(0)
    , m_wMapKind(0)
    , m_wMonsterKind(0)
    , m_bMonsterKillFlag(0)
    , m_bAllMonsterKillFlag(0)
    , m_nEquipFlag(0)
    , m_wLowExpLevel(0)
    , m_wLevelUpLevel(0)
    , m_bLevelUpFlag(0)
    , m_bAddSkillFlag(0)
    , m_bLowExpFlag(0)
    , m_wPetEatItemKind(0)
    , m_pLevelSystem(nullptr)
    , m_pQuestSystem(nullptr)
    , m_pLessonSystem(nullptr)
    , m_pSkillSystem(nullptr)
    , m_pPlayerAbility(nullptr)
{
    SetCheckFunc();
}

CSpiritSpeech::~CSpiritSpeech()
{
}

void CSpiritSpeech::Free()
{
    m_wMapId = 0;
    m_wMapKind = 0;
    m_wMonsterKind = 0;
    m_bMonsterKillFlag = 0;
    m_bAllMonsterKillFlag = 0;
    m_nEquipFlag = 0;
    m_wLowExpLevel = 0;
    m_wLevelUpLevel = 0;
    m_bLevelUpFlag = 0;
    m_bAddSkillFlag = 0;
    m_bLowExpFlag = 0;
    m_wPetEatItemKind = 0;
    m_pLevelSystem = nullptr;
    m_pQuestSystem = nullptr;
    m_pLessonSystem = nullptr;
    m_pSkillSystem = nullptr;
    m_pPlayerAbility = nullptr;
}

void CSpiritSpeech::SetCheckFunc()
{
    m_checkFuncs[0]  = nullptr;
    m_checkFuncs[1]  = &CSpiritSpeech::CheckLowLevel;
    m_checkFuncs[2]  = &CSpiritSpeech::CheckMapID;
    m_checkFuncs[3]  = &CSpiritSpeech::CheckQuestID;
    m_checkFuncs[4]  = &CSpiritSpeech::CheckLessonPoint;
    m_checkFuncs[5]  = &CSpiritSpeech::CheckHaveSkill;
    m_checkFuncs[6]  = &CSpiritSpeech::CheckHaveNotSkill;
    m_checkFuncs[7]  = &CSpiritSpeech::CheckMapKind;
    m_checkFuncs[8]  = &CSpiritSpeech::CheckMonsterKill;
    m_checkFuncs[9]  = &CSpiritSpeech::CheckUnEquip;
    m_checkFuncs[10] = &CSpiritSpeech::CheckStatUpdate;
    m_checkFuncs[11] = &CSpiritSpeech::CheckLowExp;
    m_checkFuncs[12] = &CSpiritSpeech::CheckLevelUp;
    m_checkFuncs[13] = &CSpiritSpeech::CheckAddSkill;
    m_checkFuncs[14] = &CSpiritSpeech::CheckQuestCount;
    m_checkFuncs[15] = &CSpiritSpeech::CheckHighLevel;
    m_checkFuncs[16] = &CSpiritSpeech::CheckAllMonsterKill;
    m_checkFuncs[17] = &CSpiritSpeech::CheckPetLowSatiety;
    m_checkFuncs[18] = &CSpiritSpeech::CheckPetFullSatiety;
    m_checkFuncs[19] = &CSpiritSpeech::CheckPetEatPenguine;
    m_checkFuncs[20] = &CSpiritSpeech::CheckPetEat;
    m_checkFuncs[21] = &CSpiritSpeech::CheckPetEatBone;
}

int CSpiritSpeech::CallCheckFunc(std::uint16_t condType, std::uint16_t value)
{
    if (condType == 0 || condType >= kMaxCheckFuncs)
        return 0;

    CheckFunc func = m_checkFuncs[condType];
    if (!func)
        return 0;

    return (this->*func)(value);
}

int CSpiritSpeech::SetSpiritSpeech(cltLevelSystem* levelSys, cltQuestSystem* questSys,
                                    cltLessonSystem* lessonSys, cltSkillSystem* skillSys,
                                    cltPlayerAbility* playerAbility)
{
    m_pLevelSystem   = levelSys;
    m_pQuestSystem   = questSys;
    m_pLessonSystem  = lessonSys;
    m_pPlayerAbility = playerAbility;
    m_pSkillSystem   = skillSys;
    return 1;
}

char* CSpiritSpeech::CheckSpiritSpeech()
{
    std::uint16_t spiritIds[2];
    spiritIds[0] = g_clPetSystem.GetPetKind();

    std::uint8_t level = g_clLevelSystem.GetLevel();
    if (level / 10 <= 1)
        spiritIds[1] = 1;
    else
        spiritIds[1] = level / 10;

    CSpiritSpeechParser localParser;
    g_clSpiritSpeechParser.GetSpiritSpeechInfo(localParser);

    for (int spiritIdx = 0; spiritIdx < 2; spiritIdx++)
    {
        std::uint16_t spiritId = spiritIds[spiritIdx];

        if (!g_clSpiritSpeechParser.IsExistSpiritID(spiritId))
            continue;

        const auto& speechMap = localParser.GetMap();
        auto it = speechMap.find(spiritId);
        if (it == speechMap.end())
            continue;

        const stSpeechInfo& info = it->second;
        std::uint16_t candidateCount = 0;
        std::uint16_t candidates[100];

        for (std::uint16_t scriptIdx = 0; scriptIdx < info.speechCount; scriptIdx++)
        {
            const stSpeechScript& script = info.scripts[scriptIdx];

            // Skip once-type scripts that have already been checked
            if (script.outputMode == 1 && script.onceChecked == 1)
                continue;

            // Check all conditions
            int allPassed = 1;
            int condResults[3] = {};

            if (script.conditionCount > 0)
            {
                for (int condIdx = 0; condIdx < script.conditionCount && condIdx < 3; condIdx++)
                {
                    const stSpeechCondition& cond = script.conditions[condIdx];
                    if (CallCheckFunc(cond.type, cond.value) == 1)
                        condResults[condIdx] = 1;
                }

                // Verify all conditions passed
                for (int condIdx = 0; condIdx < script.conditionCount && condIdx < 3; condIdx++)
                {
                    if (condResults[condIdx] == 0)
                        allPassed = 0;
                }
            }

            if (allPassed == 1)
            {
                int roll = (std::rand() % 100) + 1;
                if (script.probability >= static_cast<std::uint16_t>(roll))
                {
                    candidates[candidateCount++] = scriptIdx;
                }
            }
        }

        if (candidateCount > 0)
        {
            std::uint16_t selectedIdx = candidates[std::rand() % candidateCount];

            g_clSpiritSpeechParser.CheckOnceCondition(spiritId, selectedIdx);

            const stSpeechScript& selectedScript = info.scripts[selectedIdx];
            ReleaseSpiritSpeech(spiritId, selectedScript.scriptId);

            int textRoll = std::rand() % selectedScript.textCount;
            char* text = g_DCTTextManager.GetText(selectedScript.textIds[textRoll]);
            return text;
        }
    }

    return nullptr;
}

int CSpiritSpeech::ReleaseSpiritSpeech(std::uint16_t spiritId, std::uint16_t scriptId)
{
    CSpiritSpeechParser localParser;
    g_clSpiritSpeechParser.GetSpiritSpeechInfo(localParser);

    const auto& speechMap = localParser.GetMap();
    auto it = speechMap.find(spiritId);
    if (it == speechMap.end())
        return 0;

    const stSpeechInfo& info = it->second;

    // Find the script with matching scriptId
    int foundIdx = -1;
    for (int i = 0; i < info.speechCount; i++)
    {
        if (info.scripts[i].scriptId == scriptId)
        {
            foundIdx = i;
            break;
        }
    }

    if (foundIdx < 0)
        return 0;

    const stSpeechScript& script = info.scripts[foundIdx];

    // Release conditions based on their type
    for (int condIdx = 0; condIdx < script.conditionCount && condIdx < 3; condIdx++)
    {
        switch (script.conditions[condIdx].type)
        {
        case SSC_MONSTERKILL:
            ReleaseMonsterKill();
            break;
        case SSC_LOWEXP:
            ReleaseLowExpUp();
            break;
        case SSC_LEVELUP:
            ReleaseLevelUp();
            break;
        case SSC_ADDACTIVESKILL:
            ReleaseAddSkill();
            break;
        case SSC_ALLMONSTERKILL:
            ReleaseAllMonsterKill();
            break;
        case SSC_PETPENGUINEAT:
        case SSC_PETPENGUINETCEAT:
        case SSC_PETPENGUINBONEEAT:
            ReleasePetEat();
            break;
        default:
            break;
        }
    }

    return 1;
}

// --- Check functions ---

int CSpiritSpeech::CheckLowLevel(std::uint16_t value)
{
    return m_pLevelSystem->GetLevel() < value ? 1 : 0;
}

int CSpiritSpeech::CheckMapID(std::uint16_t value)
{
    return m_wMapId == value ? 1 : 0;
}

int CSpiritSpeech::CheckQuestID(std::uint16_t value)
{
    return m_pQuestSystem->GetRunningQuestInfoByQuestID(value) != nullptr ? 1 : 0;
}

int CSpiritSpeech::CheckLessonPoint(std::uint16_t value)
{
    return (m_pLessonSystem->GetBowLessonPt() >= value
         && m_pLessonSystem->GetMagicLessonPt() >= value
         && m_pLessonSystem->GetSwordLessonPt() >= value
         && m_pLessonSystem->GetTheologyLessonPt() >= value) ? 1 : 0;
}

int CSpiritSpeech::CheckHaveSkill(std::uint16_t value)
{
    return m_pSkillSystem->IsExistSkill(value) == 1 ? 1 : 0;
}

int CSpiritSpeech::CheckHaveNotSkill(std::uint16_t value)
{
    return m_pSkillSystem->IsExistSkill(value) == 0 ? 1 : 0;
}

int CSpiritSpeech::CheckMapKind(std::uint16_t value)
{
    return m_wMapKind == value ? 1 : 0;
}

int CSpiritSpeech::CheckMonsterKill(std::uint16_t value)
{
    if (m_wMonsterKind == value && m_bMonsterKillFlag == 1)
        return 1;
    return 0;
}

int CSpiritSpeech::CheckUnEquip(std::uint16_t value)
{
    return m_nEquipFlag != value ? 1 : 0;
}

int CSpiritSpeech::CheckStatUpdate(std::uint16_t value)
{
    return (value == 1 && m_pPlayerAbility->GetBonusPoint() > 1) ? 1 : 0;
}

int CSpiritSpeech::CheckLowExp(std::uint16_t value)
{
    if (m_bLowExpFlag != 1 || m_wLowExpLevel > value)
        return 0;
    return 1;
}

int CSpiritSpeech::CheckLevelUp(std::uint16_t value)
{
    if (m_wLevelUpLevel != value || m_bLevelUpFlag != 1)
        return 0;
    return 1;
}

int CSpiritSpeech::CheckAddSkill(std::uint16_t value)
{
    return m_bAddSkillFlag == 1 ? 1 : 0;
}

int CSpiritSpeech::CheckQuestCount(std::uint16_t value)
{
    return value >= m_pQuestSystem->GetPlayingQuestCount() ? 1 : 0;
}

int CSpiritSpeech::CheckHighLevel(std::uint16_t value)
{
    return value < m_pLevelSystem->GetLevel() ? 1 : 0;
}

int CSpiritSpeech::CheckAllMonsterKill(std::uint16_t value)
{
    return m_bAllMonsterKillFlag == 1 ? 1 : 0;
}

int CSpiritSpeech::CheckPetLowSatiety(std::uint16_t value)
{
    std::uint16_t petKind = g_clPetSystem.GetPetKind();
    strPetKindInfo* petInfo = g_clPetKindInfoBase.GetPetKindInfo(petKind);
    if (!petInfo)
        return 0;

    float maxSatiety = static_cast<float>(petInfo->wSaturation);
    double ratio = static_cast<double>(g_clPetSystem.GetPetSatiety()) / maxSatiety * 100.0;
    if (static_cast<double>(value) > ratio)
        return 1;
    return 0;
}

int CSpiritSpeech::CheckPetFullSatiety(std::uint16_t value)
{
    std::uint16_t petKind = g_clPetSystem.GetPetKind();
    strPetKindInfo* petInfo = g_clPetKindInfoBase.GetPetKindInfo(petKind);
    if (!petInfo)
        return 0;

    float maxSatiety = static_cast<float>(petInfo->wSaturation);
    double ratio = static_cast<double>(g_clPetSystem.GetPetSatiety()) / maxSatiety * 100.0;
    if (static_cast<double>(value) < ratio)
        return 1;
    return 0;
}

int CSpiritSpeech::CheckPetEat(std::uint16_t value)
{
    return m_wPetEatItemKind == value ? 1 : 0;
}

int CSpiritSpeech::CheckPetEatPenguine(std::uint16_t value)
{
    return m_wPetEatItemKind == value ? 1 : 0;
}

int CSpiritSpeech::CheckPetEatBone(std::uint16_t value)
{
    return m_wPetEatItemKind == value ? 1 : 0;
}

// --- Event handlers ---

void CSpiritSpeech::OnMapEntered(std::uint16_t mapId, std::uint16_t mapKind)
{
    m_wMapId = mapId;
    m_wMapKind = mapKind;
}

void CSpiritSpeech::OnMonsterKill(std::uint16_t monsterKind)
{
    m_bMonsterKillFlag = 1;
    m_wMonsterKind = monsterKind;
}

void CSpiritSpeech::OnMonsterKillAll()
{
    m_bAllMonsterKillFlag = 1;
}

void CSpiritSpeech::OnEquip(int equipFlag)
{
    m_nEquipFlag = equipFlag;
}

void CSpiritSpeech::OnLowExpUp(std::uint16_t level)
{
    m_bLowExpFlag = 1;
    m_wLowExpLevel = level;
}

void CSpiritSpeech::OnLevelUp()
{
    m_wLevelUpLevel = m_pLevelSystem->GetLevel();
    m_bLevelUpFlag = 1;
}

void CSpiritSpeech::OnAddActiveSkill()
{
    m_bAddSkillFlag = 1;
}

void CSpiritSpeech::OnPetEat(std::uint16_t itemKind)
{
    m_wPetEatItemKind = itemKind;
}

// --- Release functions ---

void CSpiritSpeech::ReleaseMonsterKill()
{
    m_wMonsterKind = 0;
    m_bMonsterKillFlag = 0;
}

void CSpiritSpeech::ReleaseLevelUp()
{
    m_wLevelUpLevel = 0;
    m_bLevelUpFlag = 0;
}

void CSpiritSpeech::ReleaseAddSkill()
{
    m_bAddSkillFlag = 0;
}

void CSpiritSpeech::ReleaseLowExpUp()
{
    m_wLowExpLevel = 0;
    m_bLowExpFlag = 0;
}

void CSpiritSpeech::ReleaseAllMonsterKill()
{
    m_bAllMonsterKillFlag = 0;
}

void CSpiritSpeech::ReleasePetEat()
{
    m_wPetEatItemKind = 0;
}
