#include "Info/cltFrontServerInfo.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <Windows.h>

// ====== 全域物件預設值 ======
// dword_BE4BB4 (mofclient.c:206043) — 預設不顯示錯誤
unsigned int  g_DebugLevel = 0;

// ----------------------------------------------------------------------------
// ctor — mofclient.c:205920
//   *((_WORD *)this + 120) = 0;     // m_wCount       (offset 240)
//   *((_WORD *)this + 121) = 0;     // m_wIndex       (offset 242)
//   *((_DWORD *)this + 61) = 1;     // m_dwFirstCall  (offset 244)
//   memset(this, 0, 0xF0u);         // 只清前 240 bytes (m_aInfos)
// ----------------------------------------------------------------------------
cltFrontServerInfo::cltFrontServerInfo() {
    m_wCount      = 0;
    m_wIndex      = 0;
    m_dwFirstCall = 1;
    std::memset(m_aInfos, 0, sizeof(m_aInfos));   // 0xF0 = 240
}

// ----------------------------------------------------------------------------
// Initialize — mofclient.c:205930 .. 206051
// ----------------------------------------------------------------------------
int cltFrontServerInfo::Initialize() {
    // v18 對應反編譯區域變數 — 預設 0 (失敗)；唯有跑到 LABEL_25 才會置 1
    int v18 = 0;

    // 反編譯：(unsigned __int8)MoFFont::GetNationCode(...) == 4
    const unsigned char nationCode = static_cast<unsigned char>(g_MoFFont.GetNationCode());

    if (nationCode == 4 ||
        FileCrypt::GetInstance()->DecoderFileCrypt("MofData/FrontServerList.dat", "tmp.dat"))
    {
        // 反編譯：NationCode==4 直接 fopen 原檔；否則 fopen tmp.dat
        const char* path = (nationCode == 4) ? "MofData/FrontServerList.dat" : "tmp.dat";
        v18 = ParseFromFile(path);
    }
    else if (g_DebugLevel == 2) {
        // 反編譯 mofclient.c:206043..206046：dword_BE4BB4 == 2 才彈錯誤訊息
        char Text[1024] = { 0 };
        std::snprintf(Text, sizeof(Text), "FrontServerInfo Error : %d", 1);
        MessageBoxA(nullptr, Text, "Master of Fantasy", 0);
    }

    // 反編譯 mofclient.c:206048..206049：if (NationCode != 4) DeleteFileA("tmp.dat");
    if (nationCode != 4) {
        DeleteFileA("tmp.dat");
    }
    return v18;
}

// ----------------------------------------------------------------------------
// GetRandomServerInfo — mofclient.c:206056 .. 206067
//   if ( !*((_DWORD *)this + 61) ) return 0;
//   v2 = ++*((_WORD *)this + 121) < *((_WORD *)this + 120);
//   *((_DWORD *)this + 61) = 0;
//   if ( !v2 ) *((_WORD *)this + 121) = 0;
//   return (char *)this + 24 * *((unsigned __int16 *)this + 121);
// ----------------------------------------------------------------------------
const stFrontServerInfo* cltFrontServerInfo::GetRandomServerInfo() {
    if (m_dwFirstCall == 0) {
        return nullptr;
    }
    m_wIndex = static_cast<uint16_t>(m_wIndex + 1);
    const bool less = (m_wIndex < m_wCount);
    m_dwFirstCall = 0;
    if (!less) {
        m_wIndex = 0;
    }
    // 忠實還原：原始實作不檢查 count 是否為 0 — 直接回傳對應槽位 (此時是 m_aInfos[0])
    return &m_aInfos[m_wIndex];
}

