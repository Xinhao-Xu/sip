#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <sys/fcntl.h>
#include <errno.h>
#include <termios.h> 
#include <unistd.h>
#include <linux/rtc.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <linux/fb.h>
#include <sys/soundcard.h>
#include <dirent.h>  
#include "sys/socket.h"
#include "netdb.h"
#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "sys/select.h"
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/io.h>

#include "dialog.h"
#include "Result.h"
#include "utils.h"
#include "debug.h"
#include "gpclib.h"
#include "eitlib.h"
#include "UI.h"

/*************************************************************************************
	Global variables
*************************************************************************************/
#define DEVPOS1					1
#define DEVPOS2					3

#define UTILS_TMPFOLDER			"./data"



#define DMIFNAME				"dmiinfo.res"
#define DMI_CPU					"Processor Information"
#define DMI_CPU_MANU			"Manufacturer:"
#define DMI_CPU_ID				"ID:"
#define DMI_CPU_VER				"Version:"
#define DMI_CPU_VOLTAGE			"Voltage:"
#define DMI_CPU_EXTCLK			"External Clock:"
#define DMI_CPU_MAXSPEED		"Max Speed:"
#define DMI_CPU_CURSPEED		"Current Speed:"

#define DMI_MEM 				"Memory Device"
#define DMI_MEM_SIZE			"Size:"
#define DMI_MEM_FREQ			"Speed:"


#define SENSORFNAME				"sensor.res"
#define SENSOR_SEC				"Adapter:"
#define SENSOR_P2_5V			"+2.5V:"
#define SENSOR_VCORE			"VCore:"
#define SENSOR_P3_3V			"+3.3V:"
#define SENSOR_P5V				"+5V:"
#define SENSOR_P12V				"+12V:"
#define SENSOR_VCC				"VCC:"
#define SENSOR_P1_5V			"+1.5V:"
#define SENSOR_P1_8V			"+1.8V:"
#define SENSOR_CHIP_TEMP		"Chip Temp:"
#define SENSOR_CPU_TEMP			"CPU Temp:"
#define SENSOR_SYS_TEMP			"Sys Temp:"
#define SENSOR_CPU0_VID			"cpu0_vid:"
#define SENSOR_FAN1				"fan1:"
#define SENSOR_FAN2				"fan2:"

#define USBFNAME				"usbinfo.res"

/* IP Header -- RFC 791 */
#define ICMP_ECHOREPLY	0
#define ICMP_ECHO		8
#define REQ_DATASIZE	32
typedef struct tagIPHDR
{
	u_char			VIHL;		/* Version and IHL */
	u_char			TOS;		/* Type Of Service */
	short			TotLen;		/* Total Length */
	short			ID;			/* Identification */
	short			FlagOff;	/* Flags and Fragment Offset */
	u_char			TTL;		/* Time To Live */
	u_char			Protocol;	/* Protocol */
	u_short			Checksum;	/* Checksum */
	struct in_addr	iaSrc;		/* Internet Address - Source */
	struct in_addr	iaDst;		/* Internet Address - Destination */
} IPHDR, *PIPHDR;

/*
 -----------------------------------------------------------------------------------------------------------------------
    ICMP Header - RFC 792
 -----------------------------------------------------------------------------------------------------------------------
 */
typedef struct tagICMPHDR
{
	u_char	Type;		/* Type */
	u_char	Code;		/* Code */
	u_short Checksum;	/* Checksum */
	u_short ID;			/* Identification */
	u_short Seq;		/* Sequence */
	char	Data;		/* Data */
} ICMPHDR, *PICMPHDR;

/*
 -----------------------------------------------------------------------------------------------------------------------
    ICMP Echo Request
 -----------------------------------------------------------------------------------------------------------------------
 */
typedef struct tagECHOREQUEST
{
	ICMPHDR			icmpHdr;
	struct timeval	echoTime;
	char			cData[REQ_DATASIZE];
} ECHOREQUEST, *PECHOREQUEST;

/*
 -----------------------------------------------------------------------------------------------------------------------
    ICMP Echo Reply
 -----------------------------------------------------------------------------------------------------------------------
 */
typedef struct tagECHOREPLY
{
	IPHDR		ipHdr;
	ECHOREQUEST echoRequest;
	char		cFiller[256];
} ECHOREPLY, *PECHOREPLY;

pid_t pid;
void tv_sub(struct timeval *out, struct timeval *in);
unsigned short checksum(unsigned short *buffer, int size);
int WaitForEchoReply(int socket);
//EarrandMan 20190103 to configure gmb6080 superio(f81865) com1 ISA share irq mode
#define BASEPORT 	0x2E
#define BASEIO	 	(BASEPORT+1)
int configure_share_irq();


/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Section1: Utility inner functions
//  
//	Descript: Inner functions for handling this unility 
/////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/*************************************************************************************
	Common function for Utilities
*************************************************************************************/
int initUtil()
{
	char szBuf[1024];

	if(access(UTILS_TMPFOLDER,F_OK))
	{
		sprintf(szBuf,"mkdir %s",UTILS_TMPFOLDER);
		system(szBuf);
	}
	if(access(RECORD_PATH,F_OK))
	{
		sprintf(szBuf,"mkdir %s",RECORD_PATH);
		system(szBuf);
	}
	if(access(RESULT_PATH,F_OK))
	{
		sprintf(szBuf,"mkdir %s",RESULT_PATH);
		system(szBuf);
	}
	if(access(ANSYS_PATH,F_OK))
	{
		sprintf(szBuf,"mkdir %s",ANSYS_PATH);
		system(szBuf);
	}
	if(access(OTHERREC_PATH,F_OK))
	{
		sprintf(szBuf,"mkdir %s",OTHERREC_PATH);
		system(szBuf);
	}
	if(access(UTILS_TMPFOLDER,F_OK))
	{
		sprintf(szBuf,"mkdir %s",UTILS_TMPFOLDER);
		system(szBuf);
	}
	if(access(RECORD_PATH_NOTUPLOAD,F_OK))
	{
		sprintf(szBuf,"mkdir %s",RECORD_PATH_NOTUPLOAD);
		system(szBuf);
	}
	if(access(RECORD_PATH_UPLOADED,F_OK))
	{
		sprintf(szBuf,"mkdir %s",RECORD_PATH_UPLOADED);
		system(szBuf);
	}
}

