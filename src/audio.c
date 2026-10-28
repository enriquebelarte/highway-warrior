#include "game.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define AUDIO_SAMPLE_RATE 44100
#define STREAM_BUFFER_SIZE 1024

// Engine Audio State
static float s_phase = 0.0f;
static float s_target_freq = 70.0f;
static float s_current_freq = 70.0f;
static float s_engine_volume = 0.35f;

// 80s Synth Music State
#define BPM 128
#define SAMPLES_PER_STEP 5168 // 44100 * 60 / (128 * 4)
#define NUM_BARS 16
#define STEPS_PER_BAR 16
#define TOTAL_STEPS (NUM_BARS * STEPS_PER_BAR)
#define TOTAL_LOOP_SAMPLES (TOTAL_STEPS * SAMPLES_PER_STEP)

// Synth Scale Note Frequencies (Hz)
#define N_REST 0.0f
#define N_D2   73.42f
#define N_E2   82.41f
#define N_F2   87.31f
#define N_G2   98.00f
#define N_A2  110.00f
#define N_BB2 116.54f
#define N_C3  130.81f
#define N_CS3 138.59f
#define N_D3  146.83f
#define N_E3  164.81f
#define N_F3  174.61f
#define N_G3  196.00f
#define N_A3  220.00f
#define N_BB3 233.08f
#define N_C4  261.63f
#define N_CS4 277.18f
#define N_D4  293.66f
#define N_E4  329.63f
#define N_F4  349.23f
#define N_G4  392.00f
#define N_A4  440.00f
#define N_BB4 466.16f
#define N_C5  523.25f
#define N_CS5 554.37f
#define N_D5  587.33f
#define N_E5  659.25f
#define N_F5  698.46f
#define N_G5  783.99f
#define N_A5  880.00f

static unsigned int s_music_sample_clock = 0;
static float s_bass_phase = 0.0f;
static float s_chord_phase[3] = { 0.0f, 0.0f, 0.0f };
static float s_chord_detune_phase[3] = { 0.0f, 0.0f, 0.0f };
static float s_lead_phase = 0.0f;
static float s_lead_freq_smooth = 0.0f;
static float s_kick_phase = 0.0f;

