#include "Image/comp.h"

// 模擬 IDA Pro 反編譯中使用的 memset32 函式
extern "C" void memset32(void* dest, int val, size_t count) {
    auto d = static_cast<int*>(dest);
    for (size_t i = 0; i < count; ++i) {
        d[i] = val;
    }
}

// 計算檔案長度 (回傳檔案大小)
FILE* file_length(const char* const fileName) {
    FILE* file = fopen(fileName, "rb");
    FILE* result = nullptr;
    if (file) {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fclose(file);
        result = reinterpret_cast<FILE*>(size);
    }
    return result;
}

/**
 * 使用 marker-based RLE 壓縮資料，標記字節為 0xB4。
 * input: 指向原始資料的指標
 * length: 資料長度
 * 回傳值: 寫入的位元組數（長度），失敗時回傳 NULL
 */
FILE* compress_bytes(const unsigned char* input, unsigned int length) {
    FILE* file = fopen("Temp.dat", "wb");
    if (!file) {
        // 若無法開啟，先將檔案屬性設為普通，再試一次
        SetFileAttributesA("Temp.dat", FILE_ATTRIBUTE_NORMAL);
        file = fopen("Temp.dat", "wb");
        if (!file) {
            return NULL;
        }
    }

    // 狀態機變數
    enum {
        STATE_NONE = 0,
        STATE_ONE = 1,
        STATE_TWO = 2,
        STATE_RUN = 3
    } state = STATE_NONE;

    unsigned int index = 0;           // 目前處理到 input[index]
    size_t bufferLen = 0;             // buffer 中已累積的資料長度
    unsigned char repeatByte = 0;     // RLE run 的重複 byte 值
    unsigned char runCount = 0;       // RLE run 的計數
    char buffer[4];                   // 暫存區，最多寫入 2-byte 標記 + 1-byte data + 1-byte count
    unsigned int bytesWritten = 0;    // 總共寫入的位元組數

    while (index < length) {
        unsigned char current = input[index];

        switch (state) {
        case STATE_NONE:
            if (current == 0xB4) {
                // 遇到標記，直接輸出兩個標記
                buffer[0] = 0xB4;
                buffer[1] = 0xB4;
                fwrite(buffer, 1, 2, file);
                bytesWritten += 2;
            }
            else {
                // 普通 byte，先存到 buffer，進入下一狀態
                buffer[0] = current;
                bufferLen = 1;
                state = STATE_ONE;
            }
            index++;
            break;

        case STATE_ONE:
            if (current != buffer[0]) {
                // 與前一 byte 不同，直接 flush buffer
                fwrite(buffer, 1, bufferLen, file);
                bytesWritten += bufferLen;
                bufferLen = 0;
                state = STATE_NONE;
                // 重新處理此 byte
            }
            else {
                // 第二次出現，累積到 buffer
                buffer[bufferLen++] = current;
                state = STATE_TWO;
                index++;
            }
            break;

        case STATE_TWO:
            if (current != buffer[1]) {
                // 第三個 byte 不相同，flush 前兩個
                fwrite(buffer, 1, bufferLen, file);
                bytesWritten += bufferLen;
                bufferLen = 0;
                state = STATE_NONE;
                // 重新處理此 byte
            }
            else {
                // 第三次出現，啟動 RLE
                repeatByte = buffer[1];      // 重複的那個 byte
                runCount = 3;              // 計數初始化為 3
                buffer[0] = 0xB4;           // 標記
                buffer[1] = repeatByte;     // 重複 byte
                buffer[2] = runCount;       // 計數
                bufferLen = 3;
                state = STATE_RUN;
                index++;
            }
            break;

        case STATE_RUN:
            if (current != repeatByte || runCount >= 254) {
                // run 結束，flush RLE 結果
                fwrite(buffer, 1, bufferLen, file);
                bytesWritten += bufferLen;
                bufferLen = 0;
                state = STATE_NONE;
                // 重新處理此 byte
            }
            else {
                // run 繼續，更新計數
                runCount++;
                buffer[2] = runCount;
                bufferLen = 3;
                index++;
            }
            break;
        }
    }

    // 處理剩餘 buffer
    if (bufferLen > 0) {
        fwrite(buffer, 1, bufferLen, file);
        bytesWritten += bufferLen;
    }

    fclose(file);
    return (FILE*)(uintptr_t)bytesWritten;
}


