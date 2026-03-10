#include "Network/CMofMsg.h"

#include <cstring>
#include <windows.h>

void (*CMofMsg::m_pOnBufferMaxBytesFuncPtr)(unsigned int, unsigned int) = nullptr;
void (*CMofMsg::m_pOnBufferFulledFuncPtr)(unsigned int, unsigned int) = nullptr;
unsigned int CMofMsg::m_dwMaxUsageSize_log = 0;

void CMofMsg::InitializeStaticVariable(void (__cdecl *a1)(unsigned int, unsigned int), void (__cdecl *a2)(unsigned int, unsigned int)) {
  m_pOnBufferMaxBytesFuncPtr = a1;
  m_pOnBufferFulledFuncPtr = a2;
}

CMofMsg::CMofMsg() {
  m_bPacketOnServer = 0;
  InitCurrentPos();
  ResetSize();
}

CMofMsg::~CMofMsg() {
}

CMofMsg::CMofMsg(unsigned __int16 a2, int a3) {
  m_bPacketOnServer = a3;
  ResetSize();
  InitCurrentPos();
  if ( m_bPacketOnServer == 1 ) {
    Put_LONG(0);
  }
  Put_WORD(a2);
}

CMofMsg::CMofMsg(unsigned __int8 *a2, unsigned int a3) {
  m_pCurrentPos = a2;
  m_pMsg = a2;
  m_bPacketOnServer = 0;
  m_dwUsageSize = a3;
}

void CMofMsg::Put(char *a2, int a3) {
  const unsigned int usage = m_dwUsageSize;
  if ( usage + static_cast<unsigned int>(a3) < 0xFFFF ) {
    std::memcpy(m_pCurrentPos, a2, a3);
    const int newUsage = a3 + m_dwUsageSize;
    const int packetOnServer = m_bPacketOnServer;
    m_pCurrentPos += a3;
    m_dwUsageSize = newUsage;
    if ( packetOnServer ) {
      *reinterpret_cast<int *>(m_pMsg) = newUsage;
    }

    const unsigned int v7 = m_dwUsageSize;
    if ( (v7 & 0xFFFFFC00) > (m_dwMaxUsageSize_log & 0xFFFFFC00) ) {
      m_dwMaxUsageSize_log = v7;
      if ( m_pOnBufferMaxBytesFuncPtr ) {
        m_pOnBufferMaxBytesFuncPtr(0, v7);
      }
    }
  } else if ( m_pOnBufferFulledFuncPtr ) {
    m_pOnBufferFulledFuncPtr(0, usage);
  }
}

void CMofMsg::Put_Z1(char *a2) {
  char *v2 = a2;
  a2 = reinterpret_cast<char *>(strlen(a2) + 1);
  Put(reinterpret_cast<char *>(&a2), 1);
  Put(v2, static_cast<char>(reinterpret_cast<uintptr_t>(a2)) - 1);
}

void CMofMsg::Put_Z2(char *a2) {
  char *v2 = a2;
  a2 = reinterpret_cast<char *>(strlen(a2) + 1);
  Put(reinterpret_cast<char *>(&a2), 2);
  Put(v2, static_cast<__int16>(reinterpret_cast<uintptr_t>(a2)) - 1);
}

void CMofMsg::Put_Z4(char *a2) {
  char *v2 = a2;
  a2 = reinterpret_cast<char *>(strlen(a2) + 1);
  Put(reinterpret_cast<char *>(&a2), 4);
  Put(v2, static_cast<int>(reinterpret_cast<uintptr_t>(a2) - 1));
}

void CMofMsg::Put_LONG(int a2) {
  Put(reinterpret_cast<char *>(&a2), 4);
}

void CMofMsg::Put_DWORD(unsigned int a2) {
  Put(reinterpret_cast<char *>(&a2), 4);
}

void CMofMsg::Put_SHORT(__int16 a2) {
  Put(reinterpret_cast<char *>(&a2), 2);
}

void CMofMsg::Put_WORD(unsigned __int16 a2) {
  Put(reinterpret_cast<char *>(&a2), 2);
}

void CMofMsg::Put_BYTE(unsigned __int8 a2) {
  Put(reinterpret_cast<char *>(&a2), 1);
}

void CMofMsg::Put_INT64(char a2, int a3) {
  (void)a3;
  Put(&a2, 8);
}

