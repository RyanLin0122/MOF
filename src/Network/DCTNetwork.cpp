#include "Network/DCTNetwork.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>

DCTNetwork::OnSocketClosedFunc DCTNetwork::m_pOnSocketClosedFuncPtr = nullptr;

void DCTNetwork::InitializeStaticVariable(void (__cdecl *func)()) {
  m_pOnSocketClosedFuncPtr = reinterpret_cast<OnSocketClosedFunc>(func);
}

DCTNetwork::DCTNetwork() {
  m_recvLength = 0;
  m_sendLength = 0;
  m_socket = static_cast<SOCKET>(-1);
}

DCTNetwork::~DCTNetwork() {
  Disconnect();
}

BOOL DCTNetwork::Initialize() {
  WSAData wsaData{};
  return !WSAStartup(0x202u, &wsaData) && wsaData.wVersion == 514;
}

int DCTNetwork::Connect(const char* cp, u_short hostshort) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(hostshort);

    if (InetPtonA(AF_INET, cp, &addr.sin_addr) != 1) {
        return 0; // IP 字串格式錯誤
    }

    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        return 0;
    }

    if (connect(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != SOCKET_ERROR) {
        return 1;
    }

    Disconnect();
    return 0;
}

void DCTNetwork::Disconnect() {
  if (m_socket != static_cast<SOCKET>(-1)) {
    closesocket(m_socket);
    m_socket = static_cast<SOCKET>(-1);
  }
}

BOOL DCTNetwork::IsConnected() const {
  return m_socket != static_cast<SOCKET>(-1);
}

BOOL DCTNetwork::IsReadable() {
  m_fdSet.fd_count = 0;
  m_fdSet.fd_array[m_fdSet.fd_count++] = m_socket;
  timeval timeout{0, 0};
  select(static_cast<int>(m_socket), &m_fdSet, nullptr, nullptr, &timeout);
  return __WSAFDIsSet(m_socket, &m_fdSet) != 0;
}

BOOL DCTNetwork::IsWriteable() {
  m_fdSet.fd_count = 0;
  m_fdSet.fd_array[m_fdSet.fd_count++] = m_socket;
  timeval timeout{0, 0};
  select(static_cast<int>(m_socket), nullptr, &m_fdSet, nullptr, &timeout);
  return __WSAFDIsSet(m_socket, &m_fdSet) != 0;
}

int DCTNetwork::SetSendData(char *src, uint16_t len) {
  const int cur = m_sendLength;
  if (cur + len > 30000) {
    return 0;
  }
  std::memcpy(m_sendBuffer + cur, src, len);
  m_sendLength += len;
  return 1;
}

int DCTNetwork::Send() {
  const int sendLen = m_sendLength;
  if (sendLen <= 0) {
    return 1;
  }

  const int written = send(m_socket, m_sendBuffer, sendLen, 0);
  if (written != -1) {
    std::memmove(m_sendBuffer, m_sendBuffer + written, m_sendLength - written);
    m_sendLength -= written;
    return 1;
  }

  Disconnect();
  if (m_pOnSocketClosedFuncPtr) {
    m_pOnSocketClosedFuncPtr();
  }
  return 0;
}

int DCTNetwork::Read() {
  const int curLen = m_recvLength;
  const int freeLen = 30000 - curLen;
  const int readLen = recv(m_socket, m_recvBuffer + curLen, freeLen, 0);
  if (readLen > 0) {
    m_recvLength += readLen;
    return freeLen;
  }

  if (m_pOnSocketClosedFuncPtr) {
    m_pOnSocketClosedFuncPtr();
  }
  Disconnect();
  return -1;
}

void DCTNetwork::Poll() {
  if (IsConnected()) {
    if (IsReadable() && Read() != -1) {
      OnRecv();
    }
    if (IsWriteable()) {
      Send();
    }
  }
}
