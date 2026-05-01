#include "Info/cltShopInfo.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <windows.h>

#include "Text/cltTextFileManager.h"
#include "Info/cltItemKindInfo.h"
#include "global.h"

namespace {
// 反編譯中所有解析皆以 "\t\n" 為 strtok 分隔字元。
constexpr char kDelim[] = "\t\n";
constexpr int  kBufferSize = 4096;
}

// GT 全域 ::IsDigit (mofclient.c:342909 / sub_0059FEA0) 等價：
//   · 첫 byte == 0 → return 1 (true)
//   · 매 반복마다 한 번 '+' / '-' 를 건너뜀
//   · 그 외 한 글자라도 isdigit 실패 시 false
bool cltShopInfo::IsDigit(const char* a1) {
    if (!a1) return false;
    const char* v1 = a1;
    if (!*v1) return true;
    while (true) {
        if (*v1 == '+' || *v1 == '-') ++v1;
        if (!std::isdigit(static_cast<unsigned char>(*v1))) return false;
        ++v1;
        if (!*v1) return true;
    }
}

// 0x590910
// GT: 只清零 m_pShopList / m_wTotalShopNum / m_wCurItemCount 三個欄位 (offsets 0/4/6)。
//     m_pPackageShopList、m_iTotalPackageShopNum 不在 ctor 內初始化 — 仰賴 BSS 全域零初始化。
cltShopInfo::cltShopInfo() {
    m_pShopList     = nullptr;
    m_wTotalShopNum = 0;
    m_wCurItemCount = 0;
}

// 0x590940
int cltShopInfo::Initialize(char* shopFile, char* packageShopFile) {
    int result = Initialize_Shop(shopFile);
    if (result) {
        result = Initialize_PackageShop(packageShopFile) != 0;
    }
    return result;
}

