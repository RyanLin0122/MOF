#pragma once
#include <cstdint>
#include <cstddef>

// =====================================================================
// cltWorldMapColorInfo
// ---------------------------------------------------------------------
// 反編譯對照（mofclient.c）：
//   ctor              : 0x004F1A60 (mofclient.c:213837)
//   InitializeInPack  : 0x00503340 (mofclient.c:213847)
//   Initialize        : 0x005033E0 (mofclient.c:213872)
//   Free              : 0x00503440 (mofclient.c:213892)
//   GetColor          : 0x00503460 (mofclient.c:213902)
//
// 載入資料：
//   * MoFData/UI/w_mapcolorinfo.dat  （世界地圖區塊色彩索引；尺寸 = 0x210D3 bytes）
//   * MoFData/UI/w_mapcolorinfo2.dat （第二地圖；尺寸同上）
//
// 注意：本類別並非解析 .txt（故未列於 text_dump_classes.md），
// 而是直接讀取 409 × 331 = 135379 bytes 之純二進位 8-bit 索引點陣。
// =====================================================================
class cltWorldMapColorInfo {
public:
    cltWorldMapColorInfo();
    ~cltWorldMapColorInfo();

    // 與反編譯一致：成功回傳 1，失敗回傳 0
    int InitializeInPack(char* pathInPack);
    int Initialize(const char* fileName);
    void Free();

    // 取得指定 (x,y) 像素之顏色索引；越界或未載入回傳 0
    int GetColor(int x, int y) const;

private:
    // ===========================================================
    // 한국어  : 월드맵 컬러 데이터 버퍼 포인터
    // 中文    : 世界地圖色彩索引資料緩衝區（指標）
    // 反編譯  : *(_DWORD *)this — 物件唯一資料成員
    // 偏移    : x86 [+0x00 .. +0x04)（4 bytes）
    //          x64 [+0x00 .. +0x08)（8 bytes）
    // 內容    : 指向動態配置之 0x210D3 bytes 緩衝；
    //          排列為 row-major，index = y * 409 + x，
    //          每一格為一個 uint8 區塊代碼（0=未定義；1..N=可進入區）
    // ===========================================================
    uint8_t* m_pData;  // [+0x00] 월드맵 컬러 픽셀 버퍼

    // 反編譯硬編碼之尺寸常量（與 GetColor 邊界檢查、配置大小一一吻合）
    static constexpr int    kWidth  = 409;     // 너비 / 寬度（GetColor 內 408*y + y = 409*y）
    static constexpr int    kHeight = 331;     // 높이 / 高度（GetColor 內 a3 < 331）
    static constexpr size_t kSize   = 0x210D3; // 데이터 크기 / 緩衝區大小（= kWidth * kHeight = 135379）
};

// 編譯期保證 layout 與反編譯所見的「單一指標物件」一致
static_assert(sizeof(cltWorldMapColorInfo) == sizeof(void*),
              "cltWorldMapColorInfo must be a single-pointer object (matches *(_DWORD *)this)");
