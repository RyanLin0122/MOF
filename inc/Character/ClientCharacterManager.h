#pragma once
#include "Character/ClientCharacter.h"

class ClientCharacterManager {
public:
	static ClientCharacterManager* GetInstance();
	ClientCharacter* GetCharByAccount(unsigned int);
private:
	ClientCharacterManager();
	~ClientCharacterManager();
	static ClientCharacterManager* s_pInstance;
};