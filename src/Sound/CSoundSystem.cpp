#include "Sound/CSoundSystem.h"

#include "Sound/CWaveFile.h"

#include <cstring>
#include <new>

CSoundManager::CSoundManager() { m_ds = nullptr; }

CSoundManager::~CSoundManager() {
    if (m_ds) {
        m_ds->Release();
        m_ds = nullptr;
    }
}

HRESULT CSoundManager::Initialize(IDirectSound8** ppDS8, HWND hwnd, UINT cooperativeLevel) {
    if (!ppDS8) return E_INVALIDARG;

    if (*ppDS8) {
        (*ppDS8)->Release();
        *ppDS8 = nullptr;
    }

    HRESULT hr = DirectSoundCreate8(nullptr, ppDS8, nullptr);
    if (FAILED(hr)) return hr;

    hr = (*ppDS8)->SetCooperativeLevel(hwnd, cooperativeLevel);
    if (FAILED(hr)) return hr;

    m_ds = *ppDS8;
    return S_OK;
}

int CSoundManager::SetPrimaryBufferFormat(UINT channels, UINT samplesPerSec, UINT bitsPerSample) {
    if (!m_ds) return DSERR_UNINITIALIZED;

    DSBUFFERDESC dsbd{};
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;

    IDirectSoundBuffer* bufferObj = nullptr;
    HRESULT hr = m_ds->CreateSoundBuffer(&dsbd, &bufferObj, nullptr);
    if (FAILED(hr)) return hr;

    WAVEFORMATEX wf{};
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = static_cast<WORD>(channels);
    wf.nSamplesPerSec = samplesPerSec;
    wf.wBitsPerSample = static_cast<WORD>(bitsPerSample);
    wf.nBlockAlign = static_cast<WORD>((channels * bitsPerSample) / 8);
    wf.nAvgBytesPerSec = samplesPerSec * wf.nBlockAlign;

    hr = bufferObj->SetFormat(&wf);
    bufferObj->Release();
    return SUCCEEDED(hr) ? 0 : hr;
}

int CSoundManager::Get3DListenerInterface(IDirectSound3DListener** outListener) {
    if (!outListener) return E_INVALIDARG;
    if (!m_ds) return DSERR_UNINITIALIZED;
    *outListener = nullptr;

    DSBUFFERDESC dsbd{};
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;

    IDirectSoundBuffer* primary = nullptr;
    HRESULT hr = m_ds->CreateSoundBuffer(&dsbd, &primary, nullptr);
    if (FAILED(hr)) return hr;

    hr = primary->QueryInterface(IID_IDirectSound3DListener, reinterpret_cast<void**>(outListener));
    primary->Release();
    return SUCCEEDED(hr) ? 0 : hr;
}

int CSoundManager::Create(CSound** outSound, LPSTR fileName, UINT creationFlags, GUID algorithm, UINT numBuffers) {
    if (!m_ds) return DSERR_UNINITIALIZED;
    if (!fileName || !outSound || !numBuffers) return E_INVALIDARG;

    auto* buffers = static_cast<IDirectSoundBuffer**>(operator new(sizeof(IDirectSoundBuffer*) * numBuffers));
    if (!buffers) return E_OUTOFMEMORY;

    auto* waveMem = static_cast<CWaveFile*>(operator new(sizeof(CWaveFile)));
    auto* wave = waveMem ? new (waveMem) CWaveFile() : nullptr;
    if (!wave) {
        operator delete(buffers);
        return E_OUTOFMEMORY;
    }

    wave->Open(fileName, nullptr, 1u);
    if (!wave->GetSize()) {
        wave->~CWaveFile();
        operator delete(wave);
        operator delete(buffers);
        return E_FAIL;
    }

    DSBUFFERDESC dsbd{};
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = creationFlags;
    dsbd.dwBufferBytes = wave->GetSize();
    dsbd.lpwfxFormat = reinterpret_cast<WAVEFORMATEX*>(const_cast<tWAVEFORMATEX*>(wave->GetFormat()));
    dsbd.guid3DAlgorithm = algorithm;

    HRESULT hrCreate = m_ds->CreateSoundBuffer(&dsbd, &buffers[0], nullptr);
    int ret = 0;
    if (hrCreate == static_cast<HRESULT>(142082058)) {
        ret = 142082058;
    }
    else if (FAILED(hrCreate)) {
        wave->~CWaveFile();
        operator delete(wave);
        operator delete(buffers);
        return hrCreate;
    }

    for (UINT i = 1; i < numBuffers; ++i) {
        HRESULT hr = ((creationFlags & DSBCAPS_CTRLFX) != 0)
            ? m_ds->CreateSoundBuffer(&dsbd, &buffers[i], nullptr)
            : m_ds->DuplicateSoundBuffer(buffers[0], &buffers[i]);
        if (FAILED(hr)) {
            for (UINT j = 0; j < i; ++j) {
                if (buffers[j]) buffers[j]->Release();
            }
            wave->~CWaveFile();
            operator delete(wave);
            operator delete(buffers);
            return hr;
        }
    }

    auto* sndMem = static_cast<CSound*>(operator new(sizeof(CSound)));
    *outSound = sndMem ? new (sndMem) CSound(buffers, wave->GetSize(), numBuffers, wave, creationFlags) : nullptr;
    operator delete(buffers);
    return ret;
}

