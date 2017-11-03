#include "EMUSWITCH.h"

#include "PALESDL.H"
#include "PALESDL_VID.H"
#include "PALE_FLTKGUI.H"
#include "PALEDISK.H"
#include "PALESDL_IO.H"

void initmem128k()
{
   z80_init_memmap();
  // z80_map_fetch(0x0000, 0xFFFF,&bank0[0]);
//   z80_map_read(0x0000, 0xFFFF,&bank0[0]);
//   z80_map_write(0x0000, 0xFFFF,&bank1[0]);

	z80_map_read(0x0000, 0x5FFF, &bank0[0x0000]); // rom 
	z80_map_fetch(0x0000, 0x5FFF, &bank0[0x0000]);


	z80_map_read(0x6000, 0xFFFF, &bank1[0x6000]); // ram 
	z80_map_fetch(0x6000, 0xFFFF, &bank1[0x6000]);
//	z80_map_write(0x0000, 0xFFFF, &bank1[0x0000]);
	z80_map_write(0x0000, 0xdFFF, &bank1[0x0000]);
	z80_map_write(0xe000, 0xFFFF, &bank1[0x0000]);

  z80_set_in(&inp);
  z80_set_out(&outp);
  z80_end_memmap();
}




void bank_latch128k(unsigned char value)
{
    bank_latch=value;
	if(debug==1)printf("Bank Switch %2x",value);


	if ((value & 0x40)==0x40)
	{	//VID banks write
 		z80_map_write(0x0000, 0xFFFF, &bank2[0x0000]); 
   	}

	if ((value & 0x80)==0)
	{  //USER RAM WRITE
		//NOTE - SHould really be a test for if the disk rom is enabled
		//if so then no writes to E000-FFFF
   		z80_map_write(0x0000, 0xFFFF, &bank1[0x0000]); 
   	}

	if ((value & 0x10)==0x10)
	{  //BANK4 RAM WRITE
		z80_map_write(0x0000, 0xFFFF, &bank4[0x0000]); 
   	}


// --------------------------------------------------------------------


	if ((value & 0x06)==0x00)			//READ 1 USER ram PURE
		{z80_map_read(0x0000, 0xFFFF, &bank1[0x0000]); z80_map_fetch(0x0000, 0xFFFF, &bank1[0x0000]); }

else
	if ((value & 0x06)==0x02)			//READ 1 USER ram priority over Bank2
		{z80_map_read(0x0000, 0xFFFF, &bank2[0x0000]); z80_map_fetch(0x0000, 0xFFFF, &bank1[0x0000]); }

//	else
//	 if ((value & 0x06)== 0x04)			//NO READ
//		{}			//z80_map_read(0x0000, 0xFFFF, &bank4[0x0000]); z80_map_fetch(0x0000, 0xFFFF, &bank2[0x0000]); }
	else
	 if ((value & 0x06)==0x06)			//READ 2 VID ram PURE
		{z80_map_read(0x0000, 0xFFFF, &bank2[0x0000]); z80_map_fetch(0x0000, 0xFFFF, &bank2[0x0000]); }
	else
	 if ((value & 0x01)==0x01)			//READ 4 PURE
		{z80_map_read(0x0000, 0xFFFF, &bank4[0x0000]); z80_map_fetch(0x0000, 0xFFFF, &bank4[0x0000]); }


	 //ROM COMES LAST SO IT TAKES PRIORITY OVER THE OTHERS

	if ((value & 0x08)==0)
    {
		z80_map_read(0x0000, 0x5FFF, &bank0[0x0000]); // rom 
		z80_map_fetch(0x0000, 0x5FFF, &bank0[0x0000]);
		if(bank0[0xe000]!=0xff) // Disk rom is special - can be turned off
		{
			if((disk_options & 0x10)==0x0)
			{
				z80_map_read(0xe000, 0xFFFF, &bank0[0xe000]); // disk rom 
		        z80_map_fetch(0xe000, 0xFFFF, &bank0[0xe000]);
			}
		}
	}


}



void vid_latch128k(UBYTE value){
 char lbl[100];

     video_latch=value;


//	if(debug==1)
//           	printf("Port &80 (%02X)\n", value);
//	if (value & 0x03) {
// Cassette/Speaker 
//       if(debug==1)
//            	printf("Cassette/Speaker accessed\n");
//	}
//	if (value & 0x04) {
//
//	}
//	if (value & 0x08) {
//
//	}
	if (value & 0x10)
	{
		show_alt_green=1;
	}
	else
	{
		show_alt_green=0;
	}
//	if (value & 0x20) {
//				// Video scanning disabled	
//	}
//	if (value & 0x40) {
//            	//Sync line blanking period
//	}	
//	if (value & 0x80) {
//		// Error (bit must be 0) 
//	}

}




void outp_128(UWORD port, UBYTE value){
    port = port & 0xFF;
	switch (port) {
		case 0x82:
			bank_latch128k(value);
			break;
		case 0x80:
			vid_latch128k(value);
			break;
		default:
			if(debug==1)
				printf("Trying to out to port %02X (%02X)\n",port,value);
	}
}

Uint8 getpixel_128(Uint16 start_offset, Uint8 bit)
{
	Uint8 red=0, blue=0, green=0,b1,b2,b3;
	Uint8 colour = 0;

	//Important for CPM
	start_offset=start_offset%0x4000;
/*
	if (start_offset>0x3fff)
	{
		b1=bank2[0x0000+start_offset];  //red mirror
		b2=bank2[0x4000+start_offset];  //blue mirror
	}
	else
*/
	{
		b1=bank2[0x4000+start_offset];  //red
		b2=bank2[0x0000+start_offset]; //blue
	}
//	if (start_offset>0x3fff)
	{
		if(show_alt_green==1)
			b3=bank2[0xc000+start_offset];	//alt green mirror
		else
			b3=bank2[0x8000+start_offset];  //green mirror
	}
//	else
//	{
//		if(show_alt_green==1)
	//		b3=bank3[0xa000+start_offset]; //alt green
//		else
//			b3=bank3[0xc000+start_offset];	//green
//	}
//			blue 	= (((bank2[0x0000+offset]) >> bit) & 0x01) ;
	//		red 	 = (((bank2[0x2000+offset]) >> bit) & 0x01) << 1;
			blue= ((b1>> bit) & 0x01) ;
			red= ((b2>> bit) & 0x01)<<1 ;
			green= ((b3>> bit) & 0x01)<<2 ;
//			green	= (((bank3[0x2000+offset]) >> bit) & 0x01) << 2;
//	}
	colour = green + red + blue;
	return(colour);
}


void update_keyrate128K()
{
		static	unsigned int new_keyval,looper=0;
		if(bank2[0]!=0)	//Check for CPM running, if so dont bother with the keyrate
		{
			if(emu_speed >2 )	//stop keyrepeat for faster emus
			{
						new_keyval=0x0c00*(emu_speed-1);
						bank1[0x6233]=new_keyval % 256;
						bank1[0x6234]=new_keyval / 256;
			}
			else
			{
						new_keyval=0x0c00;
						bank1[0x6233]=new_keyval % 256;
						bank1[0x6234]=new_keyval / 256;
			}
		}
}