/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Section2: String handle
//  
//	Descript: For handling(parse,delet,add...) string, also provide ability 
//            to get information from text files.
/////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/*************************************************************************************
	This function parsing the result file of command 'dmidecode' 
	that save in path /tmp/dmiinfo.res.
*************************************************************************************/
int parse_dmidecode(char *dmifile, char *section, char *target, int devpos, char *szRes)
{
	FILE *file=NULL;
	char *p;
	char szBuf[1024];
	int	 len;
	int dpos=1;
	
	file = fopen(dmifile,"r");
	if(file!=NULL)
	{
		while(!feof(file))
		{
			fgets(szBuf,1024,file);
			if(strstr(szBuf,section))
			{
				if(dpos<devpos)
					dpos++;
				else
				{
					while(!feof(file))
					{
						fgets(szBuf,1024,file);
						if(p=strstr(szBuf,target))
						{
							//delete the symbol of change line
							len = strlen(szBuf);
							if(szBuf[len-1]='\n')
								szBuf[len-1] = '\0';
								
							p+=strlen(target);
							while(*p==' ')
								p++;
							strcpy(szRes,p);
							goto end;
						}
					}
				}
			}
		}//end of while	
	}//end of if
end:
	fclose(file);

	return FUN_SUCC;
}


int del_str(char *strSrc,char *strDel,char *szRes)
{
	char szBuf[MAX_BUF_SIZE];
	char *p;

	strcpy(szBuf,strSrc);
	if(p=strstr(szBuf,strDel))
	{
		p--;
		while(*p==' ')
			*p=0;
		strcpy(szRes,szBuf);
		return FUN_SUCC;
	}

	return FUN_FAIL;
}

int get_dmiinfo(PDMIINFO pInfo)
{
	char szFile[128];
	char szBuf[MAX_BUF_SIZE],szBuf2[MAX_BUF_SIZE];


	sprintf(szFile,"%s/%s",UTILS_TMPFOLDER,DMIFNAME);
	sprintf(szBuf,"dmidecode > %s",szFile);
	system(szBuf);

	/*******************************************************************
		Get CPU informat
	********************************************************************/
	
	//get Manufacturer
	parse_dmidecode(szFile,DMI_CPU,DMI_CPU_MANU,DEVPOS1,szBuf);
	strcpy(pInfo->cpuinfo.manufac,szBuf);

	//get ID
	parse_dmidecode(szFile,DMI_CPU,DMI_CPU_ID,DEVPOS1,szBuf);
	strcpy(pInfo->cpuinfo.id,szBuf);

	//get Version
	parse_dmidecode(szFile,DMI_CPU,DMI_CPU_VER,DEVPOS1,szBuf);
	strcpy(pInfo->cpuinfo.ver,szBuf);

	//get Voltage
	parse_dmidecode(szFile,DMI_CPU,DMI_CPU_VOLTAGE,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->cpuinfo.voltage);

	//get External Clock
	parse_dmidecode(szFile,DMI_CPU,DMI_CPU_EXTCLK,DEVPOS1,szBuf);
	del_str(szBuf,"MHz",szBuf2);
	sscanf(szBuf2,"%d",&pInfo->cpuinfo.ext_clock);

	//get Max Speed
	parse_dmidecode(szFile,DMI_CPU,DMI_CPU_MAXSPEED,DEVPOS1,szBuf);
	del_str(szBuf,"MHz",szBuf2);
	sscanf(szBuf2,"%d",&pInfo->cpuinfo.max_speed);

	//get Current Speed
	parse_dmidecode(szFile,DMI_CPU,DMI_CPU_CURSPEED,DEVPOS1,szBuf);
	del_str(szBuf,"MHz",szBuf2);
	sscanf(szBuf2,"%d",&pInfo->cpuinfo.curSpeed);

	/*******************************************************************
		Get Memory informat
	********************************************************************/
	//get Memory device1 size
	parse_dmidecode(szFile,DMI_MEM,DMI_MEM_SIZE,DEVPOS1,szBuf);
	del_str(szBuf,"MB",szBuf2);
	sscanf(szBuf2,"%d",&pInfo->meminfo.size);
	
	//get Memory device1 frequence
	parse_dmidecode(szFile,DMI_MEM,DMI_MEM_FREQ,DEVPOS1,szBuf);
	del_str(szBuf,"MHz",szBuf2);
	sscanf(szBuf2,"%d",&pInfo->meminfo.freq);

	//get Memory device2 size
	parse_dmidecode(szFile,DMI_MEM,DMI_MEM_SIZE,DEVPOS2,szBuf);
	del_str(szBuf,"MB",szBuf2);
	sscanf(szBuf2,"%d",&pInfo->meminfo2.size);
		
	//get Memory device2 frequence
	parse_dmidecode(szFile,DMI_MEM,DMI_MEM_FREQ,DEVPOS2,szBuf);
	del_str(szBuf,"MHz",szBuf2);
	sscanf(szBuf2,"%d",&pInfo->meminfo2.freq);

	return FUN_SUCC;
}

int get_sensorinfo(PSENSOR_INFO pInfo)
{
	char szFile[128];
	char szBuf[MAX_BUF_SIZE],szBuf2[MAX_BUF_SIZE];

	sprintf(szFile,"%s/%s",UTILS_TMPFOLDER,SENSORFNAME);
	sprintf(szBuf,"sensors > %s",szFile);
	system(szBuf);

	//get +2.5V
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_P2_5V,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->p2_5v);

	//get VCore
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_VCORE,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->vcore);

	//get +3.3V
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_P3_3V,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->p3_3v);

	//get +5V
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_P5V,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->p5v);

	//get +12V
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_P12V,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->p12v);

	//get VCC
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_VCC,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->vcc);

	//get +1.5V
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_P1_5V,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->p1_5v);

	//get +1.8V
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_P1_8V,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->p1_8v);

	//get Chip Temp
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_CHIP_TEMP,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->chip_temp);

	//get CPU Temp
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_CPU_TEMP,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->cpu_temp);

	//get Sys Temp
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_SYS_TEMP,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->sys_temp);

	//get CPU0 VID
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_CPU0_VID,DEVPOS1,szBuf);
	del_str(szBuf,"V",szBuf2);
	sscanf(szBuf2,"%f",&pInfo->cpu0_vid);

	//get fan1 speed
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_FAN1,DEVPOS1,szBuf);
	del_str(szBuf,"RPM",szBuf2);
	sscanf(szBuf2,"%d",&pInfo->fan1);

	//get 
	parse_dmidecode(szFile,SENSOR_SEC,SENSOR_FAN2,DEVPOS1,szBuf);
	del_str(szBuf,"RPM",szBuf2);
	sscanf(szBuf2,"%d",&pInfo->fan2);

	//printf("v=%f\r\n",pInfo->p2_5v);
	return FUN_SUCC;
}