int CSoundManager::CreateFromMemory(CSound** outSound, unsigned char* data, UINT dataSize, tWAVEFORMATEX* wf,
    UINT creationFlags, GUID algorithm, UINT numBuffers) {
    if (!m_ds) return DSERR_UNINITIALIZED;
    if (!data || !outSound || !numBuffers || !wf) return E_INVALIDARG;

    auto* buffers = static_cast<IDirectSoundBuffer**>(operator new(sizeof(IDirectSoundBuffer*) * numBuffers));
    if (!buffers) return E_OUTOFMEMORY;

    auto* waveMem = static_cast<CWaveFile*>(operator new(sizeof(CWaveFile)));
    auto* wave = waveMem ? new (waveMem) CWaveFile() : nullptr;
    if (!wave) {
        operator delete(buffers);
        return E_OUTOFMEMORY;
    }
    wave->OpenFromMemory(data, dataSize, wf, 1u);

    DSBUFFERDESC dsbd{};
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = creationFlags;
    dsbd.dwBufferBytes = dataSize;
    dsbd.lpwfxFormat = reinterpret_cast<WAVEFORMATEX*>(wf);
    dsbd.guid3DAlgorithm = algorithm;

    HRESULT hr = m_ds->CreateSoundBuffer(&dsbd, &buffers[0], nullptr);
    if (FAILED(hr)) {
        wave->~CWaveFile();
        operator delete(wave);
        operator delete(buffers);
        return hr;
    }

    for (UINT i = 1; i < numBuffers; ++i) {
        HRESULT hri = ((creationFlags & DSBCAPS_CTRLFX) != 0)
            ? m_ds->CreateSoundBuffer(&dsbd, &buffers[i], nullptr)
            : m_ds->DuplicateSoundBuffer(buffers[0], &buffers[i]);
        if (FAILED(hri)) {
            for (UINT j = 0; j <= i; ++j) {
                if (buffers[j]) buffers[j]->Release();
            }
            wave->~CWaveFile();
            operator delete(wave);
            operator delete(buffers);
            return hri;
        }
    }

    auto* sndMem = static_cast<CSound*>(operator new(sizeof(CSound)));
    *outSound = sndMem ? new (sndMem) CSound(buffers, dataSize, numBuffers, wave, creationFlags) : nullptr;
    operator delete(buffers);
    return 0;
}

