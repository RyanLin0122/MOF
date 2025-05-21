#ifndef COGG_H
#define COGG_H

#include <fmod/fmod.h>      // FMOD 音效庫 API
#include <windows.h>   // FSOUND_SetHWND 需要 HWND

class COgg {
public:
    COgg();                          // 建構函式
    virtual ~COgg();                 // 虛擬解構函式

    // 初始化 COgg 系統，設定 FMOD 並準備播放參數
    // a2: 一個整數參數，用於決定串流的循環模式 (0: 不循環, 非0: 循環)
    void Initalize(int loopEnabled);

    // 播放指定的 Ogg Vorbis 檔案
    // filePath: Ogg 檔案的路徑或名稱 (如果 dword_829254 為 true，則為封裝內的檔案路徑)
    void Play(const char* filePath);

    // 停止目前播放的串流
    void Stop();

private:
    // 開啟音訊串流，根據 dword_829254 決定是從檔案系統直接開啟還是從 CMofPacking 載入
    // filePath: Ogg 檔案的路徑或名稱
    void OpenStreem(const char* filePath);

    // 成員變數 (根據 cogg.c 中的記憶體位移推斷)
    FSOUND_STREAM* m_pStream;        // 指向 FMOD 音訊串流的指標 (對應 this+1)
    int            m_nVolume;          // 音量大小 (0-255) (對應 this+2)
    int            m_nChannelId;       // FMOD 用於播放此串流的聲道 ID (對應 this+3)
    unsigned int   m_nStreamOpenMode;  // 開啟 FMOD 串流時使用的模式旗標 (對應 this+4)
};

#endif // COGG_H