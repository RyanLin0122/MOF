#include "Network/CMoFNetwork.h"
#include <cstring>

namespace {
int SendRaw(CMoFNetwork *network, unsigned __int16 opcode) {
  CMofMsg msg(opcode, 0);
  return network->SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}
}

CMoFNetwork::CMoFNetwork() = default;
CMoFNetwork::~CMoFNetwork() = default;

void CMoFNetwork::Initialize(void(__cdecl *a2)(unsigned __int8 *, unsigned __int16, unsigned int), unsigned int a3) {
  m_messageHandler = a2;
  m_messageHandlerContext = a3;
  this->DCTNetwork::Initialize();
}

void CMoFNetwork::Poll() {
  this->DCTNetwork::Poll();
  unsigned __int8 msg[65536] = {};
  const unsigned int msgNum = m_msgMgr.GetMsgNum();
  for (unsigned int i = 0; i < msgNum; ++i) {
    const int length = m_msgMgr.GetMsg(i, msg);
    if (length > 0 && m_messageHandler != nullptr) {
      m_messageHandler(msg, static_cast<unsigned __int16>(length), m_messageHandlerContext);
    }
  }
  m_msgMgr.ClearMsg();
}

void CMoFNetwork::OnRecv() {
  while (m_recvLength > 4) {
    const auto *header = reinterpret_cast<const unsigned __int16 *>(m_recvBuffer);
    const int packetSize = static_cast<int>(*header);
    if (packetSize < 4 || m_recvLength < packetSize) {
      break;
    }
    m_msgMgr.AddMsg(reinterpret_cast<unsigned __int8 *>(m_recvBuffer + 4), static_cast<unsigned __int16>(packetSize - 4));
    m_recvLength -= packetSize;
    std::memmove(m_recvBuffer, m_recvBuffer + packetSize, static_cast<size_t>(m_recvLength));
  }
}

int CMoFNetwork::SetSendData(char *a2, unsigned __int16 a3) {
  if (a2 == nullptr || a3 == 0) {
    return 0;
  }
  char sendBuffer[65536] = {};
  *reinterpret_cast<unsigned __int16 *>(sendBuffer) = static_cast<unsigned __int16>(a3 + 4);
  std::memcpy(sendBuffer + 4, a2, a3);
  return this->DCTNetwork::SetSendData(sendBuffer, static_cast<uint16_t>(a3 + 4));
}

