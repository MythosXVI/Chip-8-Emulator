#include "chip.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * Chip-8 interpreter by Mythos, a cooked Computer Engineering Student, Oct
 * 2026.
 */

/*
 * memory scheme:
 * 0x000 - 0x1FF chip 8 interpreter
 * 0x050 - 0x0A0 Font-set
 * 0x200 - 0xFFF Rom program
 */

/*
 * The only reason to make ch8 a struct would be if you later want
 * multiple independent CPUs, or you otherwise want to treat all
 * that state as one transferable object
 */

/*
 * making each memeber static means that main.c will not
 * be able to reach any of the state variables directly
 * and will only be able to interact with them using
 * the specified functions in ch8.h.
 *
 * Another benifit is when init() is called the variables will be created
 * and stay presistant and owned by ch8.c
 */

static unsigned char memory[4096];
static unsigned char v[16];

static unsigned short I;      /* Index register */
static unsigned short pc;     /* Program counter */
static unsigned short opcode; /* two byte opcode */

static unsigned char gfx[64 * 32]; /* Graphics */

unsigned char delay_timer; /* 60Hz, 60 opcodes per second */
unsigned char sound_timer;

static unsigned short stack[16]; /* Allows us to remember pc before jump */
static unsigned short sp;

unsigned char key[16] = {0};

static unsigned char drawFlag = 0;

static int new_key = -1;

unsigned char chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
    0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
    0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
    0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
    0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
    0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
    0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
    0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
    0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
    0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
    0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
    0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
    0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
    0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
    0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
    0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
};

void init(void) {
  int i;
  pc = 0x200;
  opcode = 0;
  I = 0;
  sp = 0;
  srand(((unsigned int)time(NULL)));

  for (i = 0; i < 80; i++)
    memory[i] = chip8_fontset[i];
}

/* TODO: load file using fopen */
void load_program(char *rom_name) {
  int i;
  long size;
  FILE *rom;
  rom = fopen(rom_name, "rb");
  if (rom == NULL) {
    fprintf(stderr, "Error: can't read rom");
    exit(1);
  }
  fseek(rom, 0, SEEK_END);
  size = ftell(rom);
  rewind(rom);

  if (size > (4096 - 0x200)) {
    fprintf(stderr, "Error: rom size is too large %ld", size);
    exit(1);
  }

  for (i = 0; i < size; i++)
    memory[512 + i] = fgetc(rom);
}

void latest_key(int key) {
  new_key = key;
}

void update_key(int k, int state) { key[k] = state ? 1 : 0; }

const unsigned char *get_gfx(void) { return gfx; }

int draw_flag() { return drawFlag; }

#define X ((opcode & 0x0F00) >> 8)
#define Y ((opcode & 0x00F0) >> 4)

