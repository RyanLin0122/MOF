#pragma once
#include <cstdint>
#include "Info/clTransportKindInfo.h"

class clTransportAniInfo;

// =============================================================================
// clClientTransportKindInfo  —  客戶端載具資訊（含動畫快取）
// mofclient.c：195094 (ctor) / 195112 (Free) / 195140 (GetTransportAniInfoUp)
//                195181 (GetTransportAniInfoDown)
//
// 從 clTransportKindInfo 派生：在 ground truth 中，全域 g_clTransportKindInfo
// 實際上是 clClientTransportKindInfo 的單一實體（mofclient.c 196923 / 208742 /
// 210185），base 與 derived 共用一份 m_pList/m_nNum，而 GetTransportAniInfoUp/
// Down 直接以 this 呼叫 base method 取 KindInfo。本還原版本同樣只保留一個
// 全域 g_clTransportKindInfo（型別為 clClientTransportKindInfo）。
//
// 大小（GT 32-bit）= base 12 bytes + 兩條 0xFFFF-entry 指標陣列 × 4 bytes
//                  = 12 + 65535*4 + 65535*4 = 524292 bytes
// 大小（x64 還原）= base 24 bytes (vftable=8 + m_pList=8 + m_nNum=4 + pad=4)
//                  + 兩條 0xFFFF-entry 指標陣列 × 8 bytes
//                  = 24 + 65535*8 + 65535*8 = 1048584 bytes (~1MB)
// 由於 g_clTransportKindInfo 是唯一全域實體且配置在 BSS，OS 自動 zero-fill，
// 實際 RAM commit 量在實際初始化前不會增加。
//
// 反編譯佐證（GT 32-bit byte 算術；x64 改以 member 存取，offset 自動雙倍）：
//   v2 = (char*)this + 4*a2 + 12;        ← Up cache 起點 = offset 12
//   v2 = (char*)this + 4*a2 + 262152;    ← Down cache 起點 = 12 + 262140
//   Free 中 v2 = 0xFFFF, do { ... } while (--v2);
//     → 共 65535 次 iteration（清 up[0..65534] 與 down[0..65534]）。
// =============================================================================
class clClientTransportKindInfo : public clTransportKindInfo
{
public:
    clClientTransportKindInfo();
    ~clClientTransportKindInfo() override;

    // 取得指定 transportKind 的「上半身」動畫資訊（lazy-load，cache 於 m_pAniInfoUp）
    // 對齊 mofclient.c 195140：
    //   - 已快取直接回傳；
    //   - 找不到 KindInfo / aniFileUp = "NONE" 時回傳 nullptr；
    //   - 否則 operator new(0x1F68) 建構並 Init；Init 失敗顯示 MessageBox 並回 0。
    clTransportAniInfo* GetTransportAniInfoUp(std::uint16_t transportKind);

    // 同上但讀「下半身」動畫，cache 於 m_pAniInfoDown。
    // 對齊 mofclient.c 195181。
    clTransportAniInfo* GetTransportAniInfoDown(std::uint16_t transportKind);

    // 釋放兩個 cache 陣列上所有非 nullptr 的 clTransportAniInfo。
    // 對齊 mofclient.c 195112：from offset 12 至 524292，共 65535 對 (up,down)。
    void Free();

private:
    // mofclient.c 195150：v2 = (char*)this + 4*a2 + 12  /  offset 12 .. 262151
    // 注意：陣列邏輯大小是 0xFFFF（65535）筆；index 0xFFFF 會踩到 m_pAniInfoDown[0]。
    // 實際 kindCode 受 TranslateKindCode 限制不會達到 0xFFFF，此越界不會發生。
    clTransportAniInfo* m_pAniInfoUp[0xFFFF];

    // mofclient.c 195191：v2 = (char*)this + 4*a2 + 262152  / GT offset 262152 .. 524291
    // 同樣 x64 上 pointer 為 8 bytes，實際偏移加倍；parser 使用 [a2] 索引，
    // 與 byte 偏移無關。
    clTransportAniInfo* m_pAniInfoDown[0xFFFF];
};
