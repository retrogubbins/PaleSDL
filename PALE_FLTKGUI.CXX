#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <fltk/run.h>

#include "EMUSWITCH.h"
#include "PALESDL_GUI.H"
#include "PALESDL_IO.H"
#include "PALESDL_CONFIG.H"
#include "PALESDL.H"
#include "RAZE_HELPER.H"
#include "PALESDL_VID.H"
#include "PALETAPS.H"
#include "PALERAWTAPE.H"
#include "PALESND.H"
#include "PALEMEM.H"
#include "PALEDISK.H"
#include <fltk/events.h>

#define COLOR_RED 0xff323a00
#define COLOR_GREEN 0x2dff1400
#define COLOR_BLUE 0x62e3ff00
#define COLOR_WHITE 0xffffff00

#define TAPE_COUNTER_DIVISOR 1000

unsigned int memscan_bno=0;
unsigned int memscan_start=0;
unsigned int memmap_bankno=0;

int show_status=0;
int show_memmap=0;
int show_disas=1;
int show_sysvars=0;
int disas_bank=0;
int mach=0;//For Rombuilder
int emu_display=0; //0 = 1x  1=2x  2=3x
unsigned char monitor_pic[256*128];


//This is here supposed to stop the ESC key from closing the forms
//But at moment it dont compile
//static int my_handler(int event)
//{
//	  if (event == SHORTCUT) return 1; // eat all shortcut keys
//	  return 0;
//}

//void my_callback(&mainwind, void*) {
//  if (Fl::event()==FL_SHORTCUT && Fl::event_key()==FL_Escape) 
//    return; // ignore Escape
//  exit(0);
//}


void set_display(int x)
{
	emu_display=x;
	switch(emu_display)
	{
		case DISPLAY_CUSTOM:		//custom
			StretchEnable=1;
			video_resize(512,512,1);
			break;
		case DISPLAY_1X:		// 1x
			StretchEnable=0;
			set_screenres();	//return to inital 1X display (actually this is a 2x default)
			break;
		case DISPLAY_2X:
			StretchEnable=1;
			video_resize(1024,1024,0);
			break;
		case DISPLAY_FULLSCREEN:	///Not used at mo due to FLTK / SDL crashes - use F7f8
			emu_display=DISPLAY_1X;
			StretchEnable=0;	//IF loaded from a config file then ignore it for now
			set_screenres();	//return to inital 1X display (actually this is a 2x default)
			break;
	}
	chcDisplay->value(emu_display);
	saveconfigfile();
}


void get_revisions()
{
	char lbl[100];
	sprintf(lbl,"Rev: %.2f",get_pale_rev());
	txtoRevisions->value(lbl);
}


// For some reason I cant update the title with a sprintf(lbl)
void set_memmap_bankno(int x)
{
	memmap_bankno=x;
	switch(x)
	{
		case 0:
			window2->label("Bank 0");
			break;
		case 1:
			window2->label("Bank 1");
			break;
		case 2:
			window2->label("Bank 2");
			break;
		case 3:
			window2->label("Bank 3");
			break;
		case 4:
			window2->label("Bank 4");
			break;
	}
	saveconfigfile();
}

Window *window2;
Window *window3;

fltk::Button* butMemMapBankPlus2;
fltk::Button* butMemMapBankMinus2;

static void cb_butMemMapBankMinus2(fltk::Button*, void*) {
  if(memmap_bankno>0)memmap_bankno--;
	set_memmap_bankno(memmap_bankno);
}


static void cb_butMemMapBankPlus2(fltk::Button*, void*) {
  if(memmap_bankno<4)memmap_bankno++;
	set_memmap_bankno(memmap_bankno);
}

static void cb_txtoBnk(fltk::Output*, void*) {
//  if(memmap_bankno<4)memmap_bankno++;

}

//Output* txtoMemBnk;
//MultilineOutput* txtoDisas;
//MultilineOutput* TDisas;
//fltk::Button* butMemMapfBankMinus2;
//fltk::Button* butMemMapfBankPlus2;

