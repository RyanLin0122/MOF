#include "Info/cltClientPortalInfo.h"
#include "Logic/Map.h"

#include <cstring>
#include <cstdlib>
#include <windows.h>

// ---------------------------------------------------------------------------
// cltClientPortalInfo — reconstructed from mofclient.c
// sub_004E1350 .. sub_004E16F0
// ---------------------------------------------------------------------------

// 靜態成員 (mofclient.c 24220 / 24221)
cltPortalInfo* cltClientPortalInfo::m_pclPortalInfo = nullptr;
Map*           cltClientPortalInfo::m_pMap          = nullptr;

// (0x004E1350)
cltClientPortalInfo::cltClientPortalInfo()
{
    // mofclient.c constructor 只 return this，未初始化任何成員。
    //   cltClientPortalInfo *__thiscall cltClientPortalInfo::cltClientPortalInfo(cltClientPortalInfo *this)
    //   { return this; }
    // g_clClientPortalInfo 為全域物件，依賴 BSS 零值初始化。忠實對齊
    // 反編譯：此處同樣不做任何欄位賦值。
}

// Ground truth 沒有使用者定義的 destructor —— 由 compiler 產生 trivial dtor。

// (0x004E1370)
void cltClientPortalInfo::InitializeStaticVariable(cltPortalInfo* a1, Map* a2)
{
    // mofclient.c:
    //   cltClientPortalInfo::m_pclPortalInfo = a1;
    //   cltClientPortalInfo::m_pMap          = a2;
    m_pclPortalInfo = a1;
    m_pMap          = a2;
}

// (0x004E1390)
void cltClientPortalInfo::Init(uint16_t a2)
{
    // mofclient.c:
    //   cltClientPortalInfo::GetPortalInfo(this, a2, (int *)this + 1);
    //
    // (int *)this + 1 指向 this 的 offset 4 開始的 int 陣列 (共 10 筆)
    GetPortalInfo(a2, m_nIndexBuffer);
}

// (0x004E13B0)
void cltClientPortalInfo::GetPortalInfo(uint16_t a2, int* a3)
{
    // mofclient.c:
    //   v4 = cltPortalInfo::GetPortalCntInMap(m_pclPortalInfo, a2, a3);
    //   *((_DWORD *)this + 11) = v4;            // m_nPortalCount
    //   v5 = operator new(40 * v4);
    //   v6 = *((_DWORD *)this + 11);
    //   *(_DWORD *)this = v5;                   // m_pPortalBuffer (直接覆寫)
    //   v9 = 0;
    //   if ( v6 > 0 ) {
    //       v10 = 0;
    //       v7 = (int *)((char *)this + 4);     // 固定讀取 this+4 的 index 陣列
    //       do {
    //           v8 = cltPortalInfo::GetPortalInfoByIndex(m_pclPortalInfo, *v7++);
    //           qmemcpy((char *)v10 + *(_DWORD *)this, v8, 0x28u);  // 40 bytes
    //           ++v9;
    //           v10 += 10;  // 10 * 4 = 40 bytes
    //       } while ( v9 < *((_DWORD *)this + 11) );
    //   }
    //
    // 忠實對齊反編譯行為：
    //   * 不檢查 m_pclPortalInfo
    //   * 不釋放舊的 m_pPortalBuffer（原始 binary 直接覆寫造成 leak，
    //     呼叫端需先呼叫 Free）
    //   * 即使 count == 0 仍呼叫 operator new
    //   * 迴圈內固定讀取 (int*)this + 1 (m_nIndexBuffer)，忽略參數 a3
    int count = cltClientPortalInfo::m_pclPortalInfo->GetPortalCntInMap(a2, a3);
    m_nPortalCount = count;

    m_pPortalBuffer = static_cast<stPortalInfo*>(
        ::operator new(sizeof(stPortalInfo) * static_cast<std::size_t>(count)));

    if (count > 0) {
        int* v7 = m_nIndexBuffer;  // 對齊 ground truth: (int *)((char *)this + 4)
        int v9 = 0;
        int v10 = 0;
        do {
            stPortalInfo* v8 =
                cltClientPortalInfo::m_pclPortalInfo->GetPortalInfoByIndex(*v7++);
            std::memcpy(reinterpret_cast<char*>(m_pPortalBuffer) + v10,
                        v8,
                        0x28u);
            ++v9;
            v10 += 40;  // v10 += 10 (int*) == +40 bytes
        } while (v9 < m_nPortalCount);
    }
}

// (0x004E1440)
stPortalInfo* cltClientPortalInfo::GetPortalInfoByPortalID(uint16_t a2)
{
    // mofclient.c: return cltPortalInfo::GetPortalInfoByID(m_pclPortalInfo, a2);
    // 忠實對齊 ground truth：不做 m_pclPortalInfo null-check。
    return cltClientPortalInfo::m_pclPortalInfo->GetPortalInfoByID(a2);
}

// (0x004E1460)
void cltClientPortalInfo::Free()
{
    // mofclient.c:
    //   if ( *(_DWORD *)this ) {
    //       operator delete(*(void **)this);
    //       *(_DWORD *)this = 0;
    //   }
    //   *((_DWORD *)this + 11) = 0;
    if (m_pPortalBuffer) {
        ::operator delete(m_pPortalBuffer);
        m_pPortalBuffer = nullptr;
    }
    m_nPortalCount = 0;
}

