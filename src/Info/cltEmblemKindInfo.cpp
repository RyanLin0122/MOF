#include "Info/cltEmblemKindInfo.h"
#include "Info/cltItemKindInfo.h"   // 只用 TranslateKindCode（若你放在別處，保留前置宣告即可）
#include "Info/cltClassKindInfo.h"  // 需能由 ClassList 解析成 bitmask
#include "Info/cltMapInfo.h"        // 不直接用到，但與同系統一致時可保留

// 靜態成員
cltClassKindInfo* cltEmblemKindInfo::m_pclClassKindInfo = nullptr;

static const char* kDelims = "\t\n";

// -------- 小工具 ---------------------------------------------------------
bool cltEmblemKindInfo::IsDigitStr(const char* s) {
    if (!s || !*s) return false;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p)
        if (!std::isdigit(*p)) return false;
    return true;
}
bool cltEmblemKindInfo::IsAlphaNumStr(const char* s) {
    if (!s || !*s) return false;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p)
        if (!std::isalnum(*p)) return false;
    return true;
}

// -------- 物件生命週期 ---------------------------------------------------
cltEmblemKindInfo::cltEmblemKindInfo()
{
    // 依反編譯：建構子將 +0..+41（共 42 個 DWORD）全設 0
    // 在此以成員預設值與 Free() 確保為 0
}

void cltEmblemKindInfo::Free()
{
    if (m_list) { operator delete(m_list); m_list = nullptr; }
    if (m_onDead) { operator delete(m_onDead);            m_onDead = nullptr; }
    if (m_onUseHP) { operator delete(m_onUseHP);           m_onUseHP = nullptr; }
    if (m_onUseMP) { operator delete(m_onUseMP);           m_onUseMP = nullptr; }
    if (m_onBuyNPC) { operator delete(m_onBuyNPC);          m_onBuyNPC = nullptr; }
    if (m_onRegistry) { operator delete(m_onRegistry);        m_onRegistry = nullptr; }
    if (m_onCompleteQuest) { operator delete(m_onCompleteQuest);   m_onCompleteQuest = nullptr; }
    if (m_onKillBoss) { operator delete(m_onKillBoss);        m_onKillBoss = nullptr; }
    if (m_onTeleportDragon) { operator delete(m_onTeleportDragon);  m_onTeleportDragon = nullptr; }
    if (m_onCompleteMeritous) { operator delete(m_onCompleteMeritous); m_onCompleteMeritous = nullptr; }
    if (m_onSwordLesson) { operator delete(m_onSwordLesson);     m_onSwordLesson = nullptr; }
    if (m_onBowLesson) { operator delete(m_onBowLesson);       m_onBowLesson = nullptr; }
    if (m_onMagicLesson) { operator delete(m_onMagicLesson);     m_onMagicLesson = nullptr; }
    if (m_onTheologyLesson) { operator delete(m_onTheologyLesson);  m_onTheologyLesson = nullptr; }
    if (m_onEnchant) { operator delete(m_onEnchant);         m_onEnchant = nullptr; }
    if (m_onCircleQuest) { operator delete(m_onCircleQuest);     m_onCircleQuest = nullptr; }
    if (m_onSellToNPC) { operator delete(m_onSellToNPC);       m_onSellToNPC = nullptr; }
    if (m_onMultiAttack) { operator delete(m_onMultiAttack);     m_onMultiAttack = nullptr; }
    if (m_onBeCrit) { operator delete(m_onBeCrit);          m_onBeCrit = nullptr; }
    if (m_onDoCrit) { operator delete(m_onDoCrit);          m_onDoCrit = nullptr; }
    if (m_onChangeClass) { operator delete(m_onChangeClass);     m_onChangeClass = nullptr; }

    m_count = 0;
    m_onDeadCnt = m_onUseHPCnt = m_onUseMPCnt = 0;
    m_onBuyNPCCnt = m_onRegistryCnt = m_onCompleteQuestCnt = 0;
    m_onKillBossCnt = m_onTeleportDragonCnt = m_onCompleteMeritousCnt = 0;
    m_onSwordLessonCnt = m_onBowLessonCnt = m_onMagicLessonCnt = m_onTheologyLessonCnt = 0;
    m_onEnchantCnt = m_onCircleQuestCnt = m_onSellToNPCCnt = m_onMultiAttackCnt = 0;
    m_onBeCritCnt = m_onDoCritCnt = m_onChangeClassCnt = 0;
}

