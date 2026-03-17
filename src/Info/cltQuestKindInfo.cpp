#include "Info/cltQuestKindInfo.h"

#include <algorithm>
#include <cstdlib>
#include <new>
#include "global.h"

namespace {

    constexpr const char* kFigRoot = "FIG"; // 反編譯輸出缺字串；依命名與同類函式推回根職業碼。
    constexpr const char* kArcRoot = "ARC";
    constexpr const char* kMagRoot = "MAG";
    constexpr const char* kClaRoot = "CLA";

    // 注意：輸入的 IDA 片段有多個 `string` 遺失，以下以 nullptr 保留未知槽位，
    // 已知代碼照原樣還原；未知代碼可在拿到更完整字串後直接補上。
    constexpr const char* kFigFamily[8] = { kFigRoot, nullptr, "KNI", "SWO", "GEN", "HER", nullptr, nullptr };
    constexpr const char* kArcFamily[8] = { kArcRoot, nullptr, "SNI", "ASS", nullptr, "SPE", nullptr, "LLC" };
    constexpr const char* kMagFamily[8] = { kMagRoot, nullptr, "MOL", "BAT", "SAG", nullptr, "SPM", "BTM" };
    constexpr const char* kClaFamily[8] = { kClaRoot, "PRI", nullptr, "PAL", nullptr, "BIS", nullptr, nullptr };

    bool MatchesFamily(std::uint16_t classCode, const char* const (&family)[8]) {
        for (const char* code : family) {
            if (code != nullptr && classCode == cltQuestKindInfo::TranslateClassCode(code)) {
                return true;
            }
        }
        return false;
    }

} // namespace

std::uint16_t stQuestKindInfo::IsFig(std::uint16_t classCode) const {
    return MatchesFamily(classCode, kFigFamily) ? cltQuestKindInfo::TranslateClassCode(kFigRoot) : 0;
}

std::uint16_t stQuestKindInfo::IsARC(std::uint16_t classCode) const {
    return MatchesFamily(classCode, kArcFamily) ? cltQuestKindInfo::TranslateClassCode(kArcRoot) : 0;
}

std::uint16_t stQuestKindInfo::IsMAG(std::uint16_t classCode) const {
    return MatchesFamily(classCode, kMagFamily) ? cltQuestKindInfo::TranslateClassCode(kMagRoot) : 0;
}

std::uint16_t stQuestKindInfo::IsCLA(std::uint16_t classCode) const {
    return MatchesFamily(classCode, kClaFamily) ? cltQuestKindInfo::TranslateClassCode(kClaRoot) : 0;
}

std::uint16_t stQuestKindInfo::GetRewardItem(std::uint16_t classCode, char groupIndex) const {
    if (IsFig(classCode)) {
        return groupIndex == 1 ? wReward1ItemId_Group1 : wReward1ItemId_Group2;
    }
    if (IsARC(classCode)) {
        return groupIndex == 1 ? wReward2ItemId_Group1 : wReward2ItemId_Group2;
    }
    if (IsMAG(classCode)) {
        return groupIndex == 1 ? wReward3ItemId_Group1 : wReward3ItemId_Group2;
    }
    if (IsCLA(classCode)) {
        return groupIndex == 1 ? wReward4ItemId_Group1 : wReward4ItemId_Group2;
    }
    return 0;
}

bool stQuestKindInfo::CanAcceptQuestByClass(std::uint16_t classCode) const {
    if (IsFig(classCode) == wAvailableClasses) {
        return true;
    }
    if (IsARC(classCode) == wAvailableClasses) {
        return true;
    }
    if (IsMAG(classCode) == wAvailableClasses) {
        return true;
    }
    return IsCLA(classCode) == wAvailableClasses;
}

cltQuestKindInfo::cltQuestKindInfo() {
    m_infos.fill(nullptr);
}

cltQuestKindInfo::~cltQuestKindInfo() {
    for (stQuestKindInfo*& info : m_infos) {
        delete info;
        info = nullptr;
    }
}

bool cltQuestKindInfo::IsDigitString(const char* s) {
    if (s == nullptr || *s == '\0') {
        return false;
    }

    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p != '\0'; ++p) {
        if (!std::isdigit(*p)) {
            return false;
        }
    }
    return true;
}

