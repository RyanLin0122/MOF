#include "Character/ClientCharacterManager.h"
#include <new>


ClientCharacterManager* ClientCharacterManager::s_pInstance = nullptr;

ClientCharacterManager* ClientCharacterManager::GetInstance() {
    if (!s_pInstance) {
        s_pInstance = new (std::nothrow) ClientCharacterManager();
    }
    return s_pInstance;
}

ClientCharacterManager::ClientCharacterManager() {

}

ClientCharacterManager::~ClientCharacterManager() {

}

ClientCharacter* ClientCharacterManager::GetCharByAccount(unsigned int) {
	return new ClientCharacter(); // 假設這裡返回一個新的 ClientCharacter 實例
}

bool ClientCharacterManager::IsMapConqueror(char* Name) {
    return true;
}