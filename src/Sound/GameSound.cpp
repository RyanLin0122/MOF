#include "Sound/GameSound.h"

#include <cctype>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <algorithm>

// external symbols from original client
extern int dword_829254;
extern int dword_C24CF4;
extern const _GUID GUID_NULL;

static int s_processIndex = 0;
static DWORD s_processTick = 0;

namespace {
int ClampVolume(int volume) {
    if (volume < 0) return 0;
    if (volume > 255) return 255;
    return volume;
}

void BuildSoundPath(char* outPath, size_t outPathSize, const char* fileName, int localizedType) {
    if (!outPath || outPathSize == 0) return;
    outPath[0] = '\0';
    if (!fileName) return;

    if (localizedType) {
        const unsigned char nation = static_cast<unsigned char>(g_MoFFont.GetNationCode());
        if (nation == 1) std::snprintf(outPath, outPathSize, "MOFData/Sound/Kor/%s", fileName);
        else if (nation == 2) std::snprintf(outPath, outPathSize, "MOFData/Sound/jp/%s", fileName);
        else if (nation == 3) std::snprintf(outPath, outPathSize, "MOFData/Sound/Tai/%s", fileName);
        else if (nation == 4) std::snprintf(outPath, outPathSize, "MOFData/Sound/In/%s", fileName);
        else if (nation == 5) std::snprintf(outPath, outPathSize, "MOFData/Sound/Hk/%s", fileName);
        else std::snprintf(outPath, outPathSize, "MOFData/Sound/%s", fileName);
    } else {
        std::snprintf(outPath, outPathSize, "MOFData/Sound/%s", fileName);
    }

    if (dword_829254) {
        for (char* p = outPath; *p; ++p) {
            *p = static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
        }
    }
}
}

std::uint16_t GameSound::ParseSoundId(char* id) {
    if (!id || std::strlen(id) != 5) {
        return 0;
    }

    int high = (std::toupper(static_cast<unsigned char>(id[0])) + 31) << 11;
    unsigned short low = static_cast<unsigned short>(std::atoi(id + 1));
    if (low >= 0x800u) {
        return 0;
    }
    return static_cast<std::uint16_t>(high | low);
}

GameSound::GameSound() {
    m_bgm.Initalize(0);
    m_ambient.Initalize(1);
    std::memset(m_soundTable, 0, sizeof(m_soundTable));
}

GameSound::~GameSound() {
    for (auto& entry : m_soundTable) {
        if (entry.sound) {
            delete entry.sound;
            entry.sound = nullptr;
        }
    }
    if (m_activeIds) {
        operator delete(m_activeIds);
        m_activeIds = nullptr;
    }
}

int GameSound::InitSound(char* listFile) {
    if (Initialize(reinterpret_cast<IDirectSound8**>(this), g_hWnd, 2u) < 0 || SetPrimaryBufferFormat(1u, 0x5622u, 0x10u) < 0) {
        m_soundInitFailed = true;
        return 0;
    }

    FILE* fp = g_clTextFileManager.fopen(listFile);
    if (!fp) {
        MessageBoxA(nullptr, "Sound file is not found.", "warning", 0);
        m_soundInitFailed = true;
        return 0;
    }

    char buf[256]{};
    std::fgets(buf, 256, fp);
    std::fgets(buf, 256, fp);

    std::uint16_t count = 0;
    char id[8]{};
    char name[256]{};
    int concurrentCount = 0;
    int localized = 0;
    while (std::fgets(buf, 256, fp)) {
        if (std::sscanf(buf, "%7s%255s%d%d", id, name, &concurrentCount, &localized) != 4) {
            continue;
        }

        std::uint16_t idx = ParseSoundId(id);
        if (!idx) {
            continue;
        }

        SoundEntry& entry = m_soundTable[idx];
        if (entry.path[0] != '\0') {
            MessageBoxA(nullptr, "already setting sound file.", "warning", 0);
            continue;
        }

        BuildSoundPath(entry.path, sizeof(entry.path), name, localized);
        entry.concurrentPlayCount = static_cast<std::uint16_t>(std::max(1, concurrentCount));
        entry.localizedType = static_cast<std::uint16_t>(localized);
        entry.loaded = 0;
        entry.expireTick = 0;
        entry.sound = nullptr;
        ++count;
    }

    if (m_activeIds) {
        operator delete(m_activeIds);
        m_activeIds = nullptr;
    }
    m_activeCount = 0;
    m_activeCapacity = count;

    if (count > 0) {
        m_activeIds = static_cast<std::uint16_t*>(operator new(sizeof(std::uint16_t) * count));
        for (std::uint16_t i = 0; i < count; ++i) {
            m_activeIds[i] = 0;
        }
    }

    g_clTextFileManager.fclose(fp);
    return 1;
}