void emulateCycle(void) {
  opcode = (memory[pc] << 8) | memory[pc + 1];
  /* Type-1 instructions */
  switch (opcode & 0xF000) {

  case 0x0000:
    /* Type-2 instructions */
    switch (opcode & 0x000F) {
    /* Type-2 instructions execution */
    case 0x0000: /* clr screen */
    {
      int i;
      for (i = 0; i < 2048; i++)
        gfx[i] = 0;
      pc += 2;
    } break;
    case 0x000E: /* return form subroutine */
      pc = stack[sp];
      sp--;
      break;
    default:
      printf("Unkwon opcode:0x%X\n", opcode);
      break;
    }
    break;
  /* Type-1 instructions execution */
  case 0x1000:
    pc = opcode & 0x0FFF;
    break;
  case 0x2000: /* 0x2NNN */
    ++sp;
    stack[sp] = pc + 2;
    pc = opcode & 0x0FFF;
    break;
  case 0x3000:
    if (v[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
      pc += 4;
    else
      pc += 2;
    break;
  case 0x4000:
    if (v[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
      pc += 4;
    else
      pc += 2;
    break;
  case 0x5000:
    if (v[(opcode & 0x0F00) >> 8] == v[(opcode & 0x00F0) >> 4])
      pc += 4;
    else
      pc += 2;
    break;
  case 0x6000:
    v[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
    pc += 2;
    break;
  case 0x7000:
    v[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
    pc += 2;
    break;
  case 0x8000:
    switch (opcode & 0x000F) {
    case 0x0000:
      v[(opcode & 0x0F00) >> 8] = v[(opcode & 0x00F0) >> 4];
      pc += 2;
      break;
    case 0x0001:
      v[(opcode & 0x0F00) >> 8] |= v[(opcode & 0x00F0) >> 4];
      pc += 2;
      break;
    case 0x0002:
      v[(opcode & 0x0F00) >> 8] &= v[(opcode & 0x00F0) >> 4];
      pc += 2;
      break;
    case 0x0003:
      v[(opcode & 0x0F00) >> 8] ^= v[(opcode & 0x00F0) >> 4];
      pc += 2;
      break;
    case 0x0004:
      if (v[(opcode & 0x00F0) >> 4] > (0xFF - v[(opcode & 0x0F00) >> 8]))
        v[0xF] = 1;
      else
        v[0xF] = 0;
      v[(opcode & 0x0F00) >> 8] += v[(opcode & 0x00F0) >> 4];
      pc += 2;
      break;
    case 0x0005:
      if (v[(opcode & 0x0F00) >> 8] < v[(opcode & 0x00F0) >> 4])
        v[0xF] = 0;
      else
        v[0xF] = 1;
      v[(opcode & 0x0F00) >> 8] -= v[(opcode & 0x00F0) >> 4];
      pc += 2;
      break;
    case 0x0006:
      v[0xF] = v[(opcode & 0x0F00) >> 8] & 0x01;
      v[(opcode & 0x0F00) >> 8] >>= 1;
      pc += 2;
      break;
    case 0x0007:
      v[0xF] = (v[Y] < v[X]) ? 0 : 1;
      v[(opcode & 0x0F00) >> 8] =
          v[(opcode & 0x00F0) >> 4] - v[(opcode & 0x0F00) >> 8];
      pc += 2;
      break;
    case 0x000E:
      v[0xF] = (v[X] & 0x80) >> 7;
      v[X] <<= 1;
      pc += 2;
      break;
    }
    break;
  case 0x9000:
    if (v[(opcode & 0x0F00) >> 8] != v[(opcode & 0x00F0) >> 4])
      pc += 4;
    else
      pc += 2;
    break;
  case 0xA000: /* ANNN */
    I = opcode & 0x0FFF;
    pc += 2;
    break;
  case 0xB000:
    pc = (opcode & 0x0FFF) + v[0x0];
    break;
  case 0xC000:
    v[X] = rand() & (opcode & 0x00FF);
    pc += 2;
    break;
  case 0xD000: /* Draw to screen */
  {
    unsigned short x = v[(opcode & 0x0F00) >> 8]; /* x = DXYN & 0100 */
    unsigned short y = v[(opcode & 0x00F0) >> 4]; /* y = DXYN & 0010 */
    unsigned short height =
        opcode & 0x000F; /* N = DXYN & 0001, number of rows */
    unsigned short pixel;
    int yline;
    int xline;
    v[0xF] = 0;
    for (yline = 0; yline < height; yline++) {
      pixel = memory[I + yline];
      for (xline = 0; xline < 8; xline++) {
        if ((pixel & (0x80 >> xline)) !=
            0) /* walks along the bits that make up the pixel */
        {
          if (gfx[x + xline + ((y + yline) * 64)] == 1)
            v[0xF] = 1;
          gfx[x + xline + ((y + yline) * 64)] ^= 1;
        }
      }
    }
    drawFlag = 1;
    pc += 2;
  } break;
  case 0xE000: /* Key input handling */
    switch (opcode & 0x00FF) {
    case 0x009E:
      if (key[v[(opcode & 0x0F00) >> 8]] != 0)
        pc += 4;
      else
        pc += 2;
      break;
    case 0x00A1:
      if (key[v[(opcode & 0x0F00) >> 8]] == 0)
        pc += 4;
      else
        pc += 2;
      break;
    }
    break;
  case 0xF000:
    switch (opcode & 0x00FF) {
    case 0x0007:
      v[X] = delay_timer;
      pc += 2;
      break;
    case 0x000A: {
      int i;
      for (i = 0; i < 16; i++) {
        if (key[i] != 0) {
          v[X] = i;
          pc += 2;
          break;
        }
      }
    } break;
    case 0x0015:
      delay_timer = v[X];
      pc += 2;
      break;
    case 0x0018:
      sound_timer = v[X];
      pc += 2;
      break;
    case 0x001e:
      I += v[X];
      pc += 2;
      break;
    case 0x0029:
      I = (v[X] & 0x0F) * 5;
      pc += 2;
      break;
    case 0x0030:
      I = v[X];
      pc += 2;
      break;
    case 0x0033:
      memory[I] = v[(opcode & 0x0F00) >> 8] / 100;
      memory[I + 1] = (v[(opcode & 0x0F00) >> 8] / 10) % 10;
      memory[I + 2] = (v[(opcode & 0x0F00) >> 8] % 100) % 10;
      pc += 2;
      break;
    case 0x0055: {
      int i;
      for (i = 0; i <= X; i++)
        memory[I + i] = v[i];
      /*I += X + 1;*/
      pc += 2;
    } break;
    case 0x0065: {
      int i;
      for (i = 0; i <= X; i++)
        v[i] = memory[I + i];
      /*I += X + 1;*/
      pc += 2;
      break;
    }
    }
    break;
  default:
    printf("Unkwon opcode:0x%X\n", opcode);
  }
}
