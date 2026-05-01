// =============================================================================
// clTransportKindInfo  —  載具種類資訊解析器
// 對齊反編譯：mofclient.c 339958 (ctor) ~ 340136 (TranslateKindCode)
// =============================================================================
#include "Info/clTransportKindInfo.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "global.h"   // g_clTextFileManager

// IsDigit 與 mofclient.c 342909 完全一致：
//   ""             → 1（true）
//   "[+-]?digits"  → 1
//   其他           → 0
static int Tk_IsDigit(const char* a1)
{
    if (!*a1) return 1;
    const char* v = a1;
    while (true)
    {
        if (*v == '-' || *v == '+')
            ++v;
        if (!std::isdigit(static_cast<unsigned char>(*v)))
            break;
        if (!*++v)
            return 1;
    }
    return 0;
}


clTransportKindInfo::clTransportKindInfo()
    : m_pList(nullptr)
    , m_nNum(0)
{
    // mofclient.c 339958：constructor 僅設定 vftable；
    // m_pList / m_nNum 在 BSS 全域上預設為 0，這裡顯式 0 對應之。
}

clTransportKindInfo::~clTransportKindInfo()
{
    // mofclient.c 339978：destructor 僅還原 vftable，不釋放 m_pList。
    // 為與 ground truth 行為一致，這裡也不主動 delete。
}

// -----------------------------------------------------------------------------
// 解析 TransportKindInfo.txt
//   Header (3 行)：
//     1) // 제목 ...
//     2) //교통 수단 ID는 ...
//     3) 자동차ID  자동차이름(기획자용)  ...
//   Data (\t 分隔，\n 結尾)：
//     I1024  붕붕카  57288  57289  car_up.txt  car_down.txt  1d000114  24  5a00004a
//
// 對齊 mofclient.c 339985：
//   1. fgets 讀過 3 行 header
//   2. fgetpos / for(;fgets;m_nNum++) 計算資料行數
//   3. operator new(84 * m_nNum) + memset 0
//   4. fsetpos / loop：每行依序 strtok("\t\n") 9 個 token，全部成功才前進 84 bytes
//   5. 任何 token 缺少 / 格式錯誤 → break；最後 v26 表示「至少有讀完最後一行 EOF」
// -----------------------------------------------------------------------------
int clTransportKindInfo::Initialize(char* String2)
{
    char Delimiter[2] = { '\t', '\n' };  // mofclient.c 340020：strcpy 寫入 "\t\n"
    char Buffer[1024] = {};
    int  v26 = 0;

    FILE* Stream = g_clTextFileManager.fopen(String2);
    if (!Stream)
        return 0;

    // -- 讀過 3 行 header；任一行 fgets 失敗就直接結束 -----------------------
    if (std::fgets(Buffer, 1023, Stream) &&
        std::fgets(Buffer, 1023, Stream) &&
        std::fgets(Buffer, 1023, Stream))
    {
        // -- 第一遍：fgetpos 後逐行計數 -------------------------------------
        fpos_t Position{};
        std::fgetpos(Stream, &Position);

        // mofclient.c：for(; fgets(...); ++*((_DWORD*)this + 2))
        for (; std::fgets(Buffer, 1023, Stream); ++m_nNum)
            ;

        // -- 配置 84 bytes × m_nNum 並清零 ----------------------------------
        m_pList = static_cast<stTransportKindInfo*>(operator new(sizeof(stTransportKindInfo) * m_nNum));
        std::memset(m_pList, 0, sizeof(stTransportKindInfo) * m_nNum);

        // -- 回到資料起點 ---------------------------------------------------
        std::fsetpos(Stream, &Position);

        // -- 第二遍：實際解析 ----------------------------------------------
        char* v7 = reinterpret_cast<char*>(m_pList);

        if (!std::fgets(Buffer, 1023, Stream))
        {
            // 整個 data 區塊空（極端情況）→ 仍視為成功
            v26 = 1;
        }
        else
        {
            while (true)
            {
                // token 0：자동차 ID  →  TranslateKindCode → kindCode (uint16, offset 0)
                char* v8 = std::strtok(Buffer, Delimiter);
                if (!v8) break;
                std::uint16_t v9 = TranslateKindCode(v8);
                *reinterpret_cast<std::uint16_t*>(v7) = v9;
                if (!v9) break;  // 339985：v9 == 0 直接 break（資料行格式錯誤或 EOF marker）

                // token 1：자동차 이름(기획자용)  →  讀過但不儲存
                if (!std::strtok(nullptr, Delimiter)) break;

                // token 2：자동차 이름 text code  →  uint16, offset 2
                char* v10 = std::strtok(nullptr, Delimiter);
                if (!v10) break;
                if (!Tk_IsDigit(v10)) break;
                *reinterpret_cast<std::uint16_t*>(v7 + 2) = static_cast<std::uint16_t>(std::atoi(v10));

                // token 3：자동차 설명 text code  →  uint16, offset 4
                char* v12 = std::strtok(nullptr, Delimiter);
                if (!v12) break;
                if (!Tk_IsDigit(v12)) break;
                *reinterpret_cast<std::uint16_t*>(v7 + 4) = static_cast<std::uint16_t>(std::atoi(v12));

                // token 4：애니메이션 정보 파일 앞쪽  →  char[32], offset 6
                //   必須包含 ".txt"；否則 break
                char* v14 = std::strtok(nullptr, Delimiter);
                if (!v14 || !std::strstr(v14, ".txt")) break;
                std::strcpy(v7 + 6, v14);

                // token 5：애니메이션 정보 파일 뒤쪽  →  char[32], offset 38
                //   必須包含 ".txt"；否則 break
                char* v16 = std::strtok(nullptr, Delimiter);
                if (!v16 || !std::strstr(v16, ".txt"))
                    break;
                std::strcpy(v7 + 38, v16);

                // token 6：아이콘 리소스 ID (hex)  →  uint32, offset 72
                char* v18 = std::strtok(nullptr, Delimiter);
                if (!v18) break;
                std::sscanf(v18, "%x", reinterpret_cast<unsigned int*>(v7 + 72));

                // token 7：아이콘 블럭 ID (10進)  →  uint16, offset 76
                char* v19 = std::strtok(nullptr, Delimiter);
                if (!v19) break;
                if (!Tk_IsDigit(v19)) break;
                *reinterpret_cast<std::uint16_t*>(v7 + 76) = static_cast<std::uint16_t>(std::atoi(v19));

                // token 8：자동차 이펙트 (hex)  →  uint32, offset 80
                char* v21 = std::strtok(nullptr, Delimiter);
                if (!v21) break;
                std::sscanf(v21, "%x", reinterpret_cast<unsigned int*>(v7 + 80));

                // -- 此筆完整解析成功，前進 84 bytes
                v7 += 84;

                // -- 讀下一行；EOF → 視為「全部成功」並跳出
                if (!std::fgets(Buffer, 1023, Stream))
                {
                    v26 = 1;
                    break;
                }
            }
        }
    }

    g_clTextFileManager.fclose(Stream);
    return v26;
}