void GameSound::PlaySoundA(char* id, int x, int y) {
    if (!GetFocus() || m_soundInitFailed || m_masterDb == -10000 || !id || id[0] == '0') return;

    std::uint16_t idx = ParseSoundId(id);
    if (!idx) return;

    SoundEntry& e = m_soundTable[idx];
    if (e.path[0] == '\0') {
        MessageBoxA(nullptr, "sound file error!", "warning", 0);
        return;
    }

    if (!e.loaded) {
        if (dword_829254) {
            WAVLoader loader;
            loader.loadWAVFileIntoBuffer(e.path);

            tWAVEFORMATEX wf{};
            std::memcpy(&wf, loader.fmtChunk, std::min(sizeof(wf), sizeof(loader.fmtChunk)));
            if (CreateFromMemory(&e.sound, loader.data, loader.dataSize, &wf,
                DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN, GUID_NULL, e.concurrentPlayCount) < 0) {
                return;
            }
        }
        else {
            if (Create(&e.sound, e.path,
                DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN, GUID_NULL, e.concurrentPlayCount) < 0) {
                return;
            }
        }

        e.loaded = 1;
        if (m_activeIds && m_activeCount < m_activeCapacity) {
            m_activeIds[m_activeCount++] = idx;
        }
    }

    if (!e.sound) {
        return;
    }

    int pan = 0;
    int freqShift = 0;
    if (x && y && m_character) {
        int dx = m_character->GetPosX() - x;
        int dy = std::abs(m_character->GetPosY() - y);
        int termX = static_cast<int>(-1.5 * std::abs(dx));
        int termY = static_cast<int>(-1.2 * dy);
        pan = termX + termY;
        freqShift = 12 * dx;

        int absX = std::abs(termX);
        if (absX <= 300 || std::abs(termY) <= 100) {
            pan = 0;
            freqShift = 0;
        } else if (absX > 800) {
            if (absX <= 1200) {
                pan = static_cast<int>(0.6 * pan);
                freqShift = static_cast<int>(0.9 * freqShift);
            } else if (absX <= 1600) {
                pan = static_cast<int>(0.8 * pan);
            }
        } else {
            pan = static_cast<int>(0.3 * pan);
            freqShift = static_cast<int>(0.7 * freqShift);
        }
    }

    e.expireTick = timeGetTime() + e.sound->GetLength();
    e.sound->Play(0, 0, pan + m_masterDb, -1, freqShift);
}

void GameSound::StopSound(char* id) {
    if (!id || m_soundInitFailed) return;
    auto& e = m_soundTable[ParseSoundId(id)];
    if (e.sound) e.sound->Stop();
}

void GameSound::ResetSound(char* id) {
    if (!id || m_soundInitFailed) return;
    auto& e = m_soundTable[ParseSoundId(id)];
    if (e.sound) e.sound->Reset();
}

