#include "Sound/COgg.h"
#include "CMOFPacking.h" // 為了使用 g_clMofPacking 及其成員
#include <cstring>      // 為了 memcpy (cogg.c 中的 qmemcpy)
#include <cstdlib>      // 為了 malloc (cogg.c 中的 _malloc)

// 全域變數定義 (這些應在專案的其他地方定義，此處僅為使 COgg.cpp 能編譯的範例)
int g_bLoadOggFromMofPack = 1;                 // 根據您的應用程式邏輯初始化

COgg::COgg() : m_pStream(nullptr), m_nVolume(255), m_nChannelId(0), m_nStreamOpenMode(0) {
    // 建構函式，初始化成員變數
    // vftable 由編譯器自動處理
}

COgg::~COgg() {
    // 解構函式
    // vftable 由編譯器自動處理
    if (m_pStream) {
        FSOUND_Stream_Stop(m_pStream);
        FSOUND_Stream_Close(m_pStream); // 雖然 cogg.c 的解構函式只呼叫 Stop，但通常 Close 也應在此
        // 原始碼 cogg.c 的 COgg::~COgg 中只有 FSOUND_Stream_Stop 和 FSOUND_Close()
        // FSOUND_Close() 是關閉整個 FMOD 系統，不應在單個物件解構時呼叫，除非此物件代表 FMOD 的唯一使用者
        // 根據 cogg.c 的 ~COgg，它確實呼叫了 FSOUND_Close()。
        // 這表示 COgg 物件的生命週期可能與 FMOD 系統的生命週期緊密綁定。
        m_pStream = nullptr;
    }
    FSOUND_Close(); // 根據 cogg.c:~COgg 的行為
}

void COgg::Initalize(int loopEnabled) {
    HWND hWnd = GetConsoleWindow();
    FSOUND_SetHWND(hWnd);
    FSOUND_SetOutput(FSOUND_OUTPUT_DSOUND); // 2 對應 FSOUND_OUTPUT_DSOUND
    FSOUND_Init(44100, 32, 0);    // 初始化 FMOD，44.1kHz, 32個軟體聲道, 無特殊旗標

    m_pStream = nullptr;
    m_nVolume = 255;  // 預設最大音量
    m_nChannelId = 0; // 假設使用聲道 0 作為預設 BGM 聲道 (cogg.c 未明確初始化此成員)

    // 根據 dword_829254 和 loopEnabled 設定串流開啟模式
    // 這些數值來自 fmod.h 中的 FSOUND_MODES 旗標組合
    // 49218 (0xC042) = FSOUND_LOADMEMORY | FSOUND_STREAMABLE | FSOUND_STEREO | FSOUND_LOOP_NORMAL
    // 49217 (0xC041) = FSOUND_LOADMEMORY | FSOUND_STREAMABLE | FSOUND_STEREO | FSOUND_LOOP_OFF
    // 16450 (0x4042) = FSOUND_STREAMABLE | FSOUND_STEREO | FSOUND_LOOP_NORMAL
    // 16449 (0x4041) = FSOUND_STREAMABLE | FSOUND_STEREO | FSOUND_LOOP_OFF
    unsigned int base_mode;
    unsigned int loop_mode = loopEnabled ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF;

    if (g_bLoadOggFromMofPack) { // 若為真，表示可能從記憶體載入 (透過 CMofPacking)
        base_mode = FSOUND_LOADMEMORY | FSOUND_STREAMABLE | FSOUND_STEREO;
        // m_nStreamOpenMode = loopEnabled ? 49218 : 49217; // 直接使用 cogg.c 的數值
    }
    else { // 否則，直接從檔案路徑開啟
        base_mode = FSOUND_STREAMABLE | FSOUND_STEREO;
        // m_nStreamOpenMode = loopEnabled ? 16450 : 16449; // 直接使用 cogg.c 的數值
    }
    m_nStreamOpenMode = base_mode | loop_mode;
}

void COgg::Play(const char* filePath) {
    if (m_pStream) { // 如果已有串流正在播放或已載入
        FSOUND_Stream_Stop(m_pStream);
        FSOUND_Stream_Close(m_pStream);
        m_pStream = nullptr;
    }

    if (g_bLoadOggFromMofPack) { // 使用 CMofPacking 載入
        OpenStreem(filePath);
        if (!m_pStream) {
            return; // 若 OpenStreem 失敗
        }
        // OpenStreem 成功後，m_pStream 已被設定
    }
    else { // 直接從檔案開啟
        m_pStream = FSOUND_Stream_Open(filePath, m_nStreamOpenMode, 0, 0);
        if (!m_pStream) {
            // 嘗試再次開啟 (cogg.c 中的重試邏輯)
            m_pStream = FSOUND_Stream_Open(filePath, m_nStreamOpenMode, 0, 0);
            if (!m_pStream) {
                return; // 若仍然失敗
            }
        }
    }

    // 播放串流並設定音量
    // cogg.c 使用 this+3 (m_nChannelId) 作為播放聲道，此聲道ID應在 Initalize 中設定或作為參數傳入
    // FSOUND_Stream_Play 的返回值是實際使用的聲道，但 cogg.c 未儲存它。
    // 這暗示 m_nChannelId 是一個預設的固定聲道。
    FSOUND_Stream_Play(m_nChannelId, m_pStream);
    FSOUND_SetVolume(m_nChannelId, m_nVolume);
}