// 16-Bar Bassline (256 steps)
static const float BASS_NOTES[16][16] = {
    // Bar 1: Dm
    { N_D2, N_D2, N_D3, N_D2, N_D2, N_D3, N_D2, N_D2, N_D2, N_D2, N_D3, N_D2, N_D2, N_D3, N_F2, N_G2 },
    // Bar 2: Bb
    { N_BB2, N_BB2, N_BB3, N_BB2, N_BB2, N_BB3, N_BB2, N_BB2, N_BB2, N_BB2, N_BB3, N_BB2, N_BB2, N_BB3, N_C3, N_D3 },
    // Bar 3: C
    { N_C3, N_C3, N_C4, N_C3, N_C3, N_C4, N_C3, N_C3, N_C3, N_C3, N_C4, N_C3, N_C3, N_C4, N_D3, N_E3 },
    // Bar 4: Am
    { N_A2, N_A2, N_A3, N_A2, N_A2, N_A3, N_A2, N_A2, N_A2, N_A2, N_A3, N_A2, N_G2, N_F2, N_E2, N_C3 },
    // Bar 5: Dm
    { N_D2, N_D2, N_D3, N_D2, N_D2, N_D3, N_D2, N_D2, N_D2, N_D2, N_D3, N_D2, N_D2, N_D3, N_F2, N_G2 },
    // Bar 6: F
    { N_F2, N_F2, N_F3, N_F2, N_F2, N_F3, N_F2, N_F2, N_F2, N_F2, N_F3, N_F2, N_F2, N_F3, N_G2, N_A2 },
    // Bar 7: Gm -> C
    { N_G2, N_G2, N_G3, N_G2, N_G2, N_G3, N_G2, N_G2, N_C3, N_C3, N_C4, N_C3, N_C3, N_C4, N_D3, N_E3 },
    // Bar 8: Dm
    { N_D2, N_D2, N_D3, N_D2, N_D2, N_D3, N_D2, N_D2, N_D2, N_D2, N_D3, N_D2, N_C3, N_A2, N_C3, N_CS3 },
    // Bar 9: Bb
    { N_BB2, N_BB2, N_BB3, N_BB2, N_BB2, N_BB3, N_BB2, N_BB2, N_BB2, N_BB2, N_BB3, N_BB2, N_BB2, N_BB3, N_C3, N_D3 },
    // Bar 10: C
    { N_C3, N_C3, N_C4, N_C3, N_C3, N_C4, N_C3, N_C3, N_C3, N_C3, N_C4, N_C3, N_C3, N_C4, N_D3, N_E3 },
    // Bar 11: Dm
    { N_D2, N_D2, N_D3, N_D2, N_D2, N_D3, N_D2, N_D2, N_D2, N_D2, N_D3, N_D2, N_D2, N_D3, N_F2, N_G2 },
    // Bar 12: F
    { N_F2, N_F2, N_F3, N_F2, N_F2, N_F3, N_F2, N_F2, N_F2, N_F2, N_F3, N_F2, N_F2, N_F3, N_G2, N_A2 },
    // Bar 13: Bb
    { N_BB2, N_BB2, N_BB3, N_BB2, N_BB2, N_BB3, N_BB2, N_BB2, N_BB2, N_BB2, N_BB3, N_BB2, N_BB2, N_BB3, N_C3, N_D3 },
    // Bar 14: C
    { N_C3, N_C3, N_C4, N_C3, N_C3, N_C4, N_C3, N_C3, N_C3, N_C3, N_C4, N_C3, N_C3, N_C4, N_D3, N_E3 },
    // Bar 15: Gm
    { N_G2, N_G2, N_G3, N_G2, N_G2, N_G3, N_G2, N_G2, N_G2, N_G2, N_G3, N_G2, N_G2, N_G3, N_A2, N_BB2 },
    // Bar 16: A7
    { N_A2, N_A2, N_A3, N_A2, N_A2, N_A3, N_A2, N_A2, N_A2, N_A2, N_A3, N_A2, N_CS3, N_E3, N_G3, N_A3 }
};

// 16-Bar Chord Triads (Bar -> 3 frequencies)
static const float CHORD_TRIADS[16][3] = {
    { N_F4, N_A4, N_D5 },   // 1: Dm
    { N_F4, N_BB4, N_D5 },  // 2: Bb
    { N_G4, N_C5, N_E5 },   // 3: C
    { N_E4, N_A4, N_C5 },   // 4: Am
    { N_F4, N_A4, N_D5 },   // 5: Dm
    { N_F4, N_A4, N_C5 },   // 6: F
    { N_G4, N_BB4, N_D5 },  // 7: Gm
    { N_F4, N_A4, N_D5 },   // 8: Dm
    { N_F4, N_BB4, N_D5 },  // 9: Bb
    { N_G4, N_C5, N_E5 },   // 10: C
    { N_F4, N_A4, N_D5 },   // 11: Dm
    { N_F4, N_A4, N_C5 },   // 12: F
    { N_F4, N_BB4, N_D5 },  // 13: Bb
    { N_G4, N_C5, N_E5 },   // 14: C
    { N_G4, N_BB4, N_D5 },  // 15: Gm
    { N_G4, N_A4, N_CS5 }   // 16: A7
};

// Syncopated chord trigger pattern (1 = stab, 0 = silent)
static const int CHORD_RHYTHM[16] = { 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0 };

