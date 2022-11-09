#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <SDL_MAINLOOP.h>
#include "buzzer.h"

#define HERTZ       600
#define STACKSIZE   64
#define MEMORY_SIZE 4096
#define CH8_W       64
#define CH8_H       32

uint8_t memory[MEMORY_SIZE];
bool display[CH8_W*CH8_H];

uint16_t PC;
uint16_t I;

uint8_t V[16];
uint8_t ST;
uint8_t DT;

uint16_t stack[STACKSIZE];
size_t stackIndex;

bool keyState[16];
bool prev_keyState[16];

void initChip8();
void loadROM(const char*);
void RunOpcode();
void cpuTimers();
void emulate();

void pushStack(int);
int popStack();

void setChip8Key(keyboard);
void unsetChip8Key(keyboard);

#endif