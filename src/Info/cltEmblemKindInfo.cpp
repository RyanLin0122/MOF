#include <cstring>
#include <cstdlib>
#include <cctype>
#include "Info/cltEmblemKindInfo.h"
#include "Info/cltItemKindInfo.h"   // 只用 TranslateKindCode（若你放在別處，保留前置宣告即可）
#include "Info/cltClassKindInfo.h"  // 需能由 ClassList 解析成 bitmask
#include "Info/cltMapInfo.h"        // 不直接用到，但與同系統一致時可保留

// ------------------------------------------------------------
// 靜態成員
// ------------------------------------------------------------
cltClassKindInfo* cltEmblemKindInfo::m_pclClassKindInfo = nullptr;

// ------------------------------------------------------------
// 依 IDA ctor：42 個 DWORD 全設 0
// ------------------------------------------------------------
cltEmblemKindInfo::cltEmblemKindInfo()
{
    std::memset(this, 0, sizeof(*this));
}

// ------------------------------------------------------------
// IDA: cltEmblemKindInfo::InitializeStaticVariable
// ------------------------------------------------------------
void cltEmblemKindInfo::InitializeStaticVariable(cltClassKindInfo* p)
{
    m_pclClassKindInfo = p;
}

// ------------------------------------------------------------
// 小工具：對應 IDA 的 IsDigit / IsAlphaNumeric
// - IDA 僅用於 validate token 格式；只要任一字元不符就視為失敗
// ------------------------------------------------------------
bool cltEmblemKindInfo::IsDigitStr(const char* s)
{
    if (!s || !*s) return false;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p)
    {
        if (!std::isdigit(*p)) return false;
    }
    return true;
}

bool cltEmblemKindInfo::IsAlphaNumericStr(const char* s)
{
    if (!s || !*s) return false;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p)
    {
        if (!std::isalnum(*p)) return false;
    }
    return true;
}

// ------------------------------------------------------------
// IDA: TranslateKindCode
// strlen==5；(toupper(s[0])+31)<<11 | atoi(s+1)；且後三位 < 0x800
// ------------------------------------------------------------
uint16_t cltEmblemKindInfo::TranslateKindCode(char* a1)
{
    if (!a1) return 0;
    if (std::strlen(a1) != 5) return 0;

    int v2 = (std::toupper((unsigned char)*a1) + 31) << 11;
    uint16_t v3 = (uint16_t)std::atoi(a1 + 1);
    if (v3 < 0x800u) return (uint16_t)(v2 | v3);
    return 0;
}

