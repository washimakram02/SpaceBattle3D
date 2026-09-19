#include "SoundManager.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#ifdef _WIN32
static DWORD WINAPI AudioThreadProc(LPVOID lpParam) {
    SoundManager* mgr = reinterpret_cast<SoundManager*>(lpParam);
    if (mgr) {
        mgr->audioThreadLoop();
    }
    return 0;
}
#endif

SoundManager::SoundManager()
    : initialized(false), running(false), muted(false), masterVolume(0.85f)
#ifdef _WIN32
    , hWaveOut(NULL), hAudioThread(NULL), hWakeEvent(NULL)
#endif
{
#ifdef _WIN32
    InitializeCriticalSection(&csVoices);
    for (int i = 0; i < NUM_BUFFERS; ++i) {
        memset(&waveHeaders[i], 0, sizeof(WAVEHDR));
        memset(audioBuffers[i], 0, sizeof(audioBuffers[i]));
    }
#endif
}

SoundManager::~SoundManager() {
    cleanup();
#ifdef _WIN32
    DeleteCriticalSection(&csVoices);
#endif
}

bool SoundManager::init() {
    if (initialized) return true;

#ifdef _WIN32
    hWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hWakeEvent) {
        return false;
    }

    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = CHANNELS;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = CHANNELS * (wfx.wBitsPerSample / 8);
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    MMRESULT res = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)hWakeEvent, 0, CALLBACK_EVENT);
    if (res != MMSYSERR_NOERROR) {
        CloseHandle(hWakeEvent);
        hWakeEvent = NULL;
        return false;
    }

    running = true;

    // Prepare initial empty buffers and queue them
    for (int i = 0; i < NUM_BUFFERS; ++i) {
        waveHeaders[i].lpData = reinterpret_cast<LPSTR>(audioBuffers[i]);
        waveHeaders[i].dwBufferLength = BUFFER_SAMPLES * CHANNELS * sizeof(short);
        waveHeaders[i].dwFlags = 0;
        waveHeaders[i].dwUser = i;
        waveOutPrepareHeader(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));
        waveOutWrite(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));
    }

    hAudioThread = CreateThread(NULL, 0, AudioThreadProc, this, 0, NULL);
    if (!hAudioThread) {
        cleanup();
        return false;
    }

    SetThreadPriority(hAudioThread, THREAD_PRIORITY_HIGHEST);
#endif

    loadAllSounds();
    initialized = true;
    return true;
}

void SoundManager::cleanup() {
    if (!running && !initialized) return;

    running = false;

#ifdef _WIN32
    if (hWakeEvent) {
        SetEvent(hWakeEvent);
    }

    if (hAudioThread) {
        WaitForSingleObject(hAudioThread, 1000);
        CloseHandle(hAudioThread);
        hAudioThread = NULL;
    }

    if (hWaveOut) {
        waveOutReset(hWaveOut);
        for (int i = 0; i < NUM_BUFFERS; ++i) {
            if (waveHeaders[i].dwFlags & WHDR_PREPARED) {
                waveOutUnprepareHeader(hWaveOut, &waveHeaders[i], sizeof(WAVEHDR));
            }
        }
        waveOutClose(hWaveOut);
        hWaveOut = NULL;
    }

    if (hWakeEvent) {
        CloseHandle(hWakeEvent);
        hWakeEvent = NULL;
    }
#endif

    initialized = false;
}

