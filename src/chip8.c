#include "chip8.h"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include <_stdlib.h>
#include <stdint.h>
#include <stdlib.h>

unsigned char chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void init(CHIP8_t *cpu) {
  srand(time(NULL));
  cpu->pc = 0x200;
  cpu->opcode = 0;
  cpu->I = 0;
  cpu->sp = 0;
  cpu->delay_timer = 0;
  cpu->sound_timer = 0;

  for (int i = 0; i < sizeof(cpu->gfx); ++i) {
    cpu->gfx[i] = 0;
  }

  for (int i = 0; i < sizeof(cpu->memory); ++i) {
    cpu->memory[i] = 0;
  }

  for (int i = 0; i < sizeof(cpu->V); ++i) {
    cpu->V[i] = 0;
  }

  for (int i = 0; i < sizeof(cpu->stack); ++i) {
    cpu->stack[i] = 0;
  }

  for (int i = 0; i < sizeof(cpu->keys); ++i) {
    cpu->keys[i] = 0;
  }

  for (int i = 0; i < sizeof(chip8_fontset); ++i) {
    cpu->memory[i] = chip8_fontset[i];
  }
}

void cycle(CHIP8_t *cpu) {
  /*
  opcode is a 16 bit (2 byte) value but CHIP8 reads memory in 8 bits (1 byte) chunks
  shifts first byte to leftmost (upper) position then moves pc to next byte and bitwise-or to combine to 16 bit value
  */
  cpu->opcode = cpu->memory[cpu->pc] << 8 | cpu->memory[cpu->pc + 1];

  printf("%.4X", cpu->opcode);

  uint8_t first = cpu->opcode >> 12; // get the opcode category

  uint8_t x = (cpu->opcode & 0x0F00) >> 8;
  uint8_t y = (cpu->opcode & 0x00F0) >> 4;
  uint8_t m;

  switch (first) {
  case 0x0:
    if (cpu->opcode == 0x00E0) {
      for (int i = 0; i < sizeof(cpu->gfx); ++i) {
        cpu->gfx[i] = 0;
      }
    } else if (cpu->opcode == 0x00EE) {
      cpu->sp--;
      cpu->pc = cpu->stack[cpu->sp];
    }
    cpu->pc += 2;
    break;

  case 0x1:
    cpu->pc = cpu->opcode & 0x0FFF;
    break;

  case 0x2:
    cpu->stack[cpu->sp] = cpu->pc;
    cpu->sp++;
    cpu->pc = cpu->opcode & 0x0FFF;
    break;

  case 0x3: {
    if (cpu->V[x] == (cpu->opcode & 0x00FF)) {
      cpu->pc += 2;
    }
    cpu->pc += 2;
    break;
  }

  case 0x4: {
    if (cpu->V[x] != (cpu->opcode & 0x00FF)) {
      cpu->pc += 2;
    }
    cpu->pc += 2;
    break;
  }

  case 0x5: {
    if (cpu->V[x] == cpu->V[y]) {
      cpu->pc += 2;
    }
    cpu->pc += 2;
    break;
  }

  case 0x6: {
    cpu->V[x] = cpu->opcode & 0x00FF;
    cpu->pc += 2;
    break;
  }

  case 0x7: {
    cpu->V[x] += cpu->opcode & 0x00FF;
    cpu->pc += 2;
    break;
  }

  case 0x8:
    m = cpu->opcode & 0x000F;

    switch (m) {
    case 0x0:
      cpu->V[x] = cpu->V[y];
      break;

    case 0x1:
      cpu->V[x] |= cpu->V[y];
      break;

    case 0x2:
      cpu->V[x] &= cpu->V[y];
      break;

    case 0x3:
      cpu->V[x] ^= cpu->V[y];
      break;

    case 0x4: {
      uint16_t sum = (uint16_t)cpu->V[x] + (uint16_t)cpu->V[y];
      if (sum > 0xFF) {
        cpu->V[0xF] = 0x1;
      } else {
        cpu->V[0xF] = 0x0;
      }
      cpu->V[x] = sum & 0xFF;
      break;
    }

    case 0x5:
      if (cpu->V[x] < cpu->V[y]) {
        cpu->V[0xF] = 0x0;
      } else {
        cpu->V[0xF] = 0x1;
      }
      cpu->V[x] -= cpu->V[y];
      break;

    case 0x6:
      cpu->V[0xF] = cpu->V[y] & 1;
      cpu->V[x] = cpu->V[y] >> 1;
      break;

    case 0x7:
      if (cpu->V[y] < cpu->V[x]) {
        cpu->V[0xF] = 0x0;
      } else {
        cpu->V[0xF] = 0x1;
      }
      cpu->V[x] = cpu->V[y] - cpu->V[x];
      break;

    case 0xE:
      cpu->V[0xF] = (cpu->V[y] & 0x80) >> 7;
      cpu->V[x] = cpu->V[y] << 1;
      break;
    }

    cpu->pc += 2;
    break;

  case 0x9:
    if (cpu->V[x] != cpu->V[y]) {
      cpu->pc += 2;
    }
    cpu->pc += 2;
    break;

  case 0xA:
    cpu->I = cpu->opcode & 0x0FFF;
    cpu->pc += 2;
    break;

  case 0xB:
    cpu->pc = (cpu->opcode & 0x0FFF) + cpu->V[0x0];
    break;

  case 0xC: {
    uint8_t rand = (uint8_t)arc4random_uniform(256);
    cpu->V[x] = rand & (cpu->opcode & 0x00FF);
    cpu->pc += 2;
    break;
  }

  // TODO DXYN Draw a sprite at position VX, VY with N bytes of sprite data starting at the address stored in I
  // Set VF to 01 if any set pixels are changed to unset, and 00 otherwise
  case 0xD:

    break;
  }
}