void init_gui()
{
	char lbl[30];
	// ------------------------------------------------------------------ 
	//	GUI STUFF
		mainwind=make_window();
	  	statuswind=make_window();
	  	wndROMbuilder=make_window();
//		wndMemScan=make_window();
//		wndAbout=make_window();

//fltk::add_handler(int (*)(int,fltk::Window*)).

//Window::add_handler(my_handler);	// Turn OFF Escape Key closing windows - but it dont compile :(((

		mainwind->position(550,30);//The main CONTROL PANEL
		get_revisions();	//Put in the PALE Rev versino number

		statuswind->position(10,580);
		wndROMbuilder->position(400,400);
		wndSysvars->position(628,540);

	  	mainwind->show();
#ifdef WIN32
		use_system_file_chooser(true);
#endif // WIN32 Override so we dont have to use FLTK's crappy chooser

	window2= new fltk::Window(200,330);
  	window2->begin();
		TiffWidget *memmapImage=new TiffWidget(0, 0, 128, 512);
		//txtoMemBnk = new fltk::Output(60, 20, 40, 25, "Bank:");
		{fltk::Button* o = butMemMapBankMinus2 = new fltk::Button(20, 5, 25, 25, "-");
	      o->callback((fltk::Callback*)cb_butMemMapBankMinus2);
	    }
		{fltk::Button* o = butMemMapBankPlus2 = new fltk::Button(45, 5, 25, 25, "+");
	      o->callback((fltk::Callback*)cb_butMemMapBankPlus2);
	    }

    {fltk::Button* o = new fltk::Button(5, 64, 40, 20, "2000");
      o->buttoncolor((fltk::Color)56);
      o->labelcolor((fltk::Color)0xffffff00);
    }
    {fltk::Button* o = new fltk::Button(5, 128, 40, 20, "4000");
      o->buttoncolor((fltk::Color)56);
      o->labelcolor((fltk::Color)0xffffff00);
    }
    {fltk::Button* o = new fltk::Button(5, 192, 40, 20, "6000");
      o->buttoncolor((fltk::Color)56);
      o->labelcolor((fltk::Color)0xffffff00);
    }
    {fltk::Button* o = new fltk::Button(5, 256, 40, 20, "8000");
      o->buttoncolor((fltk::Color)56);
      o->labelcolor((fltk::Color)0xffffff00);
    }
    {fltk::Button* o = new fltk::Button(5, 320, 40, 20, "A000");
      o->buttoncolor((fltk::Color)56);
      o->labelcolor((fltk::Color)0xffffff00);
    }
    {fltk::Button* o = new fltk::Button(5, 384, 40, 20, "C000");
      o->buttoncolor((fltk::Color)56);
      o->labelcolor((fltk::Color)0xffffff00);
    }
    {fltk::Button* o = new fltk::Button(5, 448, 40, 20, "E000");
      o->buttoncolor((fltk::Color)56);
      o->labelcolor((fltk::Color)0xffffff00);
    }

	window2->end();

	window2->resize(898,500,128,512);//Memory Map
	window2->label("Bank 0");



	window3= new fltk::Window(256,128);	//TAPE Monitor Window
  	window3->begin();
		MonitorWidget *TapeMonitorImage=new MonitorWidget(0, 0, 256, 128);
	window3->end();
	window3->label("Tape Monitor");

	//txtoMemBnk->value("66");

/*
	window3= new fltk::Window(200,330);
  	window3->begin();
    {fltk::MultilineOutput* o = TDisas = new fltk::MultilineOutput(0, 5, 130, 470, " ");
      o->type(4);
      o->set_vertical();
	}
	window3->end();
window3->show();
*/

//	disaswind=make_window2();

	// ------------------------------------------------------------------ 
}

char* itoa_mine( int val, char* result, int base );

void update_tape_gui()
{
	char lbl[20];	
	txtoTapeCounter->value(itoa_mine(raw_position,lbl,10));
	if(raw_motor)
		txtoMotor->value("ON");
	else
		txtoMotor->value("OFF");
	if(tape_override==FORCE_PLAY)
        txtoMotor->color((fltk::Color)COLOR_GREEN);//Green - Motor Enabled
	else if(tape_override==FORCE_STOP)
      	txtoMotor->color((fltk::Color)COLOR_WHITE);//WHITE	- Motor Stopped by PAUSE button
	else if(tape_override==FORCE_RECORD)
      	txtoMotor->color((fltk::Color)COLOR_RED);//RED - Record Mode
	else
        txtoMotor->color((fltk::Color)COLOR_BLUE);//Blue	- Auto Enabled (uses Lynx motor output)
	grpTapeDirect->redraw();
	sldTape->value(raw_position/TAPE_COUNTER_DIVISOR);

}