/*//////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Section3: Display handle
//  
//	Descript: To handle any thing about sight(display,screen,picture....) 
/////////////////////////////////////////////////////////////////////////////////////////////////////////*/

/*************************************************************************************
	UI functions
*************************************************************************************/

//This function is according to the parameter 'attrib' to do some thing else
int attrib_action(int attrib,int x,int y)
{
	int res;
	//int org_x,org_y;
	//get current x,y
	//org_x = POS_X;
	//org_y = POS_Y;

	//CLEAR();
	dlg_clr_result();	
	if(attrib==ATTRIB_ASK_PASS_FAIL)
	{
		SETPOS(x,y);
		res = dialog_yesno("","Please check the result",5,40);
		//SETPOS(org_x,org_y);
		if(res)
			return RESULT_FAIL;
		else
			return RESULT_PASS;
	}
}

int ShowMessage(char *title,char *context, int x, int y, int width, int height,int pause_opt)
{
	//int org_x,org_y;
	//get current x,y
	//org_x = POS_X;
	//org_y = POS_Y;

	CLEAR();
	dlg_clr_result();	
	SETPOS(x,y);
	dialog_msgbox(title,context,height,width,pause_opt);
	//SETPOS(org_x,org_y);
}

int ask_reslut(int x,int y)
{
	int res=RESULT_FAIL,keepask=1;
	
	showjpeg(x,y,"./pics/800_600/ask_fail.jpg");
	while(keepask)
	{
		switch(getch())
		{
			case KEY_LEFT:
				showjpeg(x,y,"./pics/800_600/ask_pass.jpg");
				res=RESULT_PASS;
				break;
			case KEY_RIGHT:
				showjpeg(x,y,"./pics/800_600/ask_fail.jpg");
				res=RESULT_FAIL;
				break;
			case 'y':
			case 'Y':
				keepask = 0;
			break;
		}
	}

	return res;
}

void check_yes()
{
	int keepask=1;
	
	while(keepask)
	{
		usleep(100000);
		switch(getch())
		{
			case 'y':
			case 'Y':
				keepask = 0;
			break;
		}
	}
}

void check_no()
{
	int keepask=1;
	
	while(keepask)
	{
		usleep(100000);
		switch(getch())
		{
			case 'n':
			case 'N':
				keepask = 0;
			break;
		}
	}
}

int checkno_nowait()
{	
	int i;
	char res=RESULT_FAIL;
	
	for(i=0;i<10;i++)
	{
		usleep(10000);
		switch(getch())
		{
			case 'n':
			case 'N':
				res = RESULT_PASS;
			break;
		}
	}

	return res;
}


int display_num(int x, int y, int num, int len)
{
	unsigned int i,n,base,pos_x,pos_y;

	pos_x=x;
	pos_y=y;
	for(i=len;i>0;i--)
	{
		base=pow(10,i-1);
		n=(unsigned int)(num/base);
		if(n)
		{
			if(n==1)showjpeg(pos_x, pos_y, NUM2PATH(1));
			if(n==2)showjpeg(pos_x, pos_y, NUM2PATH(2));
			if(n==3)showjpeg(pos_x, pos_y, NUM2PATH(3));
			if(n==4)showjpeg(pos_x, pos_y, NUM2PATH(4));
			if(n==5)showjpeg(pos_x, pos_y, NUM2PATH(5));
			if(n==6)showjpeg(pos_x, pos_y, NUM2PATH(6));
			if(n==7)showjpeg(pos_x, pos_y, NUM2PATH(7));
			if(n==8)showjpeg(pos_x, pos_y, NUM2PATH(8));
			if(n==9)showjpeg(pos_x, pos_y, NUM2PATH(9));
		}
		else
		{
			showjpeg(pos_x, pos_y, PATH_NUM0);		
		}
		pos_x+=NUM_WIDTH;
		num-=n*base;
	}

	return pos_x;
}

/*************************************************************************************
	System Information Items
*************************************************************************************/
int get_fbres(char *fbdev,U32 *xres,U32 *yres,U32 *bits_per_pixel)
{
 	int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;

    fbfd = open(fbdev, O_RDWR);
    
    if (!fbfd) 
    {
        DEBUGMSG("Error: cannot open framebuffer device.\r\n");
        return 1;
    }
    //DEBUGMSG("The framebuffer device was opened successfully.\r\n");

    // get fix attrib
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) 
    {
        DEBUGMSG("Error reading fixed information.\r\n");
        return 1;
    }

    // get var attrib
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) 
    {
        DEBUGMSG("Error reading variable information.\r\n");
        return 1;
    }
    xres = vinfo.xres;
    yres = vinfo.yres;
    bits_per_pixel = vinfo.bits_per_pixel;
    close(fbfd);
    
    return 0;
}

/*************************************************************************************
	USB Test Items
*************************************************************************************/
int check_usbdev(PUSBINFO pInfo)
{
	FILE *file=NULL;
	char szFile[128];
	char szBuf[MAX_BUF_SIZE],szBuf2[MAX_BUF_SIZE];

	sprintf(szFile,"%s/%s",UTILS_TMPFOLDER,USBFNAME);
	sprintf(szBuf,"lsusb -d %s:%s > %s",pInfo->vid,pInfo->pid,szFile);
	system(szBuf);
	strcpy(szBuf,"");

	file = fopen(szFile,"r");
	if(file!=NULL)
	{
		sprintf(szBuf2,"%s:%s",pInfo->vid,pInfo->pid);
		while(!feof(file))
		{
			fgets(szBuf,1024,file);		
			if(strstr(szBuf,szBuf2))
				return FUN_SUCC;
		}
	}

	return FUN_FAIL;
}

