// =============================================================================
// clTransportAniInfo  —  載具動畫資訊（單一檔對應一個物件）
// 對齊反編譯：mofclient.c 196683 (ctor) ~ 196863 (GetTotalFrameNum)
//
// x64 注意：32-bit GT 的 parser 用 `v5 = this + 4 + N*1004` 形式做 byte
// arithmetic 取得各動作 block 起點。x64 build 上 vftable 是 8 bytes，
// 不能照搬 +4。本實作改以 `m_blocks[action]` 做 block 取得，再以 byte
// offset 寫入欄位（offset 在 stTransportAniBlock 內部，與 vftable 大小無關）。
// =============================================================================
#include "Info/clTransportAniInfo.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "global.h"   // g_clTextFileManager

// IsAlphaNumeric 與 mofclient.c 342945 完全一致：
//   ""             → 1
//   全部 isalnum   → 1
//   其他           → 0
static int Ai_IsAlphaNumeric(const char* a1)
{
    if (!a1) return 0;
    if (!*a1) return 1;
    const char* v = a1;
    while (std::isalnum(static_cast<unsigned char>(*v)))
    {
        if (!*++v)
            return 1;
    }
    return 0;
}


clTransportAniInfo::clTransportAniInfo()
{
    // mofclient.c 196683 (32-bit GT)：memset((char*)this+4, 0, 0x1F60)
    //   → 8032 bytes (即 m_blocks[8] 全部) 清零。
    // x64 上不能照搬 +4；改以成員 sizeof 清空，邏輯等價。
    std::memset(m_blocks, 0, sizeof(m_blocks));
    // m_attackKey 在 GT ctor 中不被初始化；保持與 GT 同（Init 會覆寫）。
}

clTransportAniInfo::~clTransportAniInfo()
{
    // mofclient.c 196701：destructor 僅還原 vftable。
}