void SoundManager::audioThreadLoop() {
#ifdef _WIN32
    while (running) {
        WaitForSingleObject(hWakeEvent, 20);
        if (!running) break;

        for (int b = 0; b < NUM_BUFFERS; ++b) {
            if (waveHeaders[b].dwFlags & WHDR_DONE) {
                short* buffer = audioBuffers[b];

                EnterCriticalSection(&csVoices);

                if (muted || masterVolume <= 0.001f) {
                    memset(buffer, 0, BUFFER_SAMPLES * CHANNELS * sizeof(short));
                } else {
                    for (int s = 0; s < BUFFER_SAMPLES; ++s) {
                        float leftAcc = 0.0f;
                        float rightAcc = 0.0f;

                        for (int v = 0; v < MAX_VOICES; ++v) {
                            SoundVoice& voice = voices[v];
                            if (!voice.active || !voice.sample) continue;

                            const std::vector<short>& raw = voice.sample->samples;
                            if (voice.cursor >= raw.size()) {
                                if (voice.loop && !raw.empty()) {
                                    voice.cursor = 0;
                                } else {
                                    voice.active = false;
                                    continue;
                                }
                            }

                            float val = static_cast<float>(raw[voice.cursor++]) / 32767.0f;
                            float effectiveVol = voice.volume * masterVolume;

                            // Stereo panning calculation
                            float pan = voice.pan;
                            float leftGain = (1.0f - pan) * 0.5f;
                            float rightGain = (1.0f + pan) * 0.5f;

                            leftAcc += val * effectiveVol * leftGain;
                            rightAcc += val * effectiveVol * rightGain;
                        }

                        // Soft saturation limiter to prevent clipping
                        if (leftAcc > 1.0f) leftAcc = 1.0f;
                        else if (leftAcc < -1.0f) leftAcc = -1.0f;

                        if (rightAcc > 1.0f) rightAcc = 1.0f;
                        else if (rightAcc < -1.0f) rightAcc = -1.0f;

                        buffer[s * 2]     = static_cast<short>(leftAcc * 32767.0f);
                        buffer[s * 2 + 1] = static_cast<short>(rightAcc * 32767.0f);
                    }
                }

                LeaveCriticalSection(&csVoices);

                waveHeaders[b].dwFlags &= ~WHDR_DONE;
                waveOutWrite(hWaveOut, &waveHeaders[b], sizeof(WAVEHDR));
            }
        }
    }
#endif
}

void SoundManager::play(SoundID id, float volume, float pan, bool loop) {
    if (id < 0 || id >= SND_COUNT) return;
    if (muted) return;

    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    if (pan < -1.0f) pan = -1.0f;
    if (pan > 1.0f) pan = 1.0f;

    SoundSample& sample = soundBank[id];
    if (!sample.loaded || sample.samples.empty()) return;

#ifdef _WIN32
    EnterCriticalSection(&csVoices);

    // Find first inactive voice or voice with highest cursor progress
    int voiceIndex = -1;
    size_t maxProgress = 0;

    for (int i = 0; i < MAX_VOICES; ++i) {
        if (!voices[i].active) {
            voiceIndex = i;
            break;
        }
        if (voices[i].cursor > maxProgress) {
            maxProgress = voices[i].cursor;
            voiceIndex = i;
        }
    }

    if (voiceIndex >= 0) {
        SoundVoice& v = voices[voiceIndex];
        v.id = id;
        v.sample = &sample;
        v.cursor = 0;
        v.volume = volume;
        v.pan = pan;
        v.loop = loop;
        v.active = true;
    }

    LeaveCriticalSection(&csVoices);
#endif
}

void SoundManager::play3D(SoundID id, float sourceX, float sourceZ, float listenerX, float listenerZ, float baseVolume) {
    float dx = sourceX - listenerX;
    float dz = sourceZ - listenerZ;
    float dist = sqrtf(dx * dx + dz * dz);

    // Stereo panning based on X offset relative to listener
    float pan = dx / 38.0f;
    if (pan < -1.0f) pan = -1.0f;
    if (pan > 1.0f) pan = 1.0f;

    // Inverse distance attenuation
    const float maxAudibleDist = 200.0f;
    float atten = 1.0f - (dist / maxAudibleDist);
    if (atten < 0.12f) atten = 0.12f;
    if (atten > 1.0f) atten = 1.0f;

    play(id, baseVolume * atten, pan, false);
}

void SoundManager::stop(SoundID id) {
#ifdef _WIN32
    EnterCriticalSection(&csVoices);
    for (int i = 0; i < MAX_VOICES; ++i) {
        if (voices[i].active && voices[i].id == id) {
            voices[i].active = false;
        }
    }
    LeaveCriticalSection(&csVoices);
#endif
}

