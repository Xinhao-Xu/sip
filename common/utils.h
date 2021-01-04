#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#define ATTRIB_NONE				0
#define ATTRIB_ASK_RETRY		0x0001
#define ATTRIB_ASK_PASS_FAIL	0x0002

#define FUN_SUCC	0
#define FUN_FAIL	1

#define TYPE_HUMAN_TIME		1
#define TYPE_PC_TIME		2

#define MAX_BUF_SIZE	128

#define MSGBOX_PAUSE	1
#define MSGBOX_NONPAUSE 0


/*************************************************************************************


								Macros

	
*************************************************************************************/

#define HIDDEN_CURSES()	system("echo -e \"\\033[?25l\"")
#define SHOW_CURSES()	system("echo -e \"\\033[?25h\"")

/*************************************************************************************


								Data Struct

	
*************************************************************************************/

typedef struct _tagDMIINFO_CPU
{
	char		manufac[MAX_BUF_SIZE];
	char		id[MAX_BUF_SIZE];
	char		ver[MAX_BUF_SIZE];
	float		voltage;		//unit: v
	int			ext_clock;		//unit: MHz
	int			max_speed;		//unit: MHz
	int			curSpeed;		//unit: MHz
	
}DMIINFO_CPU,*PDMIINFO_CPU;

typedef struct _tagDMIINFO_MEMORY
{
	int			size;			//unit: MB
	int			freq;			//unit: MHz
	char 		locator[MAX_BUF_SIZE];	//DIMMx
	char 		type[MAX_BUF_SIZE];		//ddr,ddr3...
}DMIINFO_MEMORY,*PDMIINFO_MEMORY;

typedef struct _tagDMIINFO
{
	DMIINFO_CPU		cpuinfo;
	DMIINFO_MEMORY	meminfo;  //memory device 1
	DMIINFO_MEMORY	meminfo2; //memory device 2
}DMIINFO,*PDMIINFO;

typedef struct _tagSENSOR_INFO
{
	float		p2_5v;			//+2.5V
	float		vcore;			//VCore
	float		p3_3v;			//+3.3V
	float		p5v;			//+5V
	float		p12v;			//+12v
	float		vcc;			//VCC SRAM
	float		p1_5v;			//+1.5V
	float		p1_8v;			//+1.8V
	float		chip_temp;		//Chip Temp
	float		cpu_temp;		//CPU Temp
	float		sys_temp;		//System Temp
	float		cpu0_vid;		//CPU0 VID
	float		sys_bat;		//voltage of system battery
	int			fan1;			//Fan1 speed, unit: RPM
	int			fan2;			//Fan2 speed, unit: RPM
}SENSOR_INFO,*PSENSOR_INFO;

typedef struct _tagUSBINFO
{
	char *vid;
	char *pid;
}USBINFO,*PUSBINFO;

typedef struct _tagHDINFO
{
	char *dev;
	char id[MAX_BUF_SIZE];
}HDINFO,*PHDINFO;


typedef struct _tagSTRUCT_TEST_TIME
{
    unsigned int    totalsec;
    int     		sec;
    int     		min;
    int    			hour;
    int     		day;
    int     		mon;
    int     		year;
}STRUCT_TEST_TIME,*PSTRUCT_TEST_TIME;

/*************************************************************************************


								Function prototype

	
*************************************************************************************/

int attrib_action(int attrib,int x,int y);
int ask_reslut(int x,int y);
void check_yes();
void check_no();
int checkno_nowait();
int display_num(int x, int y, int num, int len);

/*************************************************************************************
	Common function for Utilities
*************************************************************************************/
int initUtil();

/*************************************************************************************
	System Information Items
*************************************************************************************/
int get_dmiinfo(PDMIINFO pInfo);
int get_sensorinfo(PSENSOR_INFO pInfo);

/*************************************************************************************
	Storage Test Items
*************************************************************************************/
int get_HDInfo(PHDINFO pInfo);

/*************************************************************************************
	UART Test Items
*************************************************************************************/
int uart_loopback_Test(char *dev);

/*************************************************************************************
	Timer Test Items
*************************************************************************************/
int get_RTC(PSTRUCT_TEST_TIME pRTC,int TYPE);


/*************************************************************************************
	SRAM Test Items
*************************************************************************************/
int FillSRAM(const int bank,const int ksize,const char value);

#endif