// ----------------------------------------------------------------------------
// ParseFromFile — 反編譯 mofclient.c:205966..206041 之解析子流程
// 嚴格還原 v18 的成功/失敗語意：
//   - 三行表頭任一行 fgets 失敗     → 直接回 0
//   - 計數迴圈後第一筆資料 fgets 失敗 (檔案無資料行)
//                                   → v18 = 1 (LABEL_25 入口)
//   - 解析迴圈中遇到 token 缺漏      → break，v18 仍為 0
//   - 解析迴圈中讀港埠 token 缺漏    → goto LABEL_28，v18 仍為 0
//   - 解析迴圈讀完最後一行後 fgets EOF
//                                   → goto LABEL_25，v18 = 1，符合條件則洗牌
// ----------------------------------------------------------------------------
int cltFrontServerInfo::ParseFromFile(const char* filePath) {
    int v18 = 0;
    char Text[1024] = { 0 };
    const char Delimiter[3] = { '\t', '\n', '\0' };  // 反編譯：strcpy(Delimiter, "\t\n")

    FILE* fp = std::fopen(filePath, "rb");
    if (!fp) {
        return 0; // 對齊「v17/v2 為 NULL 則最終 fclose 跳過、return v18=0」的效果
    }

    // 反編譯：if ( fgets && fgets && fgets )  讀掉前三行表頭
    if (std::fgets(Text, 1023, fp) &&
        std::fgets(Text, 1023, fp) &&
        std::fgets(Text, 1023, fp))
    {
        // 反編譯：fgetpos → for (; fgets; ++m_wCount) ; → fsetpos
        std::fpos_t Position;
        std::fgetpos(fp, &Position);
        for (; std::fgets(Text, 1023, fp); ) {
            ++m_wCount;
        }
        std::fsetpos(fp, &Position);

        // 反編譯：if ( fgets(...) ) { while(strtok(buf,...)) {...} } else { goto LABEL_25; }
        if (std::fgets(Text, 1023, fp)) {
            // v16 = (char *)this; — 寫入指標起點即 m_aInfos[0]
            // 反編譯無越界保護：strcpy/atoi 直寫 v16；超過 10 筆時會踩到 m_wCount 等成員。
            // 為與 GT 1:1，此處不做防護，忠實還原。
            char* v16 = reinterpret_cast<char*>(m_aInfos);

            bool failure = false;     // break / goto LABEL_28 路徑 → v18 維持 0
            bool reachedEof = false;  // goto LABEL_25 路徑 → v18 = 1

            while (std::strtok(Text, Delimiter)) {              // 第 1 個 token: No.
                char* v3 = std::strtok(nullptr, Delimiter);     // 第 2 個 token: Type
                if (!v3) { failure = true; break; }

                if (std::strncmp("TEST", v3, std::strlen(v3)) == 0) {
                    // TEST 列：跳過 IP 與 Port 兩個 token，計數遞減
                    if (!std::strtok(nullptr, Delimiter) ||
                        !std::strtok(nullptr, Delimiter)) {
                        failure = true; break;
                    }
                    --m_wCount;  // 反編譯：--*((_WORD *)this + 120) (允許 wrap-around)
                }
                else {
                    // 非 TEST 列：寫入 m_aInfos[written]
                    char* v4 = std::strtok(nullptr, Delimiter); // IP / Host
                    if (!v4) { failure = true; break; }

                    // 反編譯 mofclient.c:205999  strcpy(v16, v4);  — 無長度限制
                    std::strcpy(v16, v4);

                    char* v5 = std::strtok(nullptr, Delimiter); // Port
                    if (!v5) {
                        // 反編譯：goto LABEL_28 — 直接跳出且 v18 維持 0
                        failure = true;
                        break;
                    }

                    // 反編譯 mofclient.c:206006  *((_DWORD *)v16 + 5) = atoi(v5);  // offset 20 (Port)
                    *reinterpret_cast<int*>(v16 + 20) = std::atoi(v5);
                    v16 += 24;  // 下一筆 slot
                }

                // 反編譯：迴圈尾 if ( !fgets(...) ) goto LABEL_25;  (LABEL_25 ≡ EOF 成功)
                if (!std::fgets(Text, 1023, fp)) {
                    reachedEof = true;
                    break;
                }
            }

            if (!failure) {
                // 對應 LABEL_25：v18 = 1，且 m_wCount > 1 才洗牌
                v18 = 1;
                if (m_wCount > 1) {
                    ShuffleLikeDecompiled();
                }
            }
            // (failure==true 路徑：v18 維持 0；不洗牌)
            (void)reachedEof; // 僅作流程註記
        }
        else {
            // 反編譯：if ( fgets ) { ... } else { LABEL_25 ... }
            // 此分支對應「資料行為 0 筆」的成功狀態
            v18 = 1;
            if (m_wCount > 1) {  // 必為假 (count==0)，純為對齊原始程式架構
                ShuffleLikeDecompiled();
            }
        }
    }
    // 三行表頭讀取失敗 → v18 維持 0

    std::fclose(fp);
    return v18;
}

// ----------------------------------------------------------------------------
// ShuffleLikeDecompiled — mofclient.c:206018..206034
//   for (i=0; i<m_wCount; ++i) {
//       r = rand() % m_wCount;
//       tmp = a[r];           // qmemcpy v20, this+24*r, 24
//       a[r] = a[i];          // qmemcpy this+8*(3*r), v9, 24
//       a[i] = tmp;           // qmemcpy v9, v20, 24
//       v9 += 24; ++i;
//   }
// 反編譯無 r<10 / i<10 之上限保護；m_wCount > 10 時會踩到 m_wCount 等尾端成員。
// 為與 GT 1:1 等價，此處同樣不做防護。
// ----------------------------------------------------------------------------
void cltFrontServerInfo::ShuffleLikeDecompiled() {
    for (uint16_t i = 0; i < m_wCount; ++i) {
        const uint16_t r = static_cast<uint16_t>(std::rand() % m_wCount);
        stFrontServerInfo tmp = m_aInfos[r];
        m_aInfos[r] = m_aInfos[i];
        m_aInfos[i] = tmp;
    }
}
