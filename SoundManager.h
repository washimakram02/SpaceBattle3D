#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>
#endif

#include <vector>
#include <string>

enum SoundID {
    SND_LASER = 0,
    SND_LASER_TRIPLE,
    SND_ENEMY_LASER,
    SND_BOSS_LASER,
    SND_HIT,
    SND_EXPLOSION_SMALL,
    SND_EXPLOSION_MED,
    SND_EXPLOSION_BOSS,
    SND_SHIELD_HIT,
    SND_HULL_HIT,
    SND_POWERUP_SHIELD,
    SND_POWERUP_HEALTH,
    SND_POWERUP_WEAPON,
    SND_LEVEL_CLEAR,
    SND_BOSS_WARNING,
    SND_GAME_OVER,
    SND_VICTORY,
    SND_UI_CLICK,
    SND_MISSILE_LAUNCH,
    SND_COUNT
};

struct SoundSample {
    std::vector<short> samples; // 16-bit mono samples resampled to 44.1kHz
    bool loaded;

    SoundSample() : loaded(false) {}
};

struct SoundVoice {
    SoundID id;
    const SoundSample* sample;
    size_t cursor;
    float volume;
    float pan; // -1.0f (left) to +1.0f (right)
    bool active;
    bool loop;

    SoundVoice() : id(SND_LASER), sample(nullptr), cursor(0), volume(1.0f), pan(0.0f), active(false), loop(false) {}
};

class SoundManager {
public:
    static const int SAMPLE_RATE = 44100;
    static const int CHANNELS = 2;
    static const int BUFFER_SAMPLES = 2048; // ~46ms per buffer
    static const int NUM_BUFFERS = 4;
    static const int MAX_VOICES = 32;

    SoundManager();
    ~SoundManager();

    bool init();
    void cleanup();

    // Playback methods
    void play(SoundID id, float volume = 1.0f, float pan = 0.0f, bool loop = false);
    void play3D(SoundID id, float sourceX, float sourceZ, float listenerX, float listenerZ, float baseVolume = 1.0f);
    void stop(SoundID id);
    void stopAll();

    // Settings
    void toggleMute();
    void setMuted(bool mute);
    bool isMuted() const;

    void setMasterVolume(float vol);
    float getMasterVolume() const;

    void increaseVolume(float delta = 0.1f);
    void decreaseVolume(float delta = 0.1f);

    // Audio thread callback helper
    void audioThreadLoop();

private:
    bool initialized;
    bool running;
    bool muted;
    float masterVolume;

    SoundSample soundBank[SND_COUNT];
    SoundVoice voices[MAX_VOICES];

#ifdef _WIN32
    HWAVEOUT hWaveOut;
    HANDLE hAudioThread;
    HANDLE hWakeEvent;
    CRITICAL_SECTION csVoices;
    WAVEHDR waveHeaders[NUM_BUFFERS];
    short audioBuffers[NUM_BUFFERS][BUFFER_SAMPLES * CHANNELS];
#endif

    bool loadWavFile(SoundID id, const std::string& filepath);
    void synthesizeFallback(SoundID id);
    void loadAllSounds();
};

#endif // SOUNDMANAGER_H