// ------------------------------------------------------------
// IDA: Initialize
// 行為重點：
// 1) fopen
// 2) 讀三行（header）成功才進入
// 3) fgetpos，數剩餘行數 => m_nCount
// 4) new 416*m_nCount，清 0，fsetpos 回去
// 5) 逐行解析：任一欄位缺失/格式不符 => 立即失敗（return 0），但不回收已配置內容（與 IDA 一致）
// 6) 全部行解析完（遇 EOF）=> 建立各事件的指標陣列，回傳 1
// ------------------------------------------------------------
int cltEmblemKindInfo::Initialize(char* filename)
{
    char delimiter[2] = { 0 };
    std::strcpy(delimiter, "\t\n"); // IDA: strcpy(Delimiter, "\t\n")

    // IDA：Buffer 是 4095 bytes 的 fgets 緩衝；這裡用 4096（含 '\0'）近似
    char buffer[4096] = { 0 };
    char jobChangeStr[1024] = { 0 };

    int success = 0;

    FILE* stream = g_clTextFileManager.fopen(filename);
    if (!stream) return 0;

    // 讀三行 header
    if (std::fgets(buffer, (int)sizeof(buffer), stream) &&
        std::fgets(buffer, (int)sizeof(buffer), stream) &&
        std::fgets(buffer, (int)sizeof(buffer), stream))
    {
        fpos_t pos;
        std::fgetpos(stream, &pos);

        // 數剩餘行數 => m_nCount
        while (std::fgets(buffer, (int)sizeof(buffer), stream))
        {
            ++m_nCount;
        }

        // 配置 info 區塊
        m_pInfos = (strEmblemKindInfo*)::operator new(sizeof(strEmblemKindInfo) * (size_t)m_nCount);
        std::memset(m_pInfos, 0, sizeof(strEmblemKindInfo) * (size_t)m_nCount);

        std::fsetpos(stream, &pos);

        // 逐行填入
        uint8_t* rec = (uint8_t*)m_pInfos;
        if (std::fgets(buffer, (int)sizeof(buffer), stream))
        {
            while (true)
            {
                // strtok 會改寫 buffer；與 IDA 相同
                char* tok = std::strtok(buffer, delimiter); // 1 wEmblemId
                if (!tok) break;
                *(uint16_t*)(rec + 0) = TranslateKindCode(tok);

                if (!std::strtok(nullptr, delimiter)) // 2 EmblemName（僅檢查存在，未寫入）
                    break;

                tok = std::strtok(nullptr, delimiter); // 3 wEmblemNameCode
                if (!tok || !IsDigitStr(tok)) break;
                *(uint16_t*)(rec + 28) = (uint16_t)std::atoi(tok);

                tok = std::strtok(nullptr, delimiter); // 4 wEmblemDescriptionCode
                if (!tok || !IsDigitStr(tok)) break;
                *(uint16_t*)(rec + 30) = (uint16_t)std::atoi(tok);

                tok = std::strtok(nullptr, delimiter); // 5 dwPrice
                if (!tok || !IsDigitStr(tok)) break;
                *(uint32_t*)(rec + 24) = (uint32_t)std::atoi(tok);

                tok = std::strtok(nullptr, delimiter); // 6 ItemImageFileId（hex）
                if (!tok || !IsAlphaNumericStr(tok)) break;
                std::sscanf(tok, "%x", (unsigned int*)(rec + 32));

                tok = std::strtok(nullptr, delimiter); // 7 wItemImageBlockId
                if (!tok || !IsDigitStr(tok)) break;
                *(uint16_t*)(rec + 36) = (uint16_t)std::atoi(tok);

                tok = std::strtok(nullptr, delimiter); // 8 dwRarityType
                if (!tok || !IsDigitStr(tok)) break;
                *(uint32_t*)(rec + 40) = (uint32_t)std::atoi(tok);

                tok = std::strtok(nullptr, delimiter); // 9 wEquipConditionLevelFrom
                if (!tok || !IsDigitStr(tok)) break;
                *(uint16_t*)(rec + 2) = (uint16_t)std::atoi(tok);

                tok = std::strtok(nullptr, delimiter); // 10 wEquipConditionLevelTo
                if (!tok || !IsDigitStr(tok)) break;
                *(uint16_t*)(rec + 4) = (uint16_t)std::atoi(tok);

                tok = std::strtok(nullptr, delimiter); // 11 wAcquireConditionLevel
                if (!tok || !IsDigitStr(tok)) break;
                *(uint16_t*)(rec + 6) = (uint16_t)std::atoi(tok);

                tok = std::strtok(nullptr, delimiter); // 12 AcquireConditionMonsterType
                if (!tok) break;
                std::strcpy((char*)(rec + 44), tok);

                tok = std::strtok(nullptr, delimiter); // 13 dwAcquireConditionMonsterKillCount
                if (!tok || !IsDigitStr(tok)) break;
                *(uint32_t*)(rec + 172) = (uint32_t)std::atoi(tok);

                tok = std::strtok(nullptr, delimiter); // 14 wAcquireConditionQuest (TranslateKindCode from quest)
                if (!tok) break;
                *(uint16_t*)(rec + 8) = cltQuestKindInfo::TranslateKindCode(tok);

                tok = std::strtok(nullptr, delimiter); // 15 JobChange (字串暫存，稍後用 GetClassAtb)
                if (!tok) break;
                std::strcpy(jobChangeStr, tok);

                tok = std::strtok(nullptr, delimiter); // 16 dwAcquireConditionDeathRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 176) = v;
                    if (v) ++m_nOnDead; // IDA: ++*((_DWORD*)this+3)
                }

                tok = std::strtok(nullptr, delimiter); // 17 MpPotionRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 180) = v;
                    if (v) ++m_nOnUsingRecoverMP; // +7
                }

                tok = std::strtok(nullptr, delimiter); // 18 HpPotionRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 184) = v;
                    if (v) ++m_nOnUsingRecoverHP; // +5
                }

                tok = std::strtok(nullptr, delimiter); // 19 DragonUsageRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 188) = v;
                    if (v) ++m_nOnTeleportDragon; // +17
                }

                tok = std::strtok(nullptr, delimiter); // 20 CritReceivedRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 192) = v;
                    if (v) ++m_nOnBeAttackedCritically; // +37
                }

                tok = std::strtok(nullptr, delimiter); // 21 CritDealtRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 196) = v;
                    if (v) ++m_nOnAttackCritically; // +39
                }

                tok = std::strtok(nullptr, delimiter); // 22 PersonalShopRate
                if (!tok || !IsDigitStr(tok)) break;
                *(uint32_t*)(rec + 200) = (uint32_t)std::atoi(tok);

                tok = std::strtok(nullptr, delimiter); // 23 RunePayRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 204) = v;
                    if (v) ++m_nOnRegistrySellingAgency; // +11
                }

                tok = std::strtok(nullptr, delimiter); // 24 ShopUsageRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 208) = v;
                    if (v) ++m_nOnBuyItemFromNPC; // +9
                }

                tok = std::strtok(nullptr, delimiter); // 25 PublicQuestCompletionRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 212) = v;
                    if (v) ++m_nOnCompleteMeritous; // +19
                }

                tok = std::strtok(nullptr, delimiter); // 26 SwordClassCompletionRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 216) = v;
                    if (v) ++m_nOnCompleteSwordLesson; // +21
                }

                tok = std::strtok(nullptr, delimiter); // 27 BowClassCompletionRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 220) = v;
                    if (v) ++m_nOnCompleteBowLesson; // +23
                }

                tok = std::strtok(nullptr, delimiter); // 28 MagicClassCompletionRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 224) = v;
                    if (v) ++m_nOnCompleteMagicLesson; // +25
                }

                tok = std::strtok(nullptr, delimiter); // 29 TheologyClassCompletionRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 228) = v;
                    if (v) ++m_nOnCompleteTheologyLesson; // +27
                }

                tok = std::strtok(nullptr, delimiter); // 30 EnchantRate
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 232) = v;
                    if (v) ++m_nOnEnchantItem; // +29
                }

                tok = std::strtok(nullptr, delimiter); // 31 QuestImmediateAcquire (TranslateKindCode)
                if (!tok) break;
                {
                    uint32_t v = (uint32_t)cltQuestKindInfo::TranslateKindCode(tok);
                    *(uint32_t*)(rec + 236) = v;
                    if (v) ++m_nOnCompleteQuest; // +13
                }

                tok = std::strtok(nullptr, delimiter); // 32 AllBossKill
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 240) = v;
                    if (v) ++m_nOnKillBossMonster; // +15
                }

                tok = std::strtok(nullptr, delimiter); // 33 CircleTaskCompletion
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 244) = v;
                    if (v) ++m_nOnCompleteCircleQuest; // +31
                }

                tok = std::strtok(nullptr, delimiter); // 34 ItemSale
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 248) = v;
                    if (v) ++m_nOnSellItemToNPC; // +33
                }

                tok = std::strtok(nullptr, delimiter); // 35 AreaAttack
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 252) = v;
                    if (v) ++m_nOnMultiAttack; // +35
                }

                tok = std::strtok(nullptr, delimiter); // 36 JobChangeQuestCompletion
                if (!tok || !IsDigitStr(tok)) break;
                {
                    uint32_t v = (uint32_t)std::atoi(tok);
                    *(uint32_t*)(rec + 256) = v;
                    if (v) ++m_nOnChangeClass; // +41
                }

                // 37..74：皆為數值/代碼（多數不參與索引統計，IDA 也沒統計）
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 260) = (uint32_t)std::atoi(tok); // 37
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 312) = (uint32_t)std::atoi(tok); // 38
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 316) = (uint32_t)std::atoi(tok); // 39
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 320) = (uint32_t)std::atoi(tok); // 40
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 324) = (uint32_t)std::atoi(tok); // 41
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 268) = (uint32_t)std::atoi(tok); // 42
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 264) = (uint32_t)std::atoi(tok); // 43
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 272) = (uint32_t)std::atoi(tok); // 44
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 336) = (uint32_t)std::atoi(tok); // 45
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 340) = (uint32_t)std::atoi(tok); // 46
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 328) = (uint32_t)std::atoi(tok); // 47
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 332) = (uint32_t)std::atoi(tok); // 48
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 344) = (uint32_t)std::atoi(tok); // 49
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 348) = (uint32_t)std::atoi(tok); // 50
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 276) = (uint32_t)std::atoi(tok); // 51
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 280) = (uint32_t)std::atoi(tok); // 52
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 284) = (uint32_t)std::atoi(tok); // 53
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 288) = (uint32_t)std::atoi(tok); // 54
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 292) = (uint32_t)std::atoi(tok); // 55
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 296) = (uint32_t)std::atoi(tok); // 56
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 300) = (uint32_t)std::atoi(tok); // 57
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 304) = (uint32_t)std::atoi(tok); // 58
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 308) = (uint32_t)std::atoi(tok); // 59
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 352) = (uint32_t)std::atoi(tok); // 60
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 356) = (uint32_t)std::atoi(tok); // 61
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 360) = (uint32_t)std::atoi(tok); // 62
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 364) = (uint32_t)std::atoi(tok); // 63
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 368) = (uint32_t)std::atoi(tok); // 64
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 372) = (uint32_t)std::atoi(tok); // 65
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 376) = (uint32_t)std::atoi(tok); // 66
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 380) = (uint32_t)std::atoi(tok); // 67
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint32_t*)(rec + 384) = (uint32_t)std::atoi(tok); // 68
                tok = std::strtok(nullptr, delimiter); if (!tok) break; *(uint16_t*)(rec + 392) = cltItemKindInfo::TranslateKindCode(tok);          // 69
                tok = std::strtok(nullptr, delimiter); if (!tok) break; *(uint32_t*)(rec + 396) = (uint32_t)std::atoi(tok);                        // 70
                tok = std::strtok(nullptr, delimiter); if (!tok) break; *(uint32_t*)(rec + 400) = (uint32_t)std::atoi(tok);                        // 71
                tok = std::strtok(nullptr, delimiter); if (!tok) break; *(uint32_t*)(rec + 388) = (uint32_t)std::atoi(tok);                        // 72
                tok = std::strtok(nullptr, delimiter); if (!tok) break; *(uint32_t*)(rec + 404) = (uint32_t)std::atoi(tok);                        // 73
                tok = std::strtok(nullptr, delimiter); if (!tok || !IsDigitStr(tok)) break; *(uint16_t*)(rec + 408) = (uint16_t)std::atoi(tok);    // 74

                // job change string：若首字元 != '0' 則寫入 64-bit class attribute（IDA：寫到 (16,20)）
                if (jobChangeStr[0] != '0')
                {
                    *(uint64_t*)(rec + 16) = m_pclClassKindInfo->GetClassAtb(jobChangeStr);
                }

                // 下一筆
                rec += sizeof(strEmblemKindInfo);

                // 嘗試讀下一行；若 EOF => 成功收尾（建立索引陣列）
                if (!std::fgets(buffer, (int)sizeof(buffer), stream))
                {
                    // ============== LABEL_189：建立各事件指標陣列 ==============
                    auto buildIndex = [&](strEmblemKindInfo*** ppOut, int32_t count,
                        size_t fieldOffset) -> void
                        {
                            if (count <= 0)
                            {
                                *ppOut = (strEmblemKindInfo**)::operator new(0);
                                return;
                            }
                            *ppOut = (strEmblemKindInfo**)::operator new(sizeof(void*) * (size_t)count);

                            int32_t w = 0;
                            for (int32_t i = 0; i < m_nCount; ++i)
                            {
                                uint8_t* p = (uint8_t*)m_pInfos + i * sizeof(strEmblemKindInfo);
                                if (*(uint32_t*)(p + fieldOffset))
                                {
                                    (*ppOut)[w++] = (strEmblemKindInfo*)p;
                                }
                            }
                        };

                    // 依 IDA：用各欄位是否非 0 來篩選，寫入對應陣列
                    buildIndex(&m_ppOnDead, m_nOnDead, 176);
                    buildIndex(&m_ppOnUsingRecoverHP, m_nOnUsingRecoverHP, 184);
                    buildIndex(&m_ppOnUsingRecoverMP, m_nOnUsingRecoverMP, 180);
                    buildIndex(&m_ppOnBuyItemFromNPC, m_nOnBuyItemFromNPC, 208);
                    buildIndex(&m_ppOnRegistrySellingAgency, m_nOnRegistrySellingAgency, 204);
                    buildIndex(&m_ppOnCompleteQuest, m_nOnCompleteQuest, 236);
                    buildIndex(&m_ppOnKillBossMonster, m_nOnKillBossMonster, 240);
                    buildIndex(&m_ppOnTeleportDragon, m_nOnTeleportDragon, 188);
                    buildIndex(&m_ppOnCompleteMeritous, m_nOnCompleteMeritous, 212);
                    buildIndex(&m_ppOnCompleteSwordLesson, m_nOnCompleteSwordLesson, 216);
                    buildIndex(&m_ppOnCompleteBowLesson, m_nOnCompleteBowLesson, 220);
                    buildIndex(&m_ppOnCompleteMagicLesson, m_nOnCompleteMagicLesson, 224);
                    buildIndex(&m_ppOnCompleteTheologyLesson, m_nOnCompleteTheologyLesson, 228);
                    buildIndex(&m_ppOnEnchantItem, m_nOnEnchantItem, 232);
                    buildIndex(&m_ppOnCompleteCircleQuest, m_nOnCompleteCircleQuest, 244);
                    buildIndex(&m_ppOnSellItemToNPC, m_nOnSellItemToNPC, 248);
                    buildIndex(&m_ppOnMultiAttack, m_nOnMultiAttack, 252);
                    buildIndex(&m_ppOnBeAttackedCritically, m_nOnBeAttackedCritically, 192);
                    buildIndex(&m_ppOnAttackCritically, m_nOnAttackCritically, 196);
                    buildIndex(&m_ppOnChangeClass, m_nOnChangeClass, 256);

                    success = 1;
                    break;
                }
            }
        }
    }

    g_clTextFileManager.fclose(stream);
    return success;
}