void update_disk_gui()
{
	char lbl[20];	
	txtoDcommand->value(itoa_mine(disk_comreg,lbl,16));
	txtoDdata->value(itoa_mine(disk_datareg,lbl,16));
	txtoDstatus->value(itoa_mine(disk_statusreg,lbl,16));
	txtoDoptions->value(itoa_mine(disk_options,lbl,16));
	if(disk_rom_enabled==1)
		txtoDepromenable->value("Enabled");
	else
		txtoDepromenable->value("Disabled");
	txtoDmotor->value(itoa_mine(disk_motor,lbl,10));
	txtoDprecomp->value(itoa_mine(disk_precomp,lbl,10));
	if(disk_step125==1)
		txtoDsteprate->value("125ns");
	else
		txtoDsteprate->value("250ns");
	txtoDdrive->value(itoa_mine(disk_drive,lbl,10));
	txtoDhead->value(itoa_mine(disk_head,lbl,10));
	txtoDtrack->value(itoa_mine(disk_trackreg[disk_drive],lbl,10));
	txtoDsector->value(itoa_mine(disk_sectreg,lbl,10));

//	if(drive
//	veSize->value();

}


void update_tape_monitor()
{
	int f;
	unsigned char sample,x;
	char lbl[20];	
	for(f=0;f<32768;f++)
		monitor_pic[f]=0;
	for(f=0;f<256;f++)
	{
		if(raw_position<256)
			sample=0;
		else
			sample=raw_tape[raw_position+f-256];
		monitor_pic[(64+((128-sample)/4))*256+f]=255;
		x=sample;
		while(x>raw_threshold)
		{
			monitor_pic[(64+((128-x)/4))*256+f]=255;
			x--;
		}
		if(f%2==0)
			monitor_pic[(64+((128-raw_threshold)/4))*256+f]=255;
			
	}
	window3->redraw();
 
}



void update_vidlites4896()
{
	static	char lbl66[4]={0,0,0,0};
	if(video_latch & 0x01)liteVID0_4896->buttoncolor((fltk::Color)0xff000000);
	else liteVID0_4896->buttoncolor((fltk::Color)56);
	if(video_latch & 0x02)liteVID1_4896->buttoncolor((fltk::Color)0xff000000);
	else liteVID1_4896->buttoncolor((fltk::Color)56);
	if(video_latch & 0x04)liteVID2_4896->buttoncolor((fltk::Color)0xff000000);
	else liteVID2_4896->buttoncolor((fltk::Color)56);
	if(video_latch & 0x08)liteVID3_4896->buttoncolor((fltk::Color)0xff000000);
	else liteVID3_4896->buttoncolor((fltk::Color)56);
	if(video_latch & 0x10)liteVID4_4896->buttoncolor((fltk::Color)0xff000000);
	else liteVID4_4896->buttoncolor((fltk::Color)56);
	if(video_latch & 0x20)liteVID5_4896->buttoncolor((fltk::Color)0xff000000);
	else liteVID5_4896->buttoncolor((fltk::Color)56);
	if(video_latch & 0x40)liteVID6_4896->buttoncolor((fltk::Color)0xff000000);
	else liteVID6_4896->buttoncolor((fltk::Color)56);
	if(video_latch & 0x80)liteVID7_4896->buttoncolor((fltk::Color)0xff000000);
	else liteVID7_4896->buttoncolor((fltk::Color)56);
	txtVIDlites_4896->label(itoa_mine(video_latch,lbl66,16));
	grpVidlites4896->redraw();
}

void update_banklites4896()
{
	static	char lbl66[4]={0,0,0,0};
	if(bank_latch & 0x01)liteBANK0_4896->buttoncolor((fltk::Color)0xff000000);
	else liteBANK0_4896->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x02)liteBANK1_4896->buttoncolor((fltk::Color)0xff000000);
	else liteBANK1_4896->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x04)liteBANK2_4896->buttoncolor((fltk::Color)0xff000000);
	else liteBANK2_4896->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x08)liteBANK3_4896->buttoncolor((fltk::Color)0xff000000);
	else liteBANK3_4896->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x10)liteBANK4_4896->buttoncolor((fltk::Color)0xff000000);
	else liteBANK4_4896->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x20)liteBANK5_4896->buttoncolor((fltk::Color)0xff000000);
	else liteBANK5_4896->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x40)liteBANK6_4896->buttoncolor((fltk::Color)0xff000000);
	else liteBANK6_4896->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x80)liteBANK7_4896->buttoncolor((fltk::Color)0xff000000);
	else liteBANK7_4896->buttoncolor((fltk::Color)56);
	txtBANKlites_4896->label(itoa_mine(bank_latch,lbl66,16));
	grpBanklites4896->redraw();
}