// 0x590970
int cltShopInfo::Initialize_Shop(char* fileName) {
    char buffer[kBufferSize]{};
    int  success = 0;

    m_wCurItemCount = 0;

    FILE* fp = g_clTextFileManager.fopen(fileName);
    if (!fp) {
        return 0;
    }

    // 跳過前三行：title comment / blank / 欄位 header
    if (!std::fgets(buffer, kBufferSize, fp) ||
        !std::fgets(buffer, kBufferSize, fp) ||
        !std::fgets(buffer, kBufferSize, fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    // 紀錄資料起點，先掃一次計算 row 數
    fpos_t position{};
    std::fgetpos(fp, &position);
    while (std::fgets(buffer, kBufferSize, fp)) {
        ++m_wTotalShopNum;
    }

    // GT: operator new(1012 * count) + memset(0, 4*((1012*N)>>2))。stShopInfo 為 POD，
    //     1012 為 4 的倍數，因此 memset 全幅清零。
    m_pShopList = static_cast<stShopInfo*>(::operator new(sizeof(stShopInfo) * m_wTotalShopNum));
    std::memset(m_pShopList, 0, sizeof(stShopInfo) * m_wTotalShopNum);

    // 倒回資料起點
    std::fsetpos(fp, &position);

    stShopInfo* row = m_pShopList;
    if (std::fgets(buffer, kBufferSize, fp)) {
        while (true) {
            // [1] ShopID
            char* tok = std::strtok(buffer, kDelim);
            if (!tok) {
                goto fail;
            }
            std::uint16_t shopID = TranslateKindCode(tok);
            row->ShopID = shopID;
            if (!shopID) {
                goto fail;
            }

            // [2] NPC 名稱（不儲存，僅須存在）
            if (!std::strtok(nullptr, kDelim)) {
                goto fail;
            }

            // [3] 稅率（必須是純數字且 >= 1000）
            char* taxStr = std::strtok(nullptr, kDelim);
            if (!taxStr) {
                goto fail;
            }
            if (!IsDigit(taxStr)) {
                goto fail;
            }
            int tax = std::atoi(taxStr);
            row->TaxRate = tax;
            if (tax < 1000) {
                goto fail;
            }

            // [4..] 最多 500 個物品 slot；指標每次都遞增（保留稀疏 0）
            std::uint16_t* slot = row->Items;
            for (int i = 0; i < 500; ++i) {
                char* itemTok = std::strtok(nullptr, kDelim);
                if (itemTok && std::strcmp(itemTok, "0") != 0) {
                    ++m_wCurItemCount;
                    std::uint16_t code = TranslateKindCode(itemTok);
                    *slot = code;
                    if (!g_clItemKindInfo.GetItemKindInfo(code)) {
                        MessageBoxA(nullptr, "Item Code Error", "ShopList.txt", 0);
                        goto fail;
                    }
                    row->ItemCount = m_wCurItemCount;
                }
                ++slot;
            }

            // 為下一行準備：前進到下一筆 row、重置暫存計數
            ++row;
            m_wCurItemCount = 0;

            if (!std::fgets(buffer, kBufferSize, fp)) {
                break;  // 檔案讀完，正常結束
            }
        }
    }
    success = 1;

fail:
    g_clTextFileManager.fclose(fp);
    return success;
}

// 0x590C80
int cltShopInfo::Initialize_PackageShop(char* fileName) {
    char buffer[kBufferSize]{};
    int  success = 0;

    FILE* fp = g_clTextFileManager.fopen(fileName);
    if (!fp) {
        return 0;
    }

    // 同樣跳過三行 header
    if (!std::fgets(buffer, kBufferSize, fp) ||
        !std::fgets(buffer, kBufferSize, fp) ||
        !std::fgets(buffer, kBufferSize, fp)) {
        g_clTextFileManager.fclose(fp);
        return 0;
    }

    fpos_t position{};
    std::fgetpos(fp, &position);
    while (std::fgets(buffer, kBufferSize, fp)) {
        ++m_iTotalPackageShopNum;
    }

    // GT: operator new(404 * count) + memset(0, ...)
    m_pPackageShopList = static_cast<strPackageShopInfo*>(
        ::operator new(sizeof(strPackageShopInfo) * m_iTotalPackageShopNum));
    std::memset(m_pPackageShopList, 0, sizeof(strPackageShopInfo) * m_iTotalPackageShopNum);

    std::fsetpos(fp, &position);

    strPackageShopInfo* row = m_pPackageShopList;

    if (!std::fgets(buffer, kBufferSize, fp)) {
        // 沒有任何資料行也視為成功 (與反編譯 LABEL_23 行為一致)
        success = 1;
        g_clTextFileManager.fclose(fp);
        return success;
    }

    while (true) {
        // [1] ShopID
        char* tok = std::strtok(buffer, kDelim);
        if (!tok) {
            break;  // 空行 / 解析失敗
        }
        std::uint16_t shopID = TranslateKindCode(tok);
        row->ShopID = shopID;
        if (!shopID) {
            break;
        }

        // 套裝 ShopID 不可與普通 Shop list 重複
        if (m_wTotalShopNum > 0) {
            bool duplicate = false;
            for (int i = 0; i < m_wTotalShopNum; ++i) {
                if (m_pShopList[i].ShopID == shopID) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) {
                break;  // 反編譯：直接 break 到 fclose，留下 success=0
            }
        }

        // [2] NPC 名稱（不儲存，僅須存在）
        if (!std::strtok(nullptr, kDelim)) {
            break;
        }

        // [3..] 物品三元組 (Kind, Qty, Price) 重複；以 v13 = base+6 (= Items[0].ItemQty 位置) 開始
        char* kindStr = std::strtok(nullptr, kDelim);
        if (kindStr) {
            // pkr 對應反編譯之 v13；以 v13-1 寫 ItemKind、v13 寫 Qty、v13+1 寫 Price。
            // 每次 v13 += 4 等同於下一個 stPackageShopItem。
            stPackageShopItem* itemSlot = row->Items;
            do {
                itemSlot->ItemKind = cltItemKindInfo::TranslateKindCode(kindStr);

                char* qtyStr = std::strtok(nullptr, kDelim);
                if (!qtyStr) break;
                itemSlot->ItemQty = static_cast<std::uint16_t>(std::atoi(qtyStr));

                char* priceStr = std::strtok(nullptr, kDelim);
                if (!priceStr) break;
                itemSlot->Price = std::atoi(priceStr);

                ++itemSlot;
                kindStr = std::strtok(nullptr, kDelim);
            } while (kindStr);
        }

        // 前進到下一行
        ++row;
        if (!std::fgets(buffer, kBufferSize, fp)) {
            success = 1;  // 對應 LABEL_23
            break;
        }
    }

    g_clTextFileManager.fclose(fp);
    return success;
}

// 0x590F30
// GT: 配 operator new (raw)，故對應使用 operator delete (raw)。
//     Free 不重置 m_wCurItemCount (僅在 Initialize_Shop 內由 GT 主動清 0)。
void cltShopInfo::Free() {
    if (m_pShopList) {
        ::operator delete(m_pShopList);
        m_pShopList = nullptr;
    }
    if (m_pPackageShopList) {
        ::operator delete(m_pPackageShopList);
        m_pPackageShopList = nullptr;
    }
    m_wTotalShopNum        = 0;
    m_iTotalPackageShopNum = 0;
}

// 0x590F70
std::uint16_t cltShopInfo::GetTotalShopNum() {
    return m_wTotalShopNum;
}

// 0x590F80
stShopInfo* cltShopInfo::GetShopInfoByIndex(int index) {
    if (index < 0 || index >= m_wTotalShopNum) {
        return nullptr;
    }
    return &m_pShopList[index];
}

// 0x590FB0
// 編碼：strlen 必須 = 5；首字元 toupper；公式 ((c+31) << 11) | n，n = atoi(s+1) 須 < 0x800
// GT: 不對 a1 做 nullptr 檢查 (caller 須保證非 null；傳入 NULL 會在 strlen 內崩潰)。
std::uint16_t cltShopInfo::TranslateKindCode(char* code) {
    if (std::strlen(code) != 5) {
        return 0;
    }
    int high = (std::toupper(static_cast<unsigned char>(code[0])) + 31) << 11;
    int low  = std::atoi(code + 1);
    if (low >= 0x800) {
        return 0;
    }
    return static_cast<std::uint16_t>(high | low);
}

// 0x591000
int cltShopInfo::GetShopInfoByID(std::uint16_t shopId, stShopInfo** outShopInfo, strPackageShopInfo** outPackageInfo) {
    *outShopInfo    = nullptr;
    *outPackageInfo = nullptr;

    // 反編譯：先查普通 Shop list（注意：當 m_wTotalShopNum == 0 也會 fall-through 到 Package list）
    if (m_wTotalShopNum > 0) {
        for (int i = 0; i < m_wTotalShopNum; ++i) {
            if (m_pShopList[i].ShopID == shopId) {
                *outShopInfo = &m_pShopList[i];
                return 1;
            }
        }
    }

    // 再查 Package list
    if (m_iTotalPackageShopNum <= 0) {
        return 0;
    }
    for (int i = 0; i < m_iTotalPackageShopNum; ++i) {
        if (m_pPackageShopList[i].ShopID == shopId) {
            *outPackageInfo = &m_pPackageShopList[i];
            return 1;
        }
    }
    return 0;
}

// 0x5910B0
int cltShopInfo::IsPackageShop(std::uint16_t shopId) {
    if (m_iTotalPackageShopNum <= 0) {
        return 0;
    }
    for (int i = 0; i < m_iTotalPackageShopNum; ++i) {
        if (m_pPackageShopList[i].ShopID == shopId) {
            return 1;
        }
    }
    return 0;
}

// 0x5910F0
std::int64_t cltShopInfo::GetPackageItemPrice(std::uint16_t shopId, std::uint16_t itemKind, std::uint16_t itemQty) {
    if (m_iTotalPackageShopNum <= 0) {
        return 0;
    }
    for (int i = 0; i < m_iTotalPackageShopNum; ++i) {
        strPackageShopInfo* row = &m_pPackageShopList[i];
        if (row->ShopID != shopId) {
            continue;
        }
        // 在此 row 找 (kind, qty) 配對；上限 50（反編譯 v8 < 50）
        for (int j = 0; j < 50; ++j) {
            if (row->Items[j].ItemKind == itemKind && row->Items[j].ItemQty == itemQty) {
                return row->Items[j].Price;  // 反編譯：return *(int*) ；自然由 int 提升為 __int64
            }
        }
        // 反編譯：未命中時 goto LABEL_8 繼續找其他 row（以防同 ShopID 出現多次）
    }
    return 0;
}

// 0x591170
int cltShopInfo::IsShopSellItem(stShopInfo* shop, std::uint16_t itemKind, std::uint16_t itemQty) {
    // 反編譯特殊呼叫：以 (uint16_t)shop 當作 shopID 再查表，回填 shop 與 v9 (package)
    auto shopIdAsBits = static_cast<std::uint16_t>(reinterpret_cast<std::uintptr_t>(shop));
    stShopInfo*         hitShop    = nullptr;
    strPackageShopInfo* hitPackage = nullptr;
    if (!GetShopInfoByID(shopIdAsBits, &hitShop, &hitPackage)) {
        return 0;
    }
    if (hitShop) {
        // 普通 shop：循序比對 Items[]，遇到 0 即停止
        for (int i = 0; i < 500; ++i) {
            if (hitShop->Items[i] == 0) break;
            if (hitShop->Items[i] == itemKind) return 1;
        }
        return 0;
    }
    if (hitPackage) {
        // 反編譯：`while (*(_WORD *)v9)` — v9 不移動，等同於確認 ShopID 非 0；
        // 內部 v8 指針逐條前進，最多比對 50 條 (kind, qty)。
        if (hitPackage->ShopID != 0) {
            for (int i = 0; i < 50; ++i) {
                if (hitPackage->Items[i].ItemKind == itemKind &&
                    hitPackage->Items[i].ItemQty  == itemQty) {
                    return 1;
                }
            }
        }
    }
    return 0;
}
