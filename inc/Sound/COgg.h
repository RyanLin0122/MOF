#ifndef COGG_H
#define COGG_H

#include <windows.h> // For HWND
#include <fmod/fmod.h>

// FSOUND 常數 (根據 cogg.c 中的值推斷)
//const unsigned int FSOUND_LOADMEMORY = 0x4000; // 16384
//const unsigned int FSOUND_LOADRAW = 0x8000; // 32768

// 預設播放聲道 (讓 FMOD 選擇一個空閒聲道)
const int FSOUND_FREE_CHANNEL = -1; // 通常 FMOD 使用 -1 代表任意空閒聲道

// --- 結束 FSOUND 模擬 ---

// 包含 CMofPacking 的標頭檔
#include "CMOFPacking.h" // 假設此檔案在包含路徑中

// COgg 使用的全域變數 (需在其他地方定義)
extern HWND hWnd;         // 視窗控制代碼，用於 FSOUND_SetHWND
extern int dword_829254;  // 控制檔案載入方式和 FSOUND_Stream_Open 旗標的全域變數
extern CMofPacking g_clMofPacking; // CMofPacking 的全域實例

/*
 * 關於 dword_C24CF4:
 * 在 cogg.c 的 OpenStreem 函式中，使用了 &dword_C24CF4 作為 qmemcpy 的來源位址。
 * 這暗示 dword_C24CF4 是一個全域資料緩衝區的符號，CMofPacking::FileReadBackGroundLoading 負責填入此緩衝區。
 * CMofPacking::GetBufferSize() 則返回此緩衝區中有效資料的大小。
 * 為了讓 COgg.cpp 能夠運作，我們假設 CMofPacking 提供了一個方法來存取這個緩衝區的資料。
 * 在下面的實作中，我們會假設 g_clMofPacking.GetBackgroundBufferData() 可以取得此緩衝區指標，
 * 並且 g_clMofPacking.GetBufferSize() 返回的是 FileReadBackGroundLoading 載入的完整檔案大小。
 * 注意：您提供的 CMOFPacking.h/cpp 可能需要調整才能完全符合 cogg.c 的這種用法。
 */

class COgg
{
public:
    COgg();
    virtual ~COgg(); // 虛擬解構函式，因為反編譯碼顯示了 scalar deleting destructor 和 vftable

    // 初始化 FSOUND 系統和 COgg 實例的設定
    // a2: 似乎是一個迴圈旗標 (0 代表不迴圈, 非0 代表迴圈)
    void Initalize(int a2);

    // 播放 Ogg 檔案
    // filePath: Ogg 檔案的路徑或在封裝檔中的名稱
    void Play(const char* filePath);

    // 停止目前播放的 Ogg 音訊流
    void Stop();

private:
    // 內部函式，用於開啟音訊流 (可能從檔案或記憶體)
    void OpenStreem(const char* filePath);

    // 成員變數 (根據 cogg.c 中的 this 指標位移推斷)
    // this+0: vfptr (由 C++ 虛擬函式機制處理)

    // this+1 * sizeof(DWORD) -> m_streamHandle
    // FSOUND 音訊流控制代碼
    FSOUND_STREAM* m_streamHandle;

    // this+2 * sizeof(DWORD) -> m_volume
    // 音量 (0-255)
    int m_volume;

    // this+3 * sizeof(DWORD) -> m_channelId
    // FSOUND 播放聲道 ID
    // 在 cogg.c 中未明確初始化，此處給予一個常用預設值 FSOUND_FREE_CHANNEL
    int m_channelId;

    // this+4 * sizeof(DWORD) -> m_streamOpenFlags
    // 用於 FSOUND_Stream_Open 的旗標組合
    unsigned int m_streamOpenFlags;

    // 注意：cogg.c 中的 OpenStreem 函式使用 _malloc 配置的記憶體 v5 未被明確釋放。
    // 這意味著 FSOUND_Stream_Open 要麼複製了這塊記憶體，要麼期望呼叫者管理這塊記憶體的生命週期，
    // 而原始碼選擇了不釋放它（可能導致記憶體洩漏，除非 FMOD 在 Stream_Close 時處理）。
    // 此處的還原將會遵循原始碼的行為。
    void* m_pStreamDataCopy; // 用於儲存 OpenStreem 中 _malloc 的記憶體指標，以便後續釋放
};

#endif // COGG_H