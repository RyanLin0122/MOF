#ifndef SPIRIT_GOSSIP_PARSER_H
#define SPIRIT_GOSSIP_PARSER_H

#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include "global.h"

struct stGossipInfo
{
    std::uint16_t count;
    std::uint16_t textIds[100];

    stGossipInfo();
};

class CSpiritGossipParser
{
public:
    CSpiritGossipParser();
    ~CSpiritGossipParser();

    // 依照 IDA 邏輯：成功或失敗都回傳 1。
    int Initialize(char* filePath);

    // 深拷貝目前內容到輸出物件。
    void GetSpiritGossipInfo(CSpiritGossipParser& out) const;

    void AddSpiritID(std::uint16_t spiritId);
    bool IsExistSpiritID(std::uint16_t spiritId) const;
    std::uint16_t TranslateKindCode(const char* token) const;

    const std::map<std::uint16_t, stGossipInfo>& GetMap() const;

private:
    std::uint8_t m_flag0;
    std::uint8_t m_flag1;
    std::uint8_t m_padding[6];
    std::uint8_t m_forceInsertPath;
    std::map<std::uint16_t, stGossipInfo> m_gossipMap;
};

#endif // SPIRIT_GOSSIP_PARSER_H
