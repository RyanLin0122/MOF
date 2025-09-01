#include "Sound/COgg.h"
#include "FileSystem/CMOFPacking.h"
#include <cstring>
#include <cstdlib>

extern int g_bLoadOggFromMofPack = 1;

COgg::COgg()
    : m_pStream(nullptr),
    m_nVolume(255),
    m_nChannelId(0),
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

    // [新增] 釋放我們自己管理的記憶體緩衝區
    if (m_pMemoryBuffer) {
        // 假設 CMofPacking 的 FileRead 是用 new[] 分配的
        // 如果不是，請用對應的方式釋放 (例如 free)
        // 為了安全，我們讓 packer 自己管理釋放
        CMofPacking* packer = CMofPacking::GetInstance();
        if (packer) {
            // 這個設計有些耦合，但依循您現有 packer->DeleteBuffer() 的模式
            // 更好的設計是 FileRead 返回指標後，由呼叫者用 delete[] 釋放
            //packer->DeleteBuffer(m_pMemoryBuffer); // 假設 DeleteBuffer 可以接收指標
        }
        // 或者，如果 packer 沒有提供這樣的函式，您需要知道它是如何分配的
        // delete[] m_pMemoryBuffer; 
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
    m_nChannelId = 0;

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
    // [新增] 同時釋放舊的記憶體緩衝區
    if (m_pMemoryBuffer) {
        CMofPacking* packer = CMofPacking::GetInstance();
        //if (packer) packer->DeleteBuffer(m_pMemoryBuffer); // 同解構函式的假設
        // delete[] m_pMemoryBuffer;
        m_pMemoryBuffer = nullptr;
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
}

// [已重構] 開啟音訊串流的核心邏輯
void COgg::OpenStreem(const char* filePath) {
    CMofPacking* packer = CMofPacking::GetInstance();

    if (!packer || !packer->m_pNfsHandle) {
        m_pStream = nullptr;
        return;
    }

    // [修改] 將 packer->FileRead 的結果存到成員變數 m_pMemoryBuffer
    m_pMemoryBuffer = packer->FileRead(filePath);

    if (m_pMemoryBuffer) {
        int bufferSize = packer->GetBufferSize();
        if (bufferSize > 0) {
            // 使用我們自己持有的 m_pMemoryBuffer 來開啟串流
            m_pStream = FSOUND_Stream_Open(m_pMemoryBuffer, m_nStreamOpenMode, 0, bufferSize);
        }
        else {
            m_pStream = nullptr;
        }

        // [移除] 絕對不能在這裡釋放緩衝區！
        // packer->DeleteBuffer(); 
    }
    else {
        m_pStream = nullptr;
    }
}