// -------- 載入 -----------------------------------------------------------
int cltEmblemKindInfo::Initialize(char* filename)
{
    Free();
    if (!filename) return 0;

    FILE* f = g_clTextFileManager.fopen(filename);
    FILE* Stream = f;
    int ok = 0;
    if (!f) return 0;

    char line[4096] = {};
    // 跳過三行表頭
    if (std::fgets(line, sizeof(line), f) &&
        std::fgets(line, sizeof(line), f) &&
        std::fgets(line, sizeof(line), f))
    {
        // 計算資料行數
        fpos_t pos{};
        std::fgetpos(f, &pos);
        while (std::fgets(line, sizeof(line), f)) ++m_count;

        // 配置主表
        if (m_count == 0) {
            g_clTextFileManager.fclose(Stream);
            return 1; // 若無資料行，原始碼直接回傳成功
        }
        m_list = static_cast<strEmblemKindInfo*>(operator new(416 * m_count));
        std::memset(m_list, 0, 416 * m_count);

        // 回到資料起點
        std::fsetpos(f, &pos);

        // 逐列解析
        int idx = 0;
        if (!std::fgets(line, sizeof(line), f)) {
            ok = 1; // 無任何資料行（理論上不會到這）仍視為成功
        }
        else {
            do {
                strEmblemKindInfo& r = m_list[idx];
                std::memset(&r, 0, sizeof(r));

                // 依反編譯順序取 token
                char* t = nullptr;

                // 1) ID(5碼)
                t = std::strtok(line, kDelims); if (!t) break;
                r.kind = TranslateKindCode(t);

                // 2) 名稱（略過）
                if (!std::strtok(nullptr, kDelims)) break;

                // 3) 名稱文本碼
                t = std::strtok(nullptr, kDelims); if (!t || !IsDigitStr(t)) break;
                r.nameTextCode = static_cast<uint16_t>(std::atoi(t));

                // 4) 說明文本碼
                t = std::strtok(nullptr, kDelims); if (!t || !IsDigitStr(t)) break;
                r.descTextCode = static_cast<uint16_t>(std::atoi(t));

                // 5) 價格
                t = std::strtok(nullptr, kDelims); if (!t || !IsDigitStr(t)) break;
                r.price = std::atoi(t);

                // 6) 圖檔ID(hex)
                t = std::strtok(nullptr, kDelims); if (!t || !IsAlphaNumStr(t)) break;
                std::sscanf(t, "%x", &r.imageFileIdHex);

                // 7) 圖塊ID
                t = std::strtok(nullptr, kDelims); if (!t || !IsDigitStr(t)) break;
                r.imageBlockId = static_cast<uint16_t>(std::atoi(t));

                // 8) 稀有/分類
                t = std::strtok(nullptr, kDelims); if (!t || !IsDigitStr(t)) break;
                r.rareDiv = std::atoi(t);

                // 9) 裝備等級條件(下限/上限)
                t = std::strtok(nullptr, kDelims); if (!t || !IsDigitStr(t)) break;
                r.equipMinLv = static_cast<uint16_t>(std::atoi(t));

                t = std::strtok(nullptr, kDelims); if (!t || !IsDigitStr(t)) break;
                r.equipMaxLv = static_cast<uint16_t>(std::atoi(t));

                // 10) 獲得條件(等級到達)
                t = std::strtok(nullptr, kDelims); if (!t || !IsDigitStr(t)) break;
                r.acquireLevelReach = static_cast<uint16_t>(std::atoi(t));

                // 11) 獲得條件(怪物種類字串 / 多為 "0")
                t = std::strtok(nullptr, kDelims); if (!t) break;
                std::strncpy(r.condMonsterType, t, sizeof(r.condMonsterType) - 1);

                // 12) 獲得條件(怪物擊殺數)
                t = std::strtok(nullptr, kDelims); if (!t || !IsDigitStr(t)) break;
                r.condMonsterKillCnt = std::atoi(t);

                // 13) 獲得條件(任務代碼 5碼)
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.acquireQuestKind = cltQuestKindInfo::TranslateKindCode(t);

                // 14) 職業列表（例如 "KNI|SWO|..."）→ 轉成 64-bit atb
                t = std::strtok(nullptr, kDelims);   // 例如 "KNI|SWO|..." 或 "0"

                // '0' 表示沒有職業限制（對齊反編譯：if (String[0] != 48)）
                if (t[0] != '0') {
                    uint64_t atb = cltEmblemKindInfo::m_pclClassKindInfo->GetClassAtb(t);
                    r.classAtbLow = static_cast<uint32_t>(atb);
                    r.classAtbHigh = static_cast<uint32_t>(atb >> 32);
                }
                else {
                    r.classAtbLow = 0;
                    r.classAtbHigh = 0;
                }

                // 15) 之後一大串機率欄位（遇到非數字就中斷）
                auto NEXTI = [&](int32_t& dst)->bool {
                    char* x = std::strtok(nullptr, kDelims);
                    if (!x || !IsDigitStr(x)) return false;
                    dst = std::atoi(x);
                    return true;
                    };

                if (!NEXTI(r.prob_OnDead))                  break; if (r.prob_OnDead)                  ++m_onDeadCnt;
                if (!NEXTI(r.prob_OnUsingRecoverHPItem))    break; if (r.prob_OnUsingRecoverHPItem)    ++m_onUseHPCnt;
                if (!NEXTI(r.prob_OnUsingRecoverManaItem))  break; if (r.prob_OnUsingRecoverManaItem)  ++m_onUseMPCnt;
                if (!NEXTI(r.prob_OnTeleportDragon))        break; if (r.prob_OnTeleportDragon)        ++m_onTeleportDragonCnt;
                if (!NEXTI(r.prob_OnBeAttackedCritically))  break; if (r.prob_OnBeAttackedCritically)  ++m_onBeCritCnt;
                if (!NEXTI(r.prob_OnAttackCritically))      break; if (r.prob_OnAttackCritically)      ++m_onDoCritCnt;

                if (!NEXTI(r.misc200))                      break;

                if (!NEXTI(r.prob_OnRegistrySellingAgency)) break; if (r.prob_OnRegistrySellingAgency) ++m_onRegistryCnt;
                if (!NEXTI(r.prob_OnBuyItemFromNPC))        break; if (r.prob_OnBuyItemFromNPC)        ++m_onBuyNPCCnt;
                if (!NEXTI(r.prob_OnCompleteMeritous))      break; if (r.prob_OnCompleteMeritous)      ++m_onCompleteMeritousCnt;
                if (!NEXTI(r.prob_OnCompleteSwordLesson))   break; if (r.prob_OnCompleteSwordLesson)   ++m_onSwordLessonCnt;
                if (!NEXTI(r.prob_OnCompleteBowLesson))     break; if (r.prob_OnCompleteBowLesson)     ++m_onBowLessonCnt;
                if (!NEXTI(r.prob_OnCompleteMagicLesson))   break; if (r.prob_OnCompleteMagicLesson)   ++m_onMagicLessonCnt;
                if (!NEXTI(r.prob_OnCompleteTheologyLesson))break; if (r.prob_OnCompleteTheologyLesson)++m_onTheologyLessonCnt;
                if (!NEXTI(r.prob_OnEnchantItem))           break; if (r.prob_OnEnchantItem)           ++m_onEnchantCnt;

                // 另一個任務代碼（完成即獲得）
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.questKindOnComplete = cltQuestKindInfo::TranslateKindCode(t);
                if (r.questKindOnComplete) ++m_onCompleteQuestCnt;

                if (!NEXTI(r.prob_OnKillBossMonster))       break; if (r.prob_OnKillBossMonster)       ++m_onKillBossCnt;
                if (!NEXTI(r.prob_OnCompleteCircleQuest))   break; if (r.prob_OnCompleteCircleQuest)   ++m_onCircleQuestCnt;
                if (!NEXTI(r.prob_OnSellItemToNPC))         break; if (r.prob_OnSellItemToNPC)         ++m_onSellToNPCCnt;
                if (!NEXTI(r.prob_OnMultiAttack))           break; if (r.prob_OnMultiAttack)           ++m_onMultiAttackCnt;
                if (!NEXTI(r.prob_OnChangeClass))           break; if (r.prob_OnChangeClass)           ++m_onChangeClassCnt;

                // 大量數值欄位（+260..+384），依原碼寫入目標位移；這裡逐項吃掉並寫到 statsBlock 對應位置即可。
                auto NEXTI_AT = [&](size_t byteOffset)->bool {
                    char* x = std::strtok(nullptr, kDelims);
                    if (!x || !IsDigitStr(x)) return false;
                    int v = std::atoi(x);
                    // byteOffset 是相對於整筆骨架的實際位移
                    std::memcpy(reinterpret_cast<char*>(&r) + byteOffset, &v, sizeof(int32_t));
                    return true;
                    };

                // 依反編譯寫入位移：
                if (!NEXTI_AT(260)) break;      // +260  例：追加經驗(百分比) 等
                if (!NEXTI_AT(312)) break;      // +312
                if (!NEXTI_AT(316)) break;      // +316
                if (!NEXTI_AT(320)) break;      // +320
                if (!NEXTI_AT(324)) break;      // +324
                if (!NEXTI_AT(268)) break;      // +268
                if (!NEXTI_AT(264)) break;      // +264
                if (!NEXTI_AT(272)) break;      // +272
                if (!NEXTI_AT(336)) break;      // +336
                if (!NEXTI_AT(340)) break;      // +340
                if (!NEXTI_AT(328)) break;      // +328
                if (!NEXTI_AT(332)) break;      // +332
                if (!NEXTI_AT(344)) break;      // +344
                if (!NEXTI_AT(348)) break;      // +348
                if (!NEXTI_AT(276)) break;      // +276
                if (!NEXTI_AT(280)) break;      // +280
                if (!NEXTI_AT(284)) break;      // +284
                if (!NEXTI_AT(288)) break;      // +288
                if (!NEXTI_AT(292)) break;      // +292
                if (!NEXTI_AT(296)) break;      // +296
                if (!NEXTI_AT(300)) break;      // +300
                if (!NEXTI_AT(304)) break;      // +304
                if (!NEXTI_AT(308)) break;      // +308
                if (!NEXTI_AT(352)) break;      // +352
                if (!NEXTI_AT(356)) break;      // +356
                if (!NEXTI_AT(360)) break;      // +360
                if (!NEXTI_AT(364)) break;      // +364
                if (!NEXTI_AT(368)) break;      // +368
                if (!NEXTI_AT(372)) break;      // +372
                if (!NEXTI_AT(376)) break;      // +376
                if (!NEXTI_AT(380)) break;      // +380
                if (!NEXTI_AT(384)) break;      // +384 → r.stat384

                // 其餘末端欄位
                // +392：訓練卡 item kind（5碼→16bit）
                t = std::strtok(nullptr, kDelims); if (!t) break;
                r.trainingCardKind = cltItemKindInfo::TranslateKindCode(t);

                // +396
                if (!NEXTI(r.publicMonsterDiscount)) break;

                // +400
                if (!NEXTI(r.itemSellPriceDiscount)) break;

                // +388（注意反編譯寫入順序）
                if (!NEXTI(r.skillAoEIncrease)) break;

                // +404
                if (!NEXTI(r.circleScore)) break;

                // +408（WORD）
                t = std::strtok(nullptr, kDelims); if (!t || !IsDigitStr(t)) break;
                r.purchaseCondTextCode = static_cast<uint16_t>(std::atoi(t));

                // 讀下一列
                ++idx;
                if (!std::fgets(line, sizeof(line), f)) { ok = 1; break; }
            } while (idx < m_count);
        }

        // 只有當正常讀至 EOF 時才建立分類清單
        if (ok) {
            // 依各自的 count 配置陣列
            auto NEW_LIST = [&](strEmblemKindInfo*** dst, int cnt) {
                if (cnt > 0) *dst = static_cast<strEmblemKindInfo**>(operator new(cnt * sizeof(void*)));
                };

            NEW_LIST(&m_onDead, m_onDeadCnt);
            NEW_LIST(&m_onUseHP, m_onUseHPCnt);
            NEW_LIST(&m_onUseMP, m_onUseMPCnt);
            NEW_LIST(&m_onBuyNPC, m_onBuyNPCCnt);
            NEW_LIST(&m_onRegistry, m_onRegistryCnt);
            NEW_LIST(&m_onCompleteQuest, m_onCompleteQuestCnt);
            NEW_LIST(&m_onKillBoss, m_onKillBossCnt);
            NEW_LIST(&m_onTeleportDragon, m_onTeleportDragonCnt);
            NEW_LIST(&m_onCompleteMeritous, m_onCompleteMeritousCnt);
            NEW_LIST(&m_onSwordLesson, m_onSwordLessonCnt);
            NEW_LIST(&m_onBowLesson, m_onBowLessonCnt);
            NEW_LIST(&m_onMagicLesson, m_onMagicLessonCnt);
            NEW_LIST(&m_onTheologyLesson, m_onTheologyLessonCnt);
            NEW_LIST(&m_onEnchant, m_onEnchantCnt);
            NEW_LIST(&m_onCircleQuest, m_onCircleQuestCnt);
            NEW_LIST(&m_onSellToNPC, m_onSellToNPCCnt);
            NEW_LIST(&m_onMultiAttack, m_onMultiAttackCnt);
            NEW_LIST(&m_onBeCrit, m_onBeCritCnt);
            NEW_LIST(&m_onDoCrit, m_onDoCritCnt);
            NEW_LIST(&m_onChangeClass, m_onChangeClassCnt);

            // 重新走訪，填入指標
            int cOnDead = 0, cOnHP = 0, cOnMP = 0, cOnBuy = 0, cOnReg = 0, cOnCQ = 0, cOnBoss = 0, cOnTD = 0, cOnMer = 0,
                cSW = 0, cBW = 0, cMG = 0, cTH = 0, cEN = 0, cCCQ = 0, cSell = 0, cMA = 0, cBeC = 0, cDoC = 0, cChg = 0;

            for (int i = 0; i < m_count; ++i) {
                strEmblemKindInfo* p = &m_list[i];
                if (p->prob_OnDead)                 m_onDead[cOnDead++] = p;
                if (p->prob_OnUsingRecoverHPItem)   m_onUseHP[cOnHP++] = p;
                if (p->prob_OnUsingRecoverManaItem) m_onUseMP[cOnMP++] = p;
                if (p->prob_OnBuyItemFromNPC)       m_onBuyNPC[cOnBuy++] = p;
                if (p->prob_OnRegistrySellingAgency)m_onRegistry[cOnReg++] = p;
                if (p->questKindOnComplete)         m_onCompleteQuest[cOnCQ++] = p;
                if (p->prob_OnKillBossMonster)      m_onKillBoss[cOnBoss++] = p;
                if (p->prob_OnTeleportDragon)       m_onTeleportDragon[cOnTD++] = p;
                if (p->prob_OnCompleteMeritous)     m_onCompleteMeritous[cOnMer++] = p;
                if (p->prob_OnCompleteSwordLesson)  m_onSwordLesson[cSW++] = p;
                if (p->prob_OnCompleteBowLesson)    m_onBowLesson[cBW++] = p;
                if (p->prob_OnCompleteMagicLesson)  m_onMagicLesson[cMG++] = p;
                if (p->prob_OnCompleteTheologyLesson)m_onTheologyLesson[cTH++] = p;
                if (p->prob_OnEnchantItem)          m_onEnchant[cEN++] = p;
                if (p->prob_OnCompleteCircleQuest)  m_onCircleQuest[cCCQ++] = p;
                if (p->prob_OnSellItemToNPC)        m_onSellToNPC[cSell++] = p;
                if (p->prob_OnMultiAttack)          m_onMultiAttack[cMA++] = p;
                if (p->prob_OnBeAttackedCritically) m_onBeCrit[cBeC++] = p;
                if (p->prob_OnAttackCritically)     m_onDoCrit[cDoC++] = p;
                if (p->prob_OnChangeClass)          m_onChangeClass[cChg++] = p;
            }
        }
    }

    g_clTextFileManager.fclose(Stream);
    if (!ok) { Free(); }
    return ok;
}

