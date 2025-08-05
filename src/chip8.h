#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>

#define TRUE 1
#define FALSE 0

typedef struct CHIP8 {
    uint16_t opcode;
    uint8_t memory[4096];
    uint8_t gfx[256]; // 64 * 32 pixels - each pixel is 1 bit
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t stack[16];
    uint16_t sp;
    uint8_t keys[16];
} CHIP8_t;

void init(CHIP8_t *cpu);
void cycle(CHIP8_t *cpu);
#endif
