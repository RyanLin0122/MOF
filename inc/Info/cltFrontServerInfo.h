#pragma once
#include <cstdint>
#include <cstddef>
#include "global.h"
#include "Font/FileCrypt.h"


// ============================================================================
// stFrontServerInfo — 單筆前端伺服器資訊
// ----------------------------------------------------------------------------
// 反編譯位置：mofclient.c:205999 (strcpy v16, host) / 206006 (*((_DWORD*)v16+5) = port)
// 寫入時 v16 為當前 slot 起點；strcpy 把 IP/Host 寫到偏移 0；port 寫到偏移 +20
// (即 *((_DWORD*)v16 + 5))。下一筆 slot 由 v16 += 24 取得，故每筆固定 24 bytes。
// ============================================================================
struct stFrontServerInfo {
    // 호스트(IP) 문자열 / 主機 IP 字串
    // offset: 0  size: 20  (strcpy 寫入，無長度檢查 — 忠實還原)
    char Host[20];

    // 포트 번호 / 連接埠
    // offset: 20 size: 4   (來自 atoi(token))
    int  Port;
};
static_assert(sizeof(stFrontServerInfo) == 24,                "stFrontServerInfo 必須是 24 bytes");
static_assert(offsetof(stFrontServerInfo, Host) == 0,         "Host 必須位於 offset 0");
static_assert(offsetof(stFrontServerInfo, Port) == 20,        "Port 必須位於 offset 20");


// ============================================================================
// cltFrontServerInfo — 前端伺服器列表 (固定容量 10)
// ----------------------------------------------------------------------------
// 反編譯位置：
//   ctor       : mofclient.c:205920  (memset 0xF0=240 bytes 清空 m_aInfos)
//   Initialize : mofclient.c:205930  (讀取 MofData/FrontServerList.dat)
//   GetRandom  : mofclient.c:206056
//
// 反編譯中以下標表示成員存取：
//   *((_WORD *)this + 120)  → offset 240  → m_wCount
//   *((_WORD *)this + 121)  → offset 242  → m_wIndex
//   *((_DWORD*)this + 61)   → offset 244  → m_dwFirstCall
//   memset(this, 0, 0xF0u)  → 清空 0..239 (即 m_aInfos)
// ============================================================================
class cltFrontServerInfo {
public:
    cltFrontServerInfo();

    // 對應 mofclient.c:205930 cltFrontServerInfo::Initialize
    // 流程：NationCode==4 → 直接讀 dat；否則先 DecoderFileCrypt → tmp.dat
    // 成功回傳 1，失敗回傳 0
    int Initialize();

    // 對應 mofclient.c:206056 cltFrontServerInfo::GetRandomServerInfo
    // 旗標 m_dwFirstCall 為 0 時回傳 nullptr；否則 ++m_wIndex 並循環，回傳對應槽位
    const stFrontServerInfo* GetRandomServerInfo();

    // 輔助查詢 (非反編譯函式，純為其他模組使用)
    uint16_t GetCount() const { return m_wCount; }
    const stFrontServerInfo* GetArray() const { return m_aInfos; }

private:
    // 內部工具
    int  ParseFromFile(const char* filePath);
    void ShuffleLikeDecompiled();
    static bool DeleteFileCompat(const char* path);

private:
    // ------------------------------------------------------------------------
    // 메모리 레이아웃 / 記憶體佈局 — 必須與反編譯碼一致
    // ------------------------------------------------------------------------

    // 프런트 서버 정보 배열 / 前端伺服器資訊陣列 (固定 10 筆)
    // offset:   0  size: 240  (10 * 24)
    // 反編譯：memset(this, 0, 0xF0u) 清空整個陣列
    stFrontServerInfo m_aInfos[10];

    // 유효 항목 수 / 有效資料筆數
    // offset: 240  size: 2    *((_WORD *)this + 120)
    // 反編譯先以資料行數遞增，遇到 type=="TEST" 再遞減
    uint16_t m_wCount;

    // 현재 인덱스 / 目前索引 (GetRandomServerInfo 先 ++ 再讀)
    // offset: 242  size: 2    *((_WORD *)this + 121)
    uint16_t m_wIndex;

    // 첫 호출 보호 플래그 / 首次呼叫保護旗標 (1=尚未取過, 0=已取過)
    // offset: 244  size: 4    *((_DWORD *)this + 61)
    // 反編譯：建構函式設為 1；GetRandomServerInfo 第一次後立即清為 0
    uint32_t m_dwFirstCall;

    // 友元結構：在類別完整後做 offsetof 編譯期檢查 (需私有成員存取權)
    friend struct cltFrontServerInfo_LayoutCheck;
};
static_assert(sizeof(cltFrontServerInfo) == 248, "cltFrontServerInfo 必須是 248 bytes (240+2+2+4)");

struct cltFrontServerInfo_LayoutCheck {
    static_assert(offsetof(cltFrontServerInfo, m_wCount)      == 240, "m_wCount 必須位於 offset 240");
    static_assert(offsetof(cltFrontServerInfo, m_wIndex)      == 242, "m_wIndex 必須位於 offset 242");
    static_assert(offsetof(cltFrontServerInfo, m_dwFirstCall) == 244, "m_dwFirstCall 必須位於 offset 244");
};


// 對應反編譯中之 dword_BE4BB4 (除錯層級，僅在值為 2 時顯示 MessageBox 錯誤)
extern unsigned int g_DebugLevel;