// -------- 查詢/工具 ------------------------------------------------------
strEmblemKindInfo* cltEmblemKindInfo::GetEmblemKindInfo(uint16_t a2)
{
    if (m_count <= 0 || !m_list) return nullptr;
    // 反編譯：以 WORD 為單位，每筆 208 WORD (=416 bytes)
    uint16_t* base = reinterpret_cast<uint16_t*>(m_list);
    for (int i = 0; i < m_count; ++i, base += 208) {
        if (base[0] == a2)
            return reinterpret_cast<strEmblemKindInfo*>(base);
    }
    return nullptr;
}

uint16_t cltEmblemKindInfo::TranslateKindCode(char* a1)
{
    if (!a1 || std::strlen(a1) != 5) return 0;
    int hi = (std::toupper(static_cast<unsigned char>(a1[0])) + 31) << 11;
    int lo = std::atoi(a1 + 1);
    return (lo < 0x800) ? static_cast<uint16_t>(hi | lo) : 0;
}

int cltEmblemKindInfo::GetBuyableEmblems(uint16_t* a2)
{
    int cnt = 0;
    if (m_count <= 0 || !m_list) return 0;

    for (int i = 0; i < m_count; ++i) {
        char first = m_list[i].condMonsterType[0];
        // 依反編譯邏輯：滿足任一條件即加入
        if (m_list[i].acquireLevelReach ||
            m_list[i].acquireQuestKind ||
            m_list[i].classAtbLow || m_list[i].classAtbHigh ||
            first != '0')
        {
            if (a2) *a2++ = m_list[i].kind;
            ++cnt;
        }
    }
    return cnt;
}

