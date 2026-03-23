#include "Logic/SpiritGossipParser.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

stGossipInfo::stGossipInfo()
    : count(0), textIds{}
{
}

CSpiritGossipParser::CSpiritGossipParser()
    : m_flag0(0), m_flag1(0), m_padding{0, 0, 0, 0, 0, 0}, m_forceInsertPath(0), m_gossipMap()
{
}

CSpiritGossipParser::~CSpiritGossipParser()
{
    m_gossipMap.clear();
}

int CSpiritGossipParser::Initialize(char* filePath)
{
    char buffer[1024] = {};
    char delimiter[] = "\t\n";

    std::FILE* stream = g_clTextFileManager.fopen(filePath);
    if (stream == nullptr)
    {
        return 1;
    }

    // IDA 版本先連讀四次 fgets，實際第一筆會從「第 4 行內容」開始解析。
    if (std::fgets(buffer, 1023, stream) != nullptr &&
        std::fgets(buffer, 1023, stream) != nullptr &&
        std::fgets(buffer, 1023, stream) != nullptr &&
        std::fgets(buffer, 1023, stream) != nullptr)
    {
        do
        {
            char* kindToken = std::strtok(buffer, delimiter);
            if (kindToken == nullptr)
            {
                break;
            }

            const std::uint16_t spiritId = TranslateKindCode(kindToken);
            AddSpiritID(spiritId);

            char* textToken = std::strtok(nullptr, delimiter);
            if (textToken == nullptr)
            {
                break;
            }

            auto it = m_gossipMap.insert(std::make_pair(spiritId, stGossipInfo())).first;
            stGossipInfo& info = it->second;

            const std::uint16_t textId = static_cast<std::uint16_t>(std::atoi(textToken));
            const std::uint16_t index = info.count;

            // 原始碼未做越界保護，這裡保留同樣容量假設，但避免寫爆陣列。
            if (index < 100)
            {
                info.textIds[index] = textId;
            }
            ++info.count;
        }
        while (std::fgets(buffer, 1023, stream) != nullptr);
    }

    g_clTextFileManager.fclose(stream);
    return 1;
}

void CSpiritGossipParser::GetSpiritGossipInfo(CSpiritGossipParser& out) const
{
    out.m_flag0 = m_flag0;
    out.m_flag1 = m_flag1;
    std::memcpy(out.m_padding, m_padding, sizeof(m_padding));
    out.m_forceInsertPath = m_forceInsertPath;
    out.m_gossipMap = m_gossipMap;
}

void CSpiritGossipParser::AddSpiritID(std::uint16_t spiritId)
{
    if (IsExistSpiritID(spiritId))
    {
        return;
    }

    // 原始實作走 std::map 內部插入路徑；這裡保留結果一致的邏輯。
    m_gossipMap.insert(std::make_pair(spiritId, stGossipInfo()));
}

bool CSpiritGossipParser::IsExistSpiritID(std::uint16_t spiritId) const
{
    return m_gossipMap.find(spiritId) != m_gossipMap.end();
}

std::uint16_t CSpiritGossipParser::TranslateKindCode(const char* token) const
{
    if (token == nullptr)
    {
        return 0;
    }

    if (std::strlen(token) != 5)
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

const std::map<std::uint16_t, stGossipInfo>& CSpiritGossipParser::GetMap() const
{
    return m_gossipMap;
}