/*************************************************************************************
	Storage Test Items
*************************************************************************************/
int get_HDInfo(PHDINFO pInfo)
{
	struct hd_driveid id;
	int fd = open(pInfo->dev, O_RDONLY|O_NONBLOCK);

	if (fd < 0) 
	{
		perror(pInfo->dev);
		return 1;
	}

	if(!ioctl(fd, HDIO_GET_IDENTITY, &id))
	{
		strcpy(pInfo->id,id.serial_no);
	}

	
	return FUN_SUCC;
}

//ErrandMan
int configure_share_irq()
{
	unsigned char buf;

	if(ioperm(BASEPORT, 2, 1))
	{
		printf("configure_share_irq: ioperm error\r\n"); 
		exit(1);
	}

	//COM1
	outb( 0x87 , BASEPORT);
	outb( 0x87 , BASEPORT); //Entry configuration mode
	outb( 0x07 , BASEPORT); //Select register index 0x07
	outb( 0x10 , BASEIO); //Select COM1
	outb( 0xf0 , BASEPORT); //Select COM1 register index 0xf0
	outb( 0x03 , BASEIO); //Set UART 1 interrupt channel to ISA interrupt with sharing

	//verify
	buf=inb(BASEIO);
	if(buf != 0x03)
		printf("Set register fail! \r\n");
	//printf("The new register value : 0x%02x\r\n", buf);
	outb( 0xaa , BASEPORT); //Exit configuration mode

	if(ioperm(BASEPORT, 2, 0)) 
	{
		printf("ioperm error\r\n"); 
		exit(1);
	}
	return 0;
}


/*************************************************************************************
	UART Test Items
*************************************************************************************/
int initport(int fd) 
{
	struct termios options;
	// Get the current options for the port...
	tcgetattr(fd, &options);
	// Set the baud rates to 9600...
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);
	// Enable the receiver and set local mode...
	options.c_cflag |= (CLOCAL | CREAD);

	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	// Set the new options for the port...
	tcsetattr(fd, TCSANOW, &options);
	
	return FUN_SUCC;
}


/********************************************************
*
* 2016.11.15 DK serial raw init
*
********************************************************/

int initport_raw(int fd) 
{
	struct termios options;
	
	tcgetattr(fd, &options);
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;	// Character size mask
	options.c_cflag |= CS8;
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);	//Raw input is unprocessed. Input characters are passed through exactly as they are received
	options.c_oflag &= ~OPOST;		//output format handle 
	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 10;	//read timeout = 1 sec
	options.c_iflag &= ~(IXON | IXOFF | IXANY); 	//disabe software flow control
	options.c_cflag &= ~CRTSCTS;	// |= enable  &= ~ disable --- hardware flow control
	//options.c_iflag |= (IGNBRK | IGNCR | IGNPAR);
	options.c_iflag &= ~(ICRNL | IGNCR | IGNPAR | IGNBRK);
	options.c_oflag &= ~(ONLCR | OCRNL | ONLRET);
	tcflush(fd, TCIOFLUSH);
	tcsetattr(fd, TCSANOW, &options);

	usleep(10000);
	
	return FUN_SUCC;
}


int writeport(int fd, char *chars) 
{
	int len = strlen(chars);
	chars[len] = 0x0d; // stick a <CR> after the command
	chars[len+1] = 0x00; // terminate the string properly
	int n = write(fd, chars, strlen(chars));
	if (n < 0) 
	{
		fputs("write failed!\n", stderr);
		return FUN_FAIL;
	}
	return FUN_SUCC;
}

int readport(int fd, char *result) 
{
	int iIn = read(fd, result, 254);
	result[iIn-1] = 0x00;
	DEBUGMSG("\r\nresult = %s\r\n", result);
	if (iIn < 0) 
	{
		if (errno == EAGAIN) 
		{
			DEBUGMSG("SERIAL EAGAIN ERROR\n");
			return FUN_FAIL;
		} 
		else 
		{
			DEBUGMSG("SERIAL read error %d %s\n", errno, strerror(errno));
			return FUN_FAIL;
		}
	}                    
	return FUN_SUCC;
}

/*
int init_uart(char *dev)
{
	int fd;
	
	fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) 
	{
		DEBUGMSG("open_port: Unable to open %s - ",dev);
		return FUN_FAIL;
	} 
	else
	{
		fcntl(fd, F_SETFL, 0);
	}
	
	initport(fd);
	fcntl(fd, F_SETFL, FNDELAY); // don't block serial read

	return fd;
}

int read_uart(int fd)
{
	char sResult[254];
	
	if (readport(fd,sResult)) 
	{
		DEBUGMSG("read failed\n");
		close(fd);
		return FUN_FAIL;
	}
	DEBUGMSG("readport=%s\n", sResult);
}

int deinit_uart(int fd)
{
	close(fd);
	
	return FUN_SUCC;
}
*/

int getbaud(int fd) 
{
	struct termios termAttr;
	int inputSpeed = -1;
	speed_t baudRate;
	tcgetattr(fd, &termAttr);
	/* Get the input speed.                              */
	baudRate = cfgetispeed(&termAttr);
	switch (baudRate) 
	{
		case B0:      inputSpeed = 0; break;
		case B50:     inputSpeed = 50; break;
		case B110:    inputSpeed = 110; break;
		case B134:    inputSpeed = 134; break;
		case B150:    inputSpeed = 150; break;
		case B200:    inputSpeed = 200; break;
		case B300:    inputSpeed = 300; break;
		case B600:    inputSpeed = 600; break;
		case B1200:   inputSpeed = 1200; break;
		case B1800:   inputSpeed = 1800; break;
		case B2400:   inputSpeed = 2400; break;
		case B4800:   inputSpeed = 4800; break;
		case B9600:   inputSpeed = 9600; break;
		case B19200:  inputSpeed = 19200; break;
		case B38400:  inputSpeed = 38400; break;
	}
	return inputSpeed;
}

