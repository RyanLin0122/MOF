#include "Info/cltInstantDungeonMapKindInfo.h"

// 反編譯出處：mofclient.c:303942 cltInstantDungeonMapKindInfo::Initialize
//   - fgets 緩衝大小固定為 0x1FFF（原始 stack 上 8192-byte buffer）
//   - 分隔字串為 "\t\n"
//   - 跳過 3 行表頭，第 1 次 pass 計數，第 2 次 pass 解析
//   - 任一 strtok 失敗 → return 0；EOF 達成 → return 1；m_count==0 → return 1
//   - 與 disasm 一致：解析失敗時不主動釋放部分配置（由 dtor 處理）

int cltInstantDungeonMapKindInfo::Initialize(char* filename)
{
    char Delimiter[3];
    Delimiter[0] = '\t';
    Delimiter[1] = '\n';
    Delimiter[2] = '\0';

    int   v44 = 0; // 反編譯內 v44 對應到回傳值
    FILE* v3 = g_clTextFileManager.fopen(filename);
    FILE* v4 = v3;
    FILE* Stream = v3;

    if (v3)
    {
        char Buffer[8192];
        std::memset(Buffer, 0, sizeof(Buffer));

        // 表頭 3 行
        if (std::fgets(Buffer, 0x1FFF, v4) &&
            std::fgets(Buffer, 0x1FFF, v4) &&
            std::fgets(Buffer, 0x1FFF, v4))
        {
            // 第一次走訪：計算資料行數
            fpos_t Position{};
            std::fgetpos(v4, &Position);
            for (; std::fgets(Buffer, 0x1FFF, v4); ++m_count)
                ;

            int v5 = m_count;
            if (!v5)
                return 1; // ← disasm 在此處直接 return 1，未呼叫 fclose（保持一致）

            // 配置與清零
            void* v7 = ::operator new(static_cast<size_t>(236) * v5);
            int   v8 = m_count;
            m_list = static_cast<strInstantDungeonMapKindInfo*>(v7);
            std::memset(v7, 0, static_cast<size_t>(236) * v8);

            std::fsetpos(v4, &Position);

            char* v9_byte = reinterpret_cast<char*>(m_list);
            char* v42_byte = v9_byte;

            if (std::fgets(Buffer, 0x1FFF, v4))
            {
            LABEL_12:
                {
                    char* v10 = std::strtok(Buffer, Delimiter);
                    if (v10)
                    {
                        // 0x00 i_map_id (WORD)
                        *reinterpret_cast<uint16_t*>(v9_byte + 0) = TranslateKindCode(v10);

                        char* v11 = std::strtok(nullptr, Delimiter);
                        if (v11)
                        {
                            // 0x02 map_id (WORD)
                            *reinterpret_cast<uint16_t*>(v9_byte + 2) = cltMapInfo::TranslateKindCode(v11);

                            char* v12 = std::strtok(nullptr, Delimiter);
                            if (v12)
                            {
                                // 0x04 x (DWORD)
                                *reinterpret_cast<int32_t*>(v9_byte + 4) = std::atoi(v12);

                                char* v13 = std::strtok(nullptr, Delimiter);
                                if (v13)
                                {
                                    // 0x08 y (DWORD)
                                    *reinterpret_cast<int32_t*>(v9_byte + 8) = std::atoi(v13);

                                    // ----- 傳送門 #1, #2 -----
                                    // disasm: v15 = (DWORD*)(v9 + 16); 每組 4 dwords
                                    int32_t* v15 = reinterpret_cast<int32_t*>(v9_byte + 16);
                                    int v14 = 0;
                                    while (true)
                                    {
                                        char* v16 = std::strtok(nullptr, Delimiter);
                                        if (!v16) break;
                                        *(v15 - 1) = GetIndunPortalType(v16);

                                        char* v17 = std::strtok(nullptr, Delimiter);
                                        if (!v17) break;
                                        *v15 = static_cast<int32_t>(TranslateKindCode(v17));

                                        char* v18 = std::strtok(nullptr, Delimiter);
                                        if (!v18) break;
                                        v15[1] = std::atoi(v18);

                                        char* v19 = std::strtok(nullptr, Delimiter);
                                        if (!v19) break;
                                        v15[2] = std::atoi(v19);

                                        ++v14;
                                        v15 += 4;
                                        if (v14 < 2) continue;

                                        // ----- 一般怪物 m1..m4 -----
                                        // disasm: v21 = (int*)(v42 + 48)
                                        int32_t* v21 = reinterpret_cast<int32_t*>(v42_byte + 48);
                                        int v20 = 0;
                                        while (true)
                                        {
                                            char* v22 = std::strtok(nullptr, Delimiter);
                                            if (!v22) goto LABEL_44;
                                            // mob_id 在 v21 前 4 bytes (一個 DWORD 之前 = WORD 訪問)
                                            *(reinterpret_cast<uint16_t*>(v21) - 2) = cltCharKindInfo::TranslateKindCode(v22);

                                            char* v23 = std::strtok(nullptr, Delimiter);
                                            if (!v23) goto LABEL_44;
                                            *v21 = std::atoi(v23);

                                            char* v24 = std::strtok(nullptr, Delimiter);
                                            if (!v24) goto LABEL_44;
                                            v21[1] = std::atoi(v24);

                                            char* v25 = std::strtok(nullptr, Delimiter);
                                            if (!v25) goto LABEL_44;
                                            v21[2] = std::atoi(v25);

                                            char* v26 = std::strtok(nullptr, Delimiter);
                                            if (!v26) goto LABEL_44;
                                            v21[3] = std::atoi(v26);

                                            char* v27 = std::strtok(nullptr, Delimiter);
                                            if (!v27) goto LABEL_44;
                                            v21[4] = v21[2] + std::atoi(v27); // width = x + dx

                                            char* v28 = std::strtok(nullptr, Delimiter);
                                            if (!v28) goto LABEL_44;
                                            v21[5] = v21[3] + std::atoi(v28); // height = y + dy

                                            char* v29 = std::strtok(nullptr, Delimiter);
                                            if (!v29) goto LABEL_44;
                                            v21[6] = std::atoi(v29);

                                            ++v20;
                                            v21 += 8;
                                            if (v20 < 4) continue;

                                            // ----- 稀有怪物 r1..r2 -----
                                            // disasm: v31 = (int*)(v42 + 176)
                                            int32_t* v31 = reinterpret_cast<int32_t*>(v42_byte + 176);
                                            int v30 = 0;
                                            while (true)
                                            {
                                                char* v32 = std::strtok(nullptr, Delimiter);
                                                if (!v32) goto LABEL_44;
                                                *(reinterpret_cast<uint16_t*>(v31) - 2) = cltCharKindInfo::TranslateKindCode(v32);

                                                char* v33 = std::strtok(nullptr, Delimiter);
                                                if (!v33) goto LABEL_44;
                                                *v31 = std::atoi(v33);

                                                char* v34 = std::strtok(nullptr, Delimiter);
                                                if (!v34) goto LABEL_44;
                                                v31[1] = std::atoi(v34);

                                                char* v35 = std::strtok(nullptr, Delimiter);
                                                if (!v35) goto LABEL_44;
                                                v31[2] = std::atoi(v35);

                                                char* v36 = std::strtok(nullptr, Delimiter);
                                                if (!v36) goto LABEL_44;
                                                v31[3] = std::atoi(v36);

                                                char* v37 = std::strtok(nullptr, Delimiter);
                                                if (!v37) goto LABEL_44;
                                                // disasm: v31[4] = *(v31 - 30) + atoi
                                                //   v31 - 30 (DWORD 步) = 跳到 m{1|2}_x
                                                v31[4] = *(v31 - 30) + std::atoi(v37);

                                                char* v38 = std::strtok(nullptr, Delimiter);
                                                if (!v38) goto LABEL_44;
                                                // v31[5] = *(v31 - 29) + atoi  → m{1|2}_y
                                                v31[5] = *(v31 - 29) + std::atoi(v38);

                                                char* v39 = std::strtok(nullptr, Delimiter);
                                                if (!v39) goto LABEL_44;
                                                v31[6] = std::atoi(v39);

                                                ++v30;
                                                v31 += 8;
                                                if (v30 < 2) continue;

                                                // 一行解析完成，前進 236 bytes 至下一筆
                                                v42_byte += 236;
                                                char* v40 = std::fgets(Buffer, 0x1FFF, Stream);
                                                v4 = Stream;
                                                if (v40)
                                                {
                                                    v9_byte = v42_byte;
                                                    goto LABEL_12;
                                                }
                                                goto LABEL_43;
                                            }
                                        }
                                    }
                                LABEL_44:
                                    v4 = Stream;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
            LABEL_43:
                v44 = 1;
            }
        }
        g_clTextFileManager.fclose(v4);
    }
    return v44;
}

void cltInstantDungeonMapKindInfo::Free()
{
    if (m_list)
    {
        ::operator delete(m_list);
        m_list = nullptr;
    }
    m_count = 0;
}

// disasm 邏輯：以 WORD 步進每筆 118 個 WORD（118*2 = 236 bytes），比對 *i == a2
strInstantDungeonMapKindInfo*
cltInstantDungeonMapKindInfo::GetInstantDungeonMapKindInfo(uint16_t a2)
{
    int v2 = m_count;
    int v3 = 0;
    if (v2 <= 0)
        return nullptr;

    uint16_t* v4 = reinterpret_cast<uint16_t*>(m_list);
    uint16_t* i  = v4;
    while (*i != a2)
    {
        if (++v3 >= v2)
            return nullptr;
        i += 118;
    }
    return reinterpret_cast<strInstantDungeonMapKindInfo*>(&v4[118 * v3]);
}

uint16_t cltInstantDungeonMapKindInfo::TranslateKindCode(char* a1)
{
    if (std::strlen(a1) != 5)
        return 0;
    int v2 = (std::toupper(static_cast<unsigned char>(*a1)) + 31) << 11;
    uint16_t v3 = static_cast<uint16_t>(std::atoi(a1 + 1));
    return (v3 < 0x800) ? static_cast<uint16_t>(v2 | v3) : static_cast<uint16_t>(0);
}

// disasm 等價：FIX=1, RANDOM=2, EXIT=3, 其他=0
//   反編譯展開為位元運算：v2 = -(EXIT != 0); LOBYTE(v2) &= 0xFD; return v2+3;
//   數學上即等於 (EXIT 命中 ? 3 : 0)，下方為等價簡化。
int cltInstantDungeonMapKindInfo::GetIndunPortalType(char* String1)
{
    if (_stricmp(String1, "FIX") == 0)    return 1;
    if (_stricmp(String1, "RANDOM") == 0) return 2;
    if (_stricmp(String1, "EXIT") == 0)   return 3;
    return 0;
}
