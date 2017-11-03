#include <stdio.h>
#include "EMUSWITCH.h"


#include "PALESDL.H"
#include "Z80DASM.H"



void get_statusregs(char*lbl)
{
	sprintf(lbl,
		"PC:%04X IR: %02X AF:%04X HL:%04X DE:%04X BC:%04X SP:%04X IX:%04X IY:%04X",
	    z80_get_reg(Z80_REG_PC),
		z80_get_reg(Z80_REG_IR),
		z80_get_reg(Z80_REG_AF),
		z80_get_reg(Z80_REG_HL),
		z80_get_reg(Z80_REG_DE),
		z80_get_reg(Z80_REG_BC),
		z80_get_reg(Z80_REG_SP),
		z80_get_reg(Z80_REG_IX),
		z80_get_reg(Z80_REG_IY));
}

void get_statusstack(char *lbl)
{
	unsigned int i,sp;
	char lbl1[300],lbl2[300],lbl3[300],lbl4[300];
	//STACK
	sp=z80_get_reg(Z80_REG_SP);
	
	sprintf(lbl2," ");
	for (i=0;i<10;++i)
	{
		sprintf(lbl3,"%02X%02X ",bank1[sp+i*2],bank1[sp+i*2]);
		strcat(lbl2,lbl3);
	}
	strcpy(lbl,lbl2);
}



int disassemble (char *destin,unsigned int jogval,int bank)
{
	int g,retr,f,start,mid,end,linecount;
	unsigned int curp,peecee;
	char lbl[200];
	char dis_buf[4000];
	char return_char[4];


	//Poss might have to increase this
	// to avoid routine below scanning backwards into other vars space
	peecee=z80_get_reg(Z80_REG_PC);
	curp=peecee-32+jogval;
	
	//skip forward to the relevant byte
	sprintf(return_char,"\n");
	sprintf(dis_buf,"\x00");
	mid=0;
	for (f=0;f<64;f++)
	{
		if(curp==peecee)
		{
			mid=strlen(dis_buf);
			sprintf(lbl,"->");
			strcat(dis_buf,lbl);
		}
		sprintf(lbl,"%4X\t",curp);
		strcat(dis_buf,lbl);
//		retr=Z80_Dasm((unsigned char *)bank0+curp,lbl,curp);
		if(bank==0)
				retr=Z80_Dasm((unsigned char *)bank0+curp,lbl,curp);
		else if(bank==1)
				retr=Z80_Dasm((unsigned char *)bank1+curp,lbl,curp);
		else if(bank==2)
				retr=Z80_Dasm((unsigned char *)bank2+curp,lbl,curp);
		else if(bank==3)
				retr=Z80_Dasm((unsigned char *)bank3+curp,lbl,curp);
		else if(bank==4)
				retr=Z80_Dasm((unsigned char *)bank4+curp,lbl,curp);
		//		retr=Z80_Dasm((unsigned char *)bank0+curp,lbl,curp);
//		retr=Z80_Dasm((unsigned char *)bank0+curp,lbl,curp);
		if (strlen(lbl)==0) return(0);
		strcat(dis_buf,lbl);
		strcat(dis_buf,return_char);
		curp+=retr;	//advance to the next opcode
	}
	end=strlen(dis_buf);
	if (mid!=0)
	{
		//okay, we've got a listing, now search thru
		//to fnd the PC line, and then take the 14 lines either side of it
		//and copy them to the output buffer
		start=0;
		f=0;	
		//Skip to PC line
//		while(dis_buf[f++]!='_')
//		{
//			if(f>end)return(0);
//		};
		//Now go back 6 lines
		f=mid;
		for(g=0;g<5;g++)
		{
			while(dis_buf[f--]!='\x0a')
			{
				if(f<=0)return(0);
			}
		}
		start=f;
		for(f=start;f<end;f++)
			destin[f-start]=dis_buf[f];
	}
		else return(0);
	return(1);
}