int uart_loopback_directio(unsigned int port)
{
	char c,cmd[16];
	unsigned int i;
	int res=FUN_FAIL;

	ioperm(port,7,1);
	memset(cmd, 0, sizeof(cmd));
	outb(0x00, port + 1); /* Turn off interrupts - Port1 */
	outb(0x80, port + 3 ); /* SET DLAB ON */
	outb(0x03, port + 0); /* default 0x03 = 38,400 BPS */
	outb(0x00, port + 1); /* Set Baud rate - Divisor Latch High Byte */
	outb(0x03, port + 3); /* 8 Bits, No Parity, 1 Stop Bit */
	outb(0xC7, port + 2); /* FIFO Control Register */
	outb(0x0B, port + 4); /* Turn on DTR, RTS, and OUT2 */

    outb(0x41, port);
    outb(0x42, port);
    outb(0x43, port);
    outb(0x00, port);

	usleep(10000);
	
 	for(i=0;i<4;i++)
	{ 
		usleep(10000);
		c = inb(port + 5);
	    if (c & 1) 
		{
			cmd[i] = inb(port);
	     	//printf("COM get char=%c\r\n",cmd[i]);
		}
    }

	if( (cmd[0]==0x41)&&(cmd[1]==0x42)&&(cmd[2]==0x43)&&(cmd[3]==0x00) )
	{
		res = FUN_SUCC;
	}

	//DK 20181228 flush.
    outb(0x00, port);
    outb(0x00, port);
    outb(0x00, port);
    outb(0x00, port);
	usleep(10000);
	
	return res;
}


int uart_loopback_Test(char *dev)
{
	int fd;
	char sResult[254];

	//DK: fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) 
	{
		DEBUGMSG("open_port: Unable to open %s - ",dev);
		return FUN_FAIL;
	} 
	else
	{
		fcntl(fd, F_SETFL, 0);
	}
	
	initport(fd);

	char sCmd[254];
	sCmd[0] = 0x41;
	sCmd[1] = 0x42;
	sCmd[2] = 0x43;
	sCmd[3] = 0x00;

	if (writeport(fd, sCmd)) 
	{
		DEBUGMSG("write failed\n");
		close(fd);
		return FUN_FAIL;
	}

	DEBUGMSG("written:%s\n", sCmd);
	
	usleep(10000);

	fcntl(fd, F_SETFL, FNDELAY); // don't block serial read

	if (readport(fd,sResult)) 
	{
		DEBUGMSG("read failed\n");
		close(fd);
		return FUN_FAIL;
	}
	DEBUGMSG("readport=%s\n", sResult);
	close(fd);
	return FUN_SUCC;

}

/********************************************************
*
*	DK	2016.07.19
*	ignore EAGAIN, only check rx data is correct or not 
********************************************************/

int uart_loopback_Test_raw(char *dev)

{
	int fd,iIn;
	char sResult[254];
	struct termios options;

	fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) 
	{
		DEBUGMSG("open_port: Unable to open %s - ",dev);
		return FUN_FAIL;
	} 
	else
	{
		fcntl(fd, F_SETFL, 0);
	}

	tcgetattr(fd, &options);
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	options.c_cflag |= (CLOCAL | CREAD);
	//options.c_iflag &= ~(IXON | IXOFF | IXANY); 	//disabe software flow control
	//options.c_cflag &= ~CRTSCTS;	//disable hardware flow control
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | FLUSHO);	//Raw input is unprocessed. Input characters are passed through exactly as they are received
	options.c_oflag &= ~OPOST;		//raw output 
	tcsetattr(fd, TCSANOW, &options);
	usleep(10000);
	tcsetattr(fd, TCSAFLUSH, &options);

	//because share_irq, we read buf once to ensure buf data
	fcntl(fd, F_SETFL, O_NDELAY); // don't block serial read
	read(fd, sResult, 254);
	*sResult = NULL;

	//write
	char sCmd[254];
	sCmd[0] = 0x41;
	sCmd[1] = 0x42;
	sCmd[2] = 0x43;
	sCmd[3] = 0x00;

	if (writeport(fd, sCmd)) 
	{
		DEBUGMSG("write failed\n");
		close(fd);
		return FUN_FAIL;
	}

	DEBUGMSG("written:%s\n", sCmd);
	usleep(10000);

	//Read
	fcntl(fd, F_SETFL, FNDELAY); // don't block serial read

	iIn = read(fd, sResult, 254);
	sResult[iIn-1] = 0x00;

	if( (strcmp(sResult,"ABC")) == 0)
	{
		DEBUGMSG("readport= %s\n", sResult);
		close(fd);
		return RESULT_PASS;
	}
	else
	{
		DEBUGMSG("readport= %s\n", sResult);
		close(fd);
		return RESULT_FAIL;
	}
                  
	/*
	if (readport(fd,sResult)) 
	{
		DEBUGMSG("read failed\n");
		close(fd);
		return FUN_FAIL;
	}
	DEBUGMSG("readport=%s\n", sResult);
	close(fd);
	return FUN_SUCC;
	*/
}


int uart_each_other_directio(unsigned int port1,unsigned int port2)
{
	char c,cmd[16];
	unsigned int i;
	int res=FUN_FAIL;

	//init port1
	ioperm(port1,7,1);
	outb(0x00, port1 + 1); /* Turn off interrupts - Port1 */
	outb(0x80, port1 + 3 ); /* SET DLAB ON */
	outb(0x03, port1 + 0); /* default 0x03 = 38,400 BPS */
	outb(0x00, port1 + 1); /* Set Baud rate - Divisor Latch High Byte */
	outb(0x03, port1 + 3); /* 8 Bits, No Parity, 1 Stop Bit */
	outb(0xC7, port1 + 2); /* FIFO Control Register */
	outb(0x0B, port1 + 4); /* Turn on DTR, RTS, and OUT2 */

	//init port2
	ioperm(port2,7,1);
	outb(0x00, port2 + 1); /* Turn off interrupts - Port1 */
	outb(0x80, port2 + 3 ); /* SET DLAB ON */
	outb(0x03, port2 + 0); /* default 0x03 = 38,400 BPS */
	outb(0x00, port2 + 1); /* Set Baud rate - Divisor Latch High Byte */
	outb(0x03, port2 + 3); /* 8 Bits, No Parity, 1 Stop Bit */
	outb(0xC7, port2 + 2); /* FIFO Control Register */
	outb(0x0B, port2 + 4); /* Turn on DTR, RTS, and OUT2 */

    outb(0x41, port1);
    outb(0x42, port1);
    outb(0x43, port1);
    outb(0x00, port1);
	
 	for(i=0;i<4;i++)
	{ 
		usleep(10000);
		c = inb(port2 + 5);
	    if (c & 1) 
		{
			cmd[i] = inb(port2);
	     	//printf("COM4 get char=%c\r\n",cmd[i]);
		}
    }

	if( (cmd[0]==0x41)&&(cmd[1]==0x42)&&(cmd[2]==0x43)&&(cmd[3]==0x00) )
	{
		res = FUN_SUCC;
	}
	
	return res;
}