void update_vidlites128()
{
	static	char lbl66[4]={0,0,0,0};
	if(video_latch & 0x01)liteVID0_128->buttoncolor((fltk::Color)0xff000000);
	else liteVID0_128->buttoncolor((fltk::Color)56);
	if(video_latch & 0x02)liteVID1_128->buttoncolor((fltk::Color)0xff000000);
	else liteVID1_128->buttoncolor((fltk::Color)56);
	if(video_latch & 0x04)liteVID2_128->buttoncolor((fltk::Color)0xff000000);
	else liteVID2_128->buttoncolor((fltk::Color)56);
	if(video_latch & 0x08)liteVID3_128->buttoncolor((fltk::Color)0xff000000);
	else liteVID3_128->buttoncolor((fltk::Color)56);
	if(video_latch & 0x10)liteVID4_128->buttoncolor((fltk::Color)0xff000000);
	else liteVID4_128->buttoncolor((fltk::Color)56);
	if(video_latch & 0x20)liteVID5_128->buttoncolor((fltk::Color)0xff000000);
	else liteVID5_128->buttoncolor((fltk::Color)56);
	if(video_latch & 0x40)liteVID6_128->buttoncolor((fltk::Color)0xff000000);
	else liteVID6_128->buttoncolor((fltk::Color)56);
	if(video_latch & 0x80)liteVID7_128->buttoncolor((fltk::Color)0xff000000);
	else liteVID7_128->buttoncolor((fltk::Color)56);
	txtVIDlites_128->label(itoa_mine(video_latch,lbl66,16));
	grpVidlites128->redraw();
}

void update_banklites128()
{
	static	char lbl66[4]={0,0,0,0};
	if(bank_latch & 0x01)liteBANK0_128->buttoncolor((fltk::Color)0xff000000);
	else liteBANK0_128->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x02)liteBANK1_128->buttoncolor((fltk::Color)0xff000000);
	else liteBANK1_128->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x04)liteBANK2_128->buttoncolor((fltk::Color)0xff000000);
	else liteBANK2_128->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x08)liteBANK3_128->buttoncolor((fltk::Color)0xff000000);
	else liteBANK3_128->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x10)liteBANK4_128->buttoncolor((fltk::Color)0xff000000);
	else liteBANK4_128->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x20)liteBANK5_128->buttoncolor((fltk::Color)0xff000000);
	else liteBANK5_128->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x40)liteBANK6_128->buttoncolor((fltk::Color)0xff000000);
	else liteBANK6_128->buttoncolor((fltk::Color)56);
	if(bank_latch & 0x80)liteBANK7_128->buttoncolor((fltk::Color)0xff000000);
	else liteBANK7_128->buttoncolor((fltk::Color)56);
	txtBANKlites_128->label(itoa_mine(bank_latch,lbl66,16));
	grpBanklites128->redraw();
}

void update_status()
{
	char lbl[300];
	int ret;
	char lbl2[2000];

	get_statusregs(lbl);
	txtoStatusregs->value(lbl);
	get_statusstack(lbl);
	txtoStatusStack->value(lbl);
	//txtoStatusregs->redraw();

	update_6845();//And sound status


	if(show_disas)
	{
		ret= disassemble (lbl2,0,disas_bank);
		txtoDisas->value(lbl2);
	}

	//Update PORT status lights (and legends if user changed the machine type)
	if(hw_type==LYNX_HARDWARE_48 || hw_type==LYNX_HARDWARE_96)
	{
		grpVidlites4896->show();
		grpVidlites128->hide();
		grpBanklites4896->show();
		grpBanklites128->hide();
		update_vidlites4896();
		update_banklites4896();
	}
	else
	{
		grpVidlites4896->hide();
		grpVidlites128->show();
		grpBanklites4896->hide();
		grpBanklites128->show();
		update_vidlites128();
		update_banklites128();
	}

}

void update_memmap()
{
//	draw_image ( bank0, 0, 0, 100, 100 ) ;
//draw_image(const uchar *, int X, int Y, int W, int H, int D = 3, int LD = 0)


//fltk::Bitmap(const unsigned char *bits, int W, int H)
//Bitmap(bank0, 200,200);
window2->redraw();


//Memmap_box->image(new TiffWidget(0,0,200,200,bank0));


}