// -----------------------------------------------------------------------------
// 解析 car_up.txt / car_down.txt 等動畫檔（mofclient.c 196708）。
//
// 行格式（'\t' / '\n' / ',' / '[' / ']' 為 delimiter）：
//   STOP     [<resHex>,<frameDec>,<xDec>,<yDec>][...]...
//   MOVE     ...
//   DIE      ...
//   ATTACK   ...
//   N_HITTED ...
//   F_HITTED ...
//   E_HITTED ...
//   I_HITTED ...
//   ATTACK_KEY  <int>      ← 寫入 m_attackKey
//
// Reverse-engineered branch：
//   * 每讀一行：strtok 取第一個 token = 動作名稱
//   * 比對 STOP/MOVE/.../I_HITTED 設定 v5 = 該 block 起點（本實作改 cur）
//   * 若是 ATTACK_KEY：再讀一個 token 寫入 m_attackKey
//   * 之後迴圈讀後續 token：每 4 個一組 → resource(hex), frame, x, y；
//     寫入 cur->resource[count], cur->frame[count], cur->x[count], cur->y[count]
//     最後 ++cur->count
//   * IsAlphaNumeric 失敗或 token 缺漏 → break
//   * 任何 break 後仍會 fclose；只有「fgets EOF」才會把 v16 設成 1（成功）
// -----------------------------------------------------------------------------
int clTransportAniInfo::Init(char* String2)
{
    char Delimiter[8] = {};
    std::strcpy(Delimiter, "\t\n,[]");

    // GT 用 stack 上 10240 bytes 兩塊；x64 改 heap 避免 stack 浪費（與
    // cltPetAniInfo 相同對策）。
    const std::size_t kBuf = 10240;
    char* Buffer = static_cast<char*>(std::malloc(kBuf));
    char* String = static_cast<char*>(std::malloc(kBuf));
    if (!Buffer || !String)
    {
        if (Buffer) std::free(Buffer);
        if (String) std::free(String);
        return 0;
    }

    int v16 = 0;
    int v2  = 0;

    FILE* Stream = g_clTextFileManager.fopen(String2);
    if (Stream)
    {
        if (std::fgets(Buffer, static_cast<int>(kBuf - 1), Stream))
        {
            stTransportAniBlock* cur = nullptr;  // mofclient.c 局部 v5
            do
            {
                std::strcpy(String, Buffer);
                char* v7 = std::strtok(String, Delimiter);
                if (!v7) break;

                if (_stricmp(v7, "STOP") == 0)
                {
                    cur = &m_blocks[0];   // GT: this + 4
                }
                else if (_stricmp(v7, "MOVE") == 0)
                {
                    cur = &m_blocks[1];   // GT: this + 1008
                }
                else if (_stricmp(v7, "DIE") == 0)
                {
                    cur = &m_blocks[2];   // GT: this + 2012
                }
                else if (_stricmp(v7, "ATTACK") == 0)
                {
                    cur = &m_blocks[3];   // GT: this + 3016
                }
                else if (_stricmp(v7, "N_HITTED") == 0)
                {
                    cur = &m_blocks[4];   // GT: this + 4020
                }
                else if (_stricmp(v7, "F_HITTED") == 0)
                {
                    cur = &m_blocks[5];   // GT: this + 5024
                }
                else if (_stricmp(v7, "E_HITTED") == 0)
                {
                    cur = &m_blocks[6];   // GT: this + 6028
                }
                else if (_stricmp(v7, "I_HITTED") == 0)
                {
                    cur = &m_blocks[7];   // GT: this + 7032
                }
                else
                {
                    if (_stricmp(v7, "ATTACK_KEY") != 0)
                        break;  // 未知動作 → 中止解析

                    char* v8 = std::strtok(nullptr, Delimiter);
                    if (!v8) break;
                    // mofclient.c 196784：*((_WORD*)this + 4018) = atoi(v8)
                    m_attackKey = static_cast<std::uint16_t>(std::atoi(v8));
                    // 不更動 cur；下方 strtok 通常會回 null 直接進 LABEL_30。
                }

                char* v9 = std::strtok(nullptr, Delimiter);
                if (v9)
                {
                    while (Ai_IsAlphaNumeric(v9))
                    {
                        if (!cur) break;  // 防 ATTACK_KEY 為首行的極端 case

                        // resource (hex) → cur->resource[count]
                        std::sscanf(v9, "%x", &cur->resource[cur->count]);

                        // frame (dec)
                        char* v10 = std::strtok(nullptr, Delimiter);
                        if (!v10) break;
                        cur->frame[cur->count] =
                            static_cast<std::uint16_t>(std::atoi(v10));

                        // x (dec)
                        char* v11 = std::strtok(nullptr, Delimiter);
                        if (!v11) break;
                        cur->x[cur->count] =
                            static_cast<std::uint16_t>(std::atoi(v11));

                        // y (dec) — GT 在寫 y 時將 count post-increment
                        char* v12 = std::strtok(nullptr, Delimiter);
                        if (!v12) break;
                        cur->y[cur->count] =
                            static_cast<std::uint16_t>(std::atoi(v12));
                        ++cur->count;

                        v9 = std::strtok(nullptr, Delimiter);
                        if (!v9) goto LABEL_30;   // 行尾 → 進下一行
                    }
                    break;   // while 中 break 或 IsAlphaNumeric 失敗 → 中止
                }
            LABEL_30:
                v16 = 1;
            } while (std::fgets(Buffer, static_cast<int>(kBuf - 1), Stream));

            v2 = v16;
        }
        g_clTextFileManager.fclose(Stream);
    }

    std::free(Buffer);
    std::free(String);
    return v2;
}

// -----------------------------------------------------------------------------
// mofclient.c 196823：取出 (resource, frame index)。
//   *outRes = 0; *outFrame = 0; 即使早退也保留這兩行的副作用。
//   action ≥ 8：回 0。
//   curFrame ≥ block.count：回 0。
// 反編譯訪問位置（內部 byte offset）：
//   *((_WORD *)this + 502*a2 + 2)         ← block.count    (block 內 +0)
//   *((_DWORD*)this + 251*a2 + a3 + 2)    ← block.resource (block 內 +4)
//   *((_WORD *)this + 502*a2 + a3 + 204)  ← block.frame    (block 內 +404)
// -----------------------------------------------------------------------------
int clTransportAniInfo::GetFrameInfo(unsigned int action,
                                     std::uint16_t curFrame,
                                     unsigned int* outRes,
                                     std::uint16_t* outFrame)
{
    *outRes = 0;
    *outFrame = 0;

    if (action >= 8)
        return 0;

    stTransportAniBlock& blk = m_blocks[action];
    if (curFrame >= blk.count)
        return 0;

    *outRes   = blk.resource[curFrame];
    *outFrame = blk.frame[curFrame];
    return 1;
}

// -----------------------------------------------------------------------------
// mofclient.c 196854：取出 block.count；action ≥ 8 → 0。
// -----------------------------------------------------------------------------
std::uint16_t clTransportAniInfo::GetTotalFrameNum(unsigned int action)
{
    if (action >= 8)
        return 0;
    return m_blocks[action].count;
}
