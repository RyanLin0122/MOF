#include "Sound/COgg.h"
#include "CMOFPacking.h" // 為了使用 CMofPacking
#include <cstring>      // 為了 memcpy
#include <cstdlib>      // 為了 malloc, free (儘管新版不再直接使用)

// 全域變數定義 (假設在專案其他地方定義並初始化)
extern int g_bLoadOggFromMofPack = 1;

COgg::COgg() : m_pStream(nullptr), m_nVolume(255), m_nChannelId(0), m_nStreamOpenMode(0) {
    // 建構函式，初始化成員變數
}

COgg::~COgg() {
    if (m_pStream) {
        FSOUND_Stream_Stop(m_pStream);
        FSOUND_Stream_Close(m_pStream);
        m_pStream = nullptr;
    }
    // 警告：FSOUND_Close() 會關閉整個 FMOD 系統。
    // 這通常應該由一個集中的音效管理器在應用程式結束時呼叫，
    // 而不是在單一音效物件的解構函式中。
    // 為了符合原始碼分析，此處保留，但在大型專案中應考慮移除。
    FSOUND_Close();
}

void COgg::Initalize(int loopEnabled) {
    HWND hWnd = GetConsoleWindow(); // 或者您的應用程式主視窗句柄
    FSOUND_SetHWND(hWnd);
    FSOUND_SetOutput(FSOUND_OUTPUT_DSOUND); // 2 對應 FSOUND_OUTPUT_DSOUND

    // 初始化 FMOD，44.1kHz, 32個軟體聲道, 無特殊旗標
    if (FSOUND_Init(44100, 32, 0) == FALSE) {
        // 處理初始化失敗的情況
        return;
    }

    m_pStream = nullptr;
    m_nVolume = 255;  // 預設最大音量
    m_nChannelId = 0; // 預設 BGM 聲道

    // 根據是否從封裝載入以及是否循環，來設定串流開啟模式
    unsigned int base_mode;
    unsigned int loop_mode = loopEnabled ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF;

    if (g_bLoadOggFromMofPack) {
        // 從記憶體載入，需要 FSOUND_LOADMEMORY 旗標
        base_mode = FSOUND_LOADMEMORY | FSOUND_STREAMABLE | FSOUND_STEREO;
    }
    else {
        // 直接從檔案路徑開啟
        base_mode = FSOUND_STREAMABLE | FSOUND_STEREO;
    }
    m_nStreamOpenMode = base_mode | loop_mode;
}

void COgg::Play(const char* filePath) {
    if (m_pStream) {
        FSOUND_Stream_Stop(m_pStream);
        FSOUND_Stream_Close(m_pStream);
        m_pStream = nullptr;
    }

    if (g_bLoadOggFromMofPack) {
        // 從封裝檔案中載入
        OpenStreem(filePath);
    }
    else {
        // 直接從檔案系統開啟
        m_pStream = FSOUND_Stream_Open(filePath, m_nStreamOpenMode, 0, 0);
    }

    // 如果成功載入串流
    if (m_pStream) {
        // 播放並設定音量
        m_nChannelId = FSOUND_Stream_Play(FSOUND_FREE, m_pStream); // 使用 FSOUND_FREE 自動尋找可用聲道
        FSOUND_SetVolume(m_nChannelId, m_nVolume);
    }
}

void COgg::Stop() {
    if (m_pStream) {
        // 只停止，不關閉，以便之後可以繼續播放
        FSOUND_Stream_Stop(m_pStream);
    }
}

// [已重構] 開啟音訊串流的核心邏輯
void COgg::OpenStreem(const char* filePath) {
    CMofPacking* packer = CMofPacking::GetInstance();

    // **修正 #1: 移除 `packer->PackFileOpen(..)` 的呼叫。**
    // 假設 VFS 已經由外部邏輯開啟。COgg 不負責管理 VFS 的開啟與關閉。
    if (!packer || !packer->m_pNfsHandle) {
        // 如果封裝未開啟，無法繼續。
        m_pStream = nullptr;
        return;
    }

    // **修正 #2: 使用 `FileRead` API，而不是 `FileReadBackGroundLoading`。**
    // `FileRead` 會為檔案內容動態分配記憶體，並返回指標。
    char* pMemoryBuffer = packer->FileRead(filePath);

    if (pMemoryBuffer) {
        int bufferSize = packer->GetBufferSize();
        if (bufferSize > 0) {
            // 使用載入到記憶體的資料開啟 FMOD 串流。
            // m_nStreamOpenMode 已經包含了 FSOUND_LOADMEMORY 旗標。
            m_pStream = FSOUND_Stream_Open(pMemoryBuffer, m_nStreamOpenMode, 0, bufferSize);
        }
        else {
            m_pStream = nullptr;
        }

        // **修正 #3: 立即釋放由 `FileRead` 分配的緩衝區，修復記憶體洩漏。**
        // FMOD 的 FSOUND_LOADMEMORY 會複製資料，所以我們可以馬上釋放自己的緩衝區。
        packer->DeleteBuffer();
    }
    else {
        // 檔案讀取失敗
        m_pStream = nullptr;
    }
}