// 16-Bar Lead Melody (256 steps)
static const float LEAD_NOTES[16][16] = {
    // Bars 1-4: Intro groove (bass & chords)
    { 0 }, { 0 }, { 0 }, { 0 },

    // Bar 5:
    { N_A4, N_A4, N_A4, N_A4, N_D5, N_D5, N_D5, N_D5, N_C5, N_C5, N_C5, N_C5, N_A4, N_A4, N_A4, N_A4 },
    // Bar 6:
    { N_F4, N_F4, N_F4, N_F4, N_G4, N_G4, N_G4, N_G4, N_A4, N_A4, N_A4, N_A4, N_C5, N_C5, N_C5, N_C5 },
    // Bar 7:
    { N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_E5, N_E5, N_F5, N_F5, N_F5, N_F5, N_E5, N_E5, N_E5, N_E5 },
    // Bar 8:
    { N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, 0, 0, 0, 0 },

    // Bar 9: High register
    { N_F5, N_F5, N_F5, N_F5, N_E5, N_E5, N_E5, N_E5, N_D5, N_D5, N_D5, N_D5, N_C5, N_C5, N_C5, N_C5 },
    // Bar 10:
    { N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_E5, N_E5, N_E5, N_E5, N_G5, N_G5, N_G5, N_G5 },
    // Bar 11:
    { N_F5, N_F5, N_F5, N_F5, N_E5, N_E5, N_E5, N_E5, N_D5, N_D5, N_D5, N_D5, N_A4, N_A4, N_A4, N_A4 },
    // Bar 12:
    { N_C5, N_C5, N_C5, N_C5, N_C5, N_C5, N_C5, N_C5, N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_D5 },

    // Bar 13: Climax chorus
    { N_D5, N_D5, N_D5, N_D5, N_F5, N_F5, N_F5, N_F5, N_A5, N_A5, N_A5, N_A5, N_G5, N_G5, N_G5, N_G5 },
    // Bar 14:
    { N_E5, N_E5, N_E5, N_E5, N_E5, N_E5, N_E5, N_E5, N_G5, N_G5, N_G5, N_G5, N_F5, N_F5, N_F5, N_F5 },
    // Bar 15:
    { N_D5, N_D5, N_D5, N_D5, N_D5, N_D5, N_E5, N_E5, N_F5, N_F5, N_F5, N_F5, N_G5, N_G5, N_G5, N_G5 },
    // Bar 16: Turnaround to Bar 1
    { N_A5, N_A5, N_A5, N_A5, N_A5, N_A5, N_A5, N_A5, N_CS5, N_CS5, N_CS5, N_CS5, N_E5, N_E5, N_E5, N_E5 }
};

