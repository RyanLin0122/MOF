#include "Info/cltDropItemKindInfo.h"

#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

constexpr std::size_t kLineBufferSize = 1024;
constexpr std::size_t kRecordSize = sizeof(strDropItemKindInfo);

bool IsDigitString(const char* text) {
    if (!text) {
        return false;
    }

    while (*text && std::isspace(static_cast<unsigned char>(*text))) {
        ++text;
    }

    if (!*text) {
        return false;
    }

    const char* end = text + std::strlen(text);
    while (end > text && std::isspace(static_cast<unsigned char>(*(end - 1)))) {
        --end;
    }

    if (end == text) {
        return false;
    }

    for (const char* p = text; p < end; ++p) {
        if (!std::isdigit(static_cast<unsigned char>(*p))) {
            return false;
        }
    }
    return true;
}

} // namespace

cltItemKindInfo* cltDropItemKindInfo::m_pclItemKindInfo = nullptr;
cltCharKindInfo* cltDropItemKindInfo::m_pclCharKindInfo = nullptr;
cltCountrySystem* cltDropItemKindInfo::m_pclCountrySystem = nullptr;


void cltDropItemKindInfo::InitializeStaticVariable(cltItemKindInfo* itemKindInfo,
                                                   cltCharKindInfo* charKindInfo,
                                                   cltCountrySystem* countrySystem) {
    m_pclItemKindInfo = itemKindInfo;
    m_pclCharKindInfo = charKindInfo;
    m_pclCountrySystem = countrySystem;
}

cltDropItemKindInfo::cltDropItemKindInfo()
    : m_pRecords(nullptr),
      m_recordCount(0),
      padding06(0) {
}