// -----------------------------------------------------------------------------
// mofclient.c 340100：線性搜尋 kindCode 相符的條目。
// 反編譯版本以 _WORD* 步進 42（= 84 bytes / sizeof(uint16)），
// 還原為 stTransportKindInfo* +1 步進。找不到回傳 nullptr。
// -----------------------------------------------------------------------------
stTransportKindInfo* clTransportKindInfo::GetTransportKindInfo(std::uint16_t a2)
{
    if (m_nNum <= 0)
        return nullptr;

    int v3 = 0;
    stTransportKindInfo* p = m_pList;
    while (p[v3].kindCode != a2)
    {
        if (++v3 >= m_nNum)
            return nullptr;
    }
    return &p[v3];
}

// -----------------------------------------------------------------------------
// mofclient.c 340121：5-char string → 16-bit kindCode。
// 公式：(uint16_t)(((toupper(c0) + 31) << 11) | atoi(c1..c4))
// 例：'I' (0x49) → (0x49+31)<<11 = 0x68<<11 = 0x34000；
//     atoi("1024") = 0x400；
//     low-16(0x34000 | 0x400) = 0x4400 → kindCode for "I1024" = 0x4400。
// 規範：strlen != 5 或 atoi(...) ≥ 0x800 時回傳 0。
// -----------------------------------------------------------------------------
std::uint16_t clTransportKindInfo::TranslateKindCode(char* a1)
{
    if (std::strlen(a1) != 5)
        return 0;

    // 反編譯保留 IDA 的 int 中間值；再以 _WORD 截斷儲存。
    int v2 = (std::toupper(static_cast<unsigned char>(*a1)) + 31) << 11;
    int v3 = std::atoi(a1 + 1);
    if (static_cast<unsigned int>(v3) >= 0x800u)
        return 0;
    return static_cast<std::uint16_t>(v2 | v3);
}