void update_6845()
{
	char lbl[300],lbl2[300];

	query_6845(lbl,lbl2);
	txtoStatus68451->value(lbl);
	txtoStatus68452->value(lbl2);

	get_sound_status(lbl,lbl2);
	txtoPort84->value(lbl);
	txtoPort84timings->value(lbl2);

}

void update_gui_mtype(int x)
{
	chcMtype->value(x);
}

void  update_fps_counter(int x)
{
//	mainwind->txtFPS->text("66");
	txtoFPS->value("66");
}

void update_gui_speed(int x)
{
	chcSpeed->value(x);
}

void post_init_gui()
{
	SDL_WM_SetCaption  ("PaleSDL","none");
}

void gui_loop()
{
	fltk::wait(0);
}

void gui_loop_more()
{
	fltk::wait(1);
}


void gui_end()
{
	SDL_Quit();
}

void gui_error(char *lbl)
{
	fltk::alert(lbl);
}


void update_ROMfiles()
{
	txtoROM0->value(romset_files[mach*8+0]);
	txtoROM1->value(romset_files[mach*8+1]);
	txtoROM2->value(romset_files[mach*8+2]);
	txtoROM3->value(romset_files[mach*8+3]);
	txtoROM4->value(romset_files[mach*8+4]);
	txtoROM5->value(romset_files[mach*8+5]);
	txtoROM6->value(romset_files[mach*8+6]);
	txtoROM7->value(romset_files[mach*8+7]);
	chcROMHardware->value(romset_hwtypes[mach]);
}


void load_tape()
{
	char *filenam;
	char lbl[400];
	int ret;

	switch (tape_mode)
	{
		case TAP_TAPES:
			filenam=(char *)fltk::file_chooser( "Load TAP", "*.TAP", "*.TAP", 0 );
			if(filenam!=NULL)ret=load_lynx_tap(filenam,lbl,0);
			txtoTapeInfo->value(lbl);
			break;
		case LSF_TAPES:
			filenam=(char *)fltk::file_chooser( "Load LSF", "*.LSF", "*.LSF", 0 );
			if(filenam!=NULL)
				load_LSF(filenam);//,query,0);
			break;
		case RAW_TAPES:
			filenam=(char *)fltk::file_chooser( "Load RAW", "*.RAW", "*.RAW", 0 );
			if(filenam!=NULL)
				ret=load_raw(filenam);
			raw_position=ret;
			update_tape_gui();
			break;
		case WAV_TAPES:
			filenam=(char *)fltk::file_chooser( "Load WAV", "*.WAV", "*.WAV", 0 );
			if(filenam!=NULL)
				ret=load_wav(filenam);
			raw_position=ret;
			update_tape_gui();
			break;
		case LEV9_TAPES:
			filenam=(char *)fltk::file_chooser( "Load LEVEL9", "*.TAP", "*.TAP", 0 );
			if(filenam!=NULL)
				ret=load_level9_tap(filenam,lbl);
			break;
	}
	update_tape_gui();
}

void save_tape()
{
	char *filenam;
	char lbl[400];
	int ret;

	switch (tape_mode)
	{
		case TAP_TAPES:
			filenam=(char *)fltk::file_chooser( "Save TAP", "*.TAP", "*.TAP", 1 );
			if(filenam!=NULL)
				ret=save_lynx_tap();
			break;
		case LSF_TAPES:
			filenam=(char *)fltk::file_chooser( "Save LSF", "*.LSF", "*.LSF", 1 );
			if(filenam!=NULL)
				ret=save_LSF(filenam);
			break;
		case RAW_TAPES:
			filenam=(char *)fltk::file_chooser( "Save RAW", "*.RAW", "*.RAW", 1 );
			if(filenam!=NULL)
				ret=save_raw(filenam);
			break;
		case WAV_TAPES:
			filenam=(char *)fltk::file_chooser( "Save WAV", "*.WAV", "*.WAV", 1 );
			if(filenam!=NULL)
				ret=save_wav(filenam);
			break;
	}
}

void dton()
{
	start_disk_trace();
}

void dtoff()
{
	stop_disk_trace();
}


void set_d_num(int c)
{

	last_drive_sel=c;

}