bool cltQuestKindInfo::IsAlphabetString(const char* s) {
    if (s == nullptr || *s == '\0') {
        return false;
    }

    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(s); *p != '\0'; ++p) {
        if (!std::isalpha(*p)) {
            return false;
        }
    }
    return true;
}

std::uint8_t cltQuestKindInfo::ParsePlayType(const char* token) {
    if (std::strcmp(token, "COLLECT") == 0) return 1;
    if (std::strcmp(token, "DELIVERY") == 0) return 3;
    if (std::strcmp(token, "HUNT") == 0) return 2;
    if (std::strcmp(token, "LESSONSETTING") == 0) return 4;
    if (std::strcmp(token, "LESSON") == 0) return 5;
    if (std::strcmp(token, "ONEWAYDELIVERY") == 0) return 6;
    if (std::strcmp(token, "BUYEMBLEM") == 0) return 7;
    if (std::strcmp(token, "PLAYINDUN") == 0) return 8;
    if (std::strcmp(token, "BUYNPCITEM") == 0) return 9;
    if (std::strcmp(token, "STORAGEBOXUSE") == 0) return 10;
    if (std::strcmp(token, "ADDFRIEND") == 0) return 11;
    if (std::strcmp(token, "WHISPER") == 0) return 12;
    if (std::strcmp(token, "ADDSENIOR") == 0) return 13;
    if (std::strcmp(token, "SENDPOSTIT") == 0) return 14;
    if (std::strcmp(token, "EXAMINATION") == 0) return 15;
    if (std::strcmp(token, "SPECIALTYLEARN") == 0) return 16;
    if (std::strcmp(token, "WYVERNTAKE") == 0) return 17;
    if (std::strcmp(token, "BOATTAKE") == 0) return 18;
    if (std::strcmp(token, "PARTY") == 0) return 19;
    if (std::strcmp(token, "BUYSAILSAGENCY") == 0) return 20;
    if (std::strcmp(token, "SAILSAILSAGENCY") == 0) return 21;
    if (std::strcmp(token, "OPENPRIVATESHOP") == 0) return 22;
    if (std::strcmp(token, "OPENCASHSHOP") == 0) return 23;
    if (std::strcmp(token, "MERITORIOUS") == 0) return 24;
    if (std::strcmp(token, "MATERIALSUPPLY") == 0) return 25;
    if (std::strcmp(token, "ADDJUNIOR") == 0) return 26;
    if (std::strcmp(token, "ENCHANTITEM") == 0) return 27;
    if (std::strcmp(token, "HUNTMATCH") == 0) return 28;
    if (std::strcmp(token, "PVP") == 0) return 29;
    if (std::strcmp(token, "JOINCICLE") == 0) return 30;
    if (std::strcmp(token, "ADDEMBLEMTOQSL") == 0) return 31;
    return 0;
}

std::uint8_t cltQuestKindInfo::ParseDifficulty(const char* token) {
    if (std::strcmp(token, "EASY") == 0) return 1;
    if (std::strcmp(token, "NORMAL") == 0) return 2;
    if (std::strcmp(token, "DEFFICULT") == 0) return 3; // 原檔拼字即為 DEFFICULT。
    return 0;
}

bool cltQuestKindInfo::ValidateRewardGroup(const stQuestKindInfo& info, bool firstGroup) {
    const std::uint16_t a = firstGroup ? info.wReward1ItemId_Group1 : info.wReward1ItemId_Group2;
    const std::uint16_t b = firstGroup ? info.wReward2ItemId_Group1 : info.wReward2ItemId_Group2;
    const std::uint16_t c = firstGroup ? info.wReward3ItemId_Group1 : info.wReward3ItemId_Group2;
    const std::uint16_t d = firstGroup ? info.wReward4ItemId_Group1 : info.wReward4ItemId_Group2;
    const std::uint8_t count = firstGroup ? info.bGroup1ItemCount : info.bGroup2ItemCount;

    if (static_cast<std::uint32_t>(a) + b + c + d != 0) {
        return count != 0;
    }
    return true;
}