/**
 * 使用 marker-based RLE 壓縮 16-bit WORD 資料，標記 WORD 為 0xB4B4。
 * @param inputBytes  原始資料位元組陣列指標
 * @param byteLength  原始資料長度 (bytes)
 * @return 回傳壓縮後所寫入的位元組數；失敗時回傳 0
 */
int compress_words(const unsigned char* inputBytes, unsigned int byteLength) {
    // 開啟輸出檔案
    FILE* file = fopen("Temp.dat", "wb");
    if (!file) {
        SetFileAttributesA("Temp.dat", FILE_ATTRIBUTE_NORMAL);
        file = fopen("Temp.dat", "wb");
        if (!file)
            return 0;
    }

    // run-length 狀態
    enum {
        STATE_NONE = 0,    // 尚未累積
        STATE_ONE = 1,    // 已累積 1 個 WORD
        STATE_TWO = 2,    // 已累積 2 個相同 WORD
        STATE_RUN = 3     // 正在做 RLE run
    } state = STATE_NONE;

    int wordsWritten = 0;                       // 成功寫入檔案的 WORD 數
    unsigned int bufferLen = 0;                    // 緩衝區中 WORD 的數量
    short buffer[3];                               // 暫存區，最多可存三個 WORD：標記, value, count
    short repeatWord = 0;                       // RLE run 中重複的 WORD 值
    short runCount = 0;                       // RLE run 中的計數 (最少 3)

    unsigned int processedWords = 0;               // 已處理的 WORD 數量
    unsigned int totalWords = byteLength >> 1; // 總共要處理的 WORD 數

    while (processedWords < totalWords) {
        // 取出下一個 WORD
        short current = *reinterpret_cast<const short*>(inputBytes);

        switch (state) {
        case STATE_NONE:
            if (current == static_cast<short>(0xB4B4)) {
                // 遇到標記，直接輸出兩個標記 WORD
                buffer[0] = 0xB4B4;
                buffer[1] = 0xB4B4;
                fwrite(buffer, sizeof(short), 2, file);
                wordsWritten += 2;
            }
            else {
                // 普通 WORD，先存入緩衝區
                buffer[0] = current;
                bufferLen = 1;
                state = STATE_ONE;
            }
            // 移動到下一個 WORD 位址
            processedWords++;
            inputBytes += 2;
            break;

        case STATE_ONE:
            if (current != buffer[0]) {
                // 與前一 WORD 不同，flush buffer
                fwrite(buffer, sizeof(short), bufferLen, file);
                wordsWritten += bufferLen;
                bufferLen = 0;
                state = STATE_NONE;
                // 不移動 processedWords，重處理同一個 WORD
            }
            else {
                // 第二次相同，累積
                buffer[bufferLen++] = current;
                state = STATE_TWO;
                processedWords++;
                inputBytes += 2;
            }
            break;

        case STATE_TWO:
            if (current != buffer[1]) {
                // 第三個 WORD 不相同，flush前兩個 WORD
                fwrite(buffer, sizeof(short), bufferLen, file);
                wordsWritten += bufferLen;
                bufferLen = 0;
                state = STATE_NONE;
                // 重處理同一個 WORD
            }
            else {
                // 第三次相同，開始 RLE run
                repeatWord = buffer[1];   // 重複的 WORD 值
                runCount = 3;           // 初始化計數為 3
                buffer[0] = 0xB4B4;      // 標記 WORD
                buffer[1] = repeatWord;
                buffer[2] = runCount;    // 計數
                bufferLen = 3;
                state = STATE_RUN;
                processedWords++;
                inputBytes += 2;
            }
            break;

        case STATE_RUN:
            if (current != repeatWord || runCount >= 65535) {
                // run 結束，flush結果
                fwrite(buffer, sizeof(short), bufferLen, file);
                wordsWritten += bufferLen;
                bufferLen = 0;
                state = STATE_NONE;
                // 重處理同一個 WORD
            }
            else {
                // run 繼續，更新計數
                runCount++;
                buffer[2] = runCount;
                // bufferLen 永遠為 3
                processedWords++;
                inputBytes += 2;
            }
            break;
        }
    }

    // flush 剩餘的 buffer
    if (bufferLen > 0) {
        fwrite(buffer, sizeof(short), bufferLen, file);
        wordsWritten += bufferLen;
    }

    fclose(file);
    // 回傳寫入的位元組數
    return wordsWritten * sizeof(short);
}

