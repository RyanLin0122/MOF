#pragma once
#include "Character/ClientCharacter.h"

class ClientCharacterManager {
public:
	static ClientCharacterManager* GetInstance();
	ClientCharacter* GetCharByAccount(unsigned int);
	bool IsMapConqueror(char* Name);
private:
	ClientCharacterManager();
	~ClientCharacterManager();
	static ClientCharacterManager* s_pInstance;
};