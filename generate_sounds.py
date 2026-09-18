import os
import math
import struct
import wave
import random

SAMPLE_RATE = 44100

def write_wav(filename, samples, sample_rate=SAMPLE_RATE, channels=1):
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    with wave.open(filename, 'w') as wav_file:
        wav_file.setnchannels(channels)
        wav_file.setsampwidth(2) # 16-bit
        wav_file.setframerate(sample_rate)
        frames = bytearray()
        for sample in samples:
            val = max(-32767, min(32767, int(sample * 32767.0)))
            if channels == 1:
                frames.extend(struct.pack('<h', val))
            elif channels == 2:
                frames.extend(struct.pack('<hh', val, val))
        wav_file.writeframes(frames)
    print(f"Generated: {filename} ({len(samples)} samples, {len(samples)/sample_rate:.2f}s)")

def generate_laser():
    duration = 0.15
    num_samples = int(duration * SAMPLE_RATE)
    samples = []
    phase = 0.0
    for i in range(num_samples):
        t = i / num_samples
        freq = 1100.0 * (1.0 - t * 0.85)
        phase += 2.0 * math.pi * freq / SAMPLE_RATE
        env = (1.0 - t) ** 1.8
        sin_val = math.sin(phase)
        tri_val = (phase % (2.0 * math.pi)) / math.pi - 1.0
        sample = (0.7 * sin_val + 0.3 * tri_val) * env * 0.85
        samples.append(sample)
    return samples

def generate_laser_triple():
    duration = 0.22
    num_samples = int(duration * SAMPLE_RATE)
    samples = []
    phase = 0.0
    phase_sub = 0.0
    for i in range(num_samples):
        t = i / num_samples
        freq = 1400.0 * (1.0 - t * 0.88)
        sub_freq = 180.0 * (1.0 - t * 0.7)
        phase += 2.0 * math.pi * freq / SAMPLE_RATE
        phase_sub += 2.0 * math.pi * sub_freq / SAMPLE_RATE
        env = (1.0 - t) ** 1.6
        raw = math.sin(phase) * 0.7 + math.sin(phase_sub) * 0.4
        sample = math.tanh(raw * 1.5) * env * 0.9
        samples.append(sample)
    return samples

def generate_enemy_laser():
    duration = 0.16
    num_samples = int(duration * SAMPLE_RATE)
    samples = []
    phase = 0.0
    for i in range(num_samples):
        t = i / num_samples
        freq = 750.0 * (1.0 - t * 0.75) + 30.0 * math.sin(2.0 * math.pi * 45.0 * (i / SAMPLE_RATE))
        phase += 2.0 * math.pi * freq / SAMPLE_RATE
        env = (1.0 - t) ** 1.5
        saw = 2.0 * ((phase / (2.0 * math.pi)) % 1.0) - 1.0
        sample = saw * env * 0.65
        samples.append(sample)
    return samples

def generate_boss_laser():
    duration = 0.35
    num_samples = int(duration * SAMPLE_RATE)
    samples = []
    phase1 = 0.0
    phase2 = 0.0
    for i in range(num_samples):
        t = i / num_samples
        freq1 = 380.0 * (1.0 - t * 0.7)
        freq2 = 190.0 * (1.0 - t * 0.5)
        phase1 += 2.0 * math.pi * freq1 / SAMPLE_RATE
        phase2 += 2.0 * math.pi * freq2 / SAMPLE_RATE
        env = (1.0 - t) ** 1.3
        raw = math.sin(phase1) * 0.5 + math.sin(phase2) * 0.5 + (random.random() * 2.0 - 1.0) * 0.15
        sample = math.tanh(raw * 2.0) * env * 0.95
        samples.append(sample)
    return samples

def generate_hit():
    duration = 0.05
    num_samples = int(duration * SAMPLE_RATE)
    samples = []
    phase = 0.0
    for i in range(num_samples):
        t = i / num_samples
        freq = 2400.0 * (1.0 - t * 0.9)
        phase += 2.0 * math.pi * freq / SAMPLE_RATE
        env = (1.0 - t) ** 3.0
        noise = (random.random() * 2.0 - 1.0) * 0.4
        sample = (math.sin(phase) * 0.6 + noise) * env * 0.75
        samples.append(sample)
    return samples

