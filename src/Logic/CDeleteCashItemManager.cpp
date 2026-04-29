// CDeleteCashItem / CDeleteCashItemManager — 1:1 還原 mofclient.c 0x4DA9F0~0x4DBA90
#include "Logic/CDeleteCashItemManager.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "global.h"
#include "Info/cltItemKindInfo.h"
#include "Logic/cltBaseInventory.h"
#include "System/cltEquipmentSystem.h"
#include "Text/DCTTextManager.h"
#include "UI/CMessageBoxManager.h"
#include "Util/ScriptParameter.h"
#include "Util/StringStream.h"

// 對齊 mofclient.c 0x21B8F04 / dword_21BA32C 的全域對應
extern cltBaseInventory   g_clMyInventory;
extern cltEquipmentSystem g_clEquipmentSystem;
extern cltEquipmentSystem* dword_21BA32C;
extern CMessageBoxManager* g_pMsgBoxMgr;
extern DCTTextManager      g_DCTTextManager;
extern cltItemKindInfo     g_clItemKindInfo;

// ============================================================================
// CDeleteCashItem
// ============================================================================

CDeleteCashItem::CDeleteCashItem()
    : m_pad0(0), m_pad1{0,0,0}
{
    // mofclient.c InitDeleteCashItem 用 operator new(0x10u) 配置 16 bytes，並把
    // *(_BYTE *)v2 = v3 (未定義之 stack v3，行為不依賴) — 我們留空。
}

CDeleteCashItem::~CDeleteCashItem()
{
    // std::vector 會自行釋放。
}

//----- (004DAA40) --------------------------------------------------------
void CDeleteCashItem::AddDeleteItem(uint16_t itemKind, int reason)
{
    strDeleteItem item{};
    item.itemKind = itemKind;
    item.reason   = (uint8_t)((reason >> 16) & 0xFF);  // 對齊 BYTE2(a3)
    item.pad      = 0;
    m_vDeleteList.push_back(item);
}

//----- (004DABF0) --------------------------------------------------------
void CDeleteCashItem::NoticeDeletedItem()
{
    // 從尾到頭逐一 pop 並顯示。
    while (!m_vDeleteList.empty()) {
        strDeleteItem cur = m_vDeleteList.back();
        if (cur.itemKind == 0) break;

        stItemKindInfo* info = g_clItemKindInfo.GetItemKindInfo(cur.itemKind);
        if (!info) break;

        // 依刪除原因取得文字（1→4146、2→4147、3→4148）
        char reasonBuf[256] = {0};
        int reasonTextId = 0;
        switch (cur.reason) {
            case 1: reasonTextId = 4146; break;
            case 2: reasonTextId = 4147; break;
            case 3: reasonTextId = 4148; break;
            default: break;
        }
        if (reasonTextId) {
            const char* s = g_DCTTextManager.GetText(reasonTextId);
            if (s) std::strcpy(reasonBuf, s);
        }

        // 建立 parameter map（對齊 mofclient.c 三組 Parameter0/1/2）
        std::map<std::string, std::string> params;
        params["Parameter0"] = reasonBuf;
        const char* itemName = g_DCTTextManager.GetText(info->m_wTextCode);
        params["Parameter1"] = itemName ? itemName : "";
        const char* prefix   = g_DCTTextManager.GetText(58164);
        params["Parameter2"] = prefix ? prefix : "";

        const char* tmpl = g_DCTTextManager.GetText(4149);
        std::string parsed = getScriptParameter(tmpl ? tmpl : "", params);

        char outBuf[256];
        if (tmpl && parsed == tmpl) {
            // 沒被替換：保留原 fallback (mofclient.c 走了 _sprintf(Buffer, v9))
            std::snprintf(outBuf, sizeof(outBuf), "%s", tmpl);
        } else {
            std::snprintf(outBuf, sizeof(outBuf), "%s", parsed.c_str());
        }

        // CMessageBoxManager::AddOK(this(=CUIBase*=0), char*, ...) — 採 (char*) overload
        if (g_pMsgBoxMgr) {
            g_pMsgBoxMgr->AddOK((CUIBase*)nullptr, outBuf, 0, 0, 0, -1, 0, 1, 0);
        }

        m_vDeleteList.pop_back();
    }
}

