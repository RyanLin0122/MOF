#include "Logic/SpiritSpeechParser.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

namespace
{
constexpr std::size_t kMaxScripts = 100;
constexpr std::size_t kMaxConditions = 3;
constexpr std::size_t kMaxTexts = 5;
}

stSpeechCondition::stSpeechCondition()
    : type(0), value(0)
{
}

stSpeechScript::stSpeechScript()
    : scriptId(0), outputMode(0), onceChecked(0), conditionCount(0),
      conditions{}, probability(0), textCount(0), textIds{}
{
}

stSpeechInfo::stSpeechInfo()
    : speechCount(0), scripts{}
{
}

CSpiritSpeechParser::CSpiritSpeechParser()
    : m_flag0(0), m_flag1(0), m_padding{0, 0, 0, 0, 0, 0}, m_forceInsertPath(0), m_speechMap()
{
}

CSpiritSpeechParser::~CSpiritSpeechParser()
{
    m_speechMap.clear();
}

int CSpiritSpeechParser::Initialize(char* filePath)
{
    char buffer[1024] = {};
    char delimiter[] = "\t\n";

    std::FILE* stream = g_clTextFileManager.fopen(filePath);
    if (stream == nullptr)
    {
        return 1;
    }

    // IDA 版本固定先讀四行，然後直接從第 4 行內容開始解析。
    if (std::fgets(buffer, 1023, stream) != nullptr &&
        std::fgets(buffer, 1023, stream) != nullptr &&
        std::fgets(buffer, 1023, stream) != nullptr &&
        std::fgets(buffer, 1023, stream) != nullptr)
    {
        stSpeechScript overflowScript;

parse_line:
        char* token = std::strtok(buffer, delimiter);
        if (token != nullptr)
        {
            const std::uint16_t spiritId = TranslateKindCode(token);
            AddSpiritID(spiritId);

            char* stringToken = std::strtok(nullptr, delimiter);
            if (stringToken != nullptr)
            {
                stSpeechInfo& info = m_speechMap[spiritId];
                const std::uint16_t speechIndex = info.speechCount;
                stSpeechScript* script = (speechIndex < kMaxScripts) ? &info.scripts[speechIndex] : &overflowScript;
                *script = stSpeechScript();

                script->scriptId = TranslateKindCode(stringToken);

                stringToken = std::strtok(nullptr, delimiter);
                if (stringToken != nullptr)
                {
                    script->outputMode = std::atoi(stringToken);
                    script->onceChecked = 0;

                    stringToken = std::strtok(nullptr, delimiter);
                    if (stringToken != nullptr)
                    {
                        script->conditionCount = static_cast<std::uint16_t>(std::atoi(stringToken));
                        std::uint32_t conditionIndex = 0;

                        while (true)
                        {
                            char* conditionToken = std::strtok(nullptr, delimiter);
                            if (conditionToken == nullptr)
                            {
                                break;
                            }

                            char conditionName[32] = {};
                            std::strncpy(conditionName, conditionToken, sizeof(conditionName) - 1);

                            char* valueToken = std::strtok(nullptr, delimiter);
                            if (valueToken == nullptr)
                            {
                                break;
                            }

                            char valueText[32] = {};
                            std::strncpy(valueText, valueToken, sizeof(valueText) - 1);

                            SetSpiritSpeechCondition(spiritId,
                                                     speechIndex,
                                                     static_cast<std::uint16_t>(conditionIndex),
                                                     conditionName,
                                                     valueText);
                            ++conditionIndex;

                            if (conditionIndex >= kMaxConditions)
                            {
                                stringToken = std::strtok(nullptr, delimiter);
                                if (stringToken != nullptr)
                                {
                                    script->probability = static_cast<std::uint16_t>(std::atoi(stringToken));

                                    stringToken = std::strtok(nullptr, delimiter);
                                    if (stringToken != nullptr)
                                    {
                                        script->textCount = static_cast<std::uint16_t>(std::atoi(stringToken));

                                        std::uint32_t textIndex = 0;
                                        while (true)
                                        {
                                            stringToken = std::strtok(nullptr, delimiter);
                                            if (stringToken == nullptr)
                                            {
                                                break;
                                            }

                                            if (textIndex < kMaxTexts)
                                            {
                                                script->textIds[textIndex] = static_cast<std::uint16_t>(std::atoi(stringToken));
                                            }

                                            const bool hasMore = (++textIndex < kMaxTexts);
                                            if (!hasMore)
                                            {
                                                ++info.speechCount;
                                                if (std::fgets(buffer, 1023, stream) != nullptr)
                                                {
                                                    goto parse_line;
                                                }
                                                goto finish;
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

finish:
    g_clTextFileManager.fclose(stream);
    return 1;
}

void CSpiritSpeechParser::GetSpiritSpeechInfo(CSpiritSpeechParser& out) const
{
    out.m_flag0 = m_flag0;
    out.m_flag1 = m_flag1;
    std::memcpy(out.m_padding, m_padding, sizeof(m_padding));
    out.m_forceInsertPath = m_forceInsertPath;
    out.m_speechMap = m_speechMap;
}

void CSpiritSpeechParser::SetSpiritSpeechCondition(std::uint16_t spiritId,
                                                   std::uint16_t speechIndex,
                                                   std::uint16_t conditionIndex,
                                                   char* conditionToken,
                                                   char* valueToken)
{
    stSpeechInfo& info = m_speechMap[spiritId];
    if (speechIndex >= kMaxScripts || conditionIndex >= kMaxConditions)
    {
        return;
    }

    stSpeechCondition& cond = info.scripts[speechIndex].conditions[conditionIndex];

    if (strcasecmp(conditionToken, "LOWLEVEL") == 0)
    {
        cond.type = SSC_LOWLEVEL;
        cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
        return;
    }
    if (strcasecmp(conditionToken, "MAPID") == 0)
    {
        cond.type = SSC_MAPID;
        cond.value = TranslateKindCode(valueToken);
        return;
    }
    if (strcasecmp(conditionToken, "QUESTID") == 0)
    {
        cond.type = SSC_QUESTID;
        cond.value = TranslateKindCode(valueToken);
        return;
    }
    if (strcasecmp(conditionToken, "LESSONPOINT") == 0)
    {
        cond.type = SSC_LESSONPOINT;
        cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
        return;
    }
    if (strcasecmp(conditionToken, "HAVESKILL") == 0)
    {
        cond.type = SSC_HAVESKILL;
        cond.value = TranslateKindCodeBySkillID(valueToken);
        return;
    }
    if (strcasecmp(conditionToken, "HAVENOTSKILL") == 0)
    {
        cond.type = SSC_HAVENOTSKILL;
        cond.value = TranslateKindCodeBySkillID(valueToken);
        return;
    }
    if (strcasecmp(conditionToken, "MAPKIND") == 0)
    {
        cond.type = SSC_MAPKIND;
        cond.value = GetMapKindInfo(valueToken);
        return;
    }
    if (strcasecmp(conditionToken, "MONSTERKILL") == 0)
    {
        cond.type = SSC_MONSTERKILL;
        cond.value = TranslateKindCode(valueToken);
        return;
    }
    if (strcasecmp(conditionToken, "UNEQUIP") == 0)
    {
        cond.type = SSC_UNEQUIP;
        cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
        return;
    }
    if (strcasecmp(conditionToken, "STATUPDATE") == 0)
    {
        cond.type = SSC_STATUPDATE;
        cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
        return;
    }
    if (strcasecmp(conditionToken, "LOWEXP") == 0)
    {
        cond.type = SSC_LOWEXP;
        cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
        return;
    }
    if (strcasecmp(conditionToken, "LEVELUP") == 0)
    {
        cond.type = SSC_LEVELUP;
        cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
        return;
    }
    if (strcasecmp(conditionToken, "ADDACTIVESKILL") == 0)
    {
        cond.type = SSC_ADDACTIVESKILL;
        cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
        return;
    }
    if (strcasecmp(conditionToken, "QUESTCOUNT") == 0)
    {
        cond.type = SSC_QUESTCOUNT;
        cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
        return;
    }
    if (strcasecmp(conditionToken, "HIGHLEVEL") == 0)
    {
        cond.type = SSC_HIGHLEVEL;
        cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
        return;
    }
    if (strcasecmp(conditionToken, "ALLMONSTERKILL") == 0)
    {
        cond.type = SSC_ALLMONSTERKILL;
        cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
        return;
    }
    if (strcasecmp(conditionToken, "PETPENGUINLOWSATIETY") == 0)
    {
        cond.type = SSC_PETPENGUINLOWSATIETY;
        cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
        return;
    }
    if (strcasecmp(conditionToken, "PETPENGUINFULLSATIETY") == 0)
    {
        cond.type = SSC_PETPENGUINFULLSATIETY;
        cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
        return;
    }
    if (strcasecmp(conditionToken, "PETPENGUINEAT") == 0)
    {
        cond.type = SSC_PETPENGUINEAT;
        cond.value = TranslateKindCode(valueToken);
        return;
    }
    if (strcasecmp(conditionToken, "PETPENGUINETCEAT") == 0)
    {
        cond.type = SSC_PETPENGUINETCEAT;
        cond.value = TranslateKindCode(valueToken);
        return;
    }
    if (strcasecmp(conditionToken, "PETPENGUINBONEEAT") == 0)
    {
        cond.type = SSC_PETPENGUINBONEEAT;
        cond.value = TranslateKindCode(valueToken);
        return;
    }

    // IDA 最後分支寫入 type=0；未命名條件視為一般數值條件。
    cond.type = SSC_UNKNOWN;
    cond.value = static_cast<std::uint16_t>(std::atoi(valueToken));
}

std::uint16_t CSpiritSpeechParser::GetMapKindInfo(char* token) const
{
    if (strcasecmp(token, "FIELD") == 0)
    {
        return 1;
    }
    if (strcasecmp(token, "VILLAGE") == 0)
    {
        return 2;
    }
    if (strcasecmp(token, "DUNGEON") == 0)
    {
        return 3;
    }
    return 0;
}

void CSpiritSpeechParser::AddSpiritID(std::uint16_t spiritId)
{
    if (!IsExistSpiritID(spiritId))
    {
        m_speechMap.insert(std::make_pair(spiritId, stSpeechInfo()));
    }
}

bool CSpiritSpeechParser::IsExistSpiritID(std::uint16_t spiritId) const
{
    return m_speechMap.find(spiritId) != m_speechMap.end();
}

void CSpiritSpeechParser::CheckOnceCondition(std::uint16_t spiritId, std::uint16_t speechIndex)
{
    AddSpiritID(spiritId);
    if (speechIndex < kMaxScripts)
    {
        m_speechMap[spiritId].scripts[speechIndex].onceChecked = 1;
    }
}

std::uint16_t CSpiritSpeechParser::TranslateKindCode(const char* token) const
{
    if (token == nullptr || std::strlen(token) != 5)
    {
        return 0;
    }

    const int head = (std::toupper(static_cast<unsigned char>(token[0])) + 31) << 11;
    const int tail = std::atoi(token + 1);
    if (tail >= 0x800)
    {
        return 0;
    }

    return static_cast<std::uint16_t>(head | tail);
}

std::uint16_t CSpiritSpeechParser::TranslateKindCodeBySkillID(const char* token) const
{
    if (token == nullptr || std::strlen(token) != 6)
    {
        return 0;
    }

    std::uint16_t prefix = 0;
    const int head = std::toupper(static_cast<unsigned char>(token[0]));
    if (head == 'A')
    {
        prefix = 0x8000;
    }
    else if (head != 'P')
    {
        return 0;
    }

    const int tail = std::atoi(token + 1);
    if (tail >= 0x8000)
    {
        return 0;
    }

    return static_cast<std::uint16_t>(prefix | tail);
}

const std::map<std::uint16_t, stSpeechInfo>& CSpiritSpeechParser::GetMap() const
{
    return m_speechMap;
}