def generate_explosion(duration, low_decay, noise_weight, sub_freq):
    num_samples = int(duration * SAMPLE_RATE)
    samples = []
    sub_phase = 0.0
    last_noise = 0.0
    for i in range(num_samples):
        t = i / num_samples
        sub_phase += 2.0 * math.pi * (sub_freq * (1.0 - t * 0.6)) / SAMPLE_RATE
        env = (1.0 - t) ** low_decay
        raw_noise = random.random() * 2.0 - 1.0
        filter_alpha = 0.12 + 0.15 * (1.0 - t)
        last_noise += filter_alpha * (raw_noise - last_noise)
        sample = (last_noise * noise_weight + math.sin(sub_phase) * (1.0 - noise_weight)) * env
        sample = math.tanh(sample * 1.6) * 0.9
        samples.append(sample)
    return samples

def generate_shield_hit():
    duration = 0.22
    num_samples = int(duration * SAMPLE_RATE)
    samples = []
    p1 = 0.0
    p2 = 0.0
    for i in range(num_samples):
        t = i / num_samples
        freq1 = 550.0 + 50.0 * math.sin(2.0 * math.pi * 30.0 * (i / SAMPLE_RATE))
        freq2 = 825.0
        p1 += 2.0 * math.pi * freq1 / SAMPLE_RATE
        p2 += 2.0 * math.pi * freq2 / SAMPLE_RATE
        env = (1.0 - t) ** 2.0
        sample = (math.sin(p1) * 0.55 + math.sin(p2) * 0.35) * env * 0.8
        samples.append(sample)
    return samples

def generate_hull_hit():
    duration = 0.28
    num_samples = int(duration * SAMPLE_RATE)
    samples = []
    phase = 0.0
    phase_low = 0.0
    for i in range(num_samples):
        t = i / num_samples
        freq = 340.0 * (1.0 - t * 0.8)
        low_f = 85.0 * (1.0 - t * 0.5)
        phase += 2.0 * math.pi * freq / SAMPLE_RATE
        phase_low += 2.0 * math.pi * low_f / SAMPLE_RATE
        env = (1.0 - t) ** 2.2
        noise = (random.random() * 2.0 - 1.0) * 0.35
        raw = math.sin(phase) * 0.4 + math.sin(phase_low) * 0.6 + noise
        sample = math.tanh(raw * 1.8) * env * 0.9
        samples.append(sample)
    return samples

def generate_powerup_shield():
    notes = [523.25, 659.25, 783.99, 1046.50]
    note_dur = 0.07
    samples = []
    for note in notes:
        n_samples = int(note_dur * SAMPLE_RATE)
        phase = 0.0
        for i in range(n_samples):
            t = i / n_samples
            phase += 2.0 * math.pi * note / SAMPLE_RATE
            env = math.sin(t * math.pi) ** 0.8
            sample = (math.sin(phase) * 0.75 + math.sin(phase * 2.0) * 0.25) * env * 0.75
            samples.append(sample)
    return samples

def generate_powerup_health():
    notes = [587.33, 739.99, 880.00, 1174.66]
    note_dur = 0.075
    samples = []
    for note in notes:
        n_samples = int(note_dur * SAMPLE_RATE)
        phase = 0.0
        for i in range(n_samples):
            t = i / n_samples
            phase += 2.0 * math.pi * note / SAMPLE_RATE
            env = (1.0 - t) ** 1.3
            sample = (math.sin(phase) * 0.7 + math.sin(phase * 3.0) * 0.2) * env * 0.75
            samples.append(sample)
    return samples

def generate_powerup_weapon():
    duration = 0.32
    num_samples = int(duration * SAMPLE_RATE)
    samples = []
    phase = 0.0
    for i in range(num_samples):
        t = i / num_samples
        freq = 450.0 * (1.0 + t * 2.6)
        phase += 2.0 * math.pi * freq / SAMPLE_RATE
        env = (math.sin(t * math.pi * 0.85)) * (1.0 - t * 0.2)
        sample = (math.sin(phase) * 0.6 + math.sin(phase * 2.0) * 0.3) * env * 0.8
        samples.append(sample)
    return samples

def generate_level_clear():
    notes = [392.00, 523.25, 659.25, 783.99]
    durations = [0.10, 0.10, 0.12, 0.45]
    samples = []
    for note, dur in zip(notes, durations):
        n_samples = int(dur * SAMPLE_RATE)
        phase = 0.0
        for i in range(n_samples):
            t = i / n_samples
            phase += 2.0 * math.pi * note / SAMPLE_RATE
            env = (1.0 - t) ** 1.2 if dur > 0.2 else math.sin(t * math.pi) ** 0.7
            sample = (math.sin(phase) * 0.6 + math.sin(phase * 2.0) * 0.25 + math.sin(phase * 0.5) * 0.15) * env * 0.85
            samples.append(sample)
    return samples

