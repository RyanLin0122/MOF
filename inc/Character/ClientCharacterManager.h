#pragma once
#include <cstdint>
#include "Character/ClientCharacter.h"

class ClientCharacterManager {
public:
	ClientCharacterManager();
	~ClientCharacterManager();

	ClientCharacter* GetCharByAccount(unsigned int);
	bool IsMapConqueror(char* Name);
	void ResetMoveTarget();

	// Tutorial / world-setup helpers
	void AddCharacter(ClientCharacter* account, int x, int y,
	                  unsigned short charKind, unsigned short mapKind,
	                  int hp, const char* name,
	                  const char* guild, int guildMark,
	                  const char* a10, const char* a11,
	                  int a12, int a13, int a14, int a15, int a16, int a17,
	                  int a18, int a19, int a20, int a21,
	                  const char* a22, const char* a23,
	                  unsigned int teamKind, int a25, int a26, int a27, int a28,
	                  unsigned char nation, unsigned char sex, unsigned char hair,
	                  int a32, int a33);
	void SetMyAccount(unsigned int account);
	unsigned int GetMyAccount() const { return m_dwMyAccount; }
	ClientCharacter* GetMyCharacterPtr();
	void SetItem(unsigned int account, unsigned short itemKind, int qty);
	void DeleteAllChar();
	void SetMyCAClone();
	char* GetMyCharName();

	// 繪製相關
	void PrepareDrawingEtcMark();
	void DrawEtcMark();
	bool GetMyTransformationed();

	// Emoticon support
	ClientCharacter* GetCharByName(char* name);
	void SetEmoticonKind(char* name, std::uint32_t emoticonKind);
	void ReleaseEmoticon(ClientCharacter* pChar);

private:
	unsigned int m_dwMyAccount{ 0 };
};
