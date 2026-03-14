#include "Logic/cltCashShopItem.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

DCTTextManager* cltCashShopItem::m_pDCTTextManager = nullptr;

stCashShopItemList::stCashShopItemList() = default;

stCashShopItemList::~stCashShopItemList() {
    for (auto*& p : items) {
        delete p;
        p = nullptr;
    }
}

void cltCashShopItem::InitializeStaticVariable(DCTTextManager* textMgr) {
    m_pDCTTextManager = textMgr;
}

cltCashShopItem::cltCashShopItem() {
    counters_.fill(0);
    deleteSellIDs_.reserve(10);
    totalRows_ = 0;
}

cltCashShopItem::~cltCashShopItem() {
    Free();
}

int cltCashShopItem::Initialize(char* fileName) {
    if (!fileName) return 0;

    FILE* fp = g_clTextFileManager.fopen(fileName);
    if (!fp) return 0;

    Free();
    counters_.fill(0);
    totalRows_ = 0;

    char buffer[4096]{};
    char delim[] = "\t\n";

    if (!std::fgets(buffer, sizeof(buffer), fp) ||
        !std::fgets(buffer, sizeof(buffer), fp) ||
        !std::fgets(buffer, sizeof(buffer), fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    fpos_t pos{};
    std::fgetpos(fp, &pos);

    std::uint16_t rowCount = 0;
    while (std::fgets(buffer, sizeof(buffer), fp)) {
        ++rowCount;
    }

    items_.assign(rowCount, stCashShopItemList{});
    totalRows_ = rowCount;
    std::fsetpos(fp, &pos);

    std::size_t row = 0;
    while (row < items_.size() && std::fgets(buffer, sizeof(buffer), fp)) {
        stCashShopItemList& out = items_[row];

        char* tok = std::strtok(buffer, delim);
        if (!tok) break;
        out.sellID = std::atol(tok);

        (void)std::strtok(nullptr, delim); // skip col
        tok = std::strtok(nullptr, delim);
        if (!tok) break;

        const int yn = std::toupper(static_cast<unsigned char>(*tok));
        if (yn == 'N') {
            deleteSellIDs_.push_back(out.sellID);
            continue;
        }
        if (yn == 'Y') out.saleFlag = 1;
        else if (yn == 'C') out.saleFlag = 2;
        else {
            MessageBoxA(0, "FATAL FILE ERROR CashShopList-Y/N", "warning", 0);
            break;
        }

        tok = std::strtok(nullptr, delim); if (!tok) break;
        if (!ParsingItemType(&out, tok)) break;

        tok = std::strtok(nullptr, delim); if (!tok) break;
        out.unk6 = static_cast<std::uint16_t>(std::atoi(tok));

        tok = std::strtok(nullptr, delim); if (!tok) break;
        out.textCode = static_cast<std::uint16_t>(std::atol(tok));

        tok = std::strtok(nullptr, delim); if (!tok) break;
        out.descCode = static_cast<std::uint16_t>(std::atol(tok));

        tok = std::strtok(nullptr, delim); if (!tok) break;
        std::sscanf(tok, "%x", &out.classMask);

        tok = std::strtok(nullptr, delim); if (!tok) break;
        out.levelLimit = static_cast<std::uint16_t>(std::atoi(tok));

        tok = std::strtok(nullptr, delim); if (!tok) break;
        out.periodLimit = std::atol(tok);

        tok = std::strtok(nullptr, delim); if (!tok) break;
        out.packagePrice = std::atol(tok);

        out.items[0] = new stCashShopItem{};

        int price = 0;
        tok = std::strtok(nullptr, delim); if (!tok) break;
        if (!ParsingPrice(tok, &price, 0)) { MessageBoxA(0, "FATAL FILE ERROR CashShopList-F_PRICE", "warning", 0); break; }
        out.items[0]->fPrice = price;

        tok = std::strtok(nullptr, delim); if (!tok) break;
        if (!ParsingPrice(tok, &price, 0)) { MessageBoxA(0, "FATAL FILE ERROR CashShopList-Y_PRICE", "warning", 0); break; }
        out.items[0]->yPrice = price;

        tok = std::strtok(nullptr, delim); if (!tok) break;
        if (!ParsingPrice(tok, &price, 0)) { MessageBoxA(0, "FATAL FILE ERROR CashShopList-M_PRICE", "warning", 0); break; }
        out.items[0]->mPrice = price;

        tok = std::strtok(nullptr, delim); if (!tok) break;
        if (!ParsingPrice(tok, &price, 0)) { MessageBoxA(0, "FATAL FILE ERROR CashShopList-W_PRICE", "warning", 0); break; }
        out.items[0]->wPrice = price;

        tok = std::strtok(nullptr, delim); if (!tok) break;
        if (!ParsingPrice(tok, &price, 0)) { MessageBoxA(0, "FATAL FILE ERROR CashShopList-D_PRICE", "warning", 0); break; }
        out.items[0]->dPrice = price;

        std::uint8_t listRows = 0;
        for (; listRows < 9; ++listRows) {
            if (listRows > 0) {
                if (!IsPackageItem(out.itemType)) break;
                out.items[listRows] = new stCashShopItem{};
            }
            stCashShopItem* item = out.items[listRows];
            if (!item) break;

            tok = std::strtok(nullptr, delim); if (!tok) goto END_PARSE;
            item->itemKind[4] = TranslateKindCode(tok); // F
            tok = std::strtok(nullptr, delim); if (!tok) goto END_PARSE;
            item->itemKind[3] = TranslateKindCode(tok); // Y
            tok = std::strtok(nullptr, delim); if (!tok) goto END_PARSE;
            item->itemKind[2] = TranslateKindCode(tok); // M
            tok = std::strtok(nullptr, delim); if (!tok) goto END_PARSE;
            item->itemKind[1] = TranslateKindCode(tok); // W
            tok = std::strtok(nullptr, delim); if (!tok) goto END_PARSE;
            item->itemKind[0] = TranslateKindCode(tok); // D

            if (!CheckPeriodByItemKind(item)) {
                MessageBoxA(0, "FATAL FILE ERROR CashShopList-PERIOD", "warning", 0);
                goto END_PARSE;
            }

            (void)std::strtok(nullptr, delim);
            tok = std::strtok(nullptr, delim);
            if (!tok || !IsDigit(tok)) goto END_PARSE;
            item->qty = static_cast<std::uint16_t>(std::atoi(tok));

            out.itemRows = static_cast<std::uint8_t>(listRows + 1);
        }

        if (!IsPackageItem(out.itemType)) {
            int p = 0;
            while (p < 5 && out.items[0]->itemKind[p] == 0) ++p;
            if (p < 5) {
                stItemKindInfo* kind = g_clItemKindInfo.GetItemKindInfo(out.items[0]->itemKind[p]);
                if (kind) out.textCode = kind->m_wTextCode;
            }
        }

        ++row;
    }

END_PARSE:
    for (std::size_t i = 3; i <= 26; ++i) counters_[i] = static_cast<std::uint16_t>(counters_[i] + counters_[i - 1]);

    int ok = CheckCopyItemExistInNonCopyItem();
    if (!ok) MessageBoxA(0, "FATAL FILE ERROR CashShopList-COPYITEM", "warning", 0);

    g_clTextFileManager.fclose(fp);
    return ok;
}

void cltCashShopItem::Free() {
    items_.clear();
}

int cltCashShopItem::CheckPeriodByItemKind(stCashShopItem* a2) {
    if (!a2) return 0;

    for (int i = 0; i < 5; ++i) {
        stItemKindInfo* info = g_clItemKindInfo.GetItemKindInfo(a2->itemKind[i]);
        if (!info) continue;

        const std::uint16_t term = info->m_wUseTerm;
        switch (i) {
            case 0: if (term == 1) continue; break;
            case 1: if (term == 7) continue; break;
            case 2: if (term == 30 || term == 7) continue; break;
            case 3: if (term == 365 || term == 90 || term == 30) continue; break;
            case 4: if (term == 0) continue; break;
            default: break;
        }
        return 0;
    }
    return 1;
}

int cltCashShopItem::ParsingPrice(char* str, int* outPrice, int strictPositive) {
    if (!outPrice || !str) return 0;
    if (!IsDigit(str)) return 0;

    *outPrice = std::atol(str);
    if (*outPrice < 0) return 0;

    if (strictPositive) {
        if (!*outPrice) return 0;
    } else {
        if (!*outPrice) return 1;
    }
    return *outPrice <= 1000000;
}

int cltCashShopItem::CheckCopyItemExistInNonCopyItem() {
    int found = 0;
    if (!counters_[3]) return 1;

    for (std::size_t i = 0; i < items_.size(); ++i) {
        if (!GetCashShopItemBySellID(items_[i].sellID)) break;
        ++found;
        if (found >= counters_[3]) return 1;
    }
    return 0;
}

int cltCashShopItem::ParsingItemType(stCashShopItemList* a2, char* s) {
    if (!a2 || !s) return 0;

    static const char* kNames[] = {
        "HOT_N_NEW_HOT", "HOT_N_NEW_NEW", "PACKAGE_SALE", "PACKAGE_SPECIAL",
        "FASHION_FULLSET", "FASHION_CAP", "FASHION_EYE", "FASHION_FACE", "FASHION_HAIR_ACC",
        "FASHION_MANTEAU", "FASHION_SHIRTS", "FASHION_PANTS", "FASHION_GLOVE", "FASHION_SHOES",
        "FASHION_ETC", "PET_EGG", "PET_FEED", "PET_SKILL", "PET_DYE", "PET_ONETIME",
        "ONETIME_FUNCTIONAL", "ONETIME_AVATAR", "ONETIME_EMOTICON", "ONETIME_ETC", "ONETIME_DYE"
    };

    for (std::uint8_t i = 0; i < 25; ++i) {
        if (!_stricmp(s, kNames[i])) {
            a2->itemType = i;
            if (i + 2 < counters_.size()) ++counters_[i + 2];
            return 1;
        }
    }
    return 0;
}

std::uint16_t cltCashShopItem::TranslateKindCode(char* a1) {
    if (!a1 || std::strlen(a1) != 5) return 0;

    const int v2 = (std::toupper(static_cast<unsigned char>(*a1)) + 31) << 11;
    const auto v3 = static_cast<std::uint16_t>(std::atoi(a1 + 1));
    if (v3 >= 0x800u) return 0;
    return static_cast<std::uint16_t>(v2 | v3);
}

stCashShopItemList* cltCashShopItem::GetItemList(std::uint8_t a2, std::uint16_t* a3) {
    if (a2 > 0x19 || !a3) return nullptr;

    if (!a2) {
        *a3 = counters_[2];
        return items_.empty() ? nullptr : &items_[0];
    }

    *a3 = static_cast<std::uint16_t>(counters_[a2 + 2] - counters_[a2 + 1]);
    const std::size_t start = counters_[a2 + 1];
    if (start >= items_.size()) return nullptr;
    return &items_[start];
}

unsigned int cltCashShopItem::GetFashionType(unsigned int a2) {
    if (a2 > 0x20) {
        if (a2 > 0x100) {
            if (a2 == 1024) return 8;
        } else {
            if (a2 == 0x100) return 4;
            if (a2 == 0x40) return 12;
            if (a2 == 0x80) return 13;
        }
        return 14;
    }
    if (a2 == 32) return 11;
    switch (a2) {
        case 1: return 5;
        case 2: return 6;
        case 4: return 7;
        case 8: return 9;
        case 0x10: return 10;
        default: return 14;
    }
}

int cltCashShopItem::FindCashItemListFileError(std::uint8_t) {
    return 1;
}

stCashShopItemList* cltCashShopItem::GetCashShopItemBySellID(int a2) {
    const std::uint16_t start = counters_[3];
    const std::uint16_t end = counters_[26];
    if (start >= end) return nullptr;

    for (std::uint16_t i = start; i < end && i < items_.size(); ++i) {
        if (items_[i].sellID == a2) return &items_[i];
    }
    return nullptr;
}

int cltCashShopItem::GetTotalPrice(int* a2, std::uint8_t a3, std::uint8_t a4) {
    if (!a2) return 0;
    if (!a3 || a4 >= 5) return 0;

    int total = 0;
    std::uint8_t found = 0;

    for (std::uint16_t i = counters_[3]; i < counters_[26] && i < items_.size() && found != a3; ++i) {
        for (std::uint8_t k = 0; k < a3; ++k) {
            if (items_[i].sellID != a2[k]) continue;
            int add = 0;
            if (IsPackageItem(items_[i].itemType)) {
                add = items_[i].packagePrice;
            } else if (items_[i].items[0]) {
                switch (a4) {
                    case 0: add = items_[i].items[0]->dPrice; break;
                    case 1: add = items_[i].items[0]->wPrice; break;
                    case 2: add = items_[i].items[0]->mPrice; break;
                    case 3: add = items_[i].items[0]->yPrice; break;
                    case 4: add = items_[i].items[0]->fPrice; break;
                    default: break;
                }
            }
            total += add;
            ++found;
        }
    }

    return (found == a3) ? total : 0;
}

int cltCashShopItem::GetCompositionItemData(char* a2, int* a3, std::uint8_t a4, std::uint8_t a5, int) {
    if (!m_pDCTTextManager || !a2 || !a3 || !a4 || *a2 || a5 >= 5) return 0;

    char period = 'D';
    switch (a5) {
        case 0: period = 'D'; break;
        case 1: period = 'W'; break;
        case 2: period = 'M'; break;
        case 3: period = 'Y'; break;
        case 4: period = 'U'; break;
        default: break;
    }

    std::uint8_t done = 0;
    for (std::uint16_t i = counters_[3]; i < counters_[26] && i < items_.size() && done != a4; ++i) {
        for (std::uint8_t k = 0; k < a4 && done != a4; ++k) {
            if (items_[i].sellID != a3[k]) continue;
            const std::uint8_t t = items_[i].itemType;
            if (t == 0 || t == 1) continue;
            if (*a2) std::strcat(a2, ":");
            const char mode = (t == 2 || t == 3) ? 'P' : 'N';
            char tmp[2048]{};
            std::sprintf(tmp, "%c%c%d", mode, period, items_[i].sellID);
            std::strcat(a2, tmp);
            ++done;
        }
    }

    return done == a4;
}

stItemKindInfo* cltCashShopItem::IsFashionItem(stCashShopItemList* a2) {
    if (!a2) return nullptr;

    for (int row = 0; row < 9; ++row) {
        stCashShopItem* p = a2->items[row];
        if (!p) continue;
        for (int i = 0; i < 5; ++i) {
            if (!p->itemKind[i]) continue;
            if (g_clItemKindInfo.IsFashionItem(p->itemKind[i])) {
                return g_clItemKindInfo.GetItemKindInfo(p->itemKind[i]);
            }
            return nullptr;
        }
    }
    return nullptr;
}

BOOL cltCashShopItem::IsPackageItem(std::uint8_t a2) {
    return (a2 >= 2u && a2 <= 3u) ? TRUE : FALSE;
}

int cltCashShopItem::IsInDeleteSellIDList(int a2) {
    for (std::size_t i = 0; i < deleteSellIDs_.size(); ++i) {
        if (deleteSellIDs_[i] == a2) return 1;
    }
    return 0;
}

int cltCashShopItem::GetDelegatePeriod(stCashShopItemList* a2) {
    if (!a2) return 4;

    if (IsPackageItem(a2->itemType)) {
        for (int row = 0; row < 9; ++row) {
            stCashShopItem* p = a2->items[row];
            if (!p) continue;
            for (int i = 0; i < 5; ++i) {
                if (g_clItemKindInfo.IsFashionItem(p->itemKind[i])) return i;
            }
        }
        return 4;
    }

    stCashShopItem* p = a2->items[0];
    if (!p) return 4;
    for (int i = 0; i < 5; ++i) {
        if (p->itemKind[i]) return i;
    }
    return 4;
}

std::uint16_t cltCashShopItem::GetDelegateItemKind(stCashShopItemList* a2) {
    if (!a2) return 0;

    const int period = GetDelegatePeriod(a2);
    if (!IsPackageItem(a2->itemType)) {
        return a2->items[0] ? a2->items[0]->itemKind[period] : 0;
    }

    for (int row = 0; row < 9; ++row) {
        stCashShopItem* p = a2->items[row];
        if (!p) continue;
        if (p->itemKind[period]) {
            g_clItemKindInfo.IsFashionItem(p->itemKind[period]);
            return p->itemKind[period];
        }
    }
    return 0;
}

int cltCashShopItem::GetValidPeriodList(stCashShopItemList* a2, int* const a3) {
    if (!a2 || !a3 || !a2->items[0]) return 0;

    int n = 0;
    for (int i = 0; i < 5; ++i) {
        if (a2->items[0]->itemKind[i]) {
            a3[n++] = i;
        }
    }
    return n;
}

stCashShopItemList* cltCashShopItem::GetCashShopItemByIndex(std::uint16_t a2) {
    if (a2 > totalRows_ || a2 >= items_.size()) return nullptr;
    return &items_[a2];
}

void cltCashShopItem::DeleteSpecialItem() {
    std::uint16_t i = counters_[3];
    while (i < counters_[26] && i < items_.size()) {
        if (items_[i].saleFlag == 2) {
            const std::uint8_t t = items_[i].itemType;
            if (t < 24) {
                for (std::size_t w = static_cast<std::size_t>(t) + 2; w <= 26; ++w) {
                    --counters_[w];
                }
            }
            items_.erase(items_.begin() + i);
            --counters_[26];
            if (totalRows_ > 0) --totalRows_;
            continue;
        }
        ++i;
    }
}