void COgg::Stop() {
    if (m_pStream) {
        FSOUND_Stream_Stop(m_pStream);
        // cogg.c 的 Stop 函式只呼叫 FSOUND_Stream_Stop，不呼叫 Close
    }
}

void COgg::OpenStreem(const char* filePath) {
    CMofPacking* g_clMofPacking = CMofPacking::GetInstance();
    if (g_bLoadOggFromMofPack) { // 從 CMofPacking 載入並從記憶體開啟串流
        char tempPath[256];
        strcpy_s(tempPath, sizeof(tempPath), filePath); // 複製檔案路徑以供修改

        // CMofPacking::ChangeString 將字串轉為小寫
        // 假設 g_clMofPacking 的 ChangeString 修改傳入的緩衝區
        char* processedPath = g_clMofPacking->ChangeString(tempPath);

        g_clMofPacking->FileReadBackGroundLoading(processedPath);

        // cogg.c 中此處有 `if ( &dword_C24CF4 )` 檢查
        // dword_C24CF4 在 IDA 中可能是 g_clMofPacking.m_backgroundLoadBufferField 的別名。
        // `&g_clMofPacking.m_backgroundLoadBufferField` 是一個地址，此條件恆為真。
        // 一個更合理的檢查可能是檢查 GetBufferSize() > 0。
        // 為了盡可能符合 cogg.c 的行為，此處假設該 if 條件存在且通常為真。
        // 如果 g_clMofPacking.m_backgroundLoadBufferField 是公開的，可以直接存取。
        // 在提供的 CMOFPacking.h 中，m_backgroundLoadBufferField 是 public char array。
        if (true) { // 代表 cogg.c 中的 `if (&dword_C24CF4)`，該位址檢查通常為真
            size_t bufferSize = g_clMofPacking->GetBufferSize();
            if (bufferSize > 0) { // 只有在實際讀到資料時才嘗試分配和開啟
                void* pMemoryBuffer = malloc(bufferSize); // cogg.c 使用 _malloc
                if (pMemoryBuffer) {
                    // cogg.c 使用 qmemcpy(v5, &dword_C24CF4, size)
                    // &dword_C24CF4 指向 g_clMofPacking 的內部緩衝區資料
                    // 假設 m_backgroundLoadBufferField 是公開的，或者有一個 getter
                    memcpy(pMemoryBuffer, g_clMofPacking->m_backgroundLoadBufferField, bufferSize);

                    // 使用載入到記憶體的資料開啟 FMOD 串流
                    // m_nStreamOpenMode 應包含 FSOUND_LOADMEMORY 旗標 (已在 Initalize 中處理)
                    // length 參數是 bufferSize
                    m_pStream = FSOUND_Stream_Open(static_cast<const char*>(pMemoryBuffer), m_nStreamOpenMode, 0, static_cast<int>(bufferSize));

                    // 重要：cogg.c 中的 OpenStreem 並未釋放 pMemoryBuffer。
                    // 這可能是一個記憶體洩漏，除非 FMOD 在 FSOUND_Stream_Open 時複製了資料，
                    // 並且應用程式期望在 FSOUND_Stream_Close 後由 FMOD 處理。
                    // 或者，如果 FSOUND_LOADMEMORY 模式下 FMOD 不複製而是直接使用該指標，
                    // 則 pMemoryBuffer 必須在串流的生命週期內保持有效，並在之後釋放。
                    // 為了嚴格遵循 cogg.c 的行為，此處不添加 free(pMemoryBuffer)。
                    // 若 FSOUND_Stream_Open 失敗，pMemoryBuffer 也未被釋放。
                    if (!m_pStream && pMemoryBuffer) {
                        free(pMemoryBuffer); // 如果開啟失敗，我們應該釋放緩衝區
                    }

                }
                else {
                    m_pStream = nullptr; // malloc 失敗
                }
            }
            else {
                m_pStream = nullptr; // GetBufferSize 為0，無資料可載入
            }
        }
    }
    else { // 直接從檔案路徑開啟串流
        m_pStream = FSOUND_Stream_Open(filePath, m_nStreamOpenMode, 0, 0);
    }
}