#ifndef __PALE48K_H__
#define __PALE48K_H__

void initmem48k();
void update_vid_maps48k();
void outp_48(UWORD port, UBYTE value);
Uint8 getpixel_48(Uint16 offset, Uint8 bit);
void update_keyrate48K();
void bank_latch48k(unsigned char value);
void vid_latch48k(unsigned char value);

#endif
