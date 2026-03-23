#ifndef SPIRIT_SPEECH_PARSER_H
#define SPIRIT_SPEECH_PARSER_H

#include <cstdint>
#include <cstdio>
#include <map>
#include "global.h"

enum ESpiritSpeechConditionType : std::uint16_t
{
    SSC_UNKNOWN                 = 0,
    SSC_LOWLEVEL                = 1,
    SSC_MAPID                   = 2,
    SSC_QUESTID                 = 3,
    SSC_LESSONPOINT             = 4,
    SSC_HAVESKILL               = 5,
    SSC_HAVENOTSKILL            = 6,
    SSC_MAPKIND                 = 7,
    SSC_MONSTERKILL             = 8,
    SSC_UNEQUIP                 = 9,
    SSC_STATUPDATE              = 10,
    SSC_LOWEXP                  = 11,
    SSC_LEVELUP                 = 12,
    SSC_ADDACTIVESKILL          = 13,
    SSC_QUESTCOUNT              = 14,
    SSC_HIGHLEVEL               = 15,
    SSC_ALLMONSTERKILL          = 16,
    SSC_PETPENGUINLOWSATIETY    = 17,
    SSC_PETPENGUINFULLSATIETY   = 18,
    SSC_PETPENGUINEAT           = 19,
    SSC_PETPENGUINETCEAT        = 20,
    SSC_PETPENGUINBONEEAT       = 21,
};

struct stSpeechCondition
{
    std::uint16_t type;
    std::uint16_t value;

    stSpeechCondition();
};

struct stSpeechScript
{
    std::uint16_t scriptId;
    std::uint16_t conditionCount;
    std::int32_t outputMode;
    std::int32_t onceChecked;
    stSpeechCondition conditions[3];
    std::uint16_t probability;
    std::uint16_t textCount;
    std::uint16_t textIds[5];

    stSpeechScript();
};

struct stSpeechInfo
{
    std::uint16_t speechCount;
    stSpeechScript scripts[100];

    stSpeechInfo();
};

class CSpiritSpeechParser
{
public:
    CSpiritSpeechParser();
    ~CSpiritSpeechParser();

    // 依照 IDA：成功或失敗都回傳 1。
    int Initialize(char* filePath);

    // 深拷貝目前內容到輸出物件。
    void GetSpiritSpeechInfo(CSpiritSpeechParser& out) const;

    void SetSpiritSpeechCondition(std::uint16_t spiritId,
                                  std::uint16_t speechIndex,
                                  std::uint16_t conditionIndex,
                                  char* conditionToken,
                                  char* valueToken);

    std::uint16_t GetMapKindInfo(char* token) const;
    void AddSpiritID(std::uint16_t spiritId);
    bool IsExistSpiritID(std::uint16_t spiritId) const;
    void CheckOnceCondition(std::uint16_t spiritId, std::uint16_t speechIndex);

    std::uint16_t TranslateKindCode(const char* token) const;
    std::uint16_t TranslateKindCodeBySkillID(const char* token) const;

    const std::map<std::uint16_t, stSpeechInfo>& GetMap() const;

private:
    std::uint8_t m_flag0;
    std::uint8_t m_flag1;
    std::uint8_t m_padding[6];
    std::uint8_t m_forceInsertPath;
    std::map<std::uint16_t, stSpeechInfo> m_speechMap;
};

#endif // SPIRIT_SPEECH_PARSER_H
