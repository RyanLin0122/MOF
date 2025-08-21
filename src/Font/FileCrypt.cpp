#include "Font/FileCrypt.h"
#include <fstream>
#include <cstring>

// 初始化靜態成員指標
FileCrypt* FileCrypt::s_pInstance = nullptr;

// 靜態 GetInstance 方法的實現
FileCrypt* FileCrypt::GetInstance() {
    if (s_pInstance == nullptr) {
        s_pInstance = new (std::nothrow) FileCrypt();
    }
    return s_pInstance;
}

// 對應反編譯碼: 0x005C4DD0
FileCrypt::FileCrypt() {
    InitRead();
    // 原始碼中，全域變數 g_FileCrypt 的金鑰很可能在程式其他地方被初始化。
    // 建構函式將其設為 0 僅為預設行為。
    m_dwKey = 0; 
}

// 對應反編譯碼: 0x005C4E10
FileCrypt::~FileCrypt() {
    // 無需額外操作
}

// 對應反編譯碼: 0x005C4E20
void FileCrypt::InitRead() {
    m_dwFileSize = 0;
    m_dwErrorCode = 0;
    // 清空檔案緩衝區
    memset(m_pBuffer, 0, sizeof(m_pBuffer));
}

// 對應反編譯碼: 0x005C4E50
bool FileCrypt::EncoderFileCrypt(const char* inputFileName, const char* outputFileName) {
    InitRead();

    // 1. 讀取來源檔案
    std::ifstream inputFile(inputFileName, std::ios::in | std::ios::binary);
    if (!inputFile) {
        return false;
    }

    inputFile.read(m_pBuffer, sizeof(m_pBuffer));
    m_dwFileSize = static_cast<unsigned int>(inputFile.gcount());
    inputFile.close();

    // 2. 執行 XOR 加密
    char* pCurrent = m_pBuffer;
    unsigned int size_dword = m_dwFileSize / 4;
    unsigned int size_byte = m_dwFileSize % 4;

    // 一次處理 4 個位元組以提高效率
    for (unsigned int i = 0; i < size_dword; ++i) {
        *(unsigned int*)pCurrent ^= m_dwKey;
        pCurrent += 4;
    }
    // 處理剩餘的位元組
    for (unsigned int i = 0; i < size_byte; ++i) {
        *pCurrent++ ^= ((unsigned char*)&m_dwKey)[i]; // 原始碼邏輯是 HIBYTE，但分位元組 XOR 更常見
    }

    // 3. 寫入目標檔案
    if (outputFileName) {
        std::ofstream outputFile(outputFileName, std::ios::out | std::ios::binary);
        if (!outputFile) {
            return false;
        }
        // 先寫入金鑰，再寫入加密後的內容
        outputFile.write(reinterpret_cast<const char*>(&m_dwKey), sizeof(m_dwKey));
        outputFile.write(m_pBuffer, m_dwFileSize);
        outputFile.close();
    }

    return true;
}


// 對應反編譯碼: 0x005C4FE0
unsigned int FileCrypt::DecoderFileCrypt(const char* inputFileName, const char* outputFileName) {
    InitRead();

    // 1. 讀取加密檔案
    std::ifstream inputFile(inputFileName, std::ios::in | std::ios::binary);
    if (!inputFile) {
        m_dwErrorCode = 1;
        return 0;
    }

    unsigned int fileKey = 0;
    // 讀取檔案開頭的金鑰
    inputFile.read(reinterpret_cast<char*>(&fileKey), sizeof(fileKey));
    // 讀取檔案剩餘內容
    inputFile.read(m_pBuffer, sizeof(m_pBuffer));
    m_dwFileSize = static_cast<unsigned int>(inputFile.gcount());
    inputFile.close();

    // 2. 驗證金鑰
    if (fileKey != m_dwKey) {
        m_dwErrorCode = 2;
        return 0;
    }

    // 3. 執行 XOR 解密
    char* pCurrent = m_pBuffer;
    unsigned int size_dword = m_dwFileSize / 4;
    unsigned int size_byte = m_dwFileSize % 4;
    
    // 一次處理 4 個位元組
    for (unsigned int i = 0; i < size_dword; ++i) {
        *(unsigned int*)pCurrent ^= m_dwKey;
        pCurrent += 4;
    }
    // 處理剩餘的位元組
    for (unsigned int i = 0; i < size_byte; ++i) {
        *pCurrent++ ^= ((unsigned char*)&m_dwKey)[i];
    }
    
    // 4. 如果提供了輸出路徑，則寫入解密後的檔案
    if (outputFileName) {
        std::ofstream outputFile(outputFileName, std::ios::out | std::ios::binary);
        if (!outputFile) {
            m_dwErrorCode = 2; // 雖然原始碼這裡也設為2，但可以設為3表示不同錯誤
            return 0;
        }
        outputFile.write(m_pBuffer, m_dwFileSize);
        outputFile.close();
    }

    return 1; // 成功
}
