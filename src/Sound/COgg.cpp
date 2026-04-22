#include "Sound/COgg.h"
#include "FileSystem/CMOFPacking.h"
#include <cstring>
#include <cstdlib>

extern int g_bLoadOggFromMofPack = 1;

COgg::COgg()
    : m_pStream(nullptr),
    m_nVolume(255),
    m_nChannelId(-1),
    m_nStreamOpenMode(0),
    m_pMemoryBuffer(nullptr) // [修改] 初始化新增的成員
{
    // 建構函式
}

COgg::~COgg() {
    // 停止並關閉 FMOD 串流
    if (m_pStream) {
        FSOUND_Stream_Stop(m_pStream);
        FSOUND_Stream_Close(m_pStream);
        m_pStream = nullptr;
    }

    // 釋放 COgg 自己持有的獨立記憶體
    if (m_pMemoryBuffer) {
        delete[] m_pMemoryBuffer;
        m_pMemoryBuffer = nullptr;
    }


    // 警告：FSOUND_Close() 應該在應用程式結束時由音效管理器統一呼叫
    FSOUND_Close();
}

void COgg::Initalize(int loopEnabled) {
    // ... (這部分程式碼不變)
    HWND hWnd = GetConsoleWindow();
    FSOUND_SetHWND(hWnd);
    FSOUND_SetOutput(FSOUND_OUTPUT_DSOUND);

    if (FSOUND_Init(44100, 32, 0) == FALSE) {
        return;
    }

    m_pStream = nullptr;
    m_pMemoryBuffer = nullptr; // [新增] 確保初始化時為空
    m_nVolume = 255;
    m_nChannelId = -1;

    unsigned int base_mode;
    unsigned int loop_mode = loopEnabled ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF;

    if (g_bLoadOggFromMofPack) {
        base_mode = FSOUND_LOADMEMORY | FSOUND_STREAMABLE | FSOUND_STEREO;
    }
    else {
        base_mode = FSOUND_STREAMABLE | FSOUND_STEREO;
    }
    m_nStreamOpenMode = base_mode | loop_mode;
}

void COgg::Play(const char* filePath) {
    // 先停止並釋放舊的資源
    if (m_pStream) {
        FSOUND_Stream_Stop(m_pStream);
        FSOUND_Stream_Close(m_pStream);
        m_pStream = nullptr;
    }
    // 釋放舊的獨立記憶體緩衝區
    if (m_pMemoryBuffer) {
        delete[] m_pMemoryBuffer;
        m_pMemoryBuffer = nullptr;
    }

    // GameSound 是 global，其建構子於 static-init 跑 Initalize()，當時讀到的
    // g_bLoadOggFromMofPack 可能與 Play 當下不一致（例如 MiniGame debug mode
    // 會在 main() 才把旗標改成 0）。依現值修正 LOADMEMORY bit，避免走 file-path
    // 分支時殘留 FSOUND_LOADMEMORY 造成 FSOUND_Stream_Open silent fail。
    if (g_bLoadOggFromMofPack) {
        m_nStreamOpenMode |= FSOUND_LOADMEMORY;
    } else {
        m_nStreamOpenMode &= ~FSOUND_LOADMEMORY;
    }

    // 根據模式載入
    if (g_bLoadOggFromMofPack) {
        OpenStreem(filePath);
    }
    else {
        m_pStream = FSOUND_Stream_Open(filePath, m_nStreamOpenMode, 0, 0);
    }

    if (m_pStream) {
        m_nChannelId = FSOUND_Stream_Play(FSOUND_FREE, m_pStream);
        FSOUND_SetVolume(m_nChannelId, m_nVolume);
    }
}

void COgg::Stop() {
    if (m_pStream) {
        FSOUND_Stream_Stop(m_pStream);
    }
    m_nChannelId = -1;
}

void COgg::SetVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 255) volume = 255;

    m_nVolume = volume;

    if (m_nChannelId >= 0) {
        FSOUND_SetVolume(m_nChannelId, m_nVolume);
    }
}

// [已重構] 開啟音訊串流的核心邏輯
void COgg::OpenStreem(const char* filePath) {
    CMofPacking* packer = CMofPacking::GetInstance();

    if (!packer || !packer->m_pNfsHandle) {
        m_pStream = nullptr;
        return;
    }

    char localPath[256]{};
    std::strncpy(localPath, filePath, sizeof(localPath) - 1);
    char* changed = packer->ChangeString(localPath);
    packer->FileReadBackGroundLoading(changed);

    const int bufferSize = packer->GetBufferSize();
    if (bufferSize <= 0) {
        m_pStream = nullptr;
        return;
    }

    m_pMemoryBuffer = new (std::nothrow) char[bufferSize];
    if (!m_pMemoryBuffer) {
        m_pStream = nullptr;
        return;
    }

    std::memcpy(m_pMemoryBuffer, packer->m_backgroundLoadBufferField, static_cast<size_t>(bufferSize));
    m_pStream = FSOUND_Stream_Open(m_pMemoryBuffer, m_nStreamOpenMode, 0, bufferSize);
}