int uart_each_other_Test(char *dev1,char *dev2)
{
	int fd1,fd2;
	char sResult[254];
	
	fd1 = open(dev1, O_RDWR | O_NOCTTY | O_NDELAY);
	fd2 = open(dev2, O_RDWR | O_NOCTTY | O_NDELAY);

	if ( (fd1 == -1) || (fd2 == -1) )
	{
		DEBUGMSG("open_port: Unable to open dev");
		return FUN_FAIL;
	} 
	else
	{
		fcntl(fd1, F_SETFL, 0);
		fcntl(fd2, F_SETFL, 0);		
	}
	
	initport(fd1);
	initport(fd2);

	char sCmd[254];
	sCmd[0] = 0x41;
	sCmd[1] = 0x42;
	sCmd[2] = 0x43;
	sCmd[3] = 0x00;

	if (writeport(fd1, sCmd)) 
	{
		DEBUGMSG("write failed\n");
		close(fd1);
		close(fd2);		
		return FUN_FAIL;
	}

	DEBUGMSG("written:%s\n", sCmd);
	
	usleep(10000);

	fcntl(fd1, F_SETFL, FNDELAY); // don't block serial read
	fcntl(fd2, F_SETFL, FNDELAY); // don't block serial read

	if (readport(fd2,sResult)) 
	{
		DEBUGMSG("read failed\n");
		close(fd1);
		close(fd2);		
		return FUN_FAIL;
	}
	DEBUGMSG("readport=%s\n", sResult);
	close(fd1);
	close(fd2);		
	return FUN_SUCC;

}

/*************************************************************************************
	Audio Test Items
*************************************************************************************/
int wav_play(char *file)
{
   unsigned char buff[1024]; 

   int dev_fd, file_fd;
   ssize_t n, nRD, nWR;
   unsigned int format = AFMT_S16_LE;
   unsigned int channel = 2;
   unsigned int rate = 22050; 
 

   dev_fd = open("/dev/dsp", O_WRONLY);
   file_fd = open(file, O_RDONLY);
   ioctl(dev_fd, SNDCTL_DSP_SETFMT, &format);
   ioctl(dev_fd, SNDCTL_DSP_CHANNELS, &channel);
   ioctl(dev_fd, SNDCTL_DSP_SPEED, &rate); 

   while (1) 
   { 
          nRD = 0L;
          nRD = read(file_fd, buff, 1024);
          if (nRD <= 0)
                 break; 

          nWR = nRD;
          while (nWR > 0)
          { 
                 if ((n = write(dev_fd, buff + (nRD - nWR), nWR)) < 0)
                        break;
                 nWR -= n;
          }
   } 

   close(dev_fd);
   close(file_fd); 
}

/*************************************************************************************
	Timer Test Items

	TYPE :	TYPE_HUMAN_TIME, The real time scale for human. fox example, year=2009
	        TYPE_PC_TIME, The time scale for PC. for example, year=109(base on 1900)
*************************************************************************************/

int get_RTC(PSTRUCT_TEST_TIME pRTC,int TYPE)
{
    int fd, retval;
	struct rtc_time rtc_tm;
	
    fd = open("/dev/rtc", O_RDONLY);
    if (fd == -1) 
	{
            DEBUGMSG("get_RTC::open error\r\n");
            return FUN_FAIL;
    }

    /* Read the RTC time/date */
    retval = ioctl(fd, RTC_RD_TIME, &rtc_tm);
    if (retval == -1) 
	{
            DEBUGMSG("get_RTC::ioctl\r\n");
            return FUN_FAIL;
    }
    close(fd);

    pRTC->sec = rtc_tm.tm_sec;
    pRTC->min = rtc_tm.tm_min;
    pRTC->hour= rtc_tm.tm_hour;
    pRTC->day = rtc_tm.tm_mday;
	
	if(TYPE==TYPE_HUMAN_TIME)
	{
    	pRTC->mon = rtc_tm.tm_mon+1;
    	pRTC->year= rtc_tm.tm_year+1900;
	}
	else if(TYPE==TYPE_PC_TIME)
	{
    	pRTC->mon = rtc_tm.tm_mon;
    	pRTC->year= rtc_tm.tm_year;
	}
	
	return FUN_SUCC;
}


/*************************************************************************************
	SRAM Test Items

*************************************************************************************/

/*************************************************************************************
	Fill-in all datas of bank with value
*************************************************************************************/
/*
#define KBYTES	1024

int FillSRAM(const int bank,const int ksize,const char value)
{
	int i,res = FUN_SUCC;
	DSGPC_SRAM  sram;
	U08 buffer_w[KBYTES];

	memset(buffer_w,value,KBYTES);

	//switch bank
	sram.u08_bank = bank;
    sram.u32_subtarget = SRAM_BANK_SELECT;
    res = GPC_CTLCODE(GPC_SRAM,ACTION_SET,&sram);

    //fill in value
    for(i=0;i<ksize;i++)
    {
		sram.u32_subtarget = SRAM_RW;
		sram.u32_offset = i*KBYTES;
		sram.u32_size = KBYTES;
		sram.pu08_data = buffer_w;
		if(GPC_FUN_RES_SUCC != GPC_CTLCODE(GPC_SRAM,ACTION_SET,&sram))
		{
			res = FUN_FAIL;
			goto END;
		}
	}

END:
	return res;
}
*/

/*************************************************************************************
	Networking Test Items
*************************************************************************************/
unsigned short checksum(unsigned short *buffer, int size)
{
	/*~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned long	cksum = 0;
	/*~~~~~~~~~~~~~~~~~~~~~~*/

	while(size > 1)
	{
		cksum += *buffer++;
		size -= sizeof(unsigned short);
	}

	if(size)
	{
		cksum += *(unsigned char *) buffer;
	}

	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return (unsigned short) (~cksum);
}

