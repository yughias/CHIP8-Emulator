#include "buzzer.h"

SDL_AudioDeviceID audio_device;
SDL_AudioSpec audio_spec;

void initBuzzer(){
    SDL_zero(audio_spec);
    audio_spec.freq = 44100;
    audio_spec.format = AUDIO_U8;
    audio_spec.channels = 1;
    audio_spec.samples = 1024;
    audio_spec.callback = NULL;

    audio_device = SDL_OpenAudioDevice(
        NULL, 0, &audio_spec, NULL, 0);

    SDL_PauseAudioDevice(audio_device, 0);
}

void closeBuzzer(){
    SDL_CloseAudioDevice(audio_device);
}

void playBeep(){
    const int PERIOD = 500;
    const size_t sample_size = audio_spec.freq/frameRate;
    uint8_t sample[sample_size];
    
    for(int i = 0; i < sample_size; i++)
        sample[i] = (i % PERIOD < PERIOD/2)? 255 : 0;

    SDL_QueueAudio(audio_device, sample, sample_size);
}