int CSoundManager::CreateStreaming(CStreamingSound** outStreaming, LPSTR fileName, UINT creationFlags, GUID algorithm,
    UINT notifyCount, UINT notifySize, void* notifyEvent) {
    if (!m_ds) return DSERR_UNINITIALIZED;
    if (!fileName || !outStreaming || !notifyEvent) return E_INVALIDARG;

    IDirectSoundBuffer* primary = nullptr;
    IDirectSoundNotify* notify = nullptr;

    auto* waveMem = static_cast<CWaveFile*>(operator new(sizeof(CWaveFile)));
    auto* wave = waveMem ? new (waveMem) CWaveFile() : nullptr;
    if (!wave) return E_OUTOFMEMORY;
    wave->Open(fileName, nullptr, 1u);

    DSBUFFERDESC dsbd{};
    dsbd.dwSize = sizeof(dsbd);
    dsbd.dwFlags = creationFlags | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY;
    dsbd.dwBufferBytes = notifyCount * notifySize;
    dsbd.lpwfxFormat = reinterpret_cast<WAVEFORMATEX*>(const_cast<tWAVEFORMATEX*>(wave->GetFormat()));
    dsbd.guid3DAlgorithm = algorithm;

    HRESULT hr = m_ds->CreateSoundBuffer(&dsbd, &primary, nullptr);
    if (FAILED(hr)) {
        wave->~CWaveFile();
        operator delete(wave);
        return hr;
    }

    hr = primary->QueryInterface(IID_IDirectSoundNotify, reinterpret_cast<void**>(&notify));
    if (FAILED(hr)) {
        primary->Release();
        wave->~CWaveFile();
        operator delete(wave);
        return hr;
    }

    auto* positions = static_cast<DSBPOSITIONNOTIFY*>(operator new(sizeof(DSBPOSITIONNOTIFY) * notifyCount));
    if (!positions) {
        notify->Release();
        primary->Release();
        wave->~CWaveFile();
        operator delete(wave);
        return E_OUTOFMEMORY;
    }

    for (UINT i = 0; i < notifyCount; ++i) {
        positions[i].dwOffset = notifySize * (i + 1) - 1;
        positions[i].hEventNotify = static_cast<HANDLE>(notifyEvent);
    }

    HRESULT setHr = notify->SetNotificationPositions(notifyCount, positions);
    notify->Release();
    operator delete(positions);
    if (FAILED(setHr)) {
        primary->Release();
        wave->~CWaveFile();
        operator delete(wave);
        return setHr;
    }

    auto* cssMem = static_cast<CStreamingSound*>(operator new(sizeof(CStreamingSound)));
    *outStreaming = cssMem ? new (cssMem) CStreamingSound(primary, notifyCount * notifySize, wave, notifySize) : nullptr;
    return 0;
}

CSound::CSound(IDirectSoundBuffer** buffers, UINT bufferBytes, UINT numBuffers, CWaveFile* waveFile, UINT creationFlags) {
    m_buffers = static_cast<IDirectSoundBuffer**>(operator new(sizeof(IDirectSoundBuffer*) * numBuffers));
    if (m_buffers) {
        for (UINT i = 0; i < numBuffers; ++i) m_buffers[i] = buffers[i];
        m_bufferBytes = bufferBytes;
        m_numBuffers = numBuffers;
        m_waveFile = waveFile;
        m_creationFlags = creationFlags;
        FillBufferWithSound(nullptr, m_buffers[0], 0);
    }
    m_cachedLengthMs = 0;
}

CSound::~CSound() {
    if (m_numBuffers) {
        for (UINT i = 0; i < m_numBuffers; ++i) {
            if (m_buffers[i]) {
                m_buffers[i]->Release();
                m_buffers[i] = nullptr;
            }
        }
    }
    if (m_buffers) {
        operator delete(m_buffers);
        m_buffers = nullptr;
    }
    if (m_waveFile) {
        m_waveFile->~CWaveFile();
        operator delete(m_waveFile);
        m_waveFile = nullptr;
    }
}