def generate_boss_warning():
    samples = []
    for pulse in range(3):
        n_samples = int(0.18 * SAMPLE_RATE)
        freq = 820.0 if pulse % 2 == 0 else 620.0
        phase = 0.0
        for i in range(n_samples):
            t = i / n_samples
            phase += 2.0 * math.pi * freq / SAMPLE_RATE
            env = math.sin(t * math.pi) ** 0.5
            sample = (math.sin(phase) * 0.7 + math.sin(phase * 3.0) * 0.2) * env * 0.85
            samples.append(sample)
        samples.extend([0.0] * int(0.04 * SAMPLE_RATE))
    return samples

def generate_game_over():
    notes = [329.63, 261.63, 220.00, 164.81]
    durations = [0.22, 0.22, 0.25, 0.60]
    samples = []
    for note, dur in zip(notes, durations):
        n_samples = int(dur * SAMPLE_RATE)
        phase = 0.0
        for i in range(n_samples):
            t = i / n_samples
            phase += 2.0 * math.pi * note / SAMPLE_RATE
            env = (1.0 - t) ** 1.4
            sample = (math.sin(phase) * 0.65 + math.sin(phase * 0.5) * 0.35) * env * 0.8
            samples.append(sample)
    return samples

def generate_victory():
    notes = [261.63, 392.00, 523.25, 659.25, 783.99, 1046.50]
    durations = [0.12, 0.12, 0.14, 0.16, 0.20, 0.85]
    samples = []
    for note, dur in zip(notes, durations):
        n_samples = int(dur * SAMPLE_RATE)
        phase = 0.0
        for i in range(n_samples):
            t = i / n_samples
            phase += 2.0 * math.pi * note / SAMPLE_RATE
            env = (1.0 - t) ** 1.1 if dur > 0.3 else math.sin(t * math.pi) ** 0.7
            sample = (math.sin(phase) * 0.55 + math.sin(phase * 2.0) * 0.3 + math.sin(phase * 3.0) * 0.15) * env * 0.85
            samples.append(sample)
    return samples

def generate_ui_click():
    duration = 0.035
    num_samples = int(duration * SAMPLE_RATE)
    samples = []
    phase = 0.0
    for i in range(num_samples):
        t = i / num_samples
        freq = 1800.0 * (1.0 - t * 0.5)
        phase += 2.0 * math.pi * freq / SAMPLE_RATE
        env = (1.0 - t) ** 2.0
        sample = math.sin(phase) * env * 0.7
        samples.append(sample)
    return samples

def main():
    base_dir = os.path.join(os.path.dirname(__file__), "assets", "sounds")
    os.makedirs(base_dir, exist_ok=True)
    
    sound_generators = {
        "laser.wav": generate_laser(),
        "laser_triple.wav": generate_laser_triple(),
        "enemy_laser.wav": generate_enemy_laser(),
        "boss_laser.wav": generate_boss_laser(),
        "hit.wav": generate_hit(),
        "explosion_small.wav": generate_explosion(0.35, 1.8, 0.75, 110.0),
        "explosion_med.wav": generate_explosion(0.65, 1.6, 0.65, 80.0),
        "explosion_boss.wav": generate_explosion(1.50, 1.3, 0.55, 50.0),
        "shield_hit.wav": generate_shield_hit(),
        "hull_hit.wav": generate_hull_hit(),
        "powerup_shield.wav": generate_powerup_shield(),
        "powerup_health.wav": generate_powerup_health(),
        "powerup_weapon.wav": generate_powerup_weapon(),
        "level_clear.wav": generate_level_clear(),
        "boss_warning.wav": generate_boss_warning(),
        "game_over.wav": generate_game_over(),
        "victory.wav": generate_victory(),
        "ui_click.wav": generate_ui_click(),
    }

    print("Synthesizing audio sound effects...")
    for fname, data in sound_generators.items():
        filepath = os.path.join(base_dir, fname)
        write_wav(filepath, data)
    print("All audio files generated successfully!")

if __name__ == "__main__":
    main()