int CMoFNetwork::Login(int a2) {
  CMofMsg msg(1u, 0);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::Move(unsigned __int16 a2, unsigned __int16 a3, unsigned __int8 a4, unsigned __int16 a5) {
  CMofMsg msg(0x1001u, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  msg.Put_BYTE(a4);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::Chatting(char *a2, unsigned __int8 a3, unsigned __int16 a4) {
  char *checked = CheckChatMsg(a2);
  if (checked == nullptr) {
    return 0;
  }
  CMofMsg msg(0x1002u, 0);
  msg.Put_BYTE(a3);
  msg.Put_WORD(a4);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PartyChatting(char *a2) {
  char *checked = CheckChatMsg(a2);
  if (checked == nullptr) {
    return 0;
  }
  CMofMsg msg(0x1010u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::Whisper(char *a2, char *a3) {
  char *checked = CheckChatMsg(a2);
  if (checked == nullptr) {
    return 0;
  }
  CMofMsg msg(0x51u, 0);
  msg.Put_Z1(checked);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MoveMap(unsigned __int16 a2) {
  CMofMsg msg(3u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ATTACK(unsigned int a2, unsigned int a3, int a4, unsigned __int8 a5, unsigned int *a6) {
  CMofMsg msg(0x1003u, 0);
  msg.Put_DWORD(a2);
  msg.Put_DWORD(a3);
  msg.Put_LONG(a4);
  msg.Put_BYTE(a5);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PickUpItem(unsigned __int16 a2, unsigned __int16 a3) {
  CMofMsg msg(5u, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DropItem(unsigned __int16 a2, unsigned __int16 a3) {
  CMofMsg msg(0xDu, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MoveItem(unsigned __int8 a2, unsigned __int8 a3) {
  CMofMsg msg(0xFu, 0);
  msg.Put_BYTE(a2);
  msg.Put_BYTE(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::UseItem(unsigned __int16 a2, char *a3, char *a4, char *a5, unsigned int a6, unsigned int a7, unsigned int a8, unsigned int a9, unsigned int a10) {
  CMofMsg msg(7u, 0);
  msg.Put_WORD(a2);
  msg.Put_Z1(a3);
  msg.Put_Z1(a4);
  msg.Put_Z2(a5);
  msg.Put_DWORD(a6);
  msg.Put_DWORD(a7);
  msg.Put_DWORD(a8);
  msg.Put_DWORD(a9);
  msg.Put_DWORD(a10);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::BuyItem(unsigned __int16 a2, struct cltItemList *a3) {
  CMofMsg msg(9u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SellItem(unsigned __int16 a2, struct cltItemList *a3) {
  CMofMsg msg(0xBu, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SysCommand(char *a2) {
  CMofMsg msg(0x1004u, 0);
  msg.Put_Z2(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ApplyAbility(unsigned __int16 a2, unsigned __int16 a3, unsigned __int16 a4, unsigned __int16 a5) {
  CMofMsg msg(0x1005u, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  msg.Put_WORD(a4);
  msg.Put_WORD(a5);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AddToQuickSlot(unsigned __int8 a2, unsigned __int8 a3, unsigned __int8 a4, unsigned int a5) {
  CMofMsg msg(0x1006u, 0);
  msg.Put_BYTE(a2);
  msg.Put_BYTE(a3);
  msg.Put_BYTE(a4);
  msg.Put_WORD(a5);
  msg.Put_BYTE(a5);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DelFromQuickSlot(unsigned __int8 a2, unsigned __int8 a3) {
  CMofMsg msg(0x1007u, 0);
  msg.Put_BYTE(a2);
  msg.Put_BYTE(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MoveOnQuickSlot(unsigned __int8 a2, unsigned __int8 a3, unsigned __int8 a4) {
  CMofMsg msg(0x1008u, 0);
  msg.Put_BYTE(a2);
  msg.Put_BYTE(a3);
  msg.Put_BYTE(a4);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::EquipItem(unsigned __int8 a2, unsigned __int8 a3, unsigned __int16 a4) {
  CMofMsg msg(0x11u, 0);
  msg.Put_BYTE(a2);
  msg.Put_BYTE(a3);
  msg.Put_BYTE(a4);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::UnEquipItem(unsigned __int8 a2, unsigned __int8 a3, unsigned __int16 a4) {
  CMofMsg msg(0x13u, 0);
  msg.Put_BYTE(a2);
  msg.Put_BYTE(a3);
  msg.Put_BYTE(a4);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::UnEquipAllItem(unsigned int a2) {
  CMofMsg msg(0x15u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MoveEquipItem(unsigned __int8 a2, unsigned __int8 a3, unsigned int a4) {
  CMofMsg msg(0x17u, 0);
  msg.Put_BYTE(a2);
  msg.Put_BYTE(a3);
  msg.Put_BYTE(a4);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::UpgradeClass(unsigned __int16 a2) {
  CMofMsg msg(0x19u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::UpgradeClassGiftItem(unsigned __int16 a2) {
  CMofMsg msg(0xEEu, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RunState(unsigned __int8 a2) {
  CMofMsg msg(0x1009u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::UseSkill(unsigned __int16 a2, unsigned __int16 a3, unsigned int *a4) {
  CMofMsg msg(0x21u, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::LessonFinished(unsigned __int8 a2, unsigned int a3, unsigned int a4, unsigned __int8 a5, unsigned __int16 a6, unsigned int a7) {
  CMofMsg msg(0x40u, 0);
  msg.Put_BYTE(a2);
  msg.Put_DWORD(a3);
  msg.Put_DWORD(a4);
  msg.Put_BYTE(a5);
  msg.Put_WORD(a6);
  msg.Put_DWORD(a7);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SetSchedule(unsigned __int16 a2, unsigned __int8 *a3) {
  CMofMsg msg(0x23u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CancelSchedule() {
  CMofMsg msg(0x25u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::BuySkill(unsigned __int16 a2) {
  CMofMsg msg(0x27u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::EnterMinigame() {
  CMofMsg msg(0x29u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::LeaveMinigame() {
  CMofMsg msg(0x2Bu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetMyChars(unsigned __int8 a2) {
  CMofMsg msg(0x2Du, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::Auth(char *a2) {
  CMofMsg msg(0x30u, 0);
  msg.Put_Z2(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CreateChar(unsigned __int8 a2, char *a3, unsigned __int8 a4, unsigned __int16 a5, unsigned __int8 a6, unsigned __int8 a7, unsigned __int8 a8, unsigned __int8 a9, unsigned __int8 a10, unsigned __int8 a11, unsigned __int16 a12, unsigned __int16 a13, unsigned __int16 a14) {
  CMofMsg msg(0x32u, 0);
  msg.Put_BYTE(a2);
  msg.Put_Z1(a3);
  msg.Put_BYTE(a4);
  msg.Put_WORD(a5);
  msg.Put_BYTE(a6);
  msg.Put_BYTE(a7);
  msg.Put_BYTE(a8);
  msg.Put_BYTE(a9);
  msg.Put_BYTE(a10);
  msg.Put_BYTE(a11);
  msg.Put_WORD(a12);
  msg.Put_WORD(a13);
  msg.Put_WORD(a14);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DeleteChar(int a2) {
  CMofMsg msg(0x34u, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestJoinParty(char *a2) {
  CMofMsg msg(0x36u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ResponseJoinParty(unsigned __int8 a2) {
  CMofMsg msg(0x38u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::LeaveParty() {
  CMofMsg msg(0x3Au, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::KickOutParty(unsigned int a2) {
  CMofMsg msg(0x3Cu, 0);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestTrade(unsigned int a2) {
  CMofMsg msg(0x42u, 0);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ResponseTrade(unsigned __int8 a2) {
  CMofMsg msg(0x44u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestAddTradeItem(unsigned __int8 a2, unsigned __int16 a3) {
  CMofMsg msg(0x46u, 0);
  msg.Put_BYTE(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestDeleteTradeItem(unsigned __int8 a2, unsigned __int16 a3) {
  CMofMsg msg(0x48u, 0);
  msg.Put_BYTE(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestChangeTradeMoney(unsigned int a2) {
  CMofMsg msg(0x4Au, 0);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestTradeDecision() {
  CMofMsg msg(0x4Cu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestTradeCancel() {
  CMofMsg msg(0x4Eu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AcceptQuest(unsigned __int16 a2) {
  CMofMsg msg(0x53u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RewardQuest(unsigned __int16 a2) {
  CMofMsg msg(0x55u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MeetNPC(unsigned __int16 a2) {
  CMofMsg msg(0x57u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GiveupQuest(unsigned __int16 a2) {
  CMofMsg msg(0x59u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::EternalGiveupQuest(unsigned __int16 a2) {
  CMofMsg msg(0xC3u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestCharDetailInfo(char *a2) {
  CMofMsg msg(0x60u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MyCharDead(int a2) {
  CMofMsg msg(0x5Bu, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ServerLists(unsigned int a2) {
  CMofMsg msg(0x5Du, 0);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

BOOL CMoFNetwork::SendHeartBeat() {
  return SendRaw(this, 0x1011u) != 0;
}

int CMoFNetwork::SendReturnOrderSheet(unsigned __int16 a2) {
  CMofMsg msg(0xAAu, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SendTeleportOrderSheet(unsigned __int16 a2, unsigned __int16 a3) {
  CMofMsg msg(0xACu, 0);
  msg.Put_WORD(a2);
  msg.Put_DWORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SendTeleportDragon(unsigned __int16 a2) {
  CMofMsg msg(0xAEu, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SendTakeShip(unsigned __int16 a2) {
  CMofMsg msg(0xB9u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SendTownPortalOrderSheet(unsigned __int16 a2) {
  CMofMsg msg(0xBBu, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SendTownPortalReturn() {
  CMofMsg msg(0xBDu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SalesAgencyJoin() {
  CMofMsg msg(0x64u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SalesAgencyQuit() {
  CMofMsg msg(0x66u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SalesAgencyRegist(unsigned __int16 a2, unsigned __int16 a3, unsigned int a4) {
  CMofMsg msg(0x68u, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  msg.Put_DWORD(a4);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SalesAgencyGetMyList() {
  CMofMsg msg(0x70u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SalesAgencyCancelRegist(unsigned int a2, unsigned __int16 a3) {
  CMofMsg msg(0x78u, 0);
  msg.Put_WORD(a3);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SalesAgencyGetSearchList(unsigned __int16 a2) {
  CMofMsg msg(0x6Du, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SalesAgencyBuyItem(unsigned int a2, unsigned __int16 a3, unsigned __int16 a4, unsigned __int16 a5) {
  CMofMsg msg(0x6Au, 0);
  msg.Put_WORD(a3);
  msg.Put_DWORD(a2);
  msg.Put_WORD(a4);
  msg.Put_WORD(a5);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SalesAgencyGetMyReturnList() {
  CMofMsg msg(0x72u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SalesAgencyGetMyReturnMoney() {
  CMofMsg msg(0x74u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SalesAgencyReceiveItem(unsigned int a2) {
  CMofMsg msg(0x7Au, 0);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SalesAgencyReceiveMoney(unsigned int a2) {
  CMofMsg msg(0x76u, 0);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SalesAgencyGetAvgPrice(int a2, unsigned __int16 *a3) {
  CMofMsg msg(0x170u, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::LessonRanking(unsigned __int8 a2, int a3) {
  CMofMsg msg(0x90u, 0);
  msg.Put_BYTE(a2);
  msg.Put_LONG(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::OpenStorage(char *a2) {
  CMofMsg msg(0x100u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::OpenExStorage(char *a2) {
  CMofMsg msg(0x241u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CloseStorage() {
  CMofMsg msg(0x102u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CloseExStorage() {
  CMofMsg msg(0x243u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AddItemToStorage(unsigned __int8 a2, unsigned __int16 a3) {
  CMofMsg msg(0x104u, 0);
  msg.Put_BYTE(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AddItemToExStorage(unsigned __int8 a2, unsigned __int16 a3) {
  CMofMsg msg(0x245u, 0);
  msg.Put_BYTE(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DelItemFromStorage(unsigned __int8 a2, unsigned __int16 a3) {
  CMofMsg msg(0x106u, 0);
  msg.Put_BYTE(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::StorageCreatePW(char *a2) {
  CMofMsg msg(0x10Eu, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::StorageChangePW(char *a2, char *a3) {
  CMofMsg msg(0x160u, 0);
  msg.Put_Z1(a2);
  msg.Put_Z1(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DelItemFromExStorage(unsigned __int8 a2, unsigned __int16 a3) {
  CMofMsg msg(0x247u, 0);
  msg.Put_BYTE(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DepositMoney(int a2) {
  CMofMsg msg(0x108u, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ExDepositMoney(int a2) {
  CMofMsg msg(0x249u, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::WithdrawingMoney(int a2) {
  CMofMsg msg(0x10Au, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ExWithdrawingMoney(int a2) {
  CMofMsg msg(0x24Bu, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ExpandStorageGenericBag() {
  CMofMsg msg(0x10Cu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ExpandExStorageGenericBag(unsigned __int8 a2) {
  CMofMsg msg(0x24Du, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MoveItemStorage(unsigned __int8 a2, unsigned __int8 a3) {
  CMofMsg msg(0x261u, 0);
  msg.Put_BYTE(a2);
  msg.Put_BYTE(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MoveItemExStorage(unsigned __int8 a2, unsigned __int8 a3) {
  CMofMsg msg(0x263u, 0);
  msg.Put_BYTE(a2);
  msg.Put_BYTE(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RefreshFriendList() {
  CMofMsg msg(0x98u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AddFriendList(char *a2) {
  CMofMsg msg(0x92u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DelFriendList(char *a2) {
  CMofMsg msg(0x94u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetFriendList() {
  CMofMsg msg(0x96u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AddBlackList(char *a2) {
  CMofMsg msg(0x9Au, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DelBlackList(char *a2) {
  CMofMsg msg(0x9Cu, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetBlackList() {
  CMofMsg msg(0x9Eu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RefreshBlackList() {
  CMofMsg msg(0xA1u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetRMonsterBook() {
  CMofMsg msg(0x110u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetNpcBook() {
  CMofMsg msg(0xB7u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::StrikedMine(unsigned int a2, unsigned __int8 a3, unsigned int *a4) {
  CMofMsg msg(0x62u, 0);
  msg.Put_DWORD(a2);
  msg.Put_BYTE(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RecallResponse(unsigned __int8 a2) {
  CMofMsg msg(0x201u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetTestingBegin() {
  CMofMsg msg(0x114u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetTestingFinished(unsigned __int8 a2, unsigned __int8 a3, char *a4, unsigned __int16 *a5) {
  CMofMsg msg(0x116u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AcquireSpecialty(unsigned __int16 a2) {
  CMofMsg msg(0x112u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MakingItem(unsigned __int16 a2, unsigned __int16 a3) {
  CMofMsg msg(0x118u, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MakeCircle(char *a2, char *a3) {
  CMofMsg msg(0x200u, 0);
  msg.Put_Z2(a2);
  msg.Put_Z2(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetCircleInfo() {
  CMofMsg msg(0x208u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ReqAddCircleUser(char *a2) {
  CMofMsg msg(0x202u, 0);
  msg.Put_Z2(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ResAddcircleUser(unsigned __int8 a2, char *a3, char *a4) {
  CMofMsg msg(0x205u, 0);
  msg.Put_BYTE(a2);
  msg.Put_Z2(a3);
  msg.Put_Z2(a4);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DelCircleUser(char *a2) {
  CMofMsg msg(0x20Au, 0);
  msg.Put_Z2(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DissolutionCircle() {
  CMofMsg msg(0x20Cu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::LoginCircleUser() {
  CMofMsg msg(0x20Eu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::LogoutCircleUser() {
  CMofMsg msg(0x211u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CircleChat(char *a2) {
  CMofMsg msg(0x213u, 0);
  msg.Put_Z2(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CircleQuestStart() {
  CMofMsg msg(0x215u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetCircleRanking() {
  CMofMsg msg(0x223u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetMyCircleRank() {
  CMofMsg msg(0x225u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ReqAddCircleSubMaster(char *a2) {
  CMofMsg msg(0x235u, 0);
  msg.Put_Z2(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ResAppointSubMaster(unsigned __int8 a2) {
  CMofMsg msg(0x238u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::StartCircleQuiz() {
  CMofMsg msg(0x250u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SetCircleQuiz(unsigned __int16 a2, unsigned __int16 a3) {
  CMofMsg msg(0x252u, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DeleteCircleQuiz() {
  CMofMsg msg(0x254u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CompleteCircleQuiz() {
  CMofMsg msg(0x256u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetCircleQuiz() {
  CMofMsg msg(0x258u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::TradeCut(unsigned __int8 a2) {
  CMofMsg msg(0x1012u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::WhisperCut(unsigned __int8 a2) {
  CMofMsg msg(0x1013u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PartyCut(unsigned __int8 a2) {
  CMofMsg msg(0x1014u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PartyListCut(unsigned __int8 a2) {
  CMofMsg msg(0x1016u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AddFriendCut(unsigned __int8 a2) {
  CMofMsg msg(0x1015u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AddCircleCut(unsigned __int8 a2) {
  CMofMsg msg(0x1019u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AddStudentCut(unsigned __int8 a2) {
  CMofMsg msg(0x101Au, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GiveupCircleQuest() {
  CMofMsg msg(0x25Du, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::FriendListSize() {
  CMofMsg msg(0x11Au, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AddCircleUserInmap() {
  CMofMsg msg(0x227u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DelCircleUserInmap() {
  CMofMsg msg(0x229u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SetCircleNotice(char *a2) {
  CMofMsg msg(0x21Du, 0);
  msg.Put_Z2(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetCircleNotice() {
  CMofMsg msg(0x220u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetCircleUserList() {
  CMofMsg msg(0x232u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::EnchantItem(unsigned __int8 a2, unsigned __int8 a3) {
  CMofMsg msg(0x120u, 0);
  msg.Put_BYTE(a2);
  msg.Put_BYTE(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GUIDAckMsg(unsigned int a2, char *a3) {
  CMofMsg msg(0x302u, 0);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AckMsg(unsigned int a2, unsigned int a3, char *a4, unsigned __int8 a5) {
  CMofMsg msg(0x304u, 0);
  msg.Put_DWORD(a2);
  msg.Put_DWORD(a3);
  msg.Put_BYTE(a5);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::OpenCashShop() {
  CMofMsg msg(0x310u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CloseCashShop() {
  CMofMsg msg(0x312u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MoveCashItem2Inven(int a2, __int64 *a3) {
  CMofMsg msg(0x316u, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::VerifyingCharName(char *a2) {
  CMofMsg msg(0x318u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::BuyCashItem(unsigned __int8 a2, char *a3, unsigned __int8 a4, unsigned __int8 a5, int *a6) {
  CMofMsg msg(0x320u, 0);
  msg.Put_BYTE(a2);
  msg.Put_Z1(a3);
  msg.Put_BYTE(a4);
  msg.Put_BYTE(a5);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::UpdateCashMoney() {
  CMofMsg msg(0x314u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AddPrivateMarket(unsigned __int16 a2, unsigned __int16 a3, unsigned __int16 a4, unsigned int a5) {
  CMofMsg msg(0x408u, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  msg.Put_WORD(a4);
  msg.Put_DWORD(a5);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DelPrivateMarket(unsigned __int16 a2) {
  CMofMsg msg(0x40Au, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ClosePrivateMarket() {
  CMofMsg msg(0x404u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SetTitlePrivateMarket(char *a2) {
  CMofMsg msg(0x410u, 0);
  msg.Put_Z2(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ReadyPrivateMarket() {
  CMofMsg msg(0x400u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::OpenPrivateMarket() {
  CMofMsg msg(0x402u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ExitPrivateMarket() {
  CMofMsg msg(0x419u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetPrivateMarket(char *a2) {
  CMofMsg msg(0x406u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::BuyPrivateMarket(char *a2, unsigned __int16 a3, unsigned __int16 a4, unsigned __int16 a5) {
  CMofMsg msg(0x40Cu, 0);
  msg.Put_Z1(a2);
  msg.Put_WORD(a3);
  msg.Put_WORD(a4);
  msg.Put_WORD(a5);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetTitlePrivateMarket(char *a2) {
  CMofMsg msg(0x412u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ChildsDayEventBeginTime() {
  CMofMsg msg(0x701u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ChildsDayEventEndTime() {
  CMofMsg msg(0x703u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::StartWarMeritorious(unsigned __int16 a2, unsigned __int16 a3, unsigned __int16 a4) {
  CMofMsg msg(0x500u, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  msg.Put_WORD(a4);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CompleteWarMeritorious() {
  CMofMsg msg(0x502u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::StartSupplyMeritorious() {
  CMofMsg msg(0x510u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CompleteSupplyMeritorious() {
  CMofMsg msg(0x512u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RewardMeritoriousItem(unsigned __int16 a2) {
  CMofMsg msg(0x518u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SetCheerMessage(char *a2) {
  CMofMsg msg(0x705u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MakeMatch(unsigned __int8 a2, char *a3, unsigned __int16 a4, unsigned __int16 a5, char *a6, unsigned __int16 a7, unsigned __int8 a8) {
  CMofMsg msg(0x600u, 0);
  msg.Put_BYTE(a2);
  msg.Put_Z1(a3);
  msg.Put_WORD(a4);
  msg.Put_WORD(a5);
  msg.Put_Z1(a6);
  msg.Put_WORD(a7);
  msg.Put_BYTE(a8);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::JoinMatch(unsigned __int8 a2, char *a3) {
  CMofMsg msg(0x603u, 0);
  msg.Put_BYTE(a2);
  msg.Put_Z1(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ExitMatch(unsigned int a2, unsigned int a3) {
  CMofMsg msg(0x607u, 0);
  msg.Put_DWORD(a2);
  msg.Put_DWORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MoveTeam(unsigned __int8 a2, unsigned int a3) {
  CMofMsg msg(0x60Bu, 0);
  msg.Put_BYTE(a2);
  msg.Put_DWORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DestroyMatch() {
  CMofMsg msg(0x60Eu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::StartMatch() {
  CMofMsg msg(0x612u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::EscapeMatch() {
  CMofMsg msg(0x61Au, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetCircleID() {
  CMofMsg msg(0x25Bu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::WarpMatch(char *a2) {
  CMofMsg msg(0x622u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AcquireEmblem(unsigned __int16 a2) {
  CMofMsg msg(0x131u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SetUsingEmblem(unsigned __int16 a2) {
  CMofMsg msg(0x133u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SetQuizEvent(unsigned __int16 a2, unsigned __int16 a3) {
  CMofMsg msg(0x801u, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DelQuizEvent() {
  CMofMsg msg(0x803u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CompleteQuizEvent() {
  CMofMsg msg(0x805u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::FindPartyList() {
  CMofMsg msg(0x141u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GetPostIt(unsigned __int8 a2) {
  CMofMsg msg(0x812u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DelPostIt(unsigned int a2) {
  CMofMsg msg(0x814u, 0);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CheckIDPostIt(char *a2) {
  CMofMsg msg(0x817u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestStduent(char *a2) {
  CMofMsg msg(0x151u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ResponseStudent(unsigned __int8 a2) {
  CMofMsg msg(0x153u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DeleteTeacher() {
  CMofMsg msg(0x155u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::DeleteStudent(char *a2) {
  CMofMsg msg(0x157u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RefreshTasList(unsigned __int8 a2) {
  CMofMsg msg(0x159u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SendTasPostit(char *a2, char *a3) {
  CMofMsg msg(0x819u, 0);
  msg.Put_Z1(a2);
  msg.Put_Z2(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SetNickname(char *a2) {
  CMofMsg msg(0x900u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GiveupWarMeritorious() {
  CMofMsg msg(0x521u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::GiveupSupMeritorious() {
  CMofMsg msg(0x523u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RecallNpc(unsigned __int16 a2) {
  CMofMsg msg(0xC5u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::TutorialStart() {
  CMofMsg msg(0xC7u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::TutorialEnd() {
  CMofMsg msg(0xC9u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ChangeActionState(unsigned __int8 a2) {
  CMofMsg msg(0x1017u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::UseEmoticonItem(unsigned __int16 a2, unsigned __int16 a3) {
  CMofMsg msg(0x1018u, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SetSortItem(int a2) {
  CMofMsg msg(0xCBu, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::Request_Server_Notify(int a2) {
  CMofMsg msg(0xCDu, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::Request_Server_Memo(int a2) {
  CMofMsg msg(0xD3u, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MemoSaveToServer(unsigned __int8 a2, int a3, char *a4) {
  CMofMsg msg(0xD1u, 0);
  msg.Put_BYTE(a2);
  msg.Put_Z2(a4);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MemoDeleteToServer(int a2) {
  CMofMsg msg(0xD5u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestComeinPartyMember(unsigned int a2) {
  CMofMsg msg(0xD9u, 0);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::SetNewTitle(unsigned __int16 a2) {
  CMofMsg msg(0xD7u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestCreateIndun(int a2, unsigned __int8 a3, unsigned __int16 a4, unsigned __int16 a5) {
  CMofMsg msg(0x650u, 0);
  msg.Put_WORD(a5);
  msg.Put_BYTE(a3);
  msg.Put_WORD(a4);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::InstansDungeonMoveMap(int a2) {
  CMofMsg msg(0x652u, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestLeaveIndun() {
  CMofMsg msg(0x654u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::JoinHighGradeShop() {
  CMofMsg msg(0x670u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::LeaveHighGradeShop() {
  CMofMsg msg(0x672u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MapConquerorInfo() {
  CMofMsg msg(0xE2u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MapCandidateInfo() {
  CMofMsg msg(0xE4u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MapCandidateChallenge() {
  CMofMsg msg(0xE6u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MapCandidateGiveup() {
  CMofMsg msg(0xE8u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MapcandidateStart() {
  CMofMsg msg(0xEAu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MapcandidatePause() {
  CMofMsg msg(0xECu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetCreate(unsigned __int8 a2, unsigned __int16 a3) {
  CMofMsg msg(0x480u, 0);
  msg.Put_BYTE(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetSetActivity(unsigned __int8 a2) {
  CMofMsg msg(0x482u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetMoveItemToInven(unsigned __int8 a2, unsigned __int16 a3) {
  CMofMsg msg(0x484u, 0);
  msg.Put_BYTE(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetMoveItemFromInven(unsigned __int8 a2, unsigned __int16 a3) {
  CMofMsg msg(0x486u, 0);
  msg.Put_BYTE(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetPickUpItem(unsigned __int16 a2, unsigned __int16 a3) {
  CMofMsg msg(0x488u, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetSetUsingSkill(unsigned __int8 a2, unsigned __int16 a3) {
  CMofMsg msg(0x48Au, 0);
  msg.Put_BYTE(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetMoveItem(unsigned __int8 a2, unsigned __int8 a3) {
  CMofMsg msg(0x48Cu, 0);
  msg.Put_BYTE(a2);
  msg.Put_BYTE(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetReleaseUsingpet() {
  CMofMsg msg(0x48Eu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetGetkeeping() {
  CMofMsg msg(0x492u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetGetkeepingRelease() {
  CMofMsg msg(0x494u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetKeeping() {
  CMofMsg msg(0x490u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetReleaseKeepingPet(int a2) {
  CMofMsg msg(0x496u, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetTakeKeepingpet(int a2) {
  CMofMsg msg(0x498u, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetMarketSalepetlist() {
  CMofMsg msg(0x49Cu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetMarketRegistry(int a2, char *a3, char *a4) {
  CMofMsg msg(0x49Au, 0);
  msg.Put_LONG(a2);
  msg.Put_Z1(a3);
  msg.Put_Z1(a4);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetMarketSalePetInfo(int a2) {
  CMofMsg msg(0x4A2u, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetMarketMyPetInfo() {
  CMofMsg msg(0x4A4u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetMarketMypetinfoRelease() {
  CMofMsg msg(0x4A8u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetMarketRegistryCancel(int a2) {
  CMofMsg msg(0x49Eu, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetMarketAcceptMoney() {
  CMofMsg msg(0x4A6u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetMarketBuyPet(int a2, char *a3) {
  CMofMsg msg(0x4A0u, 0);
  msg.Put_LONG(a2);
  msg.Put_Z1(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PetMarketReregistry(int a2, int a3, char *a4, char *a5) {
  CMofMsg msg(0x4AAu, 0);
  msg.Put_LONG(a2);
  msg.Put_LONG(a3);
  msg.Put_Z1(a4);
  msg.Put_Z1(a5);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PKJoin(unsigned int a2, char *a3) {
  CMofMsg msg(0x542u, 0);
  msg.Put_DWORD(a2);
  msg.Put_Z1(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PKCreate(int a2, int a3, int a4, char *a5, char *a6, unsigned __int16 a7) {
  CMofMsg msg(0x540u, 0);
  msg.Put_LONG(a2);
  msg.Put_LONG(a3);
  msg.Put_LONG(a4);
  msg.Put_Z1(a5);
  msg.Put_Z1(a6);
  msg.Put_WORD(a7);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PKLeave() {
  CMofMsg msg(0x544u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PKReady() {
  CMofMsg msg(0x546u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PKReadyCancel() {
  CMofMsg msg(0x548u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PKBan(unsigned int a2) {
  CMofMsg msg(0x54Cu, 0);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PKStart() {
  CMofMsg msg(0x54Au, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PKMoveTeam(int a2) {
  CMofMsg msg(0x54Eu, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PKSetSupport(int a2, char *a3) {
  CMofMsg msg(0x550u, 0);
  msg.Put_LONG(a2);
  msg.Put_Z1(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::PvPBuyItem(unsigned __int16 a2, struct cltItemList *a3) {
  CMofMsg msg(0x552u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ExpendCircleMembers(unsigned __int8 a2) {
  CMofMsg msg(0x270u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestLogin(char *a2, char *a3) {
  CMofMsg msg(0xF000u, 0);
  msg.Put_Z1(a2);
  msg.Put_Z1(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ShoutEventAnswer(char *a2) {
  CMofMsg msg(0x809u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AttendanceEventStart() {
  CMofMsg msg(0x708u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::AttendanceEventEnd() {
  CMofMsg msg(0x70Au, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MarriagePropose(char *a2, char *a3, int a4, int a5) {
  CMofMsg msg(0x630u, 0);
  msg.Put_Z1(a2);
  msg.Put_Z1(a3);
  msg.Put_LONG(a4);
  msg.Put_LONG(a5);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::MarriageProposeCancel(char *a2) {
  CMofMsg msg(0x634u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ResponsePropose(unsigned __int8 a2) {
  CMofMsg msg(0x632u, 0);
  msg.Put_BYTE(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestMarriageDivorce() {
  CMofMsg msg(0x636u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestDivorceCancel() {
  CMofMsg msg(0x638u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestSpouseRecall() {
  CMofMsg msg(0x63Au, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestWeddingHallList() {
  CMofMsg msg(0x63Eu, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestJoinWeddingHall(unsigned int a2) {
  CMofMsg msg(0x640u, 0);
  msg.Put_DWORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestBegineWedding() {
  CMofMsg msg(0x642u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestGiveBouquet(char *a2) {
  CMofMsg msg(0x644u, 0);
  msg.Put_Z1(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestWeddingHallInfo() {
  CMofMsg msg(0x646u, 0);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::RequestWeddingMoveMap(int a2) {
  CMofMsg msg(0x648u, 0);
  msg.Put_LONG(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::UseTransport(unsigned __int16 a2) {
  CMofMsg msg(0xF6u, 0);
  msg.Put_WORD(a2);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::CanChangeCharName(unsigned __int16 a2, char *a3) {
  CMofMsg msg(0xF0u, 0);
  msg.Put_WORD(a2);
  msg.Put_Z1(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ChangeCharName(unsigned __int16 a2, char *a3) {
  CMofMsg msg(0xF2u, 0);
  msg.Put_WORD(a2);
  msg.Put_Z1(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ItemSealLock(unsigned __int16 a2, unsigned __int16 a3) {
  CMofMsg msg(0xFBu, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

int CMoFNetwork::ItemSealUnLock(unsigned __int16 a2, unsigned __int16 a3) {
  CMofMsg msg(0xFDu, 0);
  msg.Put_WORD(a2);
  msg.Put_WORD(a3);
  return SetSendData(reinterpret_cast<char *>(msg.GetMsg()), static_cast<unsigned __int16>(msg.GetLength()));
}

char *CMoFNetwork::CheckChatMsg(char *a2) {
  if (a2 == nullptr || *a2 == '\0') {
    return nullptr;
  }
  return a2;
}

void __cdecl CMoFNetwork::OnSocketClosed() {}
