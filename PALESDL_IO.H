#ifndef __PALESDL_IO_H__
#define __PALESDL_IO_H__

#include <SDL/SDL.h>
#include <SDL/SDL_keyboard.h>
#include <SDL/SDL_keysym.h>


UBYTE inp(UWORD port);
void outp(UWORD port, UBYTE value);
void clearports();
//MOVE
extern unsigned char sound_port;
extern UBYTE video_latch,bank_latch;
extern Uint8 Line_Blank;
extern Uint16 scanline;
extern Uint8 show_bank1,show_bank2,show_bank3,show_alt_green,speaker_enable;
void update_vid_bank_latches();

#endif 