void load_disk()
{
	char *filenam;
	char lbl[400];
	int ret,dt;

	//Get driveno from GUI
	last_drive_sel=chcDno->value();
//	dt=drive_types[last_drive_sel];
	dt=chcDtype->value();

	switch (dt)
	{
		case LYNX_FDD_RAW_IMAGE:
			filenam=(char *)fltk::file_chooser( "Load LDF", "*.LDF", "*.LDF", 0 );
			if(filenam!=NULL)
				ret=load_ldisk(last_drive_sel,filenam);
			break;
		case LYNX_FDD_DSK_IMAGE:
			filenam=(char *)fltk::file_chooser( "Load DSK", "*.DSK", "*.DSK", 0 );
			if(filenam!=NULL)
				ret=load_ldisk(last_drive_sel,filenam);
			break;
		case LYNX_FDD_YAZE_IMAGE:
			filenam=(char *)fltk::file_chooser( "Load YAZE", "*.YAZ", "*.YAZ", 0 );
			if(filenam!=NULL)
				ret=load_ydisk(last_drive_sel,filenam);
			break;
	}
	update_disksize_gui();

}

void update_disksize_gui()
{
	int ds;
	ds=get_drivesize(last_drive_sel);
	if(ds==40)
		txtoDrivesize->value("200k");
	else if(ds==80)
		txtoDrivesize->value("800k");
	else
		txtoDrivesize->value("Unknown");
}

void save_disk()
{
	char *filenam;
	char lbl[400];
	int ret;

	switch (drive_types[last_drive_sel])
	{
		case LYNX_FDD_RAW_IMAGE:
			filenam=(char *)fltk::file_chooser( "Save LDF", "*.LDF", "*.LDF", 1 );
			if(filenam!=NULL)
				ret=save_ldisk(last_drive_sel,filenam);
			break;
		case LYNX_FDD_DSK_IMAGE:
			filenam=(char *)fltk::file_chooser( "Save DSK", "*.DSK", "*.DSK", 1 );
			if(filenam!=NULL)
				ret=save_ldisk(last_drive_sel,filenam);
			break;
		case LYNX_FDD_YAZE_IMAGE:
			filenam=(char *)fltk::file_chooser( "Save YAZE", "*.YAZ", "*.YAZ", 1 );
			if(filenam!=NULL)
				ret=save_ydisk(last_drive_sel,filenam);
			break;
	}
}


void save_TAP_file()
{
	char *filenam;
	char lbl[40];
	int ret;
gui_error("This doesnt work yet, use SAVE xxx from the command line");
	filenam=(char *)fltk::file_chooser( "Save TAP", "*.TAP", "*.TAP", 1 );
	if(filenam!=NULL)
	{

		sprintf(lbl,"save\"%s\"\x0d",filenam);
		pump_string(lbl);
		save_lynx_tap();
	}
}

void update_memscan()
{
	char mscan[256];
	char data1[2000];
	char text1[2000];
	char lbl[20];
	unsigned char u;

	data1[0]=0;	//Teminate the strings to begin with
	text1[0]=0;
	get_memscan(memscan_bno,memscan_start,mscan);
	for(int f=0;f<256;f++)
	{
		u=mscan[f];
		if(f%8==0)
		{
			sprintf(lbl,"\n");
			strcat(data1,lbl);
			sprintf(lbl,"\n");
			strcat(text1,lbl);
			sprintf(lbl,"%04X - ",(memscan_start+f)&0xffff);
			strcat(data1,lbl);
		}
		sprintf(lbl,"%02X ",u&0xff);
		strcat(data1,lbl);

        if((u>=32) && (u<128))
			sprintf(lbl,"%c",mscan[f]);
		else
			sprintf(lbl,".");
		strcat(text1,lbl);
	}	
	txtoMemScan->value(data1);
	txtoMemScanAsc->value(text1);
}

void set_memscan_start(int start)
{
	memscan_start=start;//+(8-start%8);
	saveconfigfile();
}

void set_memscan_bno(int bankno)
{
	char lbl[10];
	memscan_bno=bankno;
	txtoMemScanBank->value(itoa_mine(memscan_bno,lbl,10));
	saveconfigfile();
}

void set_SoundEnable(int x)
{
	SoundEnable=x;
	chkSoundEnable->value(x);

	saveconfigfile();
}



/*void set_memmap(unsigned int banknum,unsigned int bankres)
{
	memmap_bankno=banknum;
	memmap_bankres=bankres;

window2->label("Bank 0");

}*/


void mem_map() //UNUSED FUNCTION
{
	int f;
	char x;
	for(f=0;f<65536;f++)
	{
		if(memmap_bankno==0)
			x=bank0[f];
		else if(memmap_bankno==1)
			x=bank1[f];
		else if(memmap_bankno==2)
			x=bank2[f];
		else if(memmap_bankno==3)
			x=bank3[f];
		else if(memmap_bankno==4)
			x=bank4[f];
	}
}