int cltDropItemKindInfo::Initialize(char* fileName) {
    char delimiter[3] = "\t\n";
    char buffer[kLineBufferSize]{};
    std::fpos_t position{};
    int result = 0;

    std::FILE* stream = g_clTextFileManager.fopen(fileName);
    if (!stream) {
        return 0;
    }

    if (std::fgets(buffer, static_cast<int>(sizeof(buffer)), stream) &&
        std::fgets(buffer, static_cast<int>(sizeof(buffer)), stream) &&
        std::fgets(buffer, static_cast<int>(sizeof(buffer)), stream)) {

        std::fgetpos(stream, &position);
        for (; std::fgets(buffer, static_cast<int>(sizeof(buffer)), stream); ++m_recordCount) {
        }

        m_pRecords = static_cast<strDropItemKindInfo*>(::operator new(kRecordSize * m_recordCount));
        std::memset(m_pRecords, 0, kRecordSize * m_recordCount);
        std::fsetpos(stream, &position);

        auto* record = m_pRecords;
        auto* currentRecord = record;

        if (std::fgets(buffer, static_cast<int>(sizeof(buffer)), stream)) {
            while (true) {
                char* token = std::strtok(buffer, delimiter);
                if (!token) {
                    break;
                }

                record->kindCode = TranslateKindCode(token);
                if (!std::strtok(nullptr, delimiter)) {
                    break;
                }
                if (!std::strtok(nullptr, delimiter)) {
                    break;
                }

                token = std::strtok(nullptr, delimiter);
                if (!token) {
                    break;
                }

                record->rareDropProbability = std::atoi(token);

                int previousCumulative = 0;
                int rareGroupBaseOffset = 0;
                std::int32_t* rareItemCountPtr = &currentRecord->rareGroups[0].itemCount;

                while (true) {
                    token = std::strtok(nullptr, delimiter);
                    if (!token) {
                        goto parse_failed;
                    }

                    *(&currentRecord->rareGroups[0].cumulativeProbability + rareGroupBaseOffset / 4) =
                        previousCumulative + std::atoi(token);

                    int rareItemIndex = 0;
                    while (true) {
                        token = std::strtok(nullptr, delimiter);
                        if (!token) {
                            goto parse_failed;
                        }

                        const std::uint16_t itemKind = m_pclItemKindInfo->TranslateKindCode(token);
                        if (!std::strtok(nullptr, delimiter)) {
                            goto parse_failed;
                        }

                        if (itemKind) {
                            if (!m_pclItemKindInfo->IsValidItem(itemKind)) {
                                MessageBoxA(nullptr,
                                            m_pclItemKindInfo->TranslateKindCode(itemKind),
                                            "Item does not exist",
                                            0);
                                goto parse_failed;
                            }

                            currentRecord->rareGroups[rareGroupBaseOffset / 16].itemKind[*rareItemCountPtr] = itemKind;
                            ++(*rareItemCountPtr);
                        }

                        if (++rareItemIndex >= 3) {
                            previousCumulative = currentRecord->rareGroups[rareGroupBaseOffset / 16].cumulativeProbability;
                            rareGroupBaseOffset += 16;
                            rareItemCountPtr += 4;
                            if (rareGroupBaseOffset < 80) {
                                break;
                            }

                            if (!record->rareDropProbability || previousCumulative < 0x7FFF) {
                                int genericIndex = 0;
                                auto* generic = &currentRecord->genericGroups[0];

                                while (true) {
                                    char* probToken = std::strtok(nullptr, delimiter);
                                    if (!probToken) {
                                        goto parse_failed;
                                    }
                                    const int probability = std::atoi(probToken);

                                    char* itemToken = std::strtok(nullptr, delimiter);
                                    if (!itemToken) {
                                        goto parse_failed;
                                    }
                                    const std::uint16_t genericItemKind = m_pclItemKindInfo->TranslateKindCode(itemToken);

                                    if (!std::strtok(nullptr, delimiter)) {
                                        goto parse_failed;
                                    }

                                    char* minToken = std::strtok(nullptr, delimiter);
                                    if (!minToken) {
                                        goto parse_failed;
                                    }
                                    const int minCount = std::atoi(minToken);

                                    char* maxToken = std::strtok(nullptr, delimiter);
                                    if (!maxToken) {
                                        goto parse_failed;
                                    }
                                    const int maxCount = std::atoi(maxToken);

                                    if (genericItemKind) {
                                        if (!m_pclItemKindInfo->IsValidItem(genericItemKind)) {
                                            MessageBoxA(nullptr,
                                                        m_pclItemKindInfo->TranslateKindCode(genericItemKind),
                                                        "Item does not exist",
                                                        0);
                                            goto parse_failed;
                                        }
                                        if (minCount <= 0) {
                                            goto parse_failed;
                                        }

                                        generic->probability = probability;
                                        generic->itemKind = genericItemKind;
                                        generic->minCount = minCount;
                                        generic->maxCount = maxCount;
                                    }

                                    ++generic;
                                    if (++genericIndex >= 5) {
                                        char* moneyProbToken = std::strtok(nullptr, delimiter);
                                        if (!moneyProbToken || !IsDigitString(moneyProbToken)) {
                                            goto parse_failed;
                                        }
                                        record->moneyDropProbability = std::atoi(moneyProbToken);

                                        char* moneyValueToken = std::strtok(nullptr, delimiter);
                                        if (!moneyValueToken || !IsDigitString(moneyValueToken)) {
                                            goto parse_failed;
                                        }
                                        record->moneyDropAmount = std::atoi(moneyValueToken);

                                        std::array<stCharKindInfo*, 65535> relatedChars{};
                                        const int charCount = m_pclCharKindInfo->GetCharKindInfoByDropItemKind(
                                            record->kindCode,
                                            relatedChars.data());

                                        for (int i = 0; i < charCount; ++i) {
                                            // ground truth: 不檢查 null，直接使用
                                            const stCharKindInfo* charInfo = relatedChars[static_cast<std::size_t>(i)];

                                            if (charInfo->moneyRule == 0) {
                                                if (record->moneyDropAmount != 2 * charInfo->levelOrRankBase + 10) {
                                                    goto parse_failed;
                                                }
                                            } else if (charInfo->moneyRule == 1) {
                                                if (record->moneyDropAmount != 20 * charInfo->levelOrRankBase) {
                                                    goto parse_failed;
                                                }
                                            } else if (charInfo->moneyRule == 2) {
                                                if (record->moneyDropAmount != 15 * charInfo->levelOrRankBase) {
                                                    goto parse_failed;
                                                }
                                            }
                                        }

                                        currentRecord = reinterpret_cast<strDropItemKindInfo*>(
                                            reinterpret_cast<char*>(currentRecord) + sizeof(strDropItemKindInfo));

                                        if (std::fgets(buffer, static_cast<int>(sizeof(buffer)), stream)) {
                                            record = currentRecord;
                                            continue;
                                        }
                                        goto parse_complete;
                                    }
                                }
                            }
                            goto parse_failed;
                        }
                    }
                }
            }
        } else {
parse_complete:
            if (m_recordCount <= 1) {
                result = 1;
                g_clTextFileManager.fclose(stream);
                return result;
            }

            for (int i = 0; i < m_recordCount - 1; ++i) {
                for (int j = i + 1; j < m_recordCount; ++j) {
                    if (m_pRecords[i].kindCode == m_pRecords[j].kindCode) {
                        g_clTextFileManager.fclose(stream);
                        return 0;
                    }
                }
            }

            result = 1;
            g_clTextFileManager.fclose(stream);
            return result;
        }
    }

parse_failed:
    g_clTextFileManager.fclose(stream);
    return result;
}

