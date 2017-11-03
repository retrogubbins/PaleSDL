#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "EMUSWITCH.h"

#include <SDL/SDL.h>
#include "SDL/SDL_timer.h"
#include <fltk/Window.h>
#include <fltk/Widget.h>
#include <fltk/run.h>
#include <fltk/ask.h>
#include <fltk/file_chooser.h>

#include "PALESDL.H"
#include "PALESDL_VID.H"
#include "PALESDL_IO.H"
#include "PALESDL_GUI.H"
#include "PALE_FLTKGUI.H"
#include "PALETAPS.H"
#include "PALESND.H"
#include "PALESDL_CONFIG.H"
#include "PALEMEM.H"
#include "PALEROM.H"
#include "PALE_KEYS.H"
#include "PALERAWTAPE.H"
#include "PALEDISK.H"
#include "RAZE_HELPER.H"


#define PALE_REV 1.32


#ifdef WIN32
#include "PALEWIN32_HEAD.H"
#endif // WIN32 Routine to Position the main SDL window

#define DEBUG 0

//using namespace std;
using namespace fltk;

//MEMORY IO STUFF

UBYTE	 z80ports_in[0x10000];          /* I/O ports */
UBYTE 	 z80ports_out[0x10000];          /* I/O ports */
UBYTE    bank0[LYNX_MAXMEM];		  /* Rom */
UBYTE    bank1[LYNX_MAXMEM];		  /* User Ram */
UBYTE    bank2[LYNX_MAXMEM];          /* Red 0x0000/Blue 0x2000*/
UBYTE    bank3[LYNX_MAXMEM];    	  /* AltGreen 0x0000/Green 0x2000*/
UBYTE    bank4[LYNX_MAXMEM];


float emuspeeds[10]={25,50,100,200,400,800,1000,2000,5000,32000};
int emu_speed=2;
int hw_type=LYNX_HARDWARE_48;
int mc_type=0;
int ss_emu=0;
int run_emu=1;
int finish_emu=0;
int debug=0;
int SoundEnable=0;

int trap_pc=0xffff;
double	time_per_fps_disp;
double	last_fps_time;
int newtime;
int framecount;
Uint16 scanline;

long emu_cycles_scanline=256;
long emu_cycles_lineblank=88;
long emu_cycles_vblank=400;
long emu_cycles=13000;

unsigned int sound_cycles=0;
unsigned int tapecyc=0; 
SDL_TimerID sound_timer;
int trace=0;
FILE *tracedump_file;

float get_pale_rev()
{
	return((float)PALE_REV);
}

void start_tracedump()
{
		char lbl[50];
		tracedump_file=fopen("TRACEDUMP.TXT","wt");
        if(tracedump_file==NULL)
        {
                sprintf(lbl,"Cannot open PALE TraceDUMP File for writing");
				gui_error(lbl);
                return;
        }
}
void stop_tracedump()
{
	fclose(tracedump_file);
}

void tracedump(UWORD pc)
{
	char lbl[200];
	static int start=0;
  
	if(z80_get_reg(Z80_REG_PC)==0xe000)
		start=1;
	if(start==1)
	{
		get_statusregs(lbl);
		fprintf(tracedump_file,lbl);
		fprintf(tracedump_file,"\n");
	}
}

void bit(unsigned char *vari,unsigned char posit,unsigned char valu)
{
        if (valu==0)
                *vari=(*vari & (255-((unsigned char)1 << posit)));
        else
                *vari=(*vari | ((unsigned char)1 << posit));
}

