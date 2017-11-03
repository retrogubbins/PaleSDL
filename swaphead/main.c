#include <stdio.h>
#include <stdlib.h>

#define LYNX_MAX_DISK_SIZE 819200
#define LYNX_SIZE_200K 204800
#define LYNX_SIZE_800K 819200

char disk0[LYNX_MAX_DISK_SIZE];
int disk_size=0;



int  load_disk(char fnam[])
{
	int ret,errorr,g,h;
	FILE *  handle,*handle2;
	int  size_read,size_written;
	int cdd,f;
	char lbl[200],work_fname[200];

	handle = fopen( fnam, "rb" );
	if( handle != NULL )
	{
		size_read = fread( disk0,1,LYNX_MAX_DISK_SIZE,handle );
		fclose( handle );
		printf("Size read is %d\n",size_read);
		
		if( size_read == 0 )
		{
				printf(":( Zero Length DISK file\n");
				return(1);
		}
		if((size_read!=LYNX_SIZE_800K) && (size_read!=LYNX_SIZE_200K) )
		{
					printf(":( BAD SIZE DISK file %u (must be 200K or 800K)\n",size_read);
					return(1);
		}
	}
	else
	{
			printf(":( Couldn't open the DISK file\n");
			return(1);
	}
	disk_size=size_read;
	return(0);			
}


int save_disk(char fnam[])
{
	FILE *  handle,*handle2;
	int size_written,size_read,ret;

	/* open a file for output		  */
	handle2 = fopen( fnam, "wb" );
	if( handle2 != NULL )
	{
		size_written = fwrite( disk0,1,disk_size,handle2 );
		fclose( handle2 );
		if( size_written != disk_size) 
		{
			printf(":( Couldn't save all bytes of the DISK file\n");
			return(1);
		}
	}
	else
	{
		printf("Couldnt Write the DISK file\n");
		return(1);
	}
	return(0);
}

void swap_heads()
{
	char temp[5120];
	int cyl,f,h1,h2;
	printf("Swapping Heads... ");
	if(disk_size==LYNX_SIZE_800K)
	{
		//For 800K disk each track is 1400h bytes long
		//which is 10 sectors x 512 bytes per sector = 5120 bytes
		for(cyl=0;cyl<80;cyl++)
		{
			//for each cylinder, swap the tracks on either side
			h1=cyl*2*5120;
			h2=((cyl*2)+1)*5120;
			for(f=0;f<5120;f++)
				temp[f]=disk0[h1+f];
			for(f=0;f<5120;f++)
				disk0[h1+f]=disk0[h2+f];
			for(f=0;f<5120;f++)
				disk0[h2+f]=temp[f];
		}
		printf("Done\n");
	}
	else
	{
		printf("Wrong Size Disk Found!\n");
	}
}

int main(int argc, char *argv[])
{
	if(argc!=2)
	{
		printf("Incorrect number of parameters. Use SWAPHEAD <infile>\n");
		return(1);
	}

	printf("File to be altered is %s\n",argv[1]);
	
	if(load_disk(argv[1])==0)
	{
		swap_heads();
		if(save_disk(argv[1])==0)
		{
			printf("Completed Ok\n");
			return(0);
		}
		else
		{
			printf("Errors saving the disk\n");
			return(1);
		}
	}
	else
	{
		printf("Errors loading the disk\n");
		return(1);
	}
	return 0;
}


