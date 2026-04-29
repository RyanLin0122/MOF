#pragma once
//
// tNPCKind — mofclient.c 還原（位址 0x4498B0）
//
// CUINPC::GetNPCKind 用 qmemcpy 64 bytes 拷貝出來的 NPC 種類描述。
// 唯一還原的 method 是 GetNPCID()，回傳 *((WORD*)this + 27) — 即 offset 54。
//
// 64-byte layout（依 mofclient.c 直接索引推導；其他欄位用途未明，保留為
// raw uint8 buffer 避免 stride 錯置）：
//   +0..+53  raw bytes
//   +54      uint16_t  m_wNPCID
//   +56..+63 raw bytes
//
#include <cstdint>

struct tNPCKind {
    uint8_t  m_aHead[54];   // +0..+53
    uint16_t m_wNPCID;      // +54
    uint8_t  m_aTail[8];    // +56..+63

    //----- (004498B0) ----------------------------------------------------
    unsigned short GetNPCID() const { return m_wNPCID; }
};

static_assert(sizeof(tNPCKind) == 64, "tNPCKind must be 64 bytes (mofclient.c qmemcpy 0x40)");