// (0x004E1490)
unsigned char cltClientPortalInfo::IsPortalAction(int pt, int pt_4,
                                                  uint16_t a4,
                                                  uint16_t* a5,
                                                  unsigned char* a6,
                                                  uint16_t* a7,
                                                  int a8)
{
    // mofclient.c:
    //   if ( !*(_DWORD *)this )  return 1;
    //   v9 = 0;
    //   *((_WORD *)this + 24) = a4;             // m_wMapKind = a4
    //   if ( m_nPortalCount <= 0 ) return 1;
    //   v10 = a8;
    //   for ( i = 0; ; i += 40 ) {
    //       v12 = i + m_pPortalBuffer;
    //       if ( a4 == *(_WORD *)(v12 + 2) ) {  // MapID_1
    //           switch ( *(_WORD *)(v12 + 12) ) {  // PortalType_1
    //               case 1: case 2: case 3: case 0x65: v10 = 10; break;
    //               case 4: case 6: case 0x6F:         v10 = 50; break;
    //               case 0x70:                          v10 = 0;  break;
    //               default: break;
    //           }
    //           rc.left   = *(_DWORD *)(v12 + 4) - 25;
    //           rc.top    = v10 + *(_DWORD *)(v12 + 8);
    //           rc.right  = *(_DWORD *)(v12 + 4) + 25;
    //           rc.bottom = rc.top + 40;
    //       }
    //       if ( PtInRect(&rc, POINT{pt, pt_4}) ) break;
    //       if ( ++v9 >= m_nPortalCount ) return 1;
    //   }
    //   *a6 = *(_BYTE  *)(40 * v9 + m_pPortalBuffer + 36);
    //   v14 = 40 * v9 + m_pPortalBuffer;
    //   v15 = *(_WORD *)(v14 + 12);
    //   if ( v15 == 1 ) return 4;
    //   if ( v15 == 6 ) return 5;
    //   if ( *(_BYTE *)(v14 + 36) > (u8)a8 ) return 2;
    //   *a5 = *(_WORD *)v14;
    //   *a7 = *(_WORD *)(40 * v9 + m_pPortalBuffer + 12);
    //   return 3;
    if (!m_pPortalBuffer)
        return 1;

    int v9 = 0;
    m_wMapKind = a4;

    if (m_nPortalCount <= 0)
        return 1;

    int v10 = a8;
    RECT rc;  // ground truth 未初始化 rc（UB 但忠實對齊）

    for (int i = 0; ; i += 40) {
        char* v12 = reinterpret_cast<char*>(m_pPortalBuffer) + i;

        if (a4 == *reinterpret_cast<uint16_t*>(v12 + 2)) {
            uint16_t portalType = *reinterpret_cast<uint16_t*>(v12 + 12);
            switch (portalType) {
                case 1: case 2: case 3: case 0x65:
                    v10 = 10;
                    break;
                case 4: case 6: case 0x6F:
                    v10 = 50;
                    break;
                case 0x70:
                    v10 = 0;
                    break;
                default:
                    break;
            }
            int32_t posX = *reinterpret_cast<int32_t*>(v12 + 4);
            int32_t posY = *reinterpret_cast<int32_t*>(v12 + 8);
            rc.left   = posX - 25;
            rc.top    = v10 + posY;
            rc.right  = posX + 25;
            rc.bottom = rc.top + 40;
        }

        POINT point;
        point.x = pt;
        point.y = pt_4;
        if (PtInRect(&rc, point))
            break;

        if (++v9 >= m_nPortalCount)
            return 1;
    }

    char* v14 = reinterpret_cast<char*>(m_pPortalBuffer) + 40 * v9;
    *a6 = *reinterpret_cast<unsigned char*>(v14 + 36);
    int16_t v15 = *reinterpret_cast<int16_t*>(v14 + 12);

    if (v15 == 1)
        return 4;
    if (v15 == 6)
        return 5;
    if (*reinterpret_cast<unsigned char*>(v14 + 36) > static_cast<unsigned char>(a8))
        return 2;

    *a5 = *reinterpret_cast<uint16_t*>(v14);
    *a7 = *reinterpret_cast<uint16_t*>(v14 + 12);
    return 3;
}

// (0x004E1670)
int cltClientPortalInfo::GetPosX(uint16_t /*a2*/, uint16_t a3)
{
    // mofclient.c: return *(_DWORD *)(*(_DWORD *)this + 40 * a3 + 4);
    return *reinterpret_cast<int32_t*>(
        reinterpret_cast<char*>(m_pPortalBuffer) + 40 * a3 + 4);
}

// (0x004E1690)
int cltClientPortalInfo::GetPosY(uint16_t /*a2*/, uint16_t a3)
{
    // mofclient.c: return *(_DWORD *)(*(_DWORD *)this + 40 * a3 + 8);
    return *reinterpret_cast<int32_t*>(
        reinterpret_cast<char*>(m_pPortalBuffer) + 40 * a3 + 8);
}

// (0x004E16B0)
int cltClientPortalInfo::GetPortalID(uint16_t /*a2*/, uint16_t a3)
{
    // mofclient.c: return *(unsigned __int16 *)(*(_DWORD *)this + 40 * a3);
    return *reinterpret_cast<uint16_t*>(
        reinterpret_cast<char*>(m_pPortalBuffer) + 40 * a3);
}

// (0x004E16D0)
stPortalInfo* cltClientPortalInfo::GetPortalInfoInMap(uint16_t a2)
{
    // mofclient.c: return (stPortalInfo *)(*(_DWORD *)this + 40 * a2);
    return reinterpret_cast<stPortalInfo*>(
        reinterpret_cast<char*>(m_pPortalBuffer) + 40 * a2);
}

// (0x004E16F0)
uint16_t cltClientPortalInfo::GetPortalType(uint16_t /*a2*/, uint16_t a3)
{
    // mofclient.c: return *(_WORD *)(*(_DWORD *)this + 40 * a3 + 12);
    return *reinterpret_cast<uint16_t*>(
        reinterpret_cast<char*>(m_pPortalBuffer) + 40 * a3 + 12);
}
