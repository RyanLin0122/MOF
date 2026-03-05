#pragma once

#include <cstdint>
#include <Windows.h>
#include <stdlib.h>
#include <cstring>
#include "Sound/CWaveFile.h"
#include "dSound.h"


class CSound {
public:
    CSound(IDirectSoundBuffer** buffers, UINT bufferBytes, UINT numBuffers, CWaveFile* waveFile, UINT creationFlags);
    virtual ~CSound();

    HRESULT FillBufferWithSound(IDirectSoundBuffer* restoreArg, IDirectSoundBuffer* buffer, int repeatWavIfBufferLarger);
    static HRESULT RestoreBuffer(IDirectSoundBuffer* restoreArg, IDirectSoundBuffer* buffer, int* wasRestored);

    IDirectSoundBuffer* GetFreeBuffer();
    IDirectSoundBuffer* GetBuffer(UINT index);
    int Get3DBufferInterface(UINT index, IDirectSound3DBuffer** out3dBuffer);
    HRESULT Play(UINT priority, UINT flags, int volume, int frequency, int pan);
    HRESULT Play3D(_DS3DBUFFER* params, UINT priority, UINT flags, int frequencyDelta);
    UINT GetLength();
    int Stop();
    int Reset();
    int IsSoundPlaying();

protected:
    IDirectSoundBuffer** m_buffers = nullptr;
    UINT m_bufferBytes = 0;
    CWaveFile* m_waveFile = nullptr;
    UINT m_numBuffers = 0;
    UINT m_creationFlags = 0;
    UINT m_cachedLengthMs = 0;
};

class CStreamingSound : public CSound {
public:
    CStreamingSound(IDirectSoundBuffer* buffer, UINT bufferBytes, CWaveFile* waveFile, UINT notifySize);
    ~CStreamingSound() override;

    HRESULT HandleWaveStreamNotification(int restoreArg, IDirectSoundBuffer* restoreBuffer, int loopedPlay);
    HRESULT Reset();

private:
    UINT m_lastPlayPos = 0;
    UINT m_playProgress = 0;
    UINT m_notifySize = 0;
    UINT m_nextWriteOffset = 0;
    UINT m_endOfFile = 0;
};

class CSoundManager {
public:
    CSoundManager();
    ~CSoundManager();

    HRESULT Initialize(IDirectSound8** ppDS8, HWND hwnd, UINT cooperativeLevel);
    int SetPrimaryBufferFormat(UINT channels, UINT samplesPerSec, UINT bitsPerSample);
    int Get3DListenerInterface(IDirectSound3DListener** outListener);

    int Create(CSound** outSound, LPSTR fileName, UINT creationFlags, GUID algorithm, UINT numBuffers);
    int CreateFromMemory(CSound** outSound, unsigned char* data, UINT dataSize, tWAVEFORMATEX* wf,
        UINT creationFlags, GUID algorithm, UINT numBuffers);
    int CreateStreaming(CStreamingSound** outStreaming, LPSTR fileName, UINT creationFlags, GUID algorithm,
        UINT notifyCount, UINT notifySize, void* notifyEvent);

    IDirectSound8* m_ds = nullptr;
};