int CMofMsg::Get(unsigned __int8 *a2, int a3) {
  const void *v3 = m_pCurrentPos;
  if ( reinterpret_cast<uintptr_t>(v3) + a3 <= reinterpret_cast<uintptr_t>(m_pMsg) + m_dwUsageSize ) {
    std::memcpy(a2, v3, a3);
    m_pCurrentPos += a3;
    return 1;
  }
  memset(a2, 0, a3);
  return 0;
}

int CMofMsg::Get_Z1(char *a2, int a3, int a4, char *a5) {
  unsigned __int8 v7[4] = {};
  int result = Get_BYTE(v7);
  if ( result ) {
    if ( v7[0] ) {
      if ( a3 && v7[0] - 1 > a3 ) {
        result = 0;
      } else {
        result = Get(reinterpret_cast<unsigned __int8 *>(a2), v7[0] - 1);
        if ( result ) {
          a2[v7[0] - 1] = 0;
          if ( !a4 || (result = VerifyStringValidity(a2)) != 0 ) {
            result = !a5 || IsIncludeViolationChars(a2, a5) != 1;
          }
        }
      }
    } else {
      result = 0;
    }
  }
  return result;
}

int CMofMsg::Get_Z2(char *a2, int a3, int a4, char *a5) {
  __int16 v7 = 0;
  int result = Get_SHORT(&v7);
  if ( result ) {
    if ( v7 > 0 ) {
      if ( a3 && v7 - 1 > a3 ) {
        result = 0;
      } else {
        result = Get(reinterpret_cast<unsigned __int8 *>(a2), v7 - 1);
        if ( result ) {
          a2[v7 - 1] = 0;
          if ( !a4 || (result = VerifyStringValidity(a2)) != 0 ) {
            result = !a5 || IsIncludeViolationChars(a2, a5) != 1;
          }
        }
      }
    } else {
      result = 0;
    }
  }
  return result;
}

int CMofMsg::Get_Z4(char *a2, int a3, int a4, char *a5) {
  int v7 = 0;
  int result = Get_LONG(&v7);
  if ( result ) {
    if ( v7 > 0 ) {
      if ( a3 && v7 - 1 > a3 ) {
        result = 0;
      } else {
        result = Get(reinterpret_cast<unsigned __int8 *>(a2), v7 - 1);
        if ( result ) {
          a2[v7 - 1] = 0;
          if ( !a4 || (result = VerifyStringValidity(a2)) != 0 ) {
            result = !a5 || IsIncludeViolationChars(a2, a5) != 1;
          }
        }
      }
    } else {
      result = 0;
    }
  }
  return result;
}

int CMofMsg::Get_BYTE(unsigned __int8 *a2) { return Get(a2, 1); }
int CMofMsg::Get_SHORT(__int16 *a2) { return Get(reinterpret_cast<unsigned __int8 *>(a2), 2); }
int CMofMsg::Get_WORD(unsigned __int16 *a2) { return Get(reinterpret_cast<unsigned __int8 *>(a2), 2); }
int CMofMsg::Get_LONG(int *a2) { return Get(reinterpret_cast<unsigned __int8 *>(a2), 4); }
int CMofMsg::Get_DWORD(unsigned int *a2) { return Get(reinterpret_cast<unsigned __int8 *>(a2), 4); }
int CMofMsg::Get_INT64(__int64 *a2) { return Get(reinterpret_cast<unsigned __int8 *>(a2), 8); }

unsigned __int8 *CMofMsg::GetMsg() { return reinterpret_cast<unsigned __int8 *>(this) + 4; }
unsigned __int8 *CMofMsg::GetCurrentPos() { return m_pCurrentPos; }
void CMofMsg::SetCurrentPos(unsigned __int8 *a2) { m_pCurrentPos = a2; }

void CMofMsg::InitCurrentPos() {
  m_pCurrentPos = reinterpret_cast<unsigned __int8 *>(this) + 4;
  m_pMsg = reinterpret_cast<unsigned __int8 *>(this) + 4;
}

void CMofMsg::ResetSize() { m_dwUsageSize = 0; }

int CMofMsg::GetLength() { return static_cast<int>(m_pCurrentPos - reinterpret_cast<unsigned __int8 *>(this) - 4); }

int CMofMsg::GetLeftUsageSize() {
  const int v1 = static_cast<int>(m_pMsg - m_pCurrentPos + m_dwUsageSize);
  return v1 != 0 ? v1 : 0;
}

unsigned long *CMofMsg::operator=(unsigned long *a2) {
  unsigned long *const result = reinterpret_cast<unsigned long *>(this);
  if ( result != a2 ) {
    std::memcpy(result + 1, a2 + 1, 0xFFFFu);
    result[16385] = a2[16385];
  }
  return result;
}