void set_speed(int x)
{
	emu_speed=x;
	update_gui_speed(x);
	saveconfigfile();

	switch(hw_type)
	{
		case LYNX_HARDWARE_48:
			emu_cycles_scanline=(long)(emuspeeds[x]*256)/100; //256 = 64us Scanline Period
			emu_cycles_lineblank=(long)(emuspeeds[x]*88)/100; //88 = 22us LineBlank Period
			emu_cycles_vblank=(long)(emuspeeds[x]*160)/100;  //160= 40us Vblank(IRQ LOW) Period
			emu_cycles=(long)(emuspeeds[x]*13600)/100;  //13600 = 3.4ms Vblank period
			break;
		case LYNX_HARDWARE_96:
			emu_cycles_scanline=(long)(emuspeeds[x]*256)/100; //256 = 64us Scanline Period
			emu_cycles_lineblank=(long)(emuspeeds[x]*88)/100; //88 = 22us LineBlank Period
			emu_cycles_vblank=(long)(emuspeeds[x]*160)/100;  //160= 40us Vblank(IRQ LOW) Period
			emu_cycles=(long)(emuspeeds[x]*13600)/100;  //13600 = 3.4ms Vblank period
			break;
		case LYNX_HARDWARE_128:
			emu_cycles_scanline=(long)(emuspeeds[x]*384)/100; // = 64us Scanline Period
			emu_cycles_lineblank=(long)(emuspeeds[x]*132)/100; // = 22us LineBlank Period
			emu_cycles_vblank=(long)(emuspeeds[x]*240)/100;  //= 40us Vblank(IRQ LOW) Period
			emu_cycles=(long)(emuspeeds[x]*20400)/100;  // = 3.4ms Vblank period
			break;
	}
}

void init_fps()
{
	/* called once, during the init phase */
	time_per_fps_disp = CLOCKS_PER_SEC * .5; /* .5 seconds */
}

void single_step_emu()
{
	ss_emu=1;
}

void tick_fps()
{
	    /* called every frame */
	last_fps_time = newtime;
	newtime = clock();
	framecount++;
	if ((newtime-last_fps_time) > time_per_fps_disp)
	{
	    update_fps_counter( (int)((CLOCKS_PER_SEC * framecount) / (newtime -last_fps_time)));
	    framecount=0;
	}
}



void initialise_Lynx(void)
{
//gui_error("In Init Lynx");

 	memset(&bank0[0x0000], 0xFF, LYNX_MAXMEM);
 	memset(&bank1[0x0000], 0xFF, LYNX_MAXMEM);
   	memset(&bank2[0x0000], 0xFF, LYNX_MAXMEM);
   	memset(&bank3[0x0000], 0xFF, LYNX_MAXMEM);
   	memset(&bank4[0x0000], 0xFF, LYNX_MAXMEM);
	clearports();
	load_romset(mc_type);
	if(!load_lynx_rom())
	{
		gui_error("Couldn't open the working ROM");
		exit(1);
	}
//save_memdump();
	initmem();//Must come after ROMset load so we know what the hardware is
	set_t_mode(tape_mode);
	init_tapestuff();
	init_diskstuff();

   // z80_set_fetch_callback(&tracedump);
	z80_reset(); 

//gui_error("OUT OF Init Lynx");

}


void set_machine(int x)
{
	mc_type=x;
	update_gui_mtype(x);

	initialise_Lynx();
	saveconfigfile();

	set_speed(emu_speed);

}

const int UPDATE_SOUND = 1;