HRESULT CSound::FillBufferWithSound(IDirectSoundBuffer* restoreArg, IDirectSoundBuffer* buffer, int repeatWavIfBufferLarger) {
    (void)restoreArg;
    if (!buffer) return DSERR_UNINITIALIZED;
    HRESULT hr = RestoreBuffer(restoreArg, buffer, nullptr);
    if (FAILED(hr) && hr != S_FALSE) return hr;

    void* p1 = nullptr;
    DWORD b1 = 0;
    UINT read = 0;
    hr = buffer->Lock(0, m_bufferBytes, &p1, &b1, nullptr, nullptr, 0);
    if (FAILED(hr)) return hr;

    m_waveFile->ResetFile();
    hr = m_waveFile->Read(static_cast<BYTE*>(p1), static_cast<UINT>(b1), &read);
    if (FAILED(hr)) {
        buffer->Unlock(p1, b1, nullptr, 0);
        return hr;
    }

    if (read < b1) {
        BYTE fill = (m_waveFile->GetFormat() && m_waveFile->GetFormat()->wBitsPerSample == 8) ? 0x80 : 0;
        if (!repeatWavIfBufferLarger) {
            DWORD remain = b1 - read;
            BYTE* dst = static_cast<BYTE*>(p1) + read;
            int pat = fill | (fill << 8);
            pat |= (pat << 16);
            memset(dst, pat, remain >> 2);
            std::memset(dst + 4 * (remain >> 2), fill, remain & 3);
        }
        else {
            DWORD total = read;
            while (total < b1) {
                hr = m_waveFile->ResetFile();
                if (FAILED(hr)) {
                    buffer->Unlock(p1, b1, nullptr, 0);
                    return hr;
                }
                hr = m_waveFile->Read(static_cast<BYTE*>(p1) + total, static_cast<UINT>(b1 - total), &read);
                if (FAILED(hr)) {
                    buffer->Unlock(p1, b1, nullptr, 0);
                    return hr;
                }
                total += read;
            }
        }
    }

    buffer->Unlock(p1, b1, nullptr, 0);
    return S_OK;
}

HRESULT CSound::RestoreBuffer(IDirectSoundBuffer* restoreArg, IDirectSoundBuffer* buffer, int* wasRestored) {
    (void)restoreArg;
    if (!buffer) return DSERR_UNINITIALIZED;
    if (wasRestored) *wasRestored = 0;

    DWORD status = 0;
    HRESULT hr = buffer->GetStatus(&status);
    if (FAILED(hr)) return hr;

    if ((status & DSBSTATUS_BUFFERLOST) != 0) {
        do {
            hr = buffer->Restore();
            if (hr == DSERR_BUFFERLOST) Sleep(10);
        } while (hr == DSERR_BUFFERLOST);

        if (FAILED(hr)) return hr;
        if (wasRestored) *wasRestored = 1;
        return S_OK;
    }
    return S_FALSE;
}

IDirectSoundBuffer* CSound::GetFreeBuffer() {
    if (!m_buffers) return nullptr;
    UINT i = 0;
    for (; i < m_numBuffers; ++i) {
        auto* b = m_buffers[i];
        if (b) {
            DWORD status = 0;
            b->GetStatus(&status);
            if ((status & DSBSTATUS_PLAYING) == 0) break;
        }
    }
    if (i == m_numBuffers) return m_buffers[static_cast<UINT>(rand()) % m_numBuffers];
    return m_buffers[i];
}

IDirectSoundBuffer* CSound::GetBuffer(UINT index) {
    if (!m_buffers) return nullptr;
    if (index < m_numBuffers) return m_buffers[index];
    return nullptr;
}

int CSound::Get3DBufferInterface(UINT index, IDirectSound3DBuffer** out3dBuffer) {
    if (!m_buffers) return DSERR_UNINITIALIZED;
    if (index >= m_numBuffers || !out3dBuffer) return E_INVALIDARG;
    *out3dBuffer = nullptr;
    auto* b = m_buffers[index];
    return b->QueryInterface(IID_IDirectSound3DBuffer, reinterpret_cast<void**>(out3dBuffer));
}