static void Fill80sSynthBuffer(short *buffer, int count) {
    for (int i = 0; i < count; i++) {
        unsigned int pos = s_music_sample_clock % TOTAL_LOOP_SAMPLES;
        int step = (pos / SAMPLES_PER_STEP) % TOTAL_STEPS;
        int bar = step / STEPS_PER_BAR;
        int bar_step = step % STEPS_PER_BAR;
        float step_t = (float)(pos % SAMPLES_PER_STEP) / (float)AUDIO_SAMPLE_RATE;
        float sixteenth_dur = (float)SAMPLES_PER_STEP / (float)AUDIO_SAMPLE_RATE;

        // ----------------------------------------------------
        // 1. Synth Bass (Analog Sawtooth + Sub)
        // ----------------------------------------------------
        float bass_freq = BASS_NOTES[bar][bar_step];
        float bass_sample = 0.0f;
        if (bass_freq > 10.0f) {
            float b_inc = (2.0f * PI * bass_freq) / (float)AUDIO_SAMPLE_RATE;
            s_bass_phase += b_inc;
            if (s_bass_phase >= 2.0f * PI) s_bass_phase -= 2.0f * PI;

            float saw = (s_bass_phase / PI) - 1.0f;
            float sub = (sinf(s_bass_phase * 0.5f) > 0.0f) ? 0.35f : -0.35f;
            float bass_env = expf(-step_t * 22.0f) * 0.75f + 0.25f * (1.0f - step_t / sixteenth_dur);
            if (bass_env < 0.0f) bass_env = 0.0f;
            bass_sample = (saw * 0.7f + sub) * bass_env;
        }

        // ----------------------------------------------------
        // 2. 80s Polysynth Chords (Dual Detuned Saws)
        // ----------------------------------------------------
        float chord_sample = 0.0f;
        int last_stab_step = bar_step;
        while (last_stab_step >= 0 && !CHORD_RHYTHM[last_stab_step]) {
            last_stab_step--;
        }
        if (last_stab_step >= 0) {
            float chord_t = (float)((bar_step - last_stab_step) * SAMPLES_PER_STEP + (pos % SAMPLES_PER_STEP)) / (float)AUDIO_SAMPLE_RATE;
            if (chord_t < 0.28f) {
                float c_env = expf(-chord_t * 7.5f) * (1.0f - chord_t / 0.28f);
                for (int v = 0; v < 3; v++) {
                    float f1 = CHORD_TRIADS[bar][v];
                    float f2 = f1 * 1.007f;
                    s_chord_phase[v] += (2.0f * PI * f1) / (float)AUDIO_SAMPLE_RATE;
                    if (s_chord_phase[v] >= 2.0f * PI) s_chord_phase[v] -= 2.0f * PI;
                    s_chord_detune_phase[v] += (2.0f * PI * f2) / (float)AUDIO_SAMPLE_RATE;
                    if (s_chord_detune_phase[v] >= 2.0f * PI) s_chord_detune_phase[v] -= 2.0f * PI;

                    float s1 = (s_chord_phase[v] / PI) - 1.0f;
                    float s2 = (s_chord_detune_phase[v] / PI) - 1.0f;
                    chord_sample += (s1 + s2) * 0.5f;
                }
                chord_sample = (chord_sample / 3.0f) * c_env;
            }
        }

        // ----------------------------------------------------
        // 3. 80s Lead Melody (Pulse Wave with Vibrato)
        // ----------------------------------------------------
        float lead_sample = 0.0f;
        float target_lead_freq = LEAD_NOTES[bar][bar_step];
        if (target_lead_freq > 10.0f) {
            if (s_lead_freq_smooth < 10.0f) s_lead_freq_smooth = target_lead_freq;
            s_lead_freq_smooth += (target_lead_freq - s_lead_freq_smooth) * 0.005f;

            float vib = sinf((float)pos * 0.00078f) * 0.012f * s_lead_freq_smooth;
            float l_inc = (2.0f * PI * (s_lead_freq_smooth + vib)) / (float)AUDIO_SAMPLE_RATE;
            s_lead_phase += l_inc;
            if (s_lead_phase >= 2.0f * PI) s_lead_phase -= 2.0f * PI;

            float pulse = (s_lead_phase < 0.8f * PI) ? 0.7f : -0.7f;
            float tri = (s_lead_phase < PI) ? ((s_lead_phase / PI) * 2.0f - 1.0f) : (3.0f - (s_lead_phase / PI) * 2.0f);
            float l_env = 0.95f;
            lead_sample = (pulse * 0.65f + tri * 0.35f) * l_env;
        } else {
            s_lead_freq_smooth = 0.0f;
        }

        // ----------------------------------------------------
        // 4. Retro Drums (Kick, Snare, Hi-hat)
        // ----------------------------------------------------
        float drum_sample = 0.0f;

        // Kick Drum
        bool is_kick_step = (bar_step == 0 || bar_step == 6 || bar_step == 8 || bar_step == 10);
        if (is_kick_step) {
            if (step_t < 0.12f) {
                float k_freq = 145.0f * expf(-step_t * 32.0f) + 48.0f;
                s_kick_phase += (2.0f * PI * k_freq) / (float)AUDIO_SAMPLE_RATE;
                float k_env = expf(-step_t * 26.0f);
                drum_sample += sinf(s_kick_phase) * k_env * 1.1f;
            }
        }

        // Snare Drum
        if (bar_step == 4 || bar_step == 12) {
            if (step_t < 0.16f) {
                float s_noise = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f;
                float s_tone = sinf(step_t * 2.0f * PI * 210.0f);
                float s_env = expf(-step_t * 20.0f);
                drum_sample += (s_noise * 0.7f + s_tone * 0.3f) * s_env * 0.9f;
            }
        }

        // Hi-Hat
        if (bar_step % 2 == 0) {
            float h_len = (bar_step == 14 && bar % 2 == 1) ? 0.09f : 0.035f;
            if (step_t < h_len) {
                float h_noise = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f;
                float h_env = expf(-step_t * (h_len > 0.05f ? 40.0f : 90.0f));
                drum_sample += h_noise * h_env * 0.42f;
            }
        }

        // Mix & Warm Analog Saturation
        float mix = bass_sample * 0.32f + chord_sample * 0.28f + lead_sample * 0.24f + drum_sample * 0.35f;
        if (mix > 1.0f) mix = 1.0f;
        else if (mix < -1.0f) mix = -1.0f;

        buffer[i] = (short)(mix * 17500.0f);
        s_music_sample_clock++;
    }
}