void cltDropItemKindInfo::Free() {
    if (m_pRecords) {
        ::operator delete(m_pRecords);
        m_pRecords = nullptr;
    }
    m_recordCount = 0;
}

int cltDropItemKindInfo::GetDropItemKindNum() const {
    return m_recordCount;
}

strDropItemKindInfo* cltDropItemKindInfo::GetDropItemKindInfo(std::uint16_t kindCode) const {
    for (int i = 0; i < m_recordCount; ++i) {
        if (m_pRecords[i].kindCode == kindCode) {
            return &m_pRecords[i];
        }
    }
    return nullptr;
}

strDropItemKindInfo* cltDropItemKindInfo::GetDropItemKindInfoByIndex(int index) const {
    return reinterpret_cast<strDropItemKindInfo*>(
        reinterpret_cast<char*>(m_pRecords) + sizeof(strDropItemKindInfo) * index);
}

std::uint16_t cltDropItemKindInfo::TranslateKindCode(char* text) {
    if (!text || std::strlen(text) != 5) {
        return 0;
    }

    const int prefix = (std::toupper(static_cast<unsigned char>(text[0])) + 31) << 11;
    const unsigned value = static_cast<unsigned>(std::atoi(text + 1));
    if (value >= 0x800u) {
        return 0;
    }
    return static_cast<std::uint16_t>(prefix | value);
}

int cltDropItemKindInfo::GenearteDropMoney(int monsterRank,
                                           int probabilityScale,
                                           double rareMultiplier,
                                           int premiumBonusPerMille,
                                           int partyBonusPercent,
                                           int moneyBonusPercent,
                                           int eventMoneyBonusFlag,
                                           std::uint16_t dropKindCode) const {
    strDropItemKindInfo* info = GetDropItemKindInfo(dropKindCode);
    if (!info || !info->moneyDropProbability || !info->moneyDropAmount) {
        return 0;
    }

    int probability = 0;
    if (rareMultiplier == 0.0) {
        probability = probabilityScale * info->moneyDropProbability / 100;
    } else {
        probability = static_cast<int>(probabilityScale * (static_cast<double>(info->moneyDropProbability) * rareMultiplier)) / 100;
    }

    if (probability <= 1) {
        probability = 1;
    }
    if (premiumBonusPerMille) {
        probability += premiumBonusPerMille * probability / 1000;
    }
    if (eventMoneyBonusFlag) {
        probability += 20 * probability / 100;
    }
    if (std::rand() % 10000 >= probability) {
        return 0;
    }

    int amount = 1;
    if (monsterRank < 10) {
        if (monsterRank < 7) {
            if (monsterRank < 5) {
                amount = info->moneyDropAmount;
            } else {
                amount = 50 * info->moneyDropAmount / 100;
            }
        } else {
            amount = 30 * info->moneyDropAmount / 100;
        }
    }

    if (partyBonusPercent) {
        amount += partyBonusPercent * amount / 100;
    }
    if (moneyBonusPercent) {
        amount += moneyBonusPercent * amount / 100;
    }
    return amount;
}