// ------------------------------------------------------------
// IDA: Free（依序 delete 各指標，歸零）
// ------------------------------------------------------------
void cltEmblemKindInfo::Free()
{
    if (m_pInfos) { ::operator delete(m_pInfos); m_pInfos = nullptr; }

    if (m_ppOnDead) { ::operator delete(m_ppOnDead); m_ppOnDead = nullptr; }
    if (m_ppOnUsingRecoverHP) { ::operator delete(m_ppOnUsingRecoverHP); m_ppOnUsingRecoverHP = nullptr; }
    if (m_ppOnUsingRecoverMP) { ::operator delete(m_ppOnUsingRecoverMP); m_ppOnUsingRecoverMP = nullptr; }
    if (m_ppOnBuyItemFromNPC) { ::operator delete(m_ppOnBuyItemFromNPC); m_ppOnBuyItemFromNPC = nullptr; }
    if (m_ppOnRegistrySellingAgency) { ::operator delete(m_ppOnRegistrySellingAgency); m_ppOnRegistrySellingAgency = nullptr; }
    if (m_ppOnCompleteQuest) { ::operator delete(m_ppOnCompleteQuest); m_ppOnCompleteQuest = nullptr; }
    if (m_ppOnKillBossMonster) { ::operator delete(m_ppOnKillBossMonster); m_ppOnKillBossMonster = nullptr; }
    if (m_ppOnTeleportDragon) { ::operator delete(m_ppOnTeleportDragon); m_ppOnTeleportDragon = nullptr; }
    if (m_ppOnCompleteMeritous) { ::operator delete(m_ppOnCompleteMeritous); m_ppOnCompleteMeritous = nullptr; }
    if (m_ppOnCompleteSwordLesson) { ::operator delete(m_ppOnCompleteSwordLesson); m_ppOnCompleteSwordLesson = nullptr; }
    if (m_ppOnCompleteBowLesson) { ::operator delete(m_ppOnCompleteBowLesson); m_ppOnCompleteBowLesson = nullptr; }
    if (m_ppOnCompleteMagicLesson) { ::operator delete(m_ppOnCompleteMagicLesson); m_ppOnCompleteMagicLesson = nullptr; }
    if (m_ppOnCompleteTheologyLesson) { ::operator delete(m_ppOnCompleteTheologyLesson); m_ppOnCompleteTheologyLesson = nullptr; }
    if (m_ppOnEnchantItem) { ::operator delete(m_ppOnEnchantItem); m_ppOnEnchantItem = nullptr; }
    if (m_ppOnCompleteCircleQuest) { ::operator delete(m_ppOnCompleteCircleQuest); m_ppOnCompleteCircleQuest = nullptr; }
    if (m_ppOnSellItemToNPC) { ::operator delete(m_ppOnSellItemToNPC); m_ppOnSellItemToNPC = nullptr; }
    if (m_ppOnMultiAttack) { ::operator delete(m_ppOnMultiAttack); m_ppOnMultiAttack = nullptr; }
    if (m_ppOnBeAttackedCritically) { ::operator delete(m_ppOnBeAttackedCritically); m_ppOnBeAttackedCritically = nullptr; }
    if (m_ppOnAttackCritically) { ::operator delete(m_ppOnAttackCritically); m_ppOnAttackCritically = nullptr; }
    if (m_ppOnChangeClass) { ::operator delete(m_ppOnChangeClass); m_ppOnChangeClass = nullptr; }

    // IDA: counts 歸零
    m_nCount = 0;

    m_nOnDead = 0;
    m_nOnUsingRecoverHP = 0;
    m_nOnUsingRecoverMP = 0;
    m_nOnBuyItemFromNPC = 0;
    m_nOnRegistrySellingAgency = 0;
    m_nOnCompleteQuest = 0;
    m_nOnKillBossMonster = 0;
    m_nOnTeleportDragon = 0;
    m_nOnCompleteMeritous = 0;
    m_nOnCompleteSwordLesson = 0;
    m_nOnCompleteBowLesson = 0;
    m_nOnCompleteMagicLesson = 0;
    m_nOnCompleteTheologyLesson = 0;
    m_nOnEnchantItem = 0;
    m_nOnCompleteCircleQuest = 0;
    m_nOnSellItemToNPC = 0;
    m_nOnMultiAttack = 0;
    m_nOnBeAttackedCritically = 0;
    m_nOnAttackCritically = 0;
    m_nOnChangeClass = 0;
}