int WaitForEchoReply(int socket)
{
	/*~~~~~~~~~~~~~~~~~~~~*/
	struct timeval	Timeout;
	fd_set			readfds;
	/*~~~~~~~~~~~~~~~~~~~~*/

	FD_ZERO(&readfds);
	FD_SET(socket, &readfds);
	Timeout.tv_sec = 1;
	Timeout.tv_usec = 0;

	return(select(socket + 1, &readfds, NULL, NULL, &Timeout));
}

void tv_sub(struct timeval *out, struct timeval *in)
{
	if((out->tv_usec -= in->tv_usec) < 0)
	{
		--out->tv_sec;
		out->tv_usec += 1000000;
	}

	out->tv_sec -= in->tv_sec;
}

int ping(char *host)
{
	int					res=FUN_FAIL;
	int					try_times=2;
	int					sockfd;
	int					nRet;
	struct sockaddr_in	addrDest;
	struct hostent		*Dest;
	float				spenttime;
	fd_set RecFd,ErrFd;
	struct timeval timeout ;
	ECHOREQUEST echoReq;
	int Seq = 0;

	if((Dest = gethostbyname(host)) == NULL)
	{

		/* get the host info */
		herror("gethostbyname");
		return FUN_FAIL;
	}

	//printf("Host name : %s\n", Dest->h_name);

	/*
	 * printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)Dest->h_addr)));
	 */
	if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
		return FUN_FAIL;		

	addrDest.sin_addr = *((struct in_addr *) Dest->h_addr);
	addrDest.sin_family = AF_INET;
	bzero(&(addrDest.sin_zero), 8);

	echoReq.icmpHdr.Type = ICMP_ECHO;
	echoReq.icmpHdr.Code = 0;
	echoReq.icmpHdr.ID = getpid();


	for(nRet = 0; nRet < REQ_DATASIZE; nRet++) echoReq.cData[nRet] = ' ' + nRet;

	while(try_times)
	{
		echoReq.icmpHdr.Seq = Seq++;

		echoReq.icmpHdr.Checksum = 0;
		gettimeofday(&echoReq.echoTime, NULL);
		echoReq.icmpHdr.Checksum = checksum((unsigned short *) &echoReq, sizeof(struct tagECHOREQUEST));
		if
		(
			sendto
				(
					sockfd,
					(struct ECHOREQUEST *) &echoReq,
					sizeof(struct tagECHOREQUEST),
					0,
					(struct sockaddr *) &addrDest,
					sizeof(addrDest)
				) < 0
		)
		{
			DEBUGMSG("sendto");
		}
		if(WaitForEchoReply(sockfd) == -1)
		{
			DEBUGMSG("select");
		}

		ECHOREPLY	icmpRecv;
		int			addr_len;


		timeout.tv_sec=1;
		timeout.tv_usec=0;
		FD_SET(sockfd, &RecFd);
		FD_SET(sockfd, &ErrFd);
		select(sockfd+1,&RecFd,NULL,&ErrFd,&timeout);

		addr_len = sizeof(struct sockaddr);

		if( FD_ISSET(sockfd, &RecFd))
		{
			if(recvfrom(sockfd,(struct ECHOREPLY *) &icmpRecv,
						sizeof(struct tagECHOREPLY),0,
						(struct sockaddr *) &addrDest,&addr_len)< 0)
			{
				DEBUGMSG("timeout!!\r\n");
				res = FUN_FAIL;
			}
			else if(icmpRecv.echoRequest.icmpHdr.Type == ICMP_ECHOREPLY)
			{
				DEBUGMSG("ping!!\r\n");
				res = FUN_SUCC;
			}
		}
		
		try_times--;
	}

	return res;
}

#define MAX_IFS 64

/*
int getMacAddr(char *szBuf)
{
    struct ifreq *ifr, *ifend;
    struct ifreq ifreq;
    struct ifconf ifc;
    struct ifreq ifs[MAX_IFS];
    int SockFD,cnt=0;
	char szTmp[256];

    SockFD = socket(AF_INET, SOCK_DGRAM, 0);


    ifc.ifc_len = sizeof(ifs);
    ifc.ifc_req = ifs;
    if (ioctl(SockFD, SIOCGIFCONF, &ifc) < 0)
    {
        DEBUGMSG("ioctl(SIOCGIFCONF): %m\r\n");
        return 0;
    }

	strcpy(szBuf,"");
    ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));
    for (ifr = ifc.ifc_req,cnt=0; ifr < ifend; ifr++,cnt++)
    {
    	if(!strcmp(ifr->ifr_name,"lo"))
    		continue;

		strcpy(szTmp,"");
        if (ifr->ifr_addr.sa_family == AF_INET)
        {
            strncpy(ifreq.ifr_name, ifr->ifr_name,sizeof(ifreq.ifr_name));
            if (ioctl (SockFD, SIOCGIFHWADDR, &ifreq) < 0)
            {
              DEBUGMSG("SIOCGIFHWADDR(%s): %m\r\n", ifreq.ifr_name);
              return 0;
            }

			sprintf(szTmp,"%02x:%02x:%02x:%02x:%02x:%02x",
			(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[0],
			(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[1],
			(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[2],
			(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[3],
			(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[4],
			(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[5]);

			if(0==((int)((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[0])&&
			  0==((int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[1])&&
			  0==((int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[2])&&
			  0==((int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[3])&&
			  0==((int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[4])&&
			  0==((int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[5]))
			{
				return FUN_FAIL;
			}
			if(cnt>0)
				strcat(szBuf," , ");			
	      	strcat(szBuf,szTmp);
 		}
    }

    return FUN_SUCC;
}
*/
int GetNetCardCount()
{
	int nLen,nCount = 0;
	char szLine[512];
	char szName[128];

	FILE *f=fopen("/proc/net/dev","r");

	if(!f)
	{
		DEBUGMSG("open /proc/net/dev fail!\r\n");
		return FUN_FAIL;
	}
	fgets(szLine,sizeof(szLine),f);
	fgets(szLine,sizeof(szLine),f);

	while(fgets(szLine,sizeof(szLine),f))
	{
		sscanf(szLine,"%s",szName);
		nLen = strlen(szName);
		if(nLen <= 0)
			continue;
		if(szName[nLen-1] == ":")
			szName[nLen-1]=0;
		if(strcmp(szName,"lo:")==0)
			continue;

		nCount++;
	}

	fclose(f);
	f = NULL;

	return nCount;
}