unsigned int CMofMsg::EnCodeCheckSum(unsigned int a2, unsigned int a3) { return a2 + (a3 ^ a2); }
unsigned int CMofMsg::DeCodeCheckSum(unsigned int a2, unsigned int a3) { return a2 ^ (a3 - a2); }
void CMofMsg::AddMsg(CMofMsg *a2) { Put(reinterpret_cast<char *>(a2->m_pMsg), a2->m_dwUsageSize); }
int CMofMsg::IsPacketOnServer() const { return m_bPacketOnServer; }

int CMofMsg::VerifyStringValidity(char *a2) {
  const unsigned int v2 = strlen(a2) + 1;
  const int v3 = static_cast<int>(v2 - 1);
  if ( v2 == 1 ) {
    return 1;
  }

  int v4 = 0;
  if ( static_cast<int>(v2 - 1) <= 0 ) {
    return 1;
  }

  while ( true ) {
    if ( IsDBCSLeadByte(static_cast<BYTE>(a2[v4])) ) {
      if ( ++v4 >= v3 ) {
        break;
      }
    }
    if ( ++v4 >= v3 ) {
      return 1;
    }
  }
  return 0;
}

int CMofMsg::IsIncludeViolationChars(char *a2, char *a3) {
  if ( strlen(a3) ) {
    strlen(a2);
  }
  return 0;
}

CMoFMsgMgr::CMoFMsgMgr() {
  ClearMsg();
}

CMoFMsgMgr::~CMoFMsgMgr() {
}

int CMoFMsgMgr::AddMsg(unsigned __int8 *a2, unsigned __int16 a3) {
  int v4 = a3;
  if ( 0x100000 - m_dwUsageSize >= a3 ) {
    const unsigned int v6 = m_dwMsgNum;
    if ( v6 < 0x2710 ) {
      MsgInfo &info = m_infos[v6];
      const auto v8 = reinterpret_cast<uintptr_t>(m_pWritePos);
      info.first = m_pWritePos;
      if ( v8 + static_cast<unsigned int>(a3) < reinterpret_cast<uintptr_t>(this) + 0x100000 ) {
        info.firstLen = a3;
        info.second = nullptr;
        info.secondLen = 0;
        std::memcpy(m_pWritePos, a2, a3);
        m_pWritePos += a3;
      } else {
        const uintptr_t v9 = reinterpret_cast<uintptr_t>(this) - v8 + 0x100000;
        info.firstLen = static_cast<unsigned __int16>(reinterpret_cast<uintptr_t>(this) - v8);
        std::memcpy(m_pWritePos, a2, v9);
        v4 = a3;
        info.second = m_buffer;
        info.secondLen = static_cast<unsigned __int16>(a3 - (reinterpret_cast<uintptr_t>(this) - v8));
        unsigned __int8 *v10 = &a2[v9];
        const unsigned int v11 = a3 - static_cast<unsigned int>(v9);
        std::memcpy(this, v10, v11);
        m_pWritePos = m_buffer + v11;
      }
      ++m_dwMsgNum;
      m_dwUsageSize += v4;
      return 1;
    }
    MessageBoxA(0, "Message info buffer is not enough size to receive.", "assert", 0);
    return 0;
  }
  MessageBoxA(0, "Message buffer is not enough size to receive.", "assert", 0);
  return 0;
}

int CMoFMsgMgr::GetMsg(unsigned int a2, unsigned __int8 *const a3) {
  if ( a2 >= m_dwMsgNum ) {
    return 0;
  }

  MsgInfo &info = m_infos[a2];
  const void *v4 = info.first;
  if ( !v4 ) {
    return 0;
  }

  std::memcpy(a3, v4, info.firstLen);
  const void *v6 = info.second;
  if ( v6 ) {
    std::memcpy(&a3[info.firstLen], v6, info.secondLen);
  }

  if ( info.second ) {
    m_pReadPos = info.second + info.secondLen;
    return info.firstLen + info.secondLen;
  }

  m_pReadPos = info.first + info.firstLen;
  return info.firstLen + info.secondLen;
}

unsigned int CMoFMsgMgr::GetMsgNum() const {
  return m_dwMsgNum;
}

void CMoFMsgMgr::ClearMsg() {
  m_pWritePos = m_buffer;
  m_dwMsgNum = 0;
  m_pReadPos = m_buffer;
  m_dwUsageSize = 0;
}