int cltDropItemKindInfo::GenerateRareDropItem(int playerLevel,
                                              int probabilityScale,
                                              double rareMultiplier,
                                              int premiumBonusPerMille,
                                              int eventBonusFlag,
                                              std::uint16_t dropKindCode,
                                              std::uint16_t* outItemKind,
                                              std::uint16_t* outItemCount) const {
    (void)playerLevel;

    strDropItemKindInfo* info = GetDropItemKindInfo(dropKindCode);
    if (!info || info->rareDropProbability <= 0 || info->rareGroups[4].itemCount == 0) {
        return 0;
    }

    int probability = 0;
    if (rareMultiplier == 0.0) {
        probability = probabilityScale * info->rareDropProbability / 100;
    } else {
        probability = static_cast<int>(probabilityScale * (static_cast<double>(info->rareDropProbability) * rareMultiplier)) / 100;
    }

    if (premiumBonusPerMille) {
        const std::uint32_t bonus =
            static_cast<std::uint32_t>((static_cast<std::uint64_t>(274877907ULL) * premiumBonusPerMille * probability) >> 32);
        probability += static_cast<int>((bonus >> 6) + static_cast<std::uint32_t>(static_cast<int>(bonus >> 6) >> 31));
    }
    if (eventBonusFlag) {
        probability += 20 * probability / 100;
    }
    if (probability <= 1) {
        probability = 1;
    }
    if (std::rand() % 10000 >= probability) {
        return 0;
    }

    const int roll = std::rand() % info->rareGroups[4].itemCount;
    int groupIndex = 0;
    while (groupIndex < 5 && roll >= info->rareGroups[groupIndex].cumulativeProbability) {
        ++groupIndex;
    }
    if (groupIndex >= 5 || info->rareGroups[groupIndex].itemCount <= 0) {
        return 1;
    }

    const std::uint16_t itemKind =
        info->rareGroups[groupIndex].itemKind[std::rand() % info->rareGroups[groupIndex].itemCount];

    if (m_pclItemKindInfo->IsQuizItem(itemKind) &&
        !m_pclCountrySystem->IsKorea() &&
        m_pclItemKindInfo->IsQuizItemConsonant(itemKind) != 2) {
        return 0;
    }

    *outItemKind = itemKind;
    *outItemCount = 1;
    return 1;
}