// ------------------------------------------------------------
// IDA: GetEmblemKindInfo（線性搜尋；步長 416 bytes）
// ------------------------------------------------------------
strEmblemKindInfo* cltEmblemKindInfo::GetEmblemKindInfo(uint16_t emblemId)
{
    if (m_nCount <= 0 || !m_pInfos) return nullptr;

    for (int i = 0; i < m_nCount; ++i)
    {
        strEmblemKindInfo* p = (strEmblemKindInfo*)((uint8_t*)m_pInfos + i * sizeof(strEmblemKindInfo));
        if (p->wEmblemId == emblemId) return p;
    }
    return nullptr;
}

// ------------------------------------------------------------
// IDA: GetBuyableEmblems
// 條件：wAcquireConditionLevel || wAcquireConditionQuest || QWORD@16 !=0 || *(BYTE*)(44)!='0'
// ------------------------------------------------------------
int cltEmblemKindInfo::GetBuyableEmblems(uint16_t* outIds)
{
    int count = 0;
    if (m_nCount <= 0 || !m_pInfos) return 0;

    for (int i = 0; i < m_nCount; ++i)
    {
        uint8_t* p = (uint8_t*)m_pInfos + i * sizeof(strEmblemKindInfo);

        const uint16_t acqLevel = *(uint16_t*)(p + 6);
        const uint16_t acqQuest = *(uint16_t*)(p + 8);
        const uint64_t jobAtb = *(uint64_t*)(p + 16);
        const char     mon0 = *(char*)(p + 44);

        if (acqLevel || acqQuest || jobAtb || (mon0 != '0'))
        {
            ++count;
            *outIds++ = *(uint16_t*)(p + 0);
        }
    }
    return count;
}