// Sound Effect Wave Fillers
static Sound GenerateProceduralSound(int num_frames, void (*fill_samples)(short *buffer, int count)) {
    short *data = (short *)malloc(num_frames * sizeof(short));
    if (fill_samples) {
        fill_samples(data, num_frames);
    } else {
        for (int i = 0; i < num_frames; i++) data[i] = 0;
    }

    Wave wave = {
        .data = data,
        .frameCount = num_frames,
        .sampleRate = AUDIO_SAMPLE_RATE,
        .sampleSize = 16,
        .channels = 1
    };

    Sound sound = LoadSoundFromWave(wave);
    UnloadWave(wave);
    return sound;
}

static void FillSkidNoise(short *buffer, int count) {
    for (int i = 0; i < count; i++) {
        float t = (float)i / (float)count;
        float env = (1.0f - t) * (0.8f + 0.2f * sinf(i * 0.05f));
        float tone = sinf(i * 0.32f) * 0.5f + ((float)rand() / (float)RAND_MAX - 0.5f) * 0.5f;
        buffer[i] = (short)(tone * env * 14000.0f);
    }
}

static void FillCrashNoise(short *buffer, int count) {
    for (int i = 0; i < count; i++) {
        float t = (float)i / (float)count;
        float env = expf(-t * 5.0f);
        float noise = ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f;
        float rumble = sinf(i * 0.015f) * 0.7f;
        buffer[i] = (short)((noise * 0.6f + rumble * 0.4f) * env * 22000.0f);
    }
}

static void FillFuelChime(short *buffer, int count) {
    for (int i = 0; i < count; i++) {
        float t = (float)i / (float)AUDIO_SAMPLE_RATE;
        float step = floorf(t * 18.0f);
        float freq = 587.33f + step * 160.0f;
        float tone = sinf(2.0f * PI * freq * t);
        float env = expf(-t * 7.0f);
        buffer[i] = (short)(tone * env * 16000.0f);
    }
}

static void FillCheckpointFanfare(short *buffer, int count) {
    for (int i = 0; i < count; i++) {
        float t = (float)i / (float)AUDIO_SAMPLE_RATE;
        float freq = (t < 0.16f) ? 523.25f : (t < 0.32f ? 659.25f : 783.99f);
        float tone = (sinf(2.0f * PI * freq * t) > 0.0f ? 1.0f : -1.0f) * 0.5f;
        float env = 1.0f - (t / 0.55f);
        if (env < 0.0f) env = 0.0f;
        buffer[i] = (short)(tone * env * 14000.0f);
    }
}

static void FillGearClick(short *buffer, int count) {
    for (int i = 0; i < count; i++) {
        float t = (float)i / (float)count;
        float env = expf(-t * 22.0f);
        float click = sinf(i * 0.25f);
        buffer[i] = (short)(click * env * 18000.0f);
    }
}

void Audio_Init(AudioSystem *as) {
    InitAudioDevice();

    // Engine sound stream
    as->engine_stream = LoadAudioStream(AUDIO_SAMPLE_RATE, 16, 1);
    SetAudioStreamVolume(as->engine_stream, 0.40f);
    PlayAudioStream(as->engine_stream);

    // 80s Synth BGM stream
    as->music_stream = LoadAudioStream(AUDIO_SAMPLE_RATE, 16, 1);
    SetAudioStreamVolume(as->music_stream, 0.55f);
    PlayAudioStream(as->music_stream);

    as->snd_skid = GenerateProceduralSound(AUDIO_SAMPLE_RATE * 0.35f, FillSkidNoise);
    as->snd_crash = GenerateProceduralSound(AUDIO_SAMPLE_RATE * 0.70f, FillCrashNoise);
    as->snd_fuel = GenerateProceduralSound(AUDIO_SAMPLE_RATE * 0.30f, FillFuelChime);
    as->snd_checkpoint = GenerateProceduralSound(AUDIO_SAMPLE_RATE * 0.55f, FillCheckpointFanfare);
    as->snd_gear = GenerateProceduralSound(AUDIO_SAMPLE_RATE * 0.08f, FillGearClick);

    as->initialized = true;
}

