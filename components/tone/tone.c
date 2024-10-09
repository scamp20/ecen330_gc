#include "tone.h"
#include <stdlib.h>
#include <math.h>

// This component is a thin layer around the sound component.
// One cycle of a waveform is generated and then given to the
// sound component to play out cyclically until told to stop.
// Macros are provided for tone functions that are aliases
// of sound functions.

#define CENTER 0x80
#define AMPLITUDE 127
#define MAX_HEIGHT 255
#define QUARTER .25f
#define THREE_QUARTERS .75f

uint8_t* toneWaveforms = NULL;
uint32_t sampleRate;

// Initialize the tone driver. Must be called before using.
// May be called again to change sample rate.
// sample_hz: sample rate in Hz to playback tone.
// Return zero if successful, or non-zero otherwise.
int32_t tone_init(uint32_t sample_hz) {
    if (sample_hz < LOWEST_FREQ * 2) return -1;
    if (sound_init(sample_hz) != 0) return -1;
    sampleRate = sample_hz;
    toneWaveforms = (uint8_t*) malloc(sample_hz / LOWEST_FREQ);
    if (!toneWaveforms) return -1;

    return 0;
}

// Free resources used for tone generation (DAC, etc.).
// Return zero if successful, or non-zero otherwise.
int32_t tone_deinit(void) {
    sampleRate = 0;
    free(toneWaveforms);
    sound_deinit();
    return 0;
}

// Start playing the specified tone.
// tone: one of the enumerated tone types.
// freq: frequency of the tone in Hz.
void tone_start(tone_t tone, uint32_t freq) {
    if (!(tone >= SINE_T && tone < LAST_T)) {
        return;
    }
    uint32_t samples = sampleRate / freq;

    float slope;
    switch(tone) {
        case SINE_T:
            for (int i = 0; i < samples; i++) {
                toneWaveforms[i] = CENTER+(AMPLITUDE*sinf(((float)i/(float)samples)*2*M_PI));
            }
            break;
        case SQUARE_T:
            for (int i = 0; i < samples; i++) {
                if (i < samples/2) toneWaveforms[i] = MAX_HEIGHT;
                else toneWaveforms[i] = 0;
            }
            break;
        case TRIANGLE_T:
            slope = (float)MAX_HEIGHT/((float)samples/2);
            for (int i = 0; i < samples; i++) {
                if (i < samples*QUARTER) {
                    toneWaveforms[i] = slope*i + CENTER;
                } else if (i < samples*THREE_QUARTERS) {
                    toneWaveforms[i] = (-1)*slope*i + MAX_HEIGHT + AMPLITUDE;
                } else {
                        toneWaveforms[i] = slope*i - MAX_HEIGHT - AMPLITUDE;
                }
            }
            break;
        case SAW_T:
            slope = (float)MAX_HEIGHT/((float)samples);
            for (int i = 0; i < samples; i++) {
                if (i < samples/2) {
                    toneWaveforms[i] = slope*i + CENTER;
                } else {
                    toneWaveforms[i] = slope*i - AMPLITUDE;
                }
            }
            break;
        default:
            break;
    }
    sound_cyclic(toneWaveforms, samples);
}
