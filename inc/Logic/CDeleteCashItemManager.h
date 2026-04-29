#pragma once
//
// CDeleteCashItem / CDeleteCashItemManager
// 還原 mofclient.c 0x4DA*–0x4DBA*
//
// CDeleteCashItem
//   - 內含一個 std::vector<strDeleteItem>，每個 strDeleteItem 是 4 bytes：
//     { uint16_t itemKind; uint8_t deleteReason; uint8_t pad; }
//   - AddDeleteItem  : push_back
//   - NoticeDeletedItem  : pop & 顯示「您的 X 已被刪除」訊息盒
//   - NoticeTimeOutItem  : 掃裝備 / 包包列出剩餘時間 < 7 天的 cash 道具
//
// CDeleteCashItemManager
//   - 只有一個 CDeleteCashItem* m_pcl 指標，做為 CDeleteCashItem 的 owner。
//   - InitDeleteCashItem  : new CDeleteCashItem  (operator new(0x10u))
//   - ClearDeleteCashItem : delete CDeleteCashItem
//
#include <cstdint>
#include <vector>

struct strDeleteItem {
    uint16_t itemKind;     // +0
    uint8_t  reason;       // +2  (1/2/3 → 對應 4146/4147/4148 文字 ID)
    uint8_t  pad;          // +3
};

class CDeleteCashItem {
public:
    CDeleteCashItem();
    ~CDeleteCashItem();

    void AddDeleteItem(uint16_t itemKind, int reason);
    void NoticeDeletedItem();
    void NoticeTimeOutItem();

private:
    uint8_t                     m_pad0;          // +0 (mofclient.c 未使用，保留 binary alignment)
    uint8_t                     m_pad1[3];       // +1..+3
    std::vector<strDeleteItem>  m_vDeleteList;   // +4..+15  (begin/end/cap, 共 12 bytes)
};

class CDeleteCashItemManager {
public:
    CDeleteCashItemManager();
    ~CDeleteCashItemManager();

    void InitDeleteCashItem();
    void ClearDeleteCashItem();
    void AddDeleteCashitem(uint16_t itemKind, int reason);
    void NoticeDeleteCashItem();

private:
    CDeleteCashItem* m_pcl;   // +0
};