void update_sysvar()
{
	char emu_query[1000];
	char emu_query2[1000];
	char lbl[100];
	//Query the System variables
	sprintf((char*)emu_query," STACK\t%4X\n",bank1[0x61ee]+256*bank1[0x61ef]);
	sprintf(lbl," TOV\t%4X\n",bank1[0x621f]+256*bank1[0x6220]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," EOB\t%4X\n",bank1[0x61fc]+256*bank1[0x61fd]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," CURRLP\t%4X\n",bank1[0x61f6]+256*bank1[0x61f7]);
	strcat((char*)emu_query,lbl);
	sprintf(lbl," CONTLP\t%4X\n",bank1[0x6209]+256*bank1[0x620a]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," SOB\t%4X\n",bank1[0x61fa]+256*bank1[0x61fb]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," DATAP\t%4X\n",bank1[0x61f8]+256*bank1[0x61f9]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," STBL\t%4X\n",bank1[0x6213]+256*bank1[0x6214]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," ATBL\t%4X\n",bank1[0x6211]+256*bank1[0x6212]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," RSP\t%4X\n",bank1[0x620d]+256*bank1[0x620e]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," RSTACK\t%4X\n",bank1[0x620b]+256*bank1[0x620c]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," POLBUF\t%4X\n",bank1[0x61fe]+256*bank1[0x61ff]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," VTBL\t%4X\n",bank1[0x620f]+256*bank1[0x6210]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," PRINTD\t%4X\n",bank1[0x6200]+256*bank1[0x6201]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," LPRINTD\t%4X\n",bank1[0x6202]+256*bank1[0x6203]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," KEYB\t%4X\n",bank1[0x6204]+256*bank1[0x6205]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," OTYPE\t%4X\n",bank1[0x6206]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," LASTDSP\t%4X\n",bank1[0x6207]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," VTYPE\t%4X\n",bank1[0x6208]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," FTBL\t%4X\n",bank1[0x6215]+256*bank1[0x6216]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," IEXT\t%4X\n",bank1[0x6222]+256*bank1[0x6222]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," EEXT\t%4X\n",bank1[0x6225]+256*bank1[0x6226]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," INPLSUB\t%4X\n",bank1[0x6227]+256*bank1[0x6228]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," EXECSUB\t%4X\n",bank1[0x622B]+256*bank1[0x622C]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," ZFLAG\t%4X\n",bank1[0x6220]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," AUTOFL\t%4X\n",bank1[0x622e]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," CRST\t%4X\n",bank1[0x622f]+256*bank1[0x6230]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," LASTK\t%4X\n",bank1[0x6235]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," STATUS\t%4X\n",bank1[0x6236]); 
	strcat((char*)emu_query,lbl);

	sprintf(lbl," KDEL\t%4X\n",bank1[0x6233]+256*bank1[0x6234]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," KREP\t%4X\n",bank1[0x6237]+256*bank1[0x6238]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," FTBL\t%4X\n",bank1[0x6215]+256*bank1[0x6216]); 
	strcat((char*)emu_query,lbl);
	sprintf(lbl," TTBL\t%4X\n",bank1[0x6217]+256*bank1[0x6218]); 
	strcat((char*)emu_query,lbl);

	sprintf(lbl," EmuSpeed\t%d\n",emu_speed); 
	strcat((char*)emu_query,lbl);

	sprintf(lbl," IPeriod\t%d\n",emu_cycles); 
	strcat((char*)emu_query,lbl);



	sprintf((char*)emu_query2," XTTBL\t%4X\n",bank1[0x6219]+256*bank1[0x621a]); 
	sprintf(lbl," XITBL\t%4X\n",bank1[0x621b]+256*bank1[0x621c]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," XETBL\t%4X\n",bank1[0x621d]+256*bank1[0x621e]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," PROTST\t%4X\n",bank1[0x626b]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," OUTB\t%4X\n",bank1[0x626d]+256*bank1[0x626e]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," CHRTBL\t%4X\n",bank1[0x626f]+256*bank1[0x6270]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," CPHTBL\t%4X\n",bank1[0x6271]+256*bank1[0x6272]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," BPERL\t%4X\n",bank1[0x6273]+256*bank1[0x6274]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," MASK\t%4X\n",bank1[0x6275]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," BLUBANK\t%4X\n",bank1[0x628e]+256*bank1[0x628f]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," REDBANK\t%4X\n",bank1[0x6290]+256*bank1[0x6291]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," GRNBANK\t%4X\n",bank1[0x6292]+256*bank1[0x6293]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," DBASLM\t%4X\n",bank1[0x6282]+256*bank1[0x6283]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," VIDEO\t%4X\n",bank1[0x6288]+256*bank1[0x6289]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," EDST\t%4X\n",bank1[0x62f1]+256*bank1[0x62f2]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," \n"); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," Lynx 128 VARS\n"); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," ----------------\n"); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," 61BA Byte\t%4X\n",bank1[0x61ba]+256*bank1[0x61bb]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," MOOST\t%4X\n",bank1[0x61e1]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," BSTO\t%4X\n",bank1[0x6952]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," DDT\t%4X\n",bank1[0x6953]+256*bank1[0x6954]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," CHRSET\t%4X\n",bank1[0x6957]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," AUXCHR\t%4X\n",bank1[0x6958]+256*bank1[0x6959]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," FNTRAM\t%4X\n",bank1[0x695b]+256*bank1[0x695c]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," TRPRAM\t%4X\n",bank1[0x695e]+256*bank1[0x695f]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," WRPRAM\t%4X\n",bank1[0x6964]+256*bank1[0x6965]); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," AGREEN\t%4X\n",bank1[0x61df]+256*bank1[0x61e0]); 
	strcat((char*)emu_query2,lbl);


	txtoSysvar1->value(emu_query);
	txtoSysvar2->value(emu_query2);

	//emu_speed_per=194;
	//	sprintf(lbl," EmuSpeedPerc\t%d\n",emu_speed_per); 
	//	strcat((char*)emu_query2,lbl);
