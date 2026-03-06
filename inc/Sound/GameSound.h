#pragma once

#include <cstdint>
#include <cstdio>
#include <Windows.h>
#include "Character/ClientCharacter.h"
#include "Sound/CSoundSystem.h"
#include "Sound/COgg.h"
#include "Text/cltTextFileManager.h"
#include "Font/MoFFont.h"
#include "FileSystem/CMOFPacking.h"
#include "global.h"

using BOOL = int;

struct SoundEntry {
    CSound* sound = nullptr;
    char path[256]{};
    std::uint16_t baseVolume = 1;
    std::uint32_t expireTick = 0;
    std::uint8_t loaded = 0;
};

class GameSound : public CSoundManager {
public:
    GameSound();
    ~GameSound();

    int InitSound(char* listFile);
    void PlaySoundA(char* id, int x, int y);
    void StopSound(char* id);
    void ResetSound(char* id);
    void Process();
    void ResetAll();

    void PlayMusic(char* filePath);
    void StopMusic();
    void PlayAmbientSound(char* filePath);
    void StopAmbientSound();

    void SetCharacter(ClientCharacter* character);
    void FadeInBGM();
    void FadeInAmbient();
    void FadeInMusic();
    void FadeOutBGM();
    void FadeOutAmbient();
    void FadeOutMusic();

    BOOL IsBGMFinish();

private:
    std::uint16_t ParseSoundId(char* id);

public:
    COgg m_bgm;
    COgg m_ambient;
    int m_masterDb = -1500;
    int m_bgmTargetVolume = 100;
    int m_ambientTargetVolume = 0;
    bool m_soundInitFailed = false;
    SoundEntry m_soundTable[0x10000]{};
    std::uint16_t* m_activeIds = nullptr;
    ClientCharacter* m_character = nullptr;
    int m_ambientFadeVolume = 0;
    int m_bgmFadeVolume = 0;
};