HRESULT CSound::Play(UINT priority, UINT flags, int volume, int frequency, int pan) {
    if (!m_buffers) return DSERR_UNINITIALIZED;
    auto* b = GetFreeBuffer();
    if (!b) return E_FAIL;

    int restored = 0;
    HRESULT hr = RestoreBuffer(nullptr, b, &restored);
    if (FAILED(hr)) return hr;
    if (restored) {
        HRESULT fillHr = FillBufferWithSound(nullptr, b, 0);
        if (FAILED(fillHr)) return fillHr;
    }

    if ((m_creationFlags & DSBCAPS_CTRLVOLUME) != 0) b->SetVolume(volume);
    if (frequency != -1 && (m_creationFlags & DSBCAPS_CTRLFREQUENCY) != 0) b->SetFrequency(static_cast<DWORD>(frequency));
    if ((m_creationFlags & DSBCAPS_CTRLPAN) != 0) b->SetPan(pan);

    return b->Play(0, priority, flags);
}

HRESULT CSound::Play3D(_DS3DBUFFER* params, UINT priority, UINT flags, int frequencyDelta) {
    if (!m_buffers) return DSERR_UNINITIALIZED;
    auto* b = GetFreeBuffer();
    if (!b) return E_FAIL;

    int restored = 0;
    HRESULT hr = RestoreBuffer(nullptr, b, &restored);
    if (FAILED(hr)) return hr;
    if (restored) {
        HRESULT fillHr = FillBufferWithSound(nullptr, b, 0);
        if (FAILED(fillHr)) return fillHr;
    }

    if (frequencyDelta && (m_creationFlags & DSBCAPS_CTRLFREQUENCY) != 0) {
        DWORD freq = 0;
        b->GetFrequency(&freq);
        b->SetFrequency(freq + frequencyDelta);
    }

    IDirectSound3DBuffer* obj3d = nullptr;
    hr = b->QueryInterface(IID_IDirectSound3DBuffer, reinterpret_cast<void**>(&obj3d));
    if (FAILED(hr)) return hr;

    hr = obj3d->SetAllParameters(params, DS3D_IMMEDIATE);
    if (SUCCEEDED(hr)) hr = b->Play(0, priority, flags);
    obj3d->Release();
    return hr;
}

UINT CSound::GetLength() {
    const tWAVEFORMATEX* fmt = m_waveFile ? m_waveFile->GetFormat() : nullptr;
    if (!fmt || fmt->nAvgBytesPerSec == 0) return 0;

    UINT len = 1000 * m_waveFile->GetSize() / fmt->nAvgBytesPerSec;
    m_cachedLengthMs = len;
    return len;
}

int CSound::Stop() {
    if (!m_buffers) return DSERR_UNINITIALIZED;
    int acc = 0;
    for (UINT i = 0; i < m_numBuffers; ++i) acc |= m_buffers[i]->Stop();
    return acc;
}

int CSound::Reset() {
    if (!m_buffers) return DSERR_UNINITIALIZED;
    int acc = 0;
    for (UINT i = 0; i < m_numBuffers; ++i) acc |= m_buffers[i]->SetCurrentPosition(0);
    return acc;
}

int CSound::IsSoundPlaying() {
    if (!m_buffers) return 0;
    int playing = 0;
    for (UINT i = 0; i < m_numBuffers; ++i) {
        DWORD status = 0;
        if (m_buffers[i] && SUCCEEDED(m_buffers[i]->GetStatus(&status))) playing |= (status & DSBSTATUS_PLAYING);
    }
    return playing;
}

CStreamingSound::CStreamingSound(IDirectSoundBuffer* buffer, UINT bufferBytes, CWaveFile* waveFile, UINT notifySize)
    : CSound(&buffer, bufferBytes, 1u, waveFile, 0u) {
    m_lastPlayPos = 0;
    m_playProgress = 0;
    m_notifySize = notifySize;
    m_nextWriteOffset = 0;
    m_endOfFile = 0;
}

CStreamingSound::~CStreamingSound() = default;