// 四位元組壓縮（已修正 buffer 殘留與 v12 覆寫問題）
int comp_DWORD(unsigned char* a1, unsigned int a2) {
    FILE* v2 = nullptr;
    int v3 = 0;             // 已寫入的 int 數量
    int state = 0;          // 壓縮狀態機：0=none, 1=one seen, 2=two seen, >=3=run
    int bufCount = 0;       // Buffer 中已累積的元素數
    int Buffer[3];          // [0]=marker/value/run-value, [1]=value, [2]=count
    int runValue = 0;       // 當進入 run-mode 時要壓縮的「重複值」
    unsigned int idx = 0;   // 已處理的 DWORD 數
    unsigned int total = a2 >> 2; // DWORD 總數

    // open file
    v2 = fopen("Temp.dat", "wb");
    if (!v2) {
        SetFileAttributesA("Temp.dat", FILE_ATTRIBUTE_NORMAL);
        v2 = fopen("Temp.dat", "wb");
        if (!v2)
            return 0;
    }

    // 主迴圈
    while (idx < total) {
        int v = *reinterpret_cast<int*>(a1);

        switch (state) {
        case 0:
            // 沒有前一筆，先看看是不是 marker
            if (v == -1263225676) {
                // 直接輸出兩個 marker
                Buffer[0] = -1263225676;
                Buffer[1] = -1263225676;
                fwrite(Buffer, sizeof(int), 2, v2);
                v3 += 2;
            }
            else {
                // 暫存第一筆
                Buffer[0] = v;
                bufCount = 1;
                state = 1;
            }
            break;

        case 1:
            // 已經看到一筆，看看第二筆是不是一樣
            if (v != Buffer[0]) {
                // 不同：輸出孤立那一筆
                fwrite(Buffer, sizeof(int), bufCount, v2);
                v3 += bufCount;
                // 重設回 0，並「重處理」當前這筆
                state = 0;
                bufCount = 0;
                continue;  // idx/a1 不會移動，下一輪再處理
            }
            else {
                // 第二筆相同，進入準備 run 模式
                bufCount = 2;
                runValue = v;
                state = 2;
            }
            break;

        case 2:
            // 已看到兩筆相同，看看第三筆是不是一樣
            if (v != runValue) {
                // 不同：輸出剛才的兩筆
                fwrite(Buffer, sizeof(int), bufCount, v2);
                v3 += bufCount;
                // 重設
                state = 0;
                bufCount = 0;
                continue;
            }
            else {
                // 第三筆相同：真正啟動 RLE
                // Buffer[0]=marker, Buffer[1]=runValue, Buffer[2]=count
                Buffer[0] = -1263225676;
                Buffer[1] = runValue;
                Buffer[2] = 3;
                bufCount = 3;
                state = 3;
            }
            break;

        default:
            // state >= 3：在 run 模式中
            if (v != runValue) {
                // 遇到不同值，先把累積的標記區塊寫出
                fwrite(Buffer, sizeof(int), bufCount, v2);
                v3 += bufCount;
                // 重設為 state=0，重新處理這筆
                state = 0;
                bufCount = 0;
                continue;
            }
            else {
                // 繼續累積
                Buffer[2]++;      // count +1
                bufCount = 3;
            }
            break;
        }

        // 正常消耗一筆
        idx++;
        a1 += 4;
    }

    // 把尾端尚未寫出的資料 flush 出去
    if (bufCount > 0) {
        fwrite(Buffer, sizeof(int), bufCount, v2);
        v3 += bufCount;
    }

    fclose(v2);
    // 回傳寫入的 byte 數
    return 4 * v3;
}

// 執行長度壓縮
FILE* run_length_comp(unsigned char* a1, unsigned int a2, unsigned char a3) {
    switch (a3) {
        case 1:
            return compress_bytes(a1, a2);
        case 2:
            return reinterpret_cast<FILE*>(static_cast<long>(compress_words(a1, a2)));
        case 4:
            return reinterpret_cast<FILE*>(static_cast<long>(comp_DWORD(a1, a2)));
        default:
            return nullptr;
    }
}