/*
	sprintf(lbl," \n"); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," Disk VARS\n"); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," -----------------\n"); 
	strcat((char*)emu_query2,lbl);
	sprintf(lbl," Options\t%2X Side\t%2X\n",disk_options,disk_side); 
	strcat((char*)emu_query2,lbl);
	
	sprintf(lbl," Control\t%2X Track\t%2X\n",disk_comreg,disk_trackreg); 
	strcat((char*)emu_query2,lbl);
	
	sprintf(lbl," Sector\t%2X Data\t%2X\n",disk_sectreg,disk_statusreg); 
	strcat((char*)emu_query2,lbl);
	
	sprintf(lbl," Status\t%2X\n",disk_statusreg); 
	strcat((char*)emu_query2,lbl);
	//for(int y=0;y<4;y++)
	//{
	//	sprintf(lbl,"%2X\t",disk_1793[y]); 
	//	strcat((char*)emu_query2,lbl);
	//}
*/
}


void set_show_status(int x)
{
	if(x==1)
	{
		statuswind->show();
		show_status=1;
		chkStatus->value(1);
	}
	else
	{
		statuswind->hide();
		show_status=0;
		chkStatus->value(0);
	}

	saveconfigfile();
}

void set_show_memmap(int x)
{
	if(x==1)
	{
		window2->show();
		show_memmap=1;
		chkMemmap->value(1);
	}
	else
	{
		window2->hide();
		show_memmap=0;
		chkMemmap->value(0);
	}
	saveconfigfile();
}

void set_show_sysvars(int x)
{
	if(x==1)
	{
		wndSysvars->show();
		show_sysvars=1;
		chkSysvars->value(1);
	}
	else
	{
		wndSysvars->hide();
		show_sysvars=0;
		chkSysvars->value(0);
	}
	saveconfigfile();
}


void toggle()
{
	static int t=0;
	if(t==0)
	{
		t=1;
		grpVidlites4896->show();
		grpVidlites128->hide();
	}
	else
	{
		t=0;
		grpVidlites4896->hide();
		grpVidlites128->show();
	}
}

char* itoa_mine( int val, char* result, int base )
{
	static char buf[32] = "\0";
	static char buf2[32] = "\0";
	char x;

	if(base==10)
		sprintf(buf,"%2d",val);
	else
		sprintf(buf,"%0X",(unsigned char)val);
	return &buf[0];
}	


/*
char* itoa_mine( int val, char* result, int base ) {

	static char buf[32] = {0};
	int i = 30;
	if(val==0)
	{
		sprintf(buf,"00");
		return &buf[0];
	}
	else
	{
		for(; val && i ; --i, val /= base)
		
			buf[i] = "0123456789abcdef"[val % base];
	}	
	return &buf[i+1];
	
}	
*/