//----- (004DB2B0) --------------------------------------------------------
void CDeleteCashItem::NoticeTimeOutItem()
{
    struct stTimeOutItem {
        uint16_t itemKind;
        uint16_t pad;
        uint32_t timestamp;
        uint32_t useTerm;
    };
    std::vector<stTimeOutItem> tmpList;

    // 1) 掃裝備 [sex × slot]
    for (int sex = 0; sex < 2; ++sex) {
        for (int slot = 0; slot < 11; ++slot) {
            uint16_t kind = dword_21BA32C
                ? dword_21BA32C->GetEquipItem(sex, slot)
                : g_clEquipmentSystem.GetEquipItem(sex, slot);
            if (!kind) continue;
            stItemKindInfo* info = g_clItemKindInfo.GetItemKindInfo(kind);
            if (!info || info->m_wUseTerm == 0) continue;
            stTimeOutItem t;
            t.itemKind  = kind;
            t.pad       = 0;
            t.timestamp = dword_21BA32C
                ? dword_21BA32C->GetEquipItemTime(sex, slot)
                : g_clEquipmentSystem.GetEquipItemTime(sex, slot);
            t.useTerm   = info->m_wUseTerm;
            tmpList.push_back(t);
        }
    }

    // 2) 掃 inventory（255 slots）
    for (int slot = 0; slot < 255; ++slot) {
        strInventoryItem* inv = g_clMyInventory.GetInventoryItem((uint16_t)slot);
        if (!inv) continue;
        if (inv->itemKind == 0) continue;
        stItemKindInfo* info = g_clItemKindInfo.GetItemKindInfo(inv->itemKind);
        if (!info || info->m_wUseTerm == 0) continue;
        stTimeOutItem t;
        t.itemKind  = inv->itemKind;
        t.pad       = 0;
        t.timestamp = inv->value0;
        t.useTerm   = info->m_wUseTerm;
        tmpList.push_back(t);
    }

    if (tmpList.empty()) return;

    // 3) 掃 tmpList，找剩餘日數低於閾值的物品
    unsigned short firstTextCode = 0;
    int hitCount = 0;
    for (auto& t : tmpList) {
        unsigned int remain = ExGetTimeOutItemRemindTime(t.timestamp, t.itemKind);
        unsigned int threshold = (t.useTerm == 365) ? 7u : 3u;
        if (remain > threshold) continue;
        if (firstTextCode == 0) {
            stItemKindInfo* info = g_clItemKindInfo.GetItemKindInfo(t.itemKind);
            if (!info) continue;
            firstTextCode = info->m_wTextCode;
        }
        ++hitCount;
    }

    if (hitCount == 0 || firstTextCode == 0) return;

    char outBuf[256];
    if (hitCount == 1) {
        const char* itemName = g_DCTTextManager.GetText(firstTextCode);
        const char* tmpl     = g_DCTTextManager.GetText(58165);
        std::snprintf(outBuf, sizeof(outBuf),
                      tmpl ? tmpl : "%s",
                      itemName ? itemName : "");
    } else {
        // multiple — 用 getScriptParameter 套 58166 模板
        std::map<std::string, std::string> params;
        StringStream ss;
        ss << (hitCount - 1);
        const char* itemName = g_DCTTextManager.GetText(firstTextCode);
        params["Parameter0"] = itemName ? itemName : "";
        params["Parameter1"] = ss.c_str();

        const char* tmpl = g_DCTTextManager.GetText(58166);
        std::string parsed = getScriptParameter(tmpl ? tmpl : "", params);

        if (tmpl && parsed == tmpl) {
            std::snprintf(outBuf, sizeof(outBuf), "%s", tmpl);
        } else {
            std::snprintf(outBuf, sizeof(outBuf), "%s", parsed.c_str());
        }
    }

    if (g_pMsgBoxMgr) {
        g_pMsgBoxMgr->AddOK((CUIBase*)nullptr, outBuf, 0, 0, 0, -1, 0, 1, 0);
    }
}

// ============================================================================
// CDeleteCashItemManager
// ============================================================================

CDeleteCashItemManager::CDeleteCashItemManager()
    : m_pcl(nullptr)
{
}

CDeleteCashItemManager::~CDeleteCashItemManager()
{
    ClearDeleteCashItem();
}

//----- (004DB9B0) --------------------------------------------------------
void CDeleteCashItemManager::InitDeleteCashItem()
{
    if (!m_pcl) {
        m_pcl = new CDeleteCashItem();
    }
}

//----- (004DB9F0) --------------------------------------------------------
void CDeleteCashItemManager::ClearDeleteCashItem()
{
    if (m_pcl) {
        delete m_pcl;
        m_pcl = nullptr;
    }
}

//----- (004DBA50) --------------------------------------------------------
void CDeleteCashItemManager::AddDeleteCashitem(uint16_t itemKind, int reason)
{
    if (m_pcl) {
        m_pcl->AddDeleteItem(itemKind, reason);
    }
}

//----- (004DBA70) --------------------------------------------------------
void CDeleteCashItemManager::NoticeDeleteCashItem()
{
    if (m_pcl) {
        m_pcl->NoticeDeletedItem();
        m_pcl->NoticeTimeOutItem();
        ClearDeleteCashItem();
    }
}