void GameSound::Process() {
    if (m_soundInitFailed || !m_activeIds) return;

    s_processTick = timeGetTime();
    s_processIndex = 0;
    for (std::uint16_t i = 0; i < m_activeCount; ++i) {
        const std::uint16_t idx = m_activeIds[i];
        SoundEntry& e = m_soundTable[idx];
        if (!e.loaded || !e.sound || !e.expireTick) continue;

        if (s_processTick >= e.expireTick && (s_processTick - e.expireTick) > 10000) {
            delete e.sound;
            e.sound = nullptr;
            e.expireTick = 0;
            e.loaded = 0;

            m_activeIds[i] = m_activeIds[m_activeCount - 1];
            --m_activeCount;
            --i;
        }
    }
}

void GameSound::ResetAll() {
    if (m_activeIds) {
        for (std::uint16_t i = 0; i < m_activeCount; ++i) {
            SoundEntry& e = m_soundTable[m_activeIds[i]];
            if (e.sound) {
                e.sound->Stop();
                delete e.sound;
                e.sound = nullptr;
            }
            e.expireTick = 0;
            e.loaded = 0;
        }
        m_activeCount = 0;
        if (m_activeCapacity > 0) {
            m_activeIds[0] = 0;
        }
    }

    m_masterDb = 0;
    m_soundInitFailed = false;
}

void GameSound::PlayMusic(char* filePath) {
    if (m_soundInitFailed || !filePath) return;

    m_bgmTargetVolume = ClampVolume(m_bgmTargetVolume);
    m_bgmFadeVolume = m_bgmTargetVolume;

    m_bgm.Stop();
    m_bgm.Play(filePath);
    m_bgm.SetVolume(m_bgmFadeVolume);
}

void GameSound::StopMusic() { m_bgm.Stop(); }

void GameSound::PlayAmbientSound(char* filePath) {
    if (m_soundInitFailed || !filePath) return;

    m_ambientTargetVolume = ClampVolume(m_ambientTargetVolume);
    m_ambientFadeVolume = m_ambientTargetVolume;

    m_ambient.Stop();
    m_ambient.Play(filePath);
    m_ambient.SetVolume(m_ambientFadeVolume);
}

void GameSound::StopAmbientSound() { m_ambient.Stop(); }

void GameSound::SetCharacter(ClientCharacter* character) { m_character = character; }

void GameSound::FadeInBGM() {
    const int target = ClampVolume(m_bgmTargetVolume);
    if (m_bgmFadeVolume < target) {
        m_bgmFadeVolume += 10;
        if (m_bgmFadeVolume > target) {
            m_bgmFadeVolume = target;
        }
        m_bgm.SetVolume(m_bgmFadeVolume);
    }
}

void GameSound::FadeInAmbient() {
    const int target = ClampVolume(m_ambientTargetVolume);
    if (m_ambientFadeVolume < target) {
        m_ambientFadeVolume += 10;
        if (m_ambientFadeVolume > target) {
            m_ambientFadeVolume = target;
        }
        m_ambient.SetVolume(m_ambientFadeVolume);
    }
}

void GameSound::FadeInMusic() {
    FadeInBGM();
    FadeInAmbient();
}

void GameSound::FadeOutBGM() {
    if (m_bgmFadeVolume > 0) {
        m_bgmFadeVolume -= 10;
        if (m_bgmFadeVolume < 0) {
            m_bgmFadeVolume = 0;
        }
        m_bgm.SetVolume(m_bgmFadeVolume);
    }
}

void GameSound::FadeOutAmbient() {
    if (m_ambientFadeVolume > 0) {
        m_ambientFadeVolume -= 10;
        if (m_ambientFadeVolume < 0) {
            m_ambientFadeVolume = 0;
        }
        m_ambient.SetVolume(m_ambientFadeVolume);
    }
}

void GameSound::FadeOutMusic() {
    FadeOutBGM();
    FadeOutAmbient();
}

BOOL GameSound::IsBGMFinish() {
    if (!m_bgm.m_pStream) {
        return 1;
    }

    int pos = FSOUND_Stream_GetPosition(m_bgm.m_pStream);
    int len = FSOUND_Stream_GetLength(m_bgm.m_pStream);
    if (pos < 0 || len <= 0) {
        return 1;
    }

    return pos > len - 60;
}