/**
 * BYTE 解壓縮
 */
void decomp_byte(const unsigned char* compressedData, unsigned int compressedLength, unsigned char* outputData) {
    unsigned int readIndex = 0;
    unsigned int writeIndex = 0;

    while (readIndex < compressedLength) {
        unsigned char current = compressedData[readIndex++];
        if (current == 0xB4) {
            unsigned char next = compressedData[readIndex++];
            if (next == 0xB4) {
                outputData[writeIndex++] = 0xB4;
            }
            else {
                unsigned char count = compressedData[readIndex++];
                int fillVal = (next << 24) | (next << 16) | (next << 8) | next;
                unsigned char* fillPtr = &outputData[writeIndex];
                size_t dwords = count >> 2;
                memset32(fillPtr, fillVal, dwords);
                for (unsigned char k = 0; k < (count & 3); ++k) {
                    fillPtr[4 * dwords + k] = next;
                }
                writeIndex += count;
            }
        }
        else {
            outputData[writeIndex++] = current;
        }
    }
}

/**
 * WORD 解壓縮
 */
void decomp_word(const unsigned char* compressedData, unsigned int compressedLength, unsigned char* outputData) {
    unsigned int readWordCount = 0;
    unsigned int writeOffset = 0;
    unsigned int totalWords = compressedLength >> 1;

    while (readWordCount < totalWords) {
        short current = *reinterpret_cast<const short*>(compressedData + readWordCount * 2);
        if (current == static_cast<short>(0xB4B4)) {
            short value = *reinterpret_cast<const short*>(compressedData + (++readWordCount) * 2);
            if (value == static_cast<short>(0xB4B4)) {
                *reinterpret_cast<short*>(&outputData[writeOffset]) = static_cast<short>(0xB4B4);
                writeOffset += 2;
            }
            else {
                unsigned short count = *reinterpret_cast<const unsigned short*>(compressedData + (++readWordCount) * 2);
                int fillVal = (value << 16) | (value & 0xFFFF);
                unsigned char* fillPtr = &outputData[writeOffset];
                size_t dwords = count >> 1;
                memset32(fillPtr, fillVal, dwords);
                if (count & 1) {
                    *reinterpret_cast<short*>(&fillPtr[4 * dwords]) = value;
                }
                writeOffset += count * 2;
            }
        }
        else {
            *reinterpret_cast<short*>(&outputData[writeOffset]) = current;
            writeOffset += 2;
        }
        ++readWordCount;
    }
}

/**
 * DWORD 解壓縮
 */
void decomp_dword(const unsigned char* compressedData, unsigned int compressedLength, unsigned char* outputData) {
    unsigned int readDwordCount = 0;
    unsigned int writeOffset = 0;
    unsigned int totalDwords = compressedLength >> 2;

    while (readDwordCount < totalDwords) {
        int current = *reinterpret_cast<const int*>(compressedData + readDwordCount * 4);
        if (current == static_cast<int>(0xB4B4B4B4)) {
            int value = *reinterpret_cast<const int*>(compressedData + (++readDwordCount) * 4);
            if (value == static_cast<int>(0xB4B4B4B4)) {
                *reinterpret_cast<int*>(&outputData[writeOffset]) = static_cast<int>(0xB4B4B4B4);
                writeOffset += 4;
            }
            else {
                unsigned int count = *reinterpret_cast<const unsigned int*>(compressedData + (++readDwordCount) * 4);
                memset32(&outputData[writeOffset], value, count);
                writeOffset += count * 4;
            }
        }
        else {
            *reinterpret_cast<int*>(&outputData[writeOffset]) = current;
            writeOffset += 4;
        }
        ++readDwordCount;
    }
}

// 執行長度解壓縮
void run_length_decomp(unsigned char* a3, unsigned int a4, unsigned char* a5, unsigned int a6, unsigned char a7) {
    if (a3 && a5) {
        switch (a7) {
            case 1:
                decomp_byte(a3, a4, a5);
                break;
            case 2:
                decomp_word(a3, a4, a5);
                break;
            case 4:
                decomp_dword(a3, a4, a5);
                break;
            default:
                break;
        }
    }
}