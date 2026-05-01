#include "Info/cltResurrectInfo.h"

cltMapInfo* cltResurrectInfo::m_pclMapInfo = nullptr;

// mofclient.c:329654
cltResurrectInfo::cltResurrectInfo()
{
    std::memset(table_, 0, sizeof(table_)); // memset(this, 0, 0x4B0u)
    count_ = 0;                              // *((DWORD*)this + 300) = 0
}

// mofclient.c:329662 — verbatim port
//
// 控制流：
//   v17 = 0；fopen；NULL → return 0。
//   連續 3 次 fgets 跳過表頭；任一失敗 → fclose / return 0。
//   再 1 次 fgets 取第一筆資料；若 NULL → goto LABEL_17（v17=1，視為成功）。
//   while (count_ < 100)：
//     第 1 欄 strtok → TranslateKindCode → 寫入 ResurrectID（GT 在檢查前就寫入；
//       此時若 v7==0，slot 維持原本來自建構式 memset 的零，無觀察差異）→ 0 break。
//     第 2 欄 strtok → cltMapInfo::TranslateKindCode → 寫 MapID → 0 break。
//     第 3 欄 strtok / IsDigit → atoi 寫 X。
//     第 4 欄 strtok / IsDigit → atoi 寫 Y。
//     v5 += 3（即下一筆 entry）；++count_。
//     fgets 下一行；NULL → goto LABEL_17（v17=1）。
//   fclose / return v17。
//
// 註：GT 不在 Initialize 內重置 table_/count_；本 port 維持此行為（建構式已 memset）。
int cltResurrectInfo::Initialize(char* filename)
{
    char Delimiter[2];
    std::strcpy(Delimiter, "\t\n");
    int v17 = 0;

    FILE* fp = g_clTextFileManager.fopen(filename);
    if (!fp) return 0;

    char buf[1024] = { 0 }; // GT: Buffer + v19[1020] + v20(2) + v21(1) = 1024 bytes

    // 表頭 3 行
    if (std::fgets(buf, 1023, fp) &&
        std::fgets(buf, 1023, fp) &&
        std::fgets(buf, 1023, fp))
    {
        if (std::fgets(buf, 1023, fp))
        {
            while (count_ < 100)
            {
                char* v6 = std::strtok(buf, Delimiter);
                if (!v6) break;
                uint16_t v7 = TranslateKindCode(v6);
                table_[count_].ResurrectID = v7;        // *((WORD*)v5 - 2) = v7
                if (!v7) break;

                char* v8 = std::strtok(nullptr, Delimiter);
                if (!v8) break;
                uint16_t v9 = cltMapInfo::TranslateKindCode(v8);
                table_[count_].MapID = v9;              // *((WORD*)v5 - 1) = v9
                if (!v9) break;

                char* v10 = std::strtok(nullptr, Delimiter);
                if (!v10) break;
                if (!IsDigitString(v10)) break;
                table_[count_].X = std::atoi(v10);      // *v5 = atoi(...)

                char* v12 = std::strtok(nullptr, Delimiter);
                if (!v12 || !IsDigitString(v12)) break;
                table_[count_].Y = std::atoi(v12);      // v5[1] = atoi(...)

                ++count_;                                // v5 += 3 + ++count_

                if (!std::fgets(buf, 1023, fp)) {
                    v17 = 1;                             // LABEL_17
                    break;
                }
            }
        }
        else
        {
            v17 = 1;                                     // 無資料行也算成功
        }
    }

    g_clTextFileManager.fclose(fp);
    return v17;
}

// mofclient.c:329744
strResurrectInfo* cltResurrectInfo::GetResurrectInfo(uint16_t a2)
{
    for (int i = 0; i < count_; ++i) {
        if (table_[i].ResurrectID == a2) return &table_[i];
    }
    return nullptr;
}