int cltDropItemKindInfo::GenerateFieldDropItem(stMapInfo* mapInfo,
                                               int playerLevel,
                                               int probabilityScale,
                                               double rareMultiplier,
                                               int premiumBonusPerMille,
                                               int eventBonusFlag,
                                               std::uint16_t,
                                               std::uint16_t* outItemKind,
                                               std::uint16_t* outItemCount) const {

    if (!mapInfo) {
        return 0;
    }

    if (mapInfo->m_dwDropRate1) {
        int probability = 0;
        if (rareMultiplier == 0.0) {
            probability = probabilityScale * mapInfo->m_dwDropRate1 / 100;
        } else {
            probability = static_cast<int>(probabilityScale * (static_cast<double>(mapInfo->m_dwDropRate1) * rareMultiplier)) / 100;
        }
        if (probability <= 1) {
            probability = 1;
        }
        if (premiumBonusPerMille) {
            probability += premiumBonusPerMille * probability / 1000;
        }
        if (eventBonusFlag) {
            probability += 20 * probability / 100;
        }
        if (std::rand() % 10000 < probability) {
            switch (std::rand() % mapInfo->m_byteDropItemCount1) {
                case 0: *outItemKind = mapInfo->m_wDropItemID1[0]; break;
                case 1: *outItemKind = mapInfo->m_wDropItemID1[1]; break;
                case 2: *outItemKind = mapInfo->m_wDropItemID1[2]; break;
                case 3: *outItemKind = mapInfo->m_wDropItemID1[3]; break;
                case 4: *outItemKind = mapInfo->m_wDropItemID1[4]; break;
                default: break;
            }

            if (!m_pclItemKindInfo->IsQuizItem(*outItemKind) ||
                (playerLevel <= 5 &&
                 (m_pclCountrySystem->IsKorea() ||
                  m_pclItemKindInfo->IsQuizItemConsonant(*outItemKind) == 2))) {
                const std::uint16_t minCount = mapInfo->m_wDropItemMin1;
                *outItemCount = static_cast<std::uint16_t>(minCount + std::rand() % (mapInfo->m_wDropItemMax1 - minCount + 1));
                return 1;
            }
            return 0;
        }
    }

    if (!mapInfo->m_dwDropRate2) {
        return 0;
    }

    int probability = 0;
    if (rareMultiplier == 0.0) {
        probability = probabilityScale * mapInfo->m_dwDropRate2 / 100;
    } else {
        probability = static_cast<int>(probabilityScale * (static_cast<double>(mapInfo->m_dwDropRate2) * rareMultiplier)) / 100;
    }
    if (probability <= 1) {
        probability = 1;
    }
    if (eventBonusFlag) {
        probability += 20 * probability / 100;
    }
    if (std::rand() % 10000 >= probability) {
        return 0;
    }

    switch (std::rand() % mapInfo->m_byteDropItemCount2) {
        case 0: *outItemKind = mapInfo->m_wDropItemID2[0]; break;
        case 1: *outItemKind = mapInfo->m_wDropItemID2[1]; break;
        case 2: *outItemKind = mapInfo->m_wDropItemID2[2]; break;
        case 3: *outItemKind = mapInfo->m_wDropItemID2[3]; break;
        case 4: *outItemKind = mapInfo->m_wDropItemID2[4]; break;
        default: break;
    }

    if (!m_pclItemKindInfo->IsQuizItem(*outItemKind) ||
        (playerLevel <= 5 &&
         (m_pclCountrySystem->IsKorea() ||
          m_pclItemKindInfo->IsQuizItemConsonant(*outItemKind) == 2))) {
        const std::uint16_t minCount = mapInfo->m_wDropItemMin2;
        *outItemCount = static_cast<std::uint16_t>(minCount + std::rand() % (mapInfo->m_wDropItemMax2 - minCount + 1));
        return 1;
    }

    return 0;
}

int cltDropItemKindInfo::GenerateGenericDropItem(int playerLevel,
                                                 int probabilityScale,
                                                 double rareMultiplier,
                                                 int premiumBonusPerMille,
                                                 int eventBonusFlag,
                                                 std::uint16_t dropKindCode,
                                                 std::uint16_t* outItemKind,
                                                 std::uint16_t* outItemCount) const {
    int generatedCount = 0;
    strDropItemKindInfo* info = GetDropItemKindInfo(dropKindCode);
    if (!info) {
        return 0;
    }

    for (int i = 0; i < 5; ++i) {
        const strDropItemGenericGroup& group = info->genericGroups[i];
        if (!group.probability ||
            (m_pclItemKindInfo->IsQuizItem(group.itemKind) &&
             !m_pclCountrySystem->IsKorea() &&
             m_pclItemKindInfo->IsQuizItemConsonant(group.itemKind) != 2) ||
            (m_pclItemKindInfo->IsCoinItem(group.itemKind) &&
             playerLevel > 5)) {
            continue;
        }

        int probability = 0;
        if (rareMultiplier == 0.0) {
            probability = probabilityScale * group.probability / 100;
        } else {
            probability = static_cast<int>(probabilityScale * (static_cast<double>(group.probability) * rareMultiplier)) / 100;
        }

        if (probability <= 1) {
            probability = 1;
        }
        if (premiumBonusPerMille) {
            probability += premiumBonusPerMille * probability / 1000;
        }
        if (eventBonusFlag) {
            probability += 20 * probability / 100;
        }
        if (std::rand() % 10000 >= probability) {
            continue;
        }

        outItemKind[generatedCount] = group.itemKind;
        outItemCount[generatedCount] = static_cast<std::uint16_t>(
            group.minCount + std::rand() % (group.maxCount - group.minCount + 1));
        ++generatedCount;
    }

    return generatedCount;
}