// 類別化陣列取回（與反編譯介面一致：*out 指向內部陣列，回傳數量）
int cltEmblemKindInfo::GetEmblemKindInfo_OnDead(strEmblemKindInfo*** a2) { *a2 = m_onDead;              return m_onDeadCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnUsingRecoverHPItem(strEmblemKindInfo*** a2) { *a2 = m_onUseHP;             return m_onUseHPCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnUsingRecoverManaItem(strEmblemKindInfo*** a2) { *a2 = m_onUseMP;             return m_onUseMPCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnBuyItemFromNPC(strEmblemKindInfo*** a2) { *a2 = m_onBuyNPC;            return m_onBuyNPCCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnRegistrySellingAgency(strEmblemKindInfo*** a2) { *a2 = m_onRegistry;          return m_onRegistryCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteQuest(strEmblemKindInfo*** a2) { *a2 = m_onCompleteQuest;     return m_onCompleteQuestCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnKillBossMonster(strEmblemKindInfo*** a2) { *a2 = m_onKillBoss;          return m_onKillBossCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnTeleportDragon(strEmblemKindInfo*** a2) { *a2 = m_onTeleportDragon;    return m_onTeleportDragonCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteMeritous(strEmblemKindInfo*** a2) { *a2 = m_onCompleteMeritous;  return m_onCompleteMeritousCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteSwordLesson(strEmblemKindInfo*** a2) { *a2 = m_onSwordLesson;       return m_onSwordLessonCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteBowLesson(strEmblemKindInfo*** a2) { *a2 = m_onBowLesson;         return m_onBowLessonCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteMagicLesson(strEmblemKindInfo*** a2) { *a2 = m_onMagicLesson;       return m_onMagicLessonCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteTheologyLesson(strEmblemKindInfo*** a2) { *a2 = m_onTheologyLesson;   return m_onTheologyLessonCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnEnchantItem(strEmblemKindInfo*** a2) { *a2 = m_onEnchant;           return m_onEnchantCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnCompleteCircleQuest(strEmblemKindInfo*** a2) { *a2 = m_onCircleQuest;       return m_onCircleQuestCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnSellItemToNPC(strEmblemKindInfo*** a2) { *a2 = m_onSellToNPC;         return m_onSellToNPCCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnMultiAttack(strEmblemKindInfo*** a2) { *a2 = m_onMultiAttack;       return m_onMultiAttackCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnBeAttackedCritically(strEmblemKindInfo*** a2) { *a2 = m_onBeCrit;            return m_onBeCritCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnAttackCritically(strEmblemKindInfo*** a2) { *a2 = m_onDoCrit;            return m_onDoCritCnt; }
int cltEmblemKindInfo::GetEmblemKindInfo_OnChangeClass(strEmblemKindInfo*** a2) { *a2 = m_onChangeClass;       return m_onChangeClassCnt; }
