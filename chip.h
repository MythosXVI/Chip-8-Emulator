#ifndef CHIP_H
#define CHIP_H

void init(void);
void load_program(char *rom_name);
void latest_key(int key);
void emulateCycle(void);
void update_key(int k, int state);
const unsigned char *get_gfx(void);
int draw_flag();

/*Timers, I don't wanna access a funciton every 1/60 of a second*/
extern unsigned char delay_timer;
extern unsigned char sound_timer;
extern unsigned char key[16];


#endif