int cltDropItemKindInfo::GenerateEventDropItem(int playerLevel,
                                               int probabilityScale,
                                               double rareMultiplier,
                                               int premiumBonusPerMille,
                                               int eventBonusFlag,
                                               int eventDropKindCount,
                                               std::uint16_t* eventDropKindList,
                                               std::uint16_t* outItemKind,
                                               std::uint16_t* outItemCount) const {
    if (playerLevel > 5 || eventDropKindCount <= 0) {
        return 0;
    }

    for (int i = 0; i < eventDropKindCount; ++i) {
        const int count = GenerateGenericDropItem(playerLevel,
                                                  probabilityScale,
                                                  rareMultiplier,
                                                  premiumBonusPerMille,
                                                  eventBonusFlag,
                                                  eventDropKindList[i],
                                                  outItemKind,
                                                  outItemCount);
        if (count) {
            return count;
        }
    }
    return 0;
}

int cltDropItemKindInfo::GenerateDropItem(stMapInfo* mapInfo,
                                          int playerLevel,
                                          int probabilityScale,
                                          double rareMultiplier,
                                          int premiumBonusPerMille,
                                          int moneyBonusPercent,
                                          int fieldMoneyBonusPercent,
                                          int eventMoneyBonusFlag,
                                          int eventDropKindCount,
                                          std::uint16_t* eventDropKindList,
                                          std::uint16_t dropKindCode,
                                          std::uint16_t* outItemKind,
                                          std::uint16_t* outItemCount,
                                          int* outMoney) const {
    int money = GenearteDropMoney(playerLevel,
                                  probabilityScale,
                                  rareMultiplier,
                                  premiumBonusPerMille,
                                  moneyBonusPercent,
                                  fieldMoneyBonusPercent,
                                  eventMoneyBonusFlag,
                                  dropKindCode);
    *outMoney = money;
    // ground truth: 不檢查 mapInfo 是否為 null，直接解參考
    if (mapInfo->m_byIsPKArea) {
        *outMoney = 2 * money;
    }

    int count = GenerateRareDropItem(playerLevel,
                                     probabilityScale,
                                     rareMultiplier,
                                     premiumBonusPerMille,
                                     eventMoneyBonusFlag,
                                     dropKindCode,
                                     outItemKind,
                                     outItemCount);
    if (count) {
        return count;
    }

    count = GenerateFieldDropItem(mapInfo,
                                  playerLevel,
                                  probabilityScale,
                                  rareMultiplier,
                                  premiumBonusPerMille,
                                  eventMoneyBonusFlag,
                                  dropKindCode,
                                  outItemKind,
                                  outItemCount);
    if (count) {
        return count;
    }

    count = GenerateGenericDropItem(playerLevel,
                                    probabilityScale,
                                    rareMultiplier,
                                    premiumBonusPerMille,
                                    eventMoneyBonusFlag,
                                    dropKindCode,
                                    outItemKind,
                                    outItemCount);
    if (eventDropKindCount) {
        count += GenerateEventDropItem(playerLevel,
                                       probabilityScale,
                                       rareMultiplier,
                                       premiumBonusPerMille,
                                       eventMoneyBonusFlag,
                                       eventDropKindCount,
                                       eventDropKindList,
                                       outItemKind + count,
                                       outItemCount + count);
    }
    return count;
}

int cltDropItemKindInfo::GenerateDropItem(std::uint16_t dropKindCode,
                                          std::uint16_t* outItemKind,
                                          std::uint16_t* outItemCount,
                                          int* outMoney) const {
    *outMoney = GenearteDropMoney(0, 100, 0.0, 0, 0, 0, 0, dropKindCode);

    int count = GenerateRareDropItem(0, 100, 0.0, 0, 0, dropKindCode, outItemKind, outItemCount);
    if (!count) {
        count = GenerateGenericDropItem(0, 100, 0.0, 0, 0, dropKindCode, outItemKind, outItemCount);
    }
    return count;
}