int cltQuestKindInfo::LoadQuestInfo(char* filePath) {
    char buffer[1024]{};
    char delimiter[] = "\t\n";
    int result = 0;

    FILE* stream = g_clTextFileManager.fopen(filePath);
    if (stream == nullptr) {
        return 0;
    }

    if (std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream)) {
        if (std::fgets(buffer, 1023, stream)) {
            while (true) {
                char* token = std::strtok(buffer, delimiter); // 1 任務_ID
                if (token == nullptr) {
                    break;
                }

                const std::uint16_t questCode = TranslateKindCode(token);
                stQuestKindInfo*& slot = m_infos[questCode];
                if (slot != nullptr) {
                    break;
                }

                slot = new stQuestKindInfo{};
                std::memset(slot, 0, sizeof(*slot));
                stQuestKindInfo* info = slot;
                info->wQuestId = questCode;

                if (std::strtok(nullptr, delimiter) == nullptr) { // 2 任務標題：只驗證存在，不落表。
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 3 任務適用與否
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->bQuestApplicable = static_cast<std::uint8_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 4 任務名稱代碼
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->wQuestNameCode = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 5 說明腳本
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->wDescriptionScript = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 6 開始對話
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->wStartDialogue = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 7 中間對話1
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->wMidDialogue1 = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 8 中間對話2
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->wMidDialogue2 = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 9 中間對話3
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->wMidDialogue3 = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 10 結束對話
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->wEndDialogue = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 11 遊玩類型
                if (token == nullptr) {
                    break;
                }
                info->bPlayType = ParsePlayType(token);

                token = std::strtok(nullptr, delimiter); // 12 難度
                if (token == nullptr) {
                    break;
                }
                info->bDifficulty = ParseDifficulty(token);

                token = std::strtok(nullptr, delimiter); // 13 條件Lv
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->bConditionLevel = static_cast<std::uint8_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 14 條件名聲
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->bConditionReputation = static_cast<std::uint8_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 15 可執行職業
                if (token == nullptr || !IsAlphabetString(token)) {
                    break;
                }
                info->wAvailableClasses = (std::strcmp(token, "ALL") == 0) ? 0 : TranslateClassCode(token);

                token = std::strtok(nullptr, delimiter); // 16 可執行職業文本ID
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->wAvailableClassesTextId = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 17 限制時間
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->dwTimeLimit = static_cast<std::uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 18 獎勵利比
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->dwRewardLib = static_cast<std::uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 19 劍術課程點數
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->dwRewardClassPoints_Sword = static_cast<std::uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 20 弓術課程點數
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->dwRewardClassPoints_Bow = static_cast<std::uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 21 魔法課程點數
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->dwRewardClassPoints_Magic = static_cast<std::uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 22 神學課程點數
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->dwRewardClassPoints_Theology = static_cast<std::uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 23 專業點數
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->wRewardMajorPoints = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 24 功績點數
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->wRewardMeritPoints = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 25 階級點數
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->dwRewardRankPoints = static_cast<std::uint32_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 26 獎勵1-1
                if (token == nullptr) {
                    break;
                }
                info->wReward1ItemId_Group1 = TranslateKindCode(token);
                if (info->wReward1ItemId_Group1 != 0 && g_clItemKindInfo.GetItemKindInfo(info->wReward1ItemId_Group1) == nullptr) {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 27 獎勵1-2
                if (token == nullptr) {
                    break;
                }
                info->wReward2ItemId_Group1 = TranslateKindCode(token);
                if (info->wReward2ItemId_Group1 != 0 && g_clItemKindInfo.GetItemKindInfo(info->wReward2ItemId_Group1) == nullptr) {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 28 獎勵1-3
                if (token == nullptr) {
                    break;
                }
                info->wReward3ItemId_Group1 = TranslateKindCode(token);
                if (info->wReward3ItemId_Group1 != 0 && g_clItemKindInfo.GetItemKindInfo(info->wReward3ItemId_Group1) == nullptr) {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 29 獎勵1-4
                if (token == nullptr) {
                    break;
                }
                info->wReward4ItemId_Group1 = TranslateKindCode(token);
                if (info->wReward4ItemId_Group1 != 0 && g_clItemKindInfo.GetItemKindInfo(info->wReward4ItemId_Group1) == nullptr) {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 30 群組1數量
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->bGroup1ItemCount = static_cast<std::uint8_t>(std::atoi(token));
                if (!ValidateRewardGroup(*info, true)) {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 31 獎勵2-1
                if (token == nullptr) {
                    break;
                }
                info->wReward1ItemId_Group2 = TranslateKindCode(token);
                if (info->wReward1ItemId_Group2 != 0 && g_clItemKindInfo.GetItemKindInfo(info->wReward1ItemId_Group2) == nullptr) {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 32 獎勵2-2
                if (token == nullptr) {
                    break;
                }
                info->wReward2ItemId_Group2 = TranslateKindCode(token);
                if (info->wReward2ItemId_Group2 != 0 && g_clItemKindInfo.GetItemKindInfo(info->wReward2ItemId_Group2) == nullptr) {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 33 獎勵2-3
                if (token == nullptr) {
                    break;
                }
                info->wReward3ItemId_Group2 = TranslateKindCode(token);
                if (info->wReward3ItemId_Group2 != 0 && g_clItemKindInfo.GetItemKindInfo(info->wReward3ItemId_Group2) == nullptr) {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 34 獎勵2-4
                if (token == nullptr) {
                    break;
                }
                info->wReward4ItemId_Group2 = TranslateKindCode(token);
                if (info->wReward4ItemId_Group2 != 0 && g_clItemKindInfo.GetItemKindInfo(info->wReward4ItemId_Group2) == nullptr) {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 35 群組2數量
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->bGroup2ItemCount = static_cast<std::uint8_t>(std::atoi(token));
                if (!ValidateRewardGroup(*info, false)) {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 36 經驗值
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->dwRewardExperience = static_cast<std::uint32_t>(std::atoi(token));
                if (info->dwRewardExperience != 0) {
                    const std::uint8_t level = info->bConditionLevel;
                    if (level > 0x32u && !IsValidRewardExpByLevel(level, info->dwRewardExperience)) {
                        break;
                    }
                }

                token = std::strtok(nullptr, delimiter); // 37 名聲
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->wRewardReputation = static_cast<std::uint16_t>(std::atoi(token));

                token = std::strtok(nullptr, delimiter); // 38 前置任務
                if (token == nullptr) {
                    break;
                }
                info->wPrerequisiteQuest = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 39 前置 NPC
                if (token == nullptr) {
                    break;
                }
                info->wPrerequisiteQuestNpc = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 40 永久放棄
                if (token == nullptr) {
                    break;
                }
                info->dwPermanentAbandon = 0;
                if (std::toupper(static_cast<unsigned char>(*token)) == 'Y') {
                    info->dwPermanentAbandon = 1;
                }

                if (!std::fgets(buffer, 1023, stream)) {
                    result = 1;
                    goto EXIT_LOAD_QUEST_INFO;
                }
            }
        }
        else {
            result = 1;
        }
    }

EXIT_LOAD_QUEST_INFO:
    g_clTextFileManager.fclose(stream);
    return result;
}

int cltQuestKindInfo::LoadCollectionQuestInfo(char* filePath) {
    char buffer[1024]{};
    char delimiter[] = "\t\n";
    int result = 0;

    FILE* stream = g_clTextFileManager.fopen(filePath);
    if (stream == nullptr) {
        return 0;
    }

    if (std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream)) {
        if (std::fgets(buffer, 1023, stream)) {
            while (true) {
                char* token = std::strtok(buffer, delimiter);
                if (token == nullptr) {
                    break;
                }

                const std::uint16_t questCode = TranslateKindCode(token);
                stQuestKindInfo* info = GetQuestKindInfo(questCode);
                if (info == nullptr) {
                    break;
                }

                token = std::strtok(nullptr, delimiter);
                if (token == nullptr) {
                    break;
                }
                info->extra.collection.wQuestItem1 = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter);
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->extra.collection.wQuestItemCount1 = static_cast<std::uint16_t>(std::atoi(token));
                if (info->extra.collection.wQuestItem1 != 0 && info->extra.collection.wQuestItemCount1 == 0) {
                    break;
                }

                token = std::strtok(nullptr, delimiter);
                if (token == nullptr) {
                    break;
                }
                info->extra.collection.wQuestItem1MonsterId = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter);
                if (token == nullptr) {
                    break;
                }
                info->extra.collection.wNpcId1 = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter);
                if (token == nullptr) {
                    break;
                }
                info->extra.collection.wQuestItem2 = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter);
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->extra.collection.wQuestItemCount2 = static_cast<std::uint16_t>(std::atoi(token));
                if (info->extra.collection.wQuestItem2 != 0 && info->extra.collection.wQuestItemCount2 == 0) {
                    break;
                }

                token = std::strtok(nullptr, delimiter);
                if (token == nullptr) {
                    break;
                }
                info->extra.collection.wQuestItem2MonsterId = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter);
                if (token == nullptr) {
                    break;
                }
                info->extra.collection.wNpcId2 = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter);
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->extra.collection.wMainHabitat = static_cast<std::uint16_t>(std::atoi(token));

                if (!std::fgets(buffer, 1023, stream)) {
                    result = 1;
                    goto EXIT_LOAD_COLLECTION;
                }
            }
        }
        else {
            result = 1;
        }
    }

EXIT_LOAD_COLLECTION:
    g_clTextFileManager.fclose(stream);
    return result;
}

int cltQuestKindInfo::LoadDeliveryQuestInfo(char* filePath) {
    char buffer[1024]{};
    char delimiter[] = "\t\n";
    int result = 0;

    FILE* stream = g_clTextFileManager.fopen(filePath);
    if (stream == nullptr) {
        return 0;
    }

    if (std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream)) {
        if (std::fgets(buffer, 1023, stream)) {
            while (true) {
                char* token = std::strtok(buffer, delimiter); // 1 任務_ID
                if (token == nullptr) {
                    break;
                }

                const std::uint16_t questCode = TranslateKindCode(token);
                stQuestKindInfo* info = GetQuestKindInfo(questCode);
                if (info == nullptr) {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 2 道具ID
                if (token == nullptr) {
                    break;
                }
                info->extra.delivery.wItemId = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 3 接收NPCID
                if (token == nullptr) {
                    break;
                }
                info->extra.delivery.wReceiveNpcId = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 4 接收NPC對話
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->extra.delivery.wReceiveNpcDialogue = static_cast<std::uint16_t>(std::atoi(token));

                if (!std::fgets(buffer, 1023, stream)) {
                    result = 1;
                    goto EXIT_LOAD_DELIVERY;
                }
            }
        }
        else {
            result = 1;
        }
    }

EXIT_LOAD_DELIVERY:
    g_clTextFileManager.fclose(stream);
    return result;
}

int cltQuestKindInfo::LoadHuntQuestInfo(char* filePath) {
    char buffer[1024]{};
    char delimiter[] = "\t\n";
    int result = 0;

    FILE* stream = g_clTextFileManager.fopen(filePath);
    if (stream == nullptr) {
        return 0;
    }

    if (std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream)) {
        if (std::fgets(buffer, 1023, stream)) {
            do {
                char* token = std::strtok(buffer, delimiter); // 1 任務_ID
                if (token != nullptr) {
                    const std::uint16_t questCode = TranslateKindCode(token);
                    stQuestKindInfo* info = GetQuestKindInfo(questCode);
                    if (info == nullptr) {
                        goto EXIT_LOAD_HUNT;
                    }

                    token = std::strtok(nullptr, delimiter); // 2 目標_ID_1
                    if (token == nullptr) {
                        goto EXIT_LOAD_HUNT;
                    }
                    info->extra.hunt.wTargetId1 = TranslateKindCode(token);

                    token = std::strtok(nullptr, delimiter); // 3 計數
                    if (token == nullptr || !IsDigitString(token)) {
                        goto EXIT_LOAD_HUNT;
                    }
                    info->extra.hunt.wCount = static_cast<std::uint16_t>(std::atoi(token));
                    if (info->extra.hunt.wTargetId1 != 0 && info->extra.hunt.wCount == 0) {
                        goto EXIT_LOAD_HUNT;
                    }

                    token = std::strtok(nullptr, delimiter); // 4 主要社團
                    if (token == nullptr || !IsDigitString(token)) {
                        goto EXIT_LOAD_HUNT;
                    }
                    info->extra.hunt.wMainHabitat = static_cast<std::uint16_t>(std::atoi(token));
                }
            } while (std::fgets(buffer, 1023, stream));
        }
        result = 1;
    }

EXIT_LOAD_HUNT:
    g_clTextFileManager.fclose(stream);
    return result;
}

int cltQuestKindInfo::LoadOneWayDeliveryQuestInfo(char* filePath) {
    char buffer[1024]{};
    char delimiter[] = "\t\n";
    int result = 0;

    FILE* stream = g_clTextFileManager.fopen(filePath);
    if (stream == nullptr) {
        return 0;
    }

    if (std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream)) {
        if (std::fgets(buffer, 1023, stream)) {
            while (true) {
                char* token = std::strtok(buffer, delimiter); // 1 任務_ID
                if (token == nullptr) {
                    break;
                }

                const std::uint16_t questCode = TranslateKindCode(token);
                stQuestKindInfo* info = GetQuestKindInfo(questCode);
                if (info == nullptr) {
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 2 物品ID
                if (token == nullptr) {
                    break;
                }
                info->extra.oneWayDelivery.wItemId = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 3 指定NPCID
                if (token == nullptr) {
                    break;
                }
                info->extra.oneWayDelivery.wAssignedNpcId = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 4 接收NPCID
                if (token == nullptr) {
                    break;
                }
                info->extra.oneWayDelivery.wReceiverNpcId = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 5 接收NPC對話
                if (token == nullptr || !IsDigitString(token)) {
                    break;
                }
                info->extra.oneWayDelivery.wReceiverNpcDialogue = static_cast<std::uint16_t>(std::atoi(token));

                if (!std::fgets(buffer, 1023, stream)) {
                    result = 1;
                    goto EXIT_LOAD_ONEWAY;
                }
            }
        }
        else {
            result = 1;
        }
    }

EXIT_LOAD_ONEWAY:
    g_clTextFileManager.fclose(stream);
    return result;
}

int cltQuestKindInfo::LoadBuyEmblem() {
    const std::uint16_t questCode = TranslateKindCode("Q0248");
    stQuestKindInfo* info = GetQuestKindInfo(questCode);
    if (info == nullptr) {
        return 0;
    }

    info->extra.buyEmblem.wEmblemItemId = TranslateKindCode("E0001");
    info->extra.buyEmblem.wEmblemNpcId = 23704;
    return 1;
}

int cltQuestKindInfo::LoadPlayIndun(char* filePath) {
    char buffer[1024]{};
    char delimiter[] = "\t\n";
    int result = 0;

    FILE* stream = g_clTextFileManager.fopen(filePath);
    if (stream == nullptr) {
        return 0;
    }

    if (std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream) &&
        std::fgets(buffer, 1023, stream)) {
        if (std::fgets(buffer, 1023, stream)) {
            while (true) {
                char* token = std::strtok(buffer, delimiter); // 1 任務代碼
                if (token == nullptr) {
                    break;
                }

                const std::uint16_t questCode = TranslateKindCode(token);
                stQuestKindInfo* info = GetQuestKindInfo(questCode);
                if (info == nullptr) {
                    break;
                }

                if (std::strtok(nullptr, delimiter) == nullptr) { // 2 副本名稱：只驗證存在，不落表。
                    break;
                }

                token = std::strtok(nullptr, delimiter); // 3 副本ID
                if (token == nullptr) {
                    break;
                }
                info->extra.playIndun.wDungeonId = TranslateKindCode(token);

                token = std::strtok(nullptr, delimiter); // 4 副本生成物品代碼
                if (token == nullptr) {
                    break;
                }
                info->extra.playIndun.wDungeonItemCode = TranslateKindCode(token);

                if (!std::fgets(buffer, 1023, stream)) {
                    result = 1;
                    goto EXIT_LOAD_PLAYINDUN;
                }
            }
        }
        else {
            result = 1;
        }
    }

EXIT_LOAD_PLAYINDUN:
    g_clTextFileManager.fclose(stream);
    return result;
}

std::uint16_t cltQuestKindInfo::TranslateKindCode(const char* code) {
    if (code == nullptr || std::strlen(code) != 5) {
        return 0;
    }

    const int high = (std::toupper(static_cast<unsigned char>(code[0])) + 31) << 11;
    const std::uint16_t low = static_cast<std::uint16_t>(std::atoi(code + 1));
    if (low >= 0x800u) {
        return 0;
    }

    return static_cast<std::uint16_t>(high | low);
}

std::uint16_t cltQuestKindInfo::TranslateClassCode(const char* code) {
    if (code == nullptr || std::strlen(code) != 3) {
        return 0;
    }

    const std::uint16_t a = static_cast<std::uint16_t>(std::toupper(static_cast<unsigned char>(code[0])) - 'A');
    const std::uint16_t b = static_cast<std::uint16_t>(std::toupper(static_cast<unsigned char>(code[1])) - 'A');
    return static_cast<std::uint16_t>((32 * (b | (32 * a))) | (std::toupper(static_cast<unsigned char>(code[2])) - 'A'));
}

stQuestKindInfo* cltQuestKindInfo::GetQuestKindInfo(std::uint16_t questCode) {
    return m_infos[questCode];
}

const stQuestKindInfo* cltQuestKindInfo::GetQuestKindInfo(std::uint16_t questCode) const {
    return m_infos[questCode];
}

std::uint8_t cltQuestKindInfo::GetQuestPlayType(std::uint16_t questCode) const {
    const stQuestKindInfo* info = GetQuestKindInfo(questCode);
    return info ? info->bPlayType : 0;
}

std::uint8_t cltQuestKindInfo::GetDemandLevel(std::uint16_t questCode) const {
    const stQuestKindInfo* info = GetQuestKindInfo(questCode);
    return info ? info->bConditionLevel : 0;
}

std::uint8_t cltQuestKindInfo::GetDemandFame(std::uint16_t questCode) const {
    const stQuestKindInfo* info = GetQuestKindInfo(questCode);
    return info ? info->bConditionReputation : 0;
}

bool cltQuestKindInfo::IsValidRewardExpByLevel(std::uint8_t level, std::int64_t exp) const {
    bool result = true;
    if (level <= m_levelSystem.GetMaxLevel()) {
        const std::int64_t baseExp = m_levelSystem.GetExpByLevel(level);
        double ratio = 0.5;

        if (level >= 0x78u) {
            ratio = 0.2;
        }
        else if (level >= 0x64u) {
            ratio = 0.5;
        }
        else if (level >= 0x41u) {
            ratio = 0.5;
        }
        else if (level >= 0x29u) {
            ratio = 0.5;
        }
        else if (level >= 0x10u) {
            ratio = 0.5;
        }

        if (static_cast<std::int64_t>(static_cast<double>(baseExp) * ratio) < exp) {
            result = false;
        }
    }
    return result;
}

int cltQuestKindInfo::IsValidRewardLibiByLevel(std::uint8_t level, int libi) const {
    if (libi == 0) {
        return 1;
    }
    if (level > m_levelSystem.GetMaxLevel()) {
        return 1;
    }

    switch (static_cast<char>(level) / 10) {
    case 0:
        if (libi < 100 || libi > 1000) return 0;
        return 1;
    case 1:
        if (libi < 550 || libi > 2400) return 0;
        return 1;
    case 2:
        if (libi < 2000 || libi > 6600) return 0;
        return 1;
    case 3:
        if (libi < 6000 || libi > 14000) return 0;
        return 1;
    case 4:
        if (libi < 12000 || libi > 23000) return 0;
        return 1;
    case 5:
        if (libi < 21000 || libi > 37000) return 0;
        return 1;
    case 6:
        if (libi < 35000 || libi > 45000) return 0;
        return 1;
    case 7:
        if (libi < 40000 || libi > 71100) return 0;
        return 1;
    case 8:
        if (libi < 57000 || libi > 85000) return 0;
        return 1;
    case 9:
    case 10:
    case 11:
    case 12:
        if (libi < 80000 || libi > 100000) return 0;
        return 1;
    default:
        return 0;
    }
}