void SoundManager::stopAll() {
#ifdef _WIN32
    EnterCriticalSection(&csVoices);
    for (int i = 0; i < MAX_VOICES; ++i) {
        voices[i].active = false;
    }
    LeaveCriticalSection(&csVoices);
#endif
}

void SoundManager::toggleMute() {
    muted = !muted;
}

void SoundManager::setMuted(bool mute) {
    muted = mute;
}

bool SoundManager::isMuted() const {
    return muted;
}

void SoundManager::setMasterVolume(float vol) {
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    masterVolume = vol;
}

float SoundManager::getMasterVolume() const {
    return masterVolume;
}

void SoundManager::increaseVolume(float delta) {
    setMasterVolume(masterVolume + delta);
}

void SoundManager::decreaseVolume(float delta) {
    setMasterVolume(masterVolume - delta);
}

bool SoundManager::loadWavFile(SoundID id, const std::string& filepath) {
    FILE* f = fopen(filepath.c_str(), "rb");
    if (!f) return false;

    char riffHeader[4];
    if (fread(riffHeader, 1, 4, f) != 4 || memcmp(riffHeader, "RIFF", 4) != 0) {
        fclose(f);
        return false;
    }

    fseek(f, 4, SEEK_CUR); // Skip file size

    char waveHeader[4];
    if (fread(waveHeader, 1, 4, f) != 4 || memcmp(waveHeader, "WAVE", 4) != 0) {
        fclose(f);
        return false;
    }

    int audioFormat = 0;
    int numChannels = 0;
    int sampleRate = 0;
    int bitsPerSample = 0;
    bool foundFmt = false;
    bool foundData = false;
    std::vector<short> rawData;

    while (!feof(f)) {
        char chunkId[4];
        if (fread(chunkId, 1, 4, f) != 4) break;

        unsigned int chunkSize = 0;
        if (fread(&chunkSize, 4, 1, f) != 1) break;

        if (memcmp(chunkId, "fmt ", 4) == 0) {
            unsigned short formatTag = 0, channels = 0, bits = 0;
            unsigned int sRate = 0;

            fread(&formatTag, 2, 1, f);
            fread(&channels, 2, 1, f);
            fread(&sRate, 4, 1, f);
            fseek(f, 6, SEEK_CUR); // Skip avgBytesPerSec and blockAlign
            fread(&bits, 2, 1, f);

            audioFormat = formatTag;
            if (audioFormat != 1) { // Only standard uncompressed PCM is supported
                fclose(f);
                return false;
            }
            numChannels = channels;
            sampleRate = sRate;
            bitsPerSample = bits;
            foundFmt = true;

            // Skip any extra format bytes
            if (chunkSize > 16) {
                fseek(f, chunkSize - 16, SEEK_CUR);
            }
        } else if (memcmp(chunkId, "data", 4) == 0) {
            if (!foundFmt) {
                fseek(f, chunkSize, SEEK_CUR);
                continue;
            }

            if (bitsPerSample == 16) {
                int numFrames = chunkSize / (numChannels * 2);
                rawData.reserve(numFrames);
                for (int i = 0; i < numFrames; ++i) {
                    short left = 0, right = 0;
                    fread(&left, 2, 1, f);
                    if (numChannels >= 2) {
                        fread(&right, 2, 1, f);
                        for (int c = 2; c < numChannels; ++c) fseek(f, 2, SEEK_CUR);
                        rawData.push_back((left + right) / 2);
                    } else {
                        rawData.push_back(left);
                    }
                }
            } else if (bitsPerSample == 8) {
                int numFrames = chunkSize / numChannels;
                rawData.reserve(numFrames);
                for (int i = 0; i < numFrames; ++i) {
                    unsigned char bLeft = 128, bRight = 128;
                    fread(&bLeft, 1, 1, f);
                    if (numChannels >= 2) {
                        fread(&bRight, 1, 1, f);
                        for (int c = 2; c < numChannels; ++c) fseek(f, 1, SEEK_CUR);
                        int avg = ((int)bLeft + (int)bRight) / 2;
                        rawData.push_back(static_cast<short>((avg - 128) * 256));
                    } else {
                        rawData.push_back(static_cast<short>(((int)bLeft - 128) * 256));
                    }
                }
            } else {
                fseek(f, chunkSize, SEEK_CUR);
            }
            foundData = true;
            break;
        } else {
            // Unknown chunk, skip
            fseek(f, chunkSize, SEEK_CUR);
        }
    }

    fclose(f);

    if (!foundData || rawData.empty()) return false;

    // Resample to SAMPLE_RATE if needed
    if (sampleRate == SAMPLE_RATE) {
        soundBank[id].samples = std::move(rawData);
    } else if (sampleRate > 0) {
        double ratio = static_cast<double>(sampleRate) / static_cast<double>(SAMPLE_RATE);
        size_t newLength = static_cast<size_t>(rawData.size() / ratio);
        std::vector<short> resampled(newLength);
        for (size_t i = 0; i < newLength; ++i) {
            double srcIdx = i * ratio;
            size_t idx0 = static_cast<size_t>(srcIdx);
            size_t idx1 = std::min(idx0 + 1, rawData.size() - 1);
            double frac = srcIdx - idx0;
            double val = rawData[idx0] * (1.0 - frac) + rawData[idx1] * frac;
            resampled[i] = static_cast<short>(val);
        }
        soundBank[id].samples = std::move(resampled);
    }

    soundBank[id].loaded = true;
    return true;
}

void SoundManager::synthesizeFallback(SoundID id) {
    std::vector<short>& s = soundBank[id].samples;
    s.clear();

    const double PI = 3.14159265358979323846;

    switch (id) {
        case SND_LASER: {
            // Swept sine 1100 -> 250 Hz in 0.15s
            int n = static_cast<int>(0.15 * SAMPLE_RATE);
            s.resize(n);
            double phase = 0.0;
            for (int i = 0; i < n; ++i) {
                double t = (double)i / n;
                double freq = 1100.0 * (1.0 - t * 0.85);
                phase += 2.0 * PI * freq / SAMPLE_RATE;
                double env = pow(1.0 - t, 1.8);
                s[i] = static_cast<short>(sin(phase) * env * 26000.0);
            }
            break;
        }
        case SND_LASER_TRIPLE: {
            // Heavy 1400 -> 180 Hz in 0.22s
            int n = static_cast<int>(0.22 * SAMPLE_RATE);
            s.resize(n);
            double phase = 0.0, pSub = 0.0;
            for (int i = 0; i < n; ++i) {
                double t = (double)i / n;
                double freq = 1400.0 * (1.0 - t * 0.88);
                double subFreq = 180.0 * (1.0 - t * 0.7);
                phase += 2.0 * PI * freq / SAMPLE_RATE;
                pSub += 2.0 * PI * subFreq / SAMPLE_RATE;
                double env = pow(1.0 - t, 1.6);
                double val = sin(phase) * 0.7 + sin(pSub) * 0.4;
                s[i] = static_cast<short>(tanh(val * 1.5) * env * 28000.0);
            }
            break;
        }
        case SND_EXPLOSION_SMALL:
        case SND_EXPLOSION_MED:
        case SND_EXPLOSION_BOSS: {
            double dur = (id == SND_EXPLOSION_BOSS) ? 1.4 : ((id == SND_EXPLOSION_MED) ? 0.6 : 0.35);
            double subFreq = (id == SND_EXPLOSION_BOSS) ? 50.0 : ((id == SND_EXPLOSION_MED) ? 80.0 : 110.0);
            int n = static_cast<int>(dur * SAMPLE_RATE);
            s.resize(n);
            double subPhase = 0.0, lastNoise = 0.0;
            for (int i = 0; i < n; ++i) {
                double t = (double)i / n;
                subPhase += 2.0 * PI * (subFreq * (1.0 - t * 0.6)) / SAMPLE_RATE;
                double env = pow(1.0 - t, 1.5);
                double rawNoise = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
                lastNoise += 0.15 * (rawNoise - lastNoise);
                double val = lastNoise * 0.7 + sin(subPhase) * 0.3;
                s[i] = static_cast<short>(tanh(val * 1.6) * env * 28000.0);
            }
            break;
        }
        case SND_MISSILE_LAUNCH: {
            // Rocket thruster ignition and whoosh (0.42s)
            int n = static_cast<int>(0.42 * SAMPLE_RATE);
            s.resize(n);
            double phase = 0.0, noiseFilter = 0.0;
            for (int i = 0; i < n; ++i) {
                double t = (double)i / n;
                // Frequency sweep: deep ignition rumble rising then whooshing away
                double freq = 140.0 + 420.0 * sin(t * PI * 0.85);
                phase += 2.0 * PI * freq / SAMPLE_RATE;
                // Noise element for rocket combustion thrust
                double rawNoise = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
                noiseFilter += 0.28 * (rawNoise - noiseFilter);
                // Envelope: quick attack, sustained burn, smooth decay
                double env = (t < 0.1) ? (t / 0.1) : pow(1.0 - (t - 0.1) / 0.9, 1.4);
                double val = sin(phase) * 0.45 + noiseFilter * 0.75;
                s[i] = static_cast<short>(tanh(val * 1.6) * env * 27000.0);
            }
            break;
        }
        default: {
            // Simple generic tone fallback
            int n = static_cast<int>(0.15 * SAMPLE_RATE);
            s.resize(n);
            double phase = 0.0;
            for (int i = 0; i < n; ++i) {
                double t = (double)i / n;
                phase += 2.0 * PI * 600.0 / SAMPLE_RATE;
                s[i] = static_cast<short>(sin(phase) * (1.0 - t) * 20000.0);
            }
            break;
        }
    }

    soundBank[id].loaded = true;
}

void SoundManager::loadAllSounds() {
    const struct {
        SoundID id;
        const char* path;
    } soundFiles[] = {
        { SND_LASER,           "assets/sounds/laser.wav" },
        { SND_LASER_TRIPLE,    "assets/sounds/laser_triple.wav" },
        { SND_ENEMY_LASER,     "assets/sounds/enemy_laser.wav" },
        { SND_BOSS_LASER,      "assets/sounds/boss_laser.wav" },
        { SND_HIT,             "assets/sounds/hit.wav" },
        { SND_EXPLOSION_SMALL, "assets/sounds/explosion_small.wav" },
        { SND_EXPLOSION_MED,   "assets/sounds/explosion_med.wav" },
        { SND_EXPLOSION_BOSS,  "assets/sounds/explosion_boss.wav" },
        { SND_SHIELD_HIT,      "assets/sounds/shield_hit.wav" },
        { SND_HULL_HIT,        "assets/sounds/hull_hit.wav" },
        { SND_POWERUP_SHIELD,  "assets/sounds/powerup_shield.wav" },
        { SND_POWERUP_HEALTH,  "assets/sounds/powerup_health.wav" },
        { SND_POWERUP_WEAPON,  "assets/sounds/powerup_weapon.wav" },
        { SND_LEVEL_CLEAR,     "assets/sounds/level_clear.wav" },
        { SND_BOSS_WARNING,    "assets/sounds/boss_warning.wav" },
        { SND_GAME_OVER,       "assets/sounds/game_over.wav" },
        { SND_VICTORY,         "assets/sounds/victory.wav" },
        { SND_UI_CLICK,        "assets/sounds/ui_click.wav" },
        { SND_MISSILE_LAUNCH,  "assets/sounds/missile_launch.wav" }
    };

    for (const auto& item : soundFiles) {
        if (!loadWavFile(item.id, item.path)) {
            // Also try relative to current directory or fallback
            synthesizeFallback(item.id);
        }
    }
}