void Audio_PlaySfx(AudioSystem *as, SfxType sfx) {
    if (!as->initialized) return;

    switch (sfx) {
        case SFX_SKID:
            PlaySound(as->snd_skid);
            break;
        case SFX_CRASH:
            PlaySound(as->snd_crash);
            break;
        case SFX_FUEL:
            PlaySound(as->snd_fuel);
            break;
        case SFX_CHECKPOINT:
            PlaySound(as->snd_checkpoint);
            break;
        case SFX_GEAR:
            PlaySound(as->snd_gear);
            break;
        default:
            break;
    }
}

void Audio_Update(AudioSystem *as, const Player *p, bool is_offroad) {
    if (!as->initialized) return;

    // Pitch scales directly with vehicle speed & gear
    float speed_ratio = p->speed / MAX_SPEED_HIGH;
    if (speed_ratio > 1.0f) speed_ratio = 1.0f;

    if (p->is_crashed) {
        s_target_freq = 0.0f;
        s_engine_volume = 0.0f;
        SetAudioStreamVolume(as->music_stream, 0.35f);
    } else {
        if (p->gear == 0) {
            float low_ratio = p->speed / MAX_SPEED_LOW;
            s_target_freq = 65.0f + low_ratio * 160.0f;
        } else {
            s_target_freq = 110.0f + speed_ratio * 260.0f;
        }
        if (is_offroad && p->speed > 5.0f) {
            s_target_freq += sinf(GetTime() * 40.0f) * 20.0f;
        }
        s_engine_volume = 0.25f + speed_ratio * 0.30f;
        SetAudioStreamVolume(as->music_stream, 0.55f);
    }

    // Smooth frequency interpolation
    s_current_freq += (s_target_freq - s_current_freq) * 0.15f;

    // Stream engine audio buffer generation
    while (IsAudioStreamProcessed(as->engine_stream)) {
        short buffer[STREAM_BUFFER_SIZE];
        float phase_inc = (2.0f * PI * s_current_freq) / (float)AUDIO_SAMPLE_RATE;

        for (int i = 0; i < STREAM_BUFFER_SIZE; i++) {
            if (s_current_freq < 20.0f || s_engine_volume <= 0.01f) {
                buffer[i] = 0;
            } else {
                float saw = (s_phase / PI) - 1.0f;
                float sub = sinf(s_phase * 0.5f);
                float sample = (saw * 0.6f + sub * 0.4f);
                buffer[i] = (short)(sample * s_engine_volume * 12000.0f);

                s_phase += phase_inc;
                if (s_phase >= 2.0f * PI) {
                    s_phase -= 2.0f * PI;
                }
            }
        }
        UpdateAudioStream(as->engine_stream, buffer, STREAM_BUFFER_SIZE);
    }

    // Stream 80s Synth BGM audio buffer generation
    while (IsAudioStreamProcessed(as->music_stream)) {
        short buffer[STREAM_BUFFER_SIZE];
        Fill80sSynthBuffer(buffer, STREAM_BUFFER_SIZE);
        UpdateAudioStream(as->music_stream, buffer, STREAM_BUFFER_SIZE);
    }
}

void Audio_Cleanup(AudioSystem *as) {
    if (!as->initialized) return;

    StopAudioStream(as->engine_stream);
    UnloadAudioStream(as->engine_stream);

    StopAudioStream(as->music_stream);
    UnloadAudioStream(as->music_stream);

    UnloadSound(as->snd_skid);
    UnloadSound(as->snd_crash);
    UnloadSound(as->snd_fuel);
    UnloadSound(as->snd_checkpoint);
    UnloadSound(as->snd_gear);

    CloseAudioDevice();
    as->initialized = false;
}
