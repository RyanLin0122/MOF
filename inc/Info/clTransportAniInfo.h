#pragma once
#include <cstdint>
#include <cstddef>

// =============================================================================
// stTransportAniBlock  —  單一動作 (STOP/MOVE/...) 的動畫 frame 表
// mofclient.c 196708 內以 v5 base + 偏移的方式同位址寫入；每個 block 1004 bytes。
// =============================================================================
//
// 反編譯佐證（v5 = block 起點）：
//   v5[0..1]                    = count (uint16)         // offset 0
//   v5[4 + 4*idx ..]            = sscanf("%x") resource  // offset 4   (uint32 ×100)
//   v5[404 + 2*idx ..]          = atoi(frameIdx)         // offset 404 (uint16 ×100)
//   v5[604 + 2*idx ..]          = atoi(x)                // offset 604 (uint16 ×100)
//   v5[804 + 2*idx ..]          = atoi(y)                // offset 804 (uint16 ×100)
// 總計 4 + 400 + 200 + 200 + 200 = 1004。
// =============================================================================
struct stTransportAniBlock
{
    // 프레임 개수  / 該動作的 frame 數量  /  offset 0
    std::uint16_t count;

    // 4-byte 對齊 padding  /  offset 2
    std::uint16_t _pad;

    // 리소스 ID (이미지 atlas)  / 每 frame 的圖集資源 ID  /  offset 4 .. 403
    std::uint32_t resource[100];

    // frame index in atlas  / 每 frame 在資源內的 block index  /  offset 404 .. 603
    std::uint16_t frame[100];

    // x 좌표  / 每 frame 的 X 偏移  /  offset 604 .. 803
    std::uint16_t x[100];

    // y 좌표  / 每 frame 的 Y 偏移  /  offset 804 .. 1003
    std::uint16_t y[100];
};

static_assert(sizeof(stTransportAniBlock) == 1004, "stTransportAniBlock must be 1004 bytes");
static_assert(offsetof(stTransportAniBlock, count)    ==   0, "offset(count)    != 0");
static_assert(offsetof(stTransportAniBlock, resource) ==   4, "offset(resource) != 4");
static_assert(offsetof(stTransportAniBlock, frame)    == 404, "offset(frame)    != 404");
static_assert(offsetof(stTransportAniBlock, x)        == 604, "offset(x)        != 604");
static_assert(offsetof(stTransportAniBlock, y)        == 804, "offset(y)        != 804");


// =============================================================================
// 動작 인덱스 / 動作索引（與 mofclient.c 196745~196775 的順序完全一致）
// =============================================================================
enum class TransportAction : unsigned int
{
    STOP     = 0,   // STOP        block 起點 = this + 4
    MOVE     = 1,   // MOVE        block 起點 = this + 1008
    DIE      = 2,   // DIE         block 起點 = this + 2012
    ATTACK   = 3,   // ATTACK      block 起點 = this + 3016
    N_HITTED = 4,   // N_HITTED    block 起점 = this + 4020
    F_HITTED = 5,   // F_HITTED    block 起點 = this + 5024
    E_HITTED = 6,   // E_HITTED    block 起點 = this + 6028
    I_HITTED = 7    // I_HITTED    block 起點 = this + 7032
};


// =============================================================================
// clTransportAniInfo  —  載具動畫資訊（一檔配一物件，由 ClientTransportKindInfo
// lazy-load 並 cache）
// mofclient.c：196683 (ctor) / 196708 (Init) / 196823 (GetFrameInfo)
//                196854 (GetTotalFrameNum)
// 大小 0x1F68 = 8040 bytes（operator new(0x1F68u) — mofclient.c 195160/195201）
// =============================================================================
class clTransportAniInfo
{
public:
    clTransportAniInfo();
    virtual ~clTransportAniInfo();

    // 解析 .txt（car_up.txt / car_down.txt 等）；成功回傳 1。
    // mofclient.c 196708：使用 "\t\n,[]" 為 delimiter 一次讀完整行。
    int Init(char* filename);

    // 取出指定動作的當前 frame 對應 (resource, frame index)。
    // mofclient.c 196823：a2 ≥ 8 或 curFrame ≥ count 時回傳 0；
    // 同時將 *outRes / *outFrame 預先寫 0（即使早退）。
    int GetFrameInfo(unsigned int action,
                     std::uint16_t curFrame,
                     unsigned int* outRes,
                     std::uint16_t* outFrame);

    // 取出指定動作的 frame 數量。
    // mofclient.c 196854：a2 ≥ 8 → 回傳 0。
    std::uint16_t GetTotalFrameNum(unsigned int action);

private:
    // mofclient.c 196686 (32-bit GT)：memset((char*)this+4, 0, 0x1F60u)
    // → GT offset 4..8035 共 8032 bytes 為 8 個 1004-byte block。
    // x64 build 上 vftable = 8 bytes，m_blocks 起點移到 offset 8；parser 全程
    // 透過 m_blocks[action] / 成員存取，與 vftable 大小無關。
    stTransportAniBlock m_blocks[8];   // GT offset 4 .. 8035

    // mofclient.c 196784：*((_WORD *)this + 4018) = atoi(token)
    // → GT byte offset 8036；ATTACK_KEY 行的攻擊發動 frame index。
    // 注意：建構式僅 memset 8 個 block，m_attackKey 在 Init 前未初始化（與 GT 同）。
    std::uint16_t m_attackKey;         // GT offset 8036

    // 4-byte 對齊 padding  /  GT offset 8038
    std::uint16_t _pad;
};