HRESULT CStreamingSound::HandleWaveStreamNotification(int restoreArg, IDirectSoundBuffer* restoreBuffer, int loopedPlay) {
    (void)restoreArg;
    auto** ppb = m_buffers;
    if (!ppb || !m_waveFile) return DSERR_UNINITIALIZED;

    int wasRestored = 0;
    HRESULT hr = CSound::RestoreBuffer(restoreBuffer, *ppb, &wasRestored);
    if (FAILED(hr)) return hr;
    if (wasRestored) {
        HRESULT fill = CSound::FillBufferWithSound(restoreBuffer, *ppb, 0);
        return SUCCEEDED(fill) ? S_OK : fill;
    }

    void* p1 = nullptr;
    DWORD b1 = 0;
    void* p2 = nullptr;
    DWORD b2 = 0;
    hr = (*ppb)->Lock(m_nextWriteOffset, m_notifySize, &p1, &b1, &p2, &b2, 0);
    if (FAILED(hr)) return hr;
    if (p2 || b2) {
        (*ppb)->Unlock(p1, b1, p2, b2);
        return E_UNEXPECTED;
    }

    UINT read = 0;
    if (m_endOfFile) {
        BYTE fill = (m_waveFile->GetFormat() && m_waveFile->GetFormat()->wBitsPerSample == 8) ? 0x80 : 0;
        int pat = fill | (fill << 8);
        pat |= (pat << 16);
        memset(p1, pat, b1 >> 2);
        std::memset(static_cast<BYTE*>(p1) + 4 * (b1 >> 2), fill, b1 & 3);
    }
    else {
        hr = m_waveFile->Read(static_cast<BYTE*>(p1), static_cast<UINT>(b1), &read);
        if (FAILED(hr)) {
            (*ppb)->Unlock(p1, b1, nullptr, 0);
            return hr;
        }

        if (read < b1) {
            BYTE fill = (m_waveFile->GetFormat() && m_waveFile->GetFormat()->wBitsPerSample == 8) ? 0x80 : 0;
            if (!loopedPlay) {
                BYTE* tail = static_cast<BYTE*>(p1) + read;
                DWORD remain = b1 - read;
                int pat = fill | (fill << 8);
                pat |= (pat << 16);
                memset(tail, pat, remain >> 2);
                std::memset(tail + 4 * (remain >> 2), fill, remain & 3);
                m_endOfFile = 1;
            }
            else {
                DWORD total = read;
                while (total < b1) {
                    hr = m_waveFile->ResetFile();
                    if (FAILED(hr)) {
                        (*ppb)->Unlock(p1, b1, nullptr, 0);
                        return hr;
                    }
                    hr = m_waveFile->Read(static_cast<BYTE*>(p1) + total, static_cast<UINT>(b1 - total), &read);
                    if (FAILED(hr)) {
                        (*ppb)->Unlock(p1, b1, nullptr, 0);
                        return hr;
                    }
                    total += read;
                }
            }
        }
    }

    (*ppb)->Unlock(p1, b1, nullptr, 0);

    DWORD playPos = 0;
    hr = (*ppb)->GetCurrentPosition(&playPos, nullptr);
    if (FAILED(hr)) return hr;

    DWORD delta = (playPos >= m_lastPlayPos) ? (playPos - m_lastPlayPos) : (playPos + m_bufferBytes - m_lastPlayPos);
    m_lastPlayPos = playPos;
    m_playProgress += delta;
    if (m_endOfFile && m_playProgress >= m_waveFile->GetSize()) (*ppb)->Stop();

    m_nextWriteOffset = (b1 + m_nextWriteOffset) % m_bufferBytes;
    return S_OK;
}

HRESULT CStreamingSound::Reset() {
    if (!m_buffers[0] || !m_waveFile) return DSERR_UNINITIALIZED;
    m_lastPlayPos = 0;
    m_playProgress = 0;
    m_nextWriteOffset = 0;
    m_endOfFile = 0;

    int restored = 0;
    HRESULT hr = CSound::RestoreBuffer(nullptr, m_buffers[0], &restored);
    if (FAILED(hr)) return hr;
    if (restored) {
        HRESULT fillHr = CSound::FillBufferWithSound(nullptr, m_buffers[0], 0);
        if (FAILED(fillHr)) return fillHr;
    }

    m_waveFile->ResetFile();
    return m_buffers[0]->SetCurrentPosition(0);
}