Uint32 SoundTimer(Uint32 interval, void* param)
{
    SDL_Event sound_event;
 // gui_error("HEllo");

    sound_event.type = SDL_USEREVENT;
    sound_event.user.code = UPDATE_SOUND;
    sound_event.user.data1 = 0;
    sound_event.user.data2 = 0;
    
    SDL_PushEvent(&sound_event);
    
    return interval;
}

  
// ------------------------------------------------------------------ 
#undef main
int main(int argc, char *argv[])
{
	FILE *handle;
	char *filenam;
	static char scan_lbl[10];
  	SDL_Event event;

	//MUST BE HERE before Initgui FOR WINXP to WORK
    initialise_display();//Must be here because loading COnfig file will reset the display size

	init_gui();
	init_ROMsets();
	if(!loadROMconfigfile())
	{
		saveROMconfigfile();
 		loadROMconfigfile();
	}

	#  ifdef WIN32
	#include "PALEWIN32_GET.H"
	#  endif

    initialise_display();//Must be here because loading COnfig file will reset the display size

	if(!loadconfigfile())
	{
		saveconfigfile();
		loadconfigfile();
	}

	init_sound();
	initialise_Lynx();
	init_fps();

	post_init_gui();



		
	scanline = 0;
 
	//START SOUND TIMER
//	sound_timer = SDL_AddTimer(40, SoundTimer, 0);

	gui_loop_more();	//Little dose of GUI to start with - get all the windows open etc.


//start_tracedump();
//z80_set_fetch_callback(void (*handler)(UWORD pc));


//z80_set_fetch_callback(&tracedump);


    while (finish_emu==0) 
    {	
		if(SDL_PollEvent(&event)==1)
		{
	        if(event.type==SDL_VIDEORESIZE)
	  			video_resize(event.resize.w,event.resize.h,1);	
//	        if(event.user.code==UPDATE_SOUND && SoundEnable)
//	  			flush_sound();	
		}
//if(z80_get_reg(Z80_REG_PC)==0x3ec1) run_emu=0;

//run_emu=0;
//ss_emu=1;

		if(run_emu==1)
	    {
			//to get at least 22khz sampling on sound 250*50fps ish chek!
			//NOTE!! This division will copmletely screw up the 
			//point of setting a definite noof cycles
			//so change this NOW!!!
			Line_Blank=0;
sound_cycles+=z80_emulate(emu_cycles_lineblank);//22 us Line Blanking Execution
sound_cycles+=z80_emulate(emu_cycles_scanline/2);//64 us Scanline execution
sound_cycles+=z80_emulate(emu_cycles_scanline/2);//64 us Scanline execution
		}
		else
		{
			if(ss_emu==1)
			{
		        z80_emulate(1);
//tracedump();
				ss_emu=0;
			}
//			else
//				gui_loop_more();			//GIVE SOME EXTRA TIME TO THE FLTK GUI
										//WHEN NOT RUNNING
										//Otherwise there are problems with Input Text Boxes
		}		
	
		// FRAME BLANKING PERIOD
		// FRAME BLANKING PERIOD
		// FRAME BLANKING PERIOD
		if(draw_scanline(scanline++)==0)	//draw returns zero if its end of frame
		{
			update_keyrate(); //Update the keyboard repetition and delay rates for diff emu speeds
		    CheckKeys();
			scanline = 0;


			if(emu_display!=DISPLAY_FULLSCREEN)
			{
				if(show_sysvars)update_sysvar();
				if(show_memmap)update_memmap();
				if(show_status)update_status();
				if (window3->visible())update_tape_monitor();
				if (wndMemScan->visible())update_memscan();
				gui_loop();
			}
			//tick_fps();

	        z80_raise_IRQ(0xFF);

			//Should be 160 T States with IRQ held low by CURSOR 6845 signal
			//if(run_emu==1)	sound_cycles+=z80_emulate(emu_cycles_vblank);

		    z80_lower_IRQ();

		   	if(run_emu==1)
			{
			//				for(int j=0;j<emu_cycles/360;j++)
			//				{
			//					sound_cycles+z80_emulate(emu_cycles/360);//main impact of speedup




sound_cycles+z80_emulate(emu_cycles);//main impact of speedup


			//EMU CYCELS is divide into 182 cycle blocks so that sound gets updated
			//tapecyc+=check_tape(emu_cycles/182);
			//				}
			}
		}
  	}
//	SDL_RemoveTimer(sound_timer);
//stop_tracedump();

	kill_disk(0);
	kill_disk(1);
	gui_end();
}

void stop_emu()
{
	run_emu=0;
}
void start_emu()
{
	run_emu=1;
}
void quit_emu()
{
	finish_emu=1;
}
void reset_emu()
{
	initialise_Lynx();

}






//        if(debug==1)
  //      	printf("pc (%04X) BSw %02X V Port %02X  HL %04X  AF %04X  BC %04X  DE %04X\n",z80_get_reg(Z80_REG_PC),bank_latch,video_latch,z80_get_reg(Z80_REG_HL),z80_get_reg(Z80_REG_AF),z80_get_reg(Z80_REG_BC),z80_get_reg(Z80_REG_DE)); 
