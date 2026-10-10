#include "chip.h"

#define bool unsigned char
#include "raylib.h"
/*
* raylib was compiled using C99 and bool there is 1 byte
* in C89 bool is an int(4 byte) enum, so we must define what
* bool is so we can read the first byte only.
*
* Wouldn't recommend doing that again, use raylib with
* C99.
*/
#include <stdio.h>
#include <stdlib.h>

#define CYCLE_PER_FRAME 10

void keyup(unsigned char k);
void keydwn(unsigned char k);

int main(int argc, char *argv[]) {

  double now, last_tick;
  int pixel_height = 10, pixel_width = 10, i, j;
  const unsigned char *graphics;
  unsigned char k = 0;

  if (argc != 2) {
    printf("usage: %s rom", *argv);
    exit(1);
  }

  init();
  load_program(argv[1]);
  printf("ROM:%s ", argv[1]);

  InitWindow(640, 320, argv[1]);
  SetTargetFPS(120);

  last_tick = GetTime();

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    /*now = GetTime();*/

    /*while(now - last_tick >= 1.0 / 60.0)
    {
        if(delay_timer > 0)
        if(sound_timer > 0)
            sound_timer--;
        last_tick += 1.0/60.0;
    }
     */
    if (delay_timer > 0)
      delay_timer--;
    if (IsKeyDown(KEY_ONE)) {
      key[0x1] = 1;
      latest_key(0x1);
    } else {
      key[0x1] = 0;
    }
    if (IsKeyDown(KEY_TWO)) {
      key[0x2] = 1;
      latest_key(0x2);
    } else {
      key[0x2] = 0;
    }
    if (IsKeyDown(KEY_THREE)) {
      key[0x3] = 1;
      latest_key(0x3);
    } else {
      key[0x3] = 0;
    }
    if (IsKeyDown(KEY_FOUR)) {
      key[0xC] = 1;
      latest_key(0xC);
    } else {
      key[0xC] = 0;
    }
    if (IsKeyDown(KEY_Q)) {
      key[0x4] = 1;
      latest_key(0x4);
    } else {
      key[0x4] = 0;
    }
    if (IsKeyDown(KEY_W)) {
      key[0x5] = 1;
      latest_key(0x5);
    } else {
      key[0x5] = 0;
    }
    if (IsKeyDown(KEY_E)) {
      key[0x6] = 1;
      latest_key(0x6);
    } else {
      key[0x6] = 0;
    }
    if (IsKeyDown(KEY_R)) {
      key[0xD] = 1;
      latest_key(0xD);
    } else {
      key[0xD] = 0;
    }
    if (IsKeyDown(KEY_A)) {
      key[0x7] = 1;
      latest_key(0x7);
    } else {
      key[0x7] = 0;
    }
    if (IsKeyDown(KEY_S)) {
      key[0x8] = 1;
      latest_key(0x8);
    } else {
      key[0x8] = 0;
    }
    if (IsKeyDown(KEY_D)) {
      key[0x9] = 1;
      latest_key(0x9);
    } else {
      key[0x9] = 0;
    }
    if (IsKeyDown(KEY_F)) {
      key[0xE] = 1;
      latest_key(0xE);
    } else {
      key[0xE] = 0;
    }
    if (IsKeyDown(KEY_Z)) {
      key[0xA] = 1;
      latest_key(0xA);
    } else {
      key[0xA] = 0;
    }
    if (IsKeyDown(KEY_X)) {
      key[0x0] = 1;
      latest_key(0x0);
    } else {
      key[0x0] = 0;
    }
    if (IsKeyDown(KEY_C)) {
      key[0xB] = 1;
      latest_key(0xB);
    } else {
      key[0xB] = 0;
    }
    if (IsKeyDown(KEY_V)) {
      key[0xF] = 1;
      latest_key(0xF);
    } else {
      key[0xF] = 0;
    }

    for(i = 0; i < CYCLE_PER_FRAME; i++){
        emulateCycle();
    }

    if (draw_flag()) {
      graphics = get_gfx();
      for (i = 0; i < 320; i += 10)
        for (j = 0; j < 640; j += 10) {
          if (*graphics++)
            DrawRectangle(j, i, pixel_width, pixel_height, WHITE);
          else
            DrawRectangle(j, i, pixel_width, pixel_height, BLACK);
        }
    }

    EndDrawing();
  }
  CloseWindow();
}
