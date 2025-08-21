#pragma once
#include <string>
#include <vector>

/**
 * @class FileCrypt
 * @brief 負責檔案的簡單 XOR 加密與解密。
 *
 * 這個類別從 MOFFont.cpp (0x005C4DD0) 的反編譯程式碼中還原。
 * 它使用一個 32 位元的金鑰對檔案內容進行 XOR 運算。
 * 注意：這個類別有 100 KB 的固定大小緩衝區，不適合處理大於此限制的檔案。
 */
class FileCrypt {
public:
    static FileCrypt* GetInstance();
    virtual ~FileCrypt();

    /**
     * @brief 加密一個檔案。
     * @param inputFileName 來源檔案路徑。
     * @param outputFileName 輸出檔案路徑。
     * @return 成功則返回 true。
     */
    bool EncoderFileCrypt(const char* inputFileName, const char* outputFileName);

    /**
     * @brief 解密一個檔案。
     * @param inputFileName 來源檔案路徑。
     * @param outputFileName 輸出檔案路徑，如果為 nullptr，則只在記憶體中解密。
     * @return 成功則返回 1，失敗則返回 0。
     */
    unsigned int DecoderFileCrypt(const char* inputFileName, const char* outputFileName);

    // 雖然反編譯程式碼中金鑰在建構函式裡設為 0，但實際應用中 (如 g_FileCrypt)
    // 應由外部在某處設定一個非零值才能生效。
    unsigned int m_dwKey;

private:
    /**
     * @brief 初始化內部狀態，清空緩衝區和錯誤碼。
     */
    void InitRead();

    FileCrypt();

    static FileCrypt* s_pInstance;

    // 儲存檔案內容的緩衝區
    char m_pBuffer[102400];

    // 從檔案讀取的實際大小
    unsigned int m_dwFileSize;

    // 解密失敗時的錯誤碼
    unsigned int m_dwErrorCode;
};