int getMacAddr(char *szBuf)
{
    struct ifreq ifr_mac;
    int i,sock_mac,ethcnt=0;
	char szEth[128];
	char szTmp[256];

    sock_mac = socket(AF_INET, SOCK_STREAM, 0);

	if(sock_mac == -1)
	{
		DEBUGMSG("create socket fail!\r\n");
		return FUN_FAIL;
	}

	ethcnt=GetNetCardCount();
	strcpy(szBuf,"");
	for(i=0;i<ethcnt;i++)
	{
		strcpy(szTmp,"");
		memset(&ifr_mac,0,sizeof(ifr_mac));
		sprintf(szEth,"eth%d",i);
		strncpy(ifr_mac.ifr_name, szEth, sizeof(ifr_mac.ifr_name)-1);

	    if (ioctl (sock_mac, SIOCGIFHWADDR, &ifr_mac) < 0)
	    {
	      DEBUGMSG("Get MacAddress ioctl fail!\r\n");
	      return FUN_FAIL;
	    }

		sprintf(szTmp,"%02x:%02x:%02x:%02x:%02x:%02x,",
		(int) ((unsigned char *) &ifr_mac.ifr_hwaddr.sa_data)[0],
		(int) ((unsigned char *) &ifr_mac.ifr_hwaddr.sa_data)[1],
		(int) ((unsigned char *) &ifr_mac.ifr_hwaddr.sa_data)[2],
		(int) ((unsigned char *) &ifr_mac.ifr_hwaddr.sa_data)[3],
		(int) ((unsigned char *) &ifr_mac.ifr_hwaddr.sa_data)[4],
		(int) ((unsigned char *) &ifr_mac.ifr_hwaddr.sa_data)[5]);

		strcat(szBuf,szTmp);
	}

    return FUN_SUCC;
}


//DK 2016.11.29
int getMacAddr2(char *szBuf)
{
    int i,ethcnt=0;
	char szEth[128],szEth1[128];
	FILE *fp;
	
	memset(szEth,0,128);
	memset(szEth1,0,128);
//#if defined(__SERVER_V1__)
//	ethcnt=GetNetCardCount()-1;	//-1 for usb wifi
//#else
	ethcnt=GetNetCardCount();
//#endif
	//printf("ethcnt = %d\r\n",ethcnt);
	system("ifconfig | grep -o '[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]' > ./data/mac_addr");

	fp = fopen("./data/mac_addr","r+");
	fgets(szEth,18,fp);
	strcat(szEth, ",");
	//printf("szEth = %s\r\n",szEth);

	if(ethcnt >= 2)
	{
		fseek(fp,18,SEEK_SET);
		fgets(szEth1,18,fp);
		strcat(szEth,szEth1);
		//printf(" >=2 : after strcat szEth= %s\r\n",szEth);
	}
	strcpy(szBuf,szEth);
		
	fclose(fp);	
    return FUN_SUCC;
}

/*************************************************************************************
	Mount all device to dest_folder,in which
	
	  prefixstr: Prefix string for device name in /dev, ex 'sd'
	dest_folder: Destination folder used for mounting all devices
	    devnum:  numbers of device will be mounted

	Note: This function will create all folder automaticly and will not delete
	      after test finish.
*************************************************************************************/
int mountAllStorage(char *prefixstr,char *dest_folder,U08 devnum,U08 skipnum)
{
	int i,res=FUN_SUCC;
	U08	szBuf[512],szDevname[256],szPath[1024];

	if(devnum<=0)
		return FUN_FAIL;
	
	//creat destnation folder
	if(access(szBuf,F_OK)) //didn't find folder
	{
		sprintf(szBuf,"mkdir %s > /dev/null 2>&1",dest_folder);
		//printf(szBuf);		
		system(szBuf);
	}

	for(i=0;i<devnum;i++)
	{
		if(i==skipnum)
			continue;
		
		//mount all device
		sprintf(szDevname,"%s%c1",prefixstr,'a'+i);
		sprintf(szBuf,"%s/%s",dest_folder,szDevname);
		if(access(szBuf,F_OK)) //didn't find folder
		{
			sprintf(szBuf,"mkdir %s/%s > /dev/null 2>&1",dest_folder,szDevname);
			system(szBuf);
		}
		sprintf(szBuf,"mount /dev/%s %s/%s > /dev/null 2>&1",szDevname,dest_folder,szDevname);
		//printf(szBuf);
		system(szBuf);
	}
	return res;
}

int unmountAllStorage(char *prefixstr,char *dest_folder,U08 devnum)
{
	int i,res=FUN_SUCC;
	U08	szBuf[512],szDevname[256];

	if(devnum<=0)
		return FUN_FAIL;
	
	for(i=0;i<devnum;i++)
	{
		//mount all device
		sprintf(szDevname,"%s%c1",prefixstr,'a'+i);
		sprintf(szBuf,"umount %s/%s > /dev/null 2>&1",dest_folder,szDevname);
		system(szBuf);
	}
	
	return res;
}

/*
int FindFile(char *path,char *file)  
{  
	int res=FUN_FAIL;
	DIR *hpDir;
	struct dirent *currFP = 0;

	hpDir = opendir(path);

	if (hpDir == NULL)
	{
		return FUN_FAIL;
	}

	while ((currFP = readdir(hpDir)) != 0)
	{
		if (!strcmp(currFP->d_name,file))
		{
			res=FUN_SUCC;
			break;
		}
	}
	
	return res;
}
*/

int FindFile(char *path,char *file)  
{  
    DIR              *pDir ;  
    struct dirent    *ent  ;  
    int               res=FUN_FAIL,i=0  ;  
    char              childpath[2048];  
	char			  szFile[512];

    pDir=opendir(path);  
    memset(childpath,0,sizeof(childpath));  


	//only srarching root path
    while((ent=readdir(pDir))!=NULL)  
    {  
        //if(ent->d_type & DT_DIR)  
        {  

            if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)  
                    continue;  

            sprintf(childpath,"%s/%s",path,ent->d_name);  
			sprintf(szFile,"%s/%s",childpath,file);
			if(!access(szFile,F_OK)) //find file
			{
				res=FUN_SUCC;
				break;
			}
			//else
			//{
            //	FindFile(childpath,file);
			//}
        }  
    }  

	return res;
} 

