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

  case 0xD: {
    uint8_t N = cpu->opcode & 0xF;
    uint8_t x_coord = cpu->V[x];
    uint8_t y_coord = cpu->V[y];

    cpu->V[0xF] = 0x0;
    for (int i = 0; i < N; ++i) {
      uint8_t sprite = cpu->memory[cpu->I + i];
      uint8_t draw_y = (y_coord + i) % 32;

      for (int bit_idx = 0; bit_idx < 8; ++bit_idx) {
        uint8_t sprite_pixel = (sprite >> (7 - bit_idx)) & 0x1;
        uint8_t draw_x = (x_coord + bit_idx) % 64;

        // access the byte index within the gfx array since it is 256 bytes = 2048 bits (64 * 32 pixels)
        // mask the bit position within this gfx[byte_idx]
        int pixel_idx = draw_x + (draw_y * 64);
        uint8_t byte_idx = pixel_idx / 8;
        uint8_t bit_pos = pixel_idx % 8;

        uint8_t cur_byte = cpu->gfx[byte_idx];
        uint8_t this_pixel = (cur_byte >> (7 - bit_pos)) & 0x1;

        if (sprite_pixel && this_pixel) {
          cpu->V[0xF] = 0x1;
        }
        // shift sprite pixel back to original position
        cpu->gfx[byte_idx] ^= sprite_pixel << (7 - bit_pos);
      }
    }

    cpu->pc += 2;
    break;
  }

  case 0xE:
    m = cpu->opcode & 0x000F;

    switch (m) {
    case 0xE: {
      uint8_t key = cpu->V[x];
      if (cpu->keys[key] == TRUE) {
        cpu->pc += 2;
      }
      break;
    }

    case 0x1: {
      uint8_t key = cpu->V[x];
      if (cpu->keys[key] == FALSE) {
        cpu->pc += 2;
      }
      break;
    }
    }
    cpu->pc += 2;
    break;

  // FX07	Store the current value of the delay timer in register VX
  // FX0A	Wait for a keypress and store the result in register VX
  // FX15	Set the delay timer to the value of register VX
  // FX18	Set the sound timer to the value of register VX
  // FX1E	Add the value stored in register VX to register I
  // FX29	Set I to the memory address of the sprite data corresponding to the hexadecimal digit stored in register
  // VX
  // FX33	Store the binary-coded decimal equivalent of the value stored in register VX at addresses I, I + 1, and
  // I + 2
  case 0xF:
    m = cpu->opcode & 0x00FF;

    switch (m) {
    case 0x07:
      cpu->V[x] = cpu->delay_timer;
      break;

    case 0x0A: {
      uint8_t was_key_pressed = FALSE;
      for (int i = 0; i < 16; ++i) {
        if (cpu->keys[i] == TRUE) {
          cpu->V[x] = i;
          was_key_pressed = TRUE;
          break;
        }
      }

      if (!was_key_pressed) {
        return;
      }

      break;
    }

    case 0x15:
      cpu->delay_timer = cpu->V[x];
      break;

    case 0x18:
      cpu->sound_timer = cpu->V[x];
      break;

    case 0x1E:
      cpu->I += cpu->V[x];
      break;

    case 0x29:
      cpu->I = cpu->V[x] * 0x5;
      break;

    case 0x33: {
      uint8_t decimal_digit = cpu->V[x];
      for (int i = 0; i < 3; ++i) {
        uint8_t digit = decimal_digit % 10;
        cpu->memory[cpu->I + i] = digit;
        decimal_digit /= 10;
      }
      break;
    }

    // FX55	Store the values of registers V0 to VX inclusive in memory starting at address I
    // I is set to I + X + 1 after operation²
    case 0x55:
      for (int i = 0; i <= x; ++i) {
        cpu->memory[cpu->I + i] = cpu->V[i];
      }
      cpu->I += x + 1;
      break;

    // FX65	Fill registers V0 to VX inclusive with the values stored in memory starting at address I
    // I is set to I + X + 1 after operation²
    case 0x65:
      for (int i = 0; i <= x; ++i) {
        cpu->V[i] = cpu->memory[cpu->I + i];
      }

      cpu->I += x + 1;
      break;
    }

    cpu->pc += 2;
    break;
  }
}
