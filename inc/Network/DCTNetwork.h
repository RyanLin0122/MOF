#pragma once

#include <cstdint>
#include <winsock2.h>

class DCTNetwork {
public:
  using OnSocketClosedFunc = int (*)(void);

  static void InitializeStaticVariable(void (__cdecl *func)());

  DCTNetwork();
  virtual ~DCTNetwork();

  BOOL Initialize();
  int Connect(const char* cp, u_short hostshort);
  void Disconnect();

  BOOL IsConnected() const;
  BOOL IsReadable();
  BOOL IsWriteable();

  int SetSendData(char *src, uint16_t len);
  int Send();
  int Read();
  void Poll();

  virtual void OnRecv() = 0;

protected:
  SOCKET m_socket;
  fd_set m_fdSet;
  char m_recvBuffer[30000];
  int m_recvLength;
  int m_sendLength;
  char m_sendBuffer[30000];

  static OnSocketClosedFunc m_pOnSocketClosedFuncPtr;
};