// ------------------------------------------------------------
// IDA: 一票 Getter（回傳 count，並回填指標陣列位址）
// ------------------------------------------------------------
int cltEmblemKindInfo::GetEmblemKindInfo_OnDead(strEmblemKindInfo*** out) { *out = m_ppOnDead;                  return m_nOnDead; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnUsingRecoverHPItem(strEmblemKindInfo*** out) { *out = m_ppOnUsingRecoverHP;        return m_nOnUsingRecoverHP; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnUsingRecoverManaItem(strEmblemKindInfo*** out) { *out = m_ppOnUsingRecoverMP;        return m_nOnUsingRecoverMP; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnBuyItemFromNPC(strEmblemKindInfo*** out) { *out = m_ppOnBuyItemFromNPC;        return m_nOnBuyItemFromNPC; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnRegistrySellingAgency(strEmblemKindInfo*** out) { *out = m_ppOnRegistrySellingAgency; return m_nOnRegistrySellingAgency; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteQuest(strEmblemKindInfo*** out) { *out = m_ppOnCompleteQuest;         return m_nOnCompleteQuest; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnKillBossMonster(strEmblemKindInfo*** out) { *out = m_ppOnKillBossMonster;       return m_nOnKillBossMonster; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnTeleportDragon(strEmblemKindInfo*** out) { *out = m_ppOnTeleportDragon;        return m_nOnTeleportDragon; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteMeritous(strEmblemKindInfo*** out) { *out = m_ppOnCompleteMeritous;      return m_nOnCompleteMeritous; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteSwordLesson(strEmblemKindInfo*** out) { *out = m_ppOnCompleteSwordLesson;   return m_nOnCompleteSwordLesson; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteBowLesson(strEmblemKindInfo*** out) { *out = m_ppOnCompleteBowLesson;     return m_nOnCompleteBowLesson; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteMagicLesson(strEmblemKindInfo*** out) { *out = m_ppOnCompleteMagicLesson;   return m_nOnCompleteMagicLesson; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteTheologyLesson(strEmblemKindInfo*** out) { *out = m_ppOnCompleteTheologyLesson; return m_nOnCompleteTheologyLesson; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnEnchantItem(strEmblemKindInfo*** out) { *out = m_ppOnEnchantItem;           return m_nOnEnchantItem; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteCircleQuest(strEmblemKindInfo*** out) { *out = m_ppOnCompleteCircleQuest;   return m_nOnCompleteCircleQuest; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnSellItemToNPC(strEmblemKindInfo*** out) { *out = m_ppOnSellItemToNPC;         return m_nOnSellItemToNPC; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnMultiAttack(strEmblemKindInfo*** out) { *out = m_ppOnMultiAttack;           return m_nOnMultiAttack; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnBeAttackedCritically(strEmblemKindInfo*** out) { *out = m_ppOnBeAttackedCritically;  return m_nOnBeAttackedCritically; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnAttackCritically(strEmblemKindInfo*** out) { *out = m_ppOnAttackCritically;      return m_nOnAttackCritically; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnChangeClass(strEmblemKindInfo*** out) { *out = m_ppOnChangeClass;           return m_nOnChangeClass; }