#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/io.h>


#include "Debug.h"
#include "gpclib.h"
#include "eitlib.h"
#include "Diag.h"
#include "Result.h"
#include "FixedTest.h"
#include "Test_items.h"
#include "utils.h"
#include "ExtMode.h"
#include "UI.h"
#include "SIO.h"
#include "tcil.h"

int auto_final = 0;		//flag for auto test finish or not
extern DIAGINFO			g_diaginfo;
TEST_ARGU	g_testargu;

/*
typedef struct _tagTEST_ARGU
{
	char	cpu_name[64];
	U08		barcode_comnum;
	U08     dip_nums;
	U32		sram_size;
	U08		sram_banknum;
	U32		dram_size;
	U32		battery_upval;
	U32		battery_downval;
	U08     fan1_duty;
	U08     fan2_duty;
	U08     fan3_duty;
	int		audio_hw;
	U32		sip_uuid;
}TEST_ARGU,*PTEST_ARGU;
*/

typedef struct _tagTimeCompare
{
	struct timeval  tv1;
	struct timeval  tv2;
	struct timeval  tvdiff;
	sem_t			sem_finish;	
}TimeCompare,*PTimeCompare;

pthread_mutexattr_t g_mutex_pic; //for avoid threads to paint screen at the same time

TimeCompare g_tc;

#define PATH_PIC_GAMEIO		"./pics/gameio/"
#define PATH_PIC_BTNDWN		PATH_PIC_GAMEIO 	"button_down.jpg"
#define PATH_PIC_BTNUP		PATH_PIC_GAMEIO 	"button_up.jpg"
#define PATH_PIC_DIPDWN		PATH_PIC_GAMEIO 	"dip_down.jpg"
#define PATH_PIC_DIPUP		PATH_PIC_GAMEIO 	"dip_up.jpg"
#define PATH_PIC_DIPOFF		PATH_PIC_GAMEIO 	"dip_off.jpg"
#define PATH_PIC_BACKGROUND PATH_PIC_GAMEIO 	"board/gameio_000.3.jpg"
#define PATH_PIC_FRAMEBTN   PATH_PIC_GAMEIO 	"frame_button.jpg"
#define PATH_PIC_FRAMEDIP   PATH_PIC_GAMEIO 	"frame_dip.jpg"
#define PATH_PIC_IBTNNOTP   "./pics/800_600/ibuttontest0.jpg"				//ibutton not present
#define PATH_PIC_IBTNP	    "./pics/800_600/ibuttontest1.jpg"				//ibutton present

#define MASK_ITRVALUE			0x1F		//default ITR value for all 4 bits down
#define MASK_ITRVALUE_LOGBOX	0x08
#define MASK_ITRVALUE_BILLDOOR	0x01
#define MASK_ITRVALUE_CASHDOOR	0x10
#define MASK_ITRVALUE_MAINDOOR	0x02

#define GAMEIO_TYPE_BTN		1
#define GAMEIO_TYPE_DIP		2
#define GAMEIO_TYPE_LAMP	3

#define TPM_VERSION_STR		"01010000"

static U08 g_itrup=0,g_itrdown=0;

//Alan added for led
int LED0_OFFSET = 0xD8;
int LED1_OFFSET = 0xB8;
int LED2_OFFSET = 0x90;
int LED3_OFFSET = 0x98;
int GPC_IO_OFFSET = 0xE000; //for gmb8651

#define CFG_INDEX_ADR 0x0CF8
#define CFG_DATA_ADR  0x0CFC
#define CFG_LPC_BRIDGE 0
#define VID 0x8086
#define DID 0x8C5C
#define IO_BASE_REG_ADR 0x90
#define GPC_IO_OFFSET_MASK 0xFF
#define MEM_BASE_REG_ADR 0x98
#define GPC_VID 0x1573
#define TUL_VID 0x425A



void IO_OUT(int adr, int data)
{
    outb(data, adr);
    //outp() data is 2 byte in dos.
}
void IO_OUT32(int adr, int data)
{
    outl(data, adr);
}
int IO_IN(int adr)
{
    int data;
    data = inb(adr);
    //inp() data is 2 byte in dos.

    return data;
}
int IO_IN32(int adr)
{
    int data;
    data = inl(adr);
    return data;
}
void SET_SPEED(char LED_SPEED)
{
    IO_OUT(GPC_IO_OFFSET+LED0_OFFSET+0, 0x80);
    IO_OUT(GPC_IO_OFFSET+LED0_OFFSET+1, LED_SPEED);
    IO_OUT(GPC_IO_OFFSET+LED0_OFFSET+0, 0x00);
    IO_OUT(GPC_IO_OFFSET+LED1_OFFSET+0, 0x80);
    IO_OUT(GPC_IO_OFFSET+LED1_OFFSET+1, LED_SPEED);
    IO_OUT(GPC_IO_OFFSET+LED1_OFFSET+0, 0x00);
    IO_OUT(GPC_IO_OFFSET+LED2_OFFSET+0, 0x80);
    IO_OUT(GPC_IO_OFFSET+LED2_OFFSET+1, LED_SPEED);
    IO_OUT(GPC_IO_OFFSET+LED2_OFFSET+0, 0x00);
    IO_OUT(GPC_IO_OFFSET+LED3_OFFSET+0, 0x80);
    IO_OUT(GPC_IO_OFFSET+LED3_OFFSET+1, LED_SPEED);
    IO_OUT(GPC_IO_OFFSET+LED3_OFFSET+0, 0x00);  
    
}


int PCI_Scan()
{
    int bus, device, function, reg, index;
    unsigned int data;
    int CFG_EN = 0x80000000;
    int GPC_INT_OFFSET = 0xA8;
    int I2C_OFFSET = 0xC8;
    int SPI_int_baseadd = 0x0;
    int SPI_OFFSET = 0xC0;
    int i2c_int_baseadd = 0x0;
	int GPC_MEM_OFFSET = 0xDF400000;


    for (bus = 0; bus < 256; bus++)
    {
        for (device = 0; device < 32; device++)
        {
            for (function = 0; function < 8; function++)
            {
                for (reg = 0; reg < 1; reg++)
                {
                    index = CFG_EN | (bus << 16) | (device << 11) | (function << 8) | (reg << 2);
                    IO_OUT32(CFG_INDEX_ADR, index);
                    data = IO_IN32(CFG_DATA_ADR);
                    if (CFG_LPC_BRIDGE)
                    {
                        if ((data & 0xFFFF) == VID && ((data >> 16) & 0xFFFF) == DID)
                        {
                            //printf("CFG_INDEX_ADR = %x\n", index);
                            //printf("DEVICE = %u\n", device);
                            //printf("FUNCTION = %u\n", function);
                            //printf("VID = %x\n", data & 0xFFFF);
                            //printf("PID = %x\n", (data >> 16) & 0xFFFF);

                            reg = IO_BASE_REG_ADR;
                            index = CFG_EN | (bus << 16) | (device << 11) | (function << 8) | (reg);
                            data = 0x1 | (GPC_IO_OFFSET_MASK << 16) | GPC_IO_OFFSET;
                            IO_OUT32(CFG_INDEX_ADR, index);
                            IO_OUT32(CFG_DATA_ADR, data);
                            //printf("CFG_INDEX_ADR = %x\n", index);
                            //printf("CFG_DATA_ADR = %x\n", data);

                            reg = MEM_BASE_REG_ADR;
                            index = CFG_EN | (bus << 16) | (device << 11) | (function << 8) | (reg);
                            data = 0x1 | GPC_MEM_OFFSET;
                            IO_OUT32(CFG_INDEX_ADR, index);
                            IO_OUT32(CFG_DATA_ADR, data);
                            // printf("CFG_INDEX_ADR = %x\n", index);
                            // printf("CFG_DATA_ADR = %x\n", data);
                        }
                    }
                    else
                    {
                        if ((data & 0xFFFF) == GPC_VID || (data & 0xFFFF) == TUL_VID)
                        {
                            // printf("DEVICE = %u\n", device);
                            // printf("FUNCTION = %u\n", function);
                            // printf("VID = %x\n", data & 0xFFFF);
                            // printf("PID = %x\n", (data >> 16) & 0xFFFF);

                            reg = 4;
                            index = CFG_EN | (bus << 16) | (device << 11) | (function << 8) | (reg << 2);
                            IO_OUT32(CFG_INDEX_ADR, index);
                            GPC_MEM_OFFSET = IO_IN32(CFG_DATA_ADR);
                            //printf("GPC_MEM_BASE = %x\n", GPC_MEM_OFFSET);

                            reg = 5;
                            index = CFG_EN | (bus << 16) | (device << 11) | (function << 8) | (reg << 2);
                            IO_OUT32(CFG_INDEX_ADR, index);
                            GPC_IO_OFFSET = IO_IN32(CFG_DATA_ADR) & 0xFFFE;
                            //printf("GPC_IO_BASE = %x\n", GPC_IO_OFFSET);

                            if ((data & 0xFFFF) == GPC_VID)
                            {
                                GPC_INT_OFFSET = 0xA8;
                                I2C_OFFSET = 0xC8;
                                SPI_int_baseadd = 0xA8;
                                SPI_OFFSET = 0xC0;
                                i2c_int_baseadd = 0xA8;
                            }
                            else
                            {
                                GPC_INT_OFFSET = 0xA8;
                                I2C_OFFSET = 0xD8;
                                SPI_int_baseadd = 0xA8;
                                SPI_OFFSET = 0x88;
                                i2c_int_baseadd = 0xA8;
                            }
                        }
                    }
                }
            }
        }
    }
}



//

/*************************************************************************************
	System  
*************************************************************************************/
void tcil_test(int argc,char *argv[])
{
	//printf("fun_T01006001[%s]\r\n",fun_T01006001()==RESULT_PASS?"Pass":"Fail");
	fun_T02017026();
	fun_T02017027();
	fun_T02017028();
	fun_T02017029();
	fun_T02017030();	
}

void showjpeg_lock(int start_x,int start_y,char *filename)
{
	pthread_mutex_lock(&g_mutex_pic);
	showjpeg(start_x,start_y,filename);
	pthread_mutex_unlock(&g_mutex_pic);	
}

void tcil_callbackfun(U32 cb_type, U16 cb_src, U16 cb_data, void *context)
{
	//printf("cb_type=0x%08X,cb_src=0x%08x,cb_data=0x%08x\r\n",cb_type,cb_src,cb_data);	
}

int init_tcil()
{
	int i,res;
	unsigned char reg;
	DSGPC_LIB	libsetting;
	DSEIT_LIB_SETTING	 eitlib;
	pthread_mutexattr_t mAttr;
	DSGPC_PORT port;

	//mount all devices
	mountAllStorage("sd","/mnt/sipdevs",11,0xFF);

	// setup recursive mutex for mutex attribute
	pthread_mutexattr_settype(&mAttr, PTHREAD_MUTEX_RECURSIVE_NP);
	// Use the mutex attribute to create the mutex
	pthread_mutex_init(&g_mutex_pic, &mAttr);
	// Mutex attribute can be destroy after initializing the mutex variable
	pthread_mutexattr_destroy(&mAttr);		

  	//initiate gpc library 3800
	libsetting.u32_subtarget = GPC_LIB_INIT;
	res = GPC_CTLCODE(GPC_LIB,ACTION_SET,&libsetting);
	if(res != GPC_FUN_RES_SUCC)

	{
	  DEBUGMSG("GPC_LIB_SETTING_INIT = %d !!!\r\n", res);
	}

	//initiate EIT Library
	eitlib.u32_subtarget = EIT_LIB_SETTING_INIT;		
	eitlib.u32_interface = EIT_INTERFACE_GPC;		
	//eitlib.u08_eittype = TYPE_EIT020A;		
	//eitlib.u08_battype = TYPE_BATTERY_SANYO14430;		
	res = EIT_CTLCODE(EIT_LIB_SETTING,ACTION_SET,&eitlib); 
	if(res != EIT_FUN_RES_SUCC)
	{
	  DEBUGMSG("EIT_LIB_SETTING_INIT = %d !!!\r\n", res);
	}
	
	//SET CALLBACK
	libsetting.u32_subtarget =  GPC_LIB_CALLBACKFUN;
	libsetting.int_callback_fun = tcil_callbackfun;
	res = GPC_CTLCODE(GPC_LIB,ACTION_SET,&libsetting);
	if(res != GPC_FUN_RES_SUCC)
		printf("Please Check Callback function \n");

	//GPC 3800 Init input Port Access 20160314
	port.u32_subtarget = PORT_ACTIVE;
	port.u32_portnum = 0x3FFF;
	port.u08_onoff = SET_ON;
	port.u08_data = 0xFF;
	res = GPC_CTLCODE(GPC_CFG_PORT, ACTION_SET,&port);
	DEBUGMSG("Enable all intput ports,res= 0x%X\r\n", res);

	//Setting debounce for all ports
	for(i=0;i<=13;i++)
	{
		port.u32_subtarget = PORT_DEBOUNCE;
		port.u32_portnum = 1<<i;
		port.u08_debounce = 50+i;
		res = GPC_CTLCODE(GPC_CFG_PORT,ACTION_SET,&port);
		DEBUGMSG("Setting debounce of all intput ports,res= 0x%X\r\n", res);
	}


	//init super io
	SIO_HW_Init(SIO_CFG_ADDR1,SIO_DAT_ADDR1,0x60,0x61,5);
	SELECT_SIO_LOGICAL_DEV(0x04); //switch to HWM logic device
	CFG_SIO_REG(0x30);	//active HWM
	WRITE_SIO_DATA(0x01);
	g_diaginfo.bFinalDipswitch = false;
	sem_init(&g_tc.sem_finish, 0, 0); 
	
	return FUN_SUCC;
}

int deinit_tcil()
{

	SIO_HW_DeInit();
	pthread_mutex_destroy(&g_mutex_pic);

	//umount all devices
	unmountAllStorage("sd","/mnt/sipdevs",11);
	return FUN_SUCC;
}

int get_argument(int argc,char *argv[])
{
	char* input_file = NULL ;
	char *query=NULL;
	char opt_char=0;
	dictionary  *ini ;
	char        *s ;
	char		hwsec[64],key[128];
	char 		arg_uuid[512];
	memset(&g_testargu,0,sizeof(TEST_ARGU));
	//set section to default
	strcpy(hwsec,SEC_HWINFO);

    while ((opt_char = getopt(argc, argv, "s:S:")) != -1)
    {
        switch(opt_char)
        {
            case 's':	//selec section
            case 'S':
            	//strcpy(hwsec,optarg);
            	break;

		    default:
        		break;
        }
    }
	DEBUGMSG("HWINFO section=%s\r\n",hwsec);
	
    //ini = iniparser_load(FILE_SIPARGU);
    ini = iniparser_load("sip.ini");
	if (ini==NULL) 
	{        
		fprintf(stderr, "Cannot open setting file[%s]\r\n",FILE_SIPARGU);
		return -1 ;
	}

	//get CPU name string
	SETKEY(key,hwsec,ARGU_CPUNAME);
	s = iniparser_getstring(ini,key, NULL);
	strcpy(g_testargu.cpu_name,s);
	DEBUGMSG("%s = %s\r\n",ARGU_CPUNAME,s);

	//get DRAM size
	SETKEY(key,hwsec,ARGU_DRAM_SIZE);
	s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_DRAM_SIZE,s);
	g_testargu.dram_size = atoi(s);

	//get SRAM size
	SETKEY(key,hwsec,ARGU_SRAM_SIZE);
	s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_SRAM_SIZE,s);
	g_testargu.sram_size = atoi(s);

	//get SRAM Bank numbers
	SETKEY(key,hwsec,ARGU_SRAM_BANKNUM);
	s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_SRAM_BANKNUM,s);
	g_testargu.sram_banknum = atoi(s);

	//get DIP Switch pin numbers
	SETKEY(key,hwsec,ARGU_DIP_NUMS)	;
	s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_DIP_NUMS,s);
	g_testargu.dip_nums = atoi(s);

	//20160330 DK add fan duty control for gpc3800
	//get Fan1~3 duty control value
	SETKEY(key,hwsec,ARGU_FAN1_DUTY)	;
	s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_FAN1_DUTY,s);
	g_testargu.fan1_duty = atoi(s);

	SETKEY(key,hwsec,ARGU_FAN2_DUTY)	;
	s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_FAN2_DUTY,s);
	g_testargu.fan2_duty = atoi(s);

	SETKEY(key,hwsec,ARGU_FAN3_DUTY)	;
	s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_FAN3_DUTY,s);
	g_testargu.fan3_duty = atoi(s);

	//20181225 ErrandMan added fan rpm definition.
	SETKEY(key,hwsec,ARGU_FAN1_RPM)	;
    s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_FAN1_RPM,s);
	g_testargu.fan1_rpm = atoi(s);
	
	SETKEY(key,hwsec,ARGU_FAN2_RPM)	;
    s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_FAN2_RPM,s);
	g_testargu.fan2_rpm = atoi(s);
	
	SETKEY(key,hwsec,ARGU_FAN3_RPM)	;
    s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_FAN3_RPM,s);
	g_testargu.fan3_rpm = atoi(s);

	//20161128 DK add audio hw num determine
	SETKEY(key,hwsec,ARGU_AUDIO_HW)	;
	s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_AUDIO_HW,s);
	g_testargu.audio_hw = atoi(s);

	//20200521 Alan added
	SETKEY(key,hwsec,ARGU_FAN1_UPPER);
   	 s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("ARGU_FAN1_UPPER = %s\r\n",s);
	g_testargu.fan1_upper = atoi(s);
	
	SETKEY(key,hwsec,ARGU_FAN1_LOWER);
   	 s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("ARGU_FAN1_LOWER = %s\r\n",s);
	g_testargu.fan1_lower = atoi(s);

	SETKEY(key,hwsec,ARGU_FAN2_UPPER);
   	 s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("ARGU_FAN2_UPPER = %s\r\n",s);
	g_testargu.fan2_upper = atoi(s);
	
	SETKEY(key,hwsec,ARGU_FAN2_LOWER);
   	 s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("ARGU_FAN2_LOWER = %s\r\n",s);
	g_testargu.fan2_lower = atoi(s);
	
	SETKEY(key,hwsec,ARGU_FAN3_UPPER);
   	 s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("ARGU_FAN3_UPPER = %s\r\n",s);
	g_testargu.fan3_upper = atoi(s);
	
	SETKEY(key,hwsec,ARGU_FAN3_LOWER);
   	 s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("ARGU_FAN3_LOWER = %s\r\n",s);
	g_testargu.fan3_lower = atoi(s);

	SETKEY(key,hwsec,ARGU_FAN1_FULL_THRESHOLD);
   	 s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("FAN1_FULL_THRESHOLD = %s\r\n",s);
	g_testargu.fan1_full_threshold = atoi(s);
	
	SETKEY(key,hwsec,ARGU_FAN2_FULL_THRESHOLD);
   	 s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("FAN2_FULL_THRESHOLD = %s\r\n",s);
	g_testargu.fan2_full_threshold = atoi(s);
	
	SETKEY(key,hwsec,ARGU_FAN3_FULL_THRESHOLD);
   	 s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("FAN3_FULL_THRESHOLD = %s\r\n",s);
	g_testargu.fan3_full_threshold = atoi(s);
	//

	//20200302 Alan added for MAC address check
	SETKEY(key,hwsec,ARGU_HWADDR1);
	s = iniparser_getstring(ini,key, NULL);
	strcpy(g_testargu.hwaddr1, s);
	DEBUGMSG("%s = %s\r\n",ARGU_HWADDR1,s);

	SETKEY(key,hwsec,ARGU_HWADDR2);
	s = iniparser_getstring(ini,key, NULL);
	strcpy(g_testargu.hwaddr2, s);
	DEBUGMSG("%s = %s\r\n",ARGU_HWADDR2,s);
	//
	

	g_testargu.sip_uuid = NULL;
	SETKEY(key,hwsec,ARGU_SIP_UUID) ;
	s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_SIP_UUID,s);
	//printf("%s = %s\r\n",ARGU_SIP_UUID,s);
	if(s != NULL)
	{
		g_testargu.sip_uuid = atoi(s);
		//printf("get_argument():uuid exist, g_testargu.sip_uuid = %ld\r\n",g_testargu.sip_uuid);
	}


	//get EIT battery upper limit value
	SETKEY(key,hwsec,ARGU_EIT_BATUP)	;
    s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_EIT_BATUP,s);
	g_testargu.battery_upval= atoi(s);

	//get EIT battery down limit value
	SETKEY(key,hwsec,ARGU_EIT_BATDW)	;
    s = iniparser_getstring(ini,key, NULL);
	DEBUGMSG("%s = %s\r\n",ARGU_EIT_BATDW,s);
	g_testargu.battery_downval= atoi(s);

	iniparser_freedict(ini);

	//we only generate UUID once when UUID is not in sip.ini
	if(g_testargu.sip_uuid == NULL)
	{
		DEBUGMSG("sip.ini: SIP_UUID = NULL, create new uuid:\r\n");
		g_testargu.sip_uuid = rand_int(10000,99999);
		sprintf(arg_uuid,"%ld",g_testargu.sip_uuid); 
		FILE *fp = fopen("sip.ini","r+");
		fseek(fp,0,SEEK_END);
		fprintf(fp,"SIP_UUID = %s\r\n",arg_uuid);
		fclose(fp);
	}

}

int get_SIOinfo(PSENSOR_INFO pInfo)
{
	//for super IO F81865
	CFG_HWMON_REG(0x20);
	pInfo->p3_3v = READ_HWMON_DATA()*16;

	CFG_HWMON_REG(0x21);
	pInfo->vcore = READ_HWMON_DATA()*8;

	CFG_HWMON_REG(0x22);
	pInfo->p12v = READ_HWMON_DATA()*8*11; //plus 11 for its shared voltage

	CFG_HWMON_REG(0x23);
	pInfo->p5v = READ_HWMON_DATA()*8*4;   //plus 4 for its shared voltage

	CFG_HWMON_REG(0x24);
	pInfo->vcc = READ_HWMON_DATA()*8*4;   //plus 4 for its shared voltage

	CFG_HWMON_REG(0x26);
	pInfo->sys_bat = READ_HWMON_DATA()*16;

	CFG_HWMON_REG(0x72);
	pInfo->cpu_temp = READ_HWMON_DATA();

	CFG_HWMON_REG(0x74);
	pInfo->sys_temp = READ_HWMON_DATA();
	/*
	printf("\r\nget_SIOinfo:p3_3v=%f\r\n",pInfo->p3_3v);
	printf("get_SIOinfo:vcore=%f\r\n",pInfo->vcore);
	printf("get_SIOinfo:p12v=%f\r\n",pInfo->p12v);
	printf("get_SIOinfo:p5v=%f\r\n",pInfo->p5v);
	printf("get_SIOinfo:Vcc=%f\r\n",pInfo->vcc);
	printf("get_SIOinfo:sys_bat=%f\r\n",pInfo->sys_bat);
	printf("get_SIOinfo:cpu_temp=%f\r\n",pInfo->cpu_temp);
	printf("get_SIOinfo:sys_temp=%f\r\n",pInfo->sys_temp);
	*/
}

//mSATA1
int fun_T01001001()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_msata1"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//SD Card1
int fun_T01001002()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_sd1"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//TPM
int fun_T01001003()
{
	int res=RESULT_FAIL;
	char szBuf[256];
	FILE *fp;
	
	sprintf(szBuf,"tpm_version");
	fp = popen(szBuf,"r");

	while((fgets(szBuf, 256, fp)) != NULL)
	{
		if(strstr(szBuf,TPM_VERSION_STR))
		{
			DEBUGMSG("TPM Found!!,[%s]\r\n",szBuf);
			res=RESULT_PASS;
			break;
		}
	}
	pclose(fp);
	
	return res;
}

//Product ID
int fun_T01002001()
{
	CPU_TEST Info;
		
	//Datas for CPU test
	//Info.szCPUInfo = "AMD G-T48E Proces";
	DEBUGMSG("Info.szCPUInfo = %s\r\n",Info.szCPUInfo);
	Info.szCPUInfo = g_testargu.cpu_name;
	DEBUGMSG("fun_T01002001:  Info.szCPUInfo = %s\r\n",Info.szCPUInfo);
	//Info.speed = 1400;	//MHz
	//Info.range = 100;	//MHz
	
	return fun_TestCPU_002(&Info);
}

//DRAM
int fun_T01003001()
{
	DEBUGMSG("fun_T01003001(),g_testargu.dram_size=%dMB\r\n",g_testargu.dram_size);
	return fun_TestMemory_003(g_testargu.dram_size);
}

//IDE0
int fun_T01004001()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_001("/dev/sda1","/mnt/ide0"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//IDE1
int fun_T01004002()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_001("/dev/sdb1","/mnt/ide1"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//SATA1
int fun_T01015001()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_sata1"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//SATA2
int fun_T01015002()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_sata2"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//SATA3
int fun_T01015003()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_sata3"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//SATA4
int fun_T01015004()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_sata4"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//dio OnBoard Storage
int fun_T01015005()
{	
	const char *fp= "./data/disk";
	int res = RESULT_FAIL;
	system("fdisk -l > data/disk");
	if(FUN_SUCC == OnBoard_Storage01(20,"0x00000000",fp))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;
	return res;
}

//M.2 by ErrAndMan 20190312
int fun_T01015006()
{	
	const char *file= "./data/disk";
	char *key = "nvme0n1";
	int res = RESULT_FAIL;
	char ssbuf[9];
	
	system("fdisk -l | grep -o /dev/nvme0n1 > data/disk");
	FILE *fp=fopen(file,"r+");
	fseek(fp, 5, SEEK_SET);	//nvme0n1
	fgets(ssbuf, 8, fp);
	//printf("fun_T01015006(): ssbuf = %s\r\n", ssbuf);
	fclose(fp);
	res = strcmp(ssbuf, key);
	//printf("strcmp: res =%d\r\n", res);
	if(res == 0)
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

//Audio General
int fun_T01005001()
{
	int audio_select;
	audio_select = g_testargu.audio_hw;
	fun_TestSnd_002(audio_select,"./sound/right_left_both.WAV");
	return ask_reslut(0,0);
}

//DK 20160307 Audio 4.0 channel
int fun_T01005002()
{
	system("speaker-test -D surround40:CARD=PCH -c 4 -l 1 -r 44100 -t wav -W  ./sound -w 4channel.wav > /dev/null 2>&1");
	system("clear");
	return ask_reslut(0,0);
}

//Audio Left
int fun_T01005003()
{
	return RESULT_FAIL;
}

//Audio Bass
int fun_T01005004()
{
	return RESULT_FAIL;
}

//LAN1
int fun_T01006001()
{
	FILE *fp;
	int res = RESULT_FAIL;
	char buf[20];

	//
	int i = 0;

	system("ifconfig eth1 down > /dev/null 2>&1");
	system("ifconfig eth0 up > /dev/null 2>&1");
	sleep(3);

	while(i < 3)
	{
		system("ifconfig eth0 | grep 'inet [0-9][0-9][0-9][.][0-9][0-9][0-9]' > ./data/lan_ip");
		fp = fopen("./data/lan_ip", "r+");
		fgets(buf, 20, fp);
		fclose(fp);
		if (buf[8] == 'i' && buf[9] == 'n' && buf[10] == 'e' && buf[11] == 't')
		{
			res = fun_TestLAN_004("192.168.137.1","eth0");
		}
		if (res == RESULT_PASS)
			break;
		i++;
		system("ifconfig eth0 up > /dev/null 2>&1");
		sleep(5);
	}

	return res;
	//


	system("ifconfig eth1 down > /dev/null 2>&1");
	sleep(1);
	res = fun_TestLAN_004("192.168.137.1","eth0");
	if (res == RESULT_FAIL)
	{
		system("ifconfig eth0 up > /dev/null 2>&1");
		sleep(6);
		res = fun_TestLAN_004("192.168.137.1","eth0");
	}
	system("ifconfig eth1 up > /dev/null 2>&1");
	return res;

	/*
	system("ifconfig | grep -o 'eth[0-9]' > ./data/lan_eth");
	sleep(1);
	fp = fopen("./data/lan_eth","r+");
	fgets(buf,5,fp);
	fclose(fp);

	res = fun_TestLAN_004("192.168.137.1","eth0");
	if (res == RESULT_FAIL)
	{
		system("ifconfig eth0 up > /dev/null 2>&1");
		sleep(6);
		res = fun_TestLAN_004("192.168.137.1","eth0");
	}
	return res;
	*/
	
	/*
	system("ifconfig eth0 up > /dev/null 2>&1");
	sleep(1);
	system("ifconfig eth1 up > /dev/null 2>&1");
	sleep(1);
	system("ifconfig eth2 up > /dev/null 2>&1");
	sleep(6);
	system("ifconfig | grep -o 'eth[0-9]' > ./data/lan_eth");
	usleep(100000);

	fp = fopen("./data/lan_eth","r+");
	fgets(buf,5,fp);
	//buf[4]='\0';
	DEBUGMSG("fun_T01006001: buf = %s\r\n",buf);
	fclose(fp);
	usleep(300000);
	//system("ping -c 3 -I enp1s0 192.168.0.1");
	//sprintf(szBuf,"ping -c 3 -I %s %s  > /dev/null 2>&1",src_interface,dest_ip);
	return fun_TestLAN_004("192.168.137.1",buf);
	*/
	
}

//LAN2
int fun_T01006002()
{
	FILE *fp;
	int res = RESULT_FAIL;
	char buf[20];

//
	int i = 0;

	system("ifconfig eth0 down > /dev/null 2>&1");
	system("ifconfig eth1 up > /dev/null 2>&1");
	sleep(3);

	while(i < 3)
	{
		system("ifconfig eth1 | grep 'inet [0-9][0-9][0-9][.][0-9][0-9][0-9]' > ./data/lan_ip");
		fp = fopen("./data/lan_ip", "r+");
		fgets(buf, 20, fp);
		fclose(fp);
		if (buf[8] == 'i' && buf[9] == 'n' && buf[10] == 'e' && buf[11] == 't')
		{
			res = fun_TestLAN_004("192.168.137.1","eth1");
		}
		if (res == RESULT_PASS)
			break;
		i++;
		system("ifconfig eth1 up > /dev/null 2>&1");
		sleep(5);
	}

	system("ifconfig eth0 up > /dev/null 2>&1");
	return res;
	//


	char buf1[8];

	system("ifconfig eth0 down > /dev/null 2>&1");
	sleep(1);
	res = fun_TestLAN_004("192.168.137.1","eth1");
	if (res == RESULT_FAIL)
	{
		system("ifconfig eth1 up > /dev/null 2>&1");
		sleep(6);
		res = fun_TestLAN_004("192.168.137.1","eth1");
	}
	system("ifconfig eth0 up > /dev/null 2>&1");
	return res;

	/*
	system("ifconfig | grep -o 'eth[0-9]' > ./data/lan_eth");
	sleep(1);
	fp = fopen("./data/lan_eth","r+");
	fseek(fp,5,SEEK_SET);
	fgets(buf,5,fp);
	fclose(fp);
	
	res = fun_TestLAN_004("192.168.137.1","eth1");
	if (res == RESULT_FAIL)
	{
		system("ifconfig eth1 up > /dev/null 2>&1");
		sleep(6);
		res = fun_TestLAN_004("192.168.137.1","eth1");
	}
	return res;
	*/
	
	/*
	system("ifconfig | grep -o 'eth[0-9]' > ./data/lan_eth");
	usleep(100000);
	fp = fopen("./data/lan_eth","r+");
	fseek(fp,5,SEEK_SET);
	fgets(buf,5,fp);
	//buf[4]='\0';
	//printf("fun_T01006002: buf = %s\r\n",buf);
	fclose(fp);
	usleep(300000);
	system("ping -c 3 -I enp3s0 192.168.0.1");
	//res = fun_TestLAN_004("192.168.137.1",buf);
	
	if(res == RESULT_FAIL)
	{
		printf("\r\nfun_T01006002 RESULT_FAIL\r\n");
		system("ifconfig | grep -o 'rename[0-9]' > ./data/lan_eth");
		usleep(100000);
		fp = fopen("./data/lan_eth","r+");
		fgets(buf1,8,fp);
		//printf("fun_T01006002: buf1 = %s\r\n",buf1);
		res = fun_TestLAN_004("192.168.137.1",buf1);
		fclose(fp);
		usleep(300000);
	}

	return res;*/
}

//VGA1
int fun_T01007001()
{
	int res;

	fun_TestVGA_002("/dev/fb0");
	res = ask_reslut(0,0);
	system("clear");
	return res;
}

//VGA2
int fun_T01007002()
{
	return RESULT_FAIL;	
}

//HDMI1
int fun_T01008001()
{
	return RESULT_FAIL;
}

//HDMI2
int fun_T01008002()
{
	return RESULT_FAIL;
}

//VCore
int fun_T01009001()
{
	SENSOR_INFO info;

	get_SIOinfo(&info);
	
	return check_voltage_range(1200,info.vcore,20.0);
}

//CPU Temp.
int fun_T01010001()
{
	SENSOR_INFO info;

	get_SIOinfo(&info);

	return RESULT_FAIL;
}

//System Temp.
int fun_T01010002()
{
	SENSOR_INFO info;

	get_SIOinfo(&info);

	return RESULT_FAIL;
}

//+12V
int fun_T01009002()
{
	SENSOR_INFO info;

	get_SIOinfo(&info);
	return check_voltage_range(12000,info.p12v,10.0);
}

//+5V
int fun_T01009003()
{
	SENSOR_INFO info;

	get_SIOinfo(&info);
	return check_voltage_range(5000,info.p5v,10.0);
}

//+3.3V
int fun_T01009004()
{
	SENSOR_INFO info;

	get_SIOinfo(&info);
	return check_voltage_range(3300,info.p3_3v,10.0);
}

//Vcc SRAM
int fun_T01009005()
{
	SENSOR_INFO info;

	get_SIOinfo(&info);
	return check_voltage_range(3300,info.p3_3v,10.0);
}

//PS2 Mouse
int fun_T01011001()
{
	return RESULT_FAIL;
}

//PS2 Keyboard
int fun_T01011002()
{
	return RESULT_FAIL;
}

//USB1
int fun_T01012001()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_usb1"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//USB2
int fun_T01012002()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_usb2"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//USB3
int fun_T01012003()
{	
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_usb3"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//USB4
//DK 20170309 for db version, we only check usb-hub connect or not
int fun_T01012004()
{	
	int ret,res = RESULT_FAIL;
	char buf[128];
	FILE *fp;

	ret = system("ifconfig | grep wlan > ./data/wlan_chk");
	fp = fopen("./data/wlan_chk","r+");
	fgets(buf,10,fp);
	if( (strcmp(buf,"wlan")) >0)
		res = RESULT_PASS;	
	fclose(fp);

	return res;
/*
	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_usb4"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;
*/
}


//USB5
int fun_T01012005()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_usb5"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//USB6
int fun_T01012006()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_usb6"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//USB7
int fun_T01012007()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_usb7"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//USB8
int fun_T01012008()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_usb8"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

//COM1
int fun_T01013001()
{
	return fun_TestUART_001("/dev/ttyS0");
}

//COM2
int fun_T01013002()
{
	return fun_TestUART_003(0x2f8);
}

//COM3
int fun_T01013003()
{
	return fun_TestUART_003(0x3e8);
}

//COM4
int fun_T01013004()
{
	//return fun_TestUART_001("/dev/ttyS3");
	return fun_TestUART_003(0x2e8);

}

//COM5
int fun_T01013005()
{
	//return fun_TestUART_001("/dev/ttyS4");
	return fun_TestUART_003(0x280);

}

//COM6
int fun_T01013006()
{
	//return fun_TestUART_001("/dev/ttyS5");
	return fun_TestUART_003(0x288);

}

//GPIO-A
int fun_T01014001()
{
	return RESULT_FAIL;
}

//GPIO-B
int fun_T01014002()
{
	return RESULT_FAIL;
}

//GPIO-C
int fun_T01014003()
{
	return RESULT_FAIL;
}

//GPIO-D
int fun_T01014004()
{
	return RESULT_FAIL;
}

//CFAST1
int fun_T01016001()
{
	int res = RESULT_FAIL;

	if(FUN_SUCC == fun_MountStorage_002("/mnt/sipdevs","vf_cfast1"))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;

	return res;
}

/***********************************************
*20160315
*Fan test
*all fan use duty 50% & 100% for calculate RPM
*cal: 60000(60s*ms) / (X / 10(fpga sample rate) * 2(a pulse))  
***********************************************/

int fan_tester(int *res1, int *res2, int *res3)
{
	int res = RESULT_FAIL;
	int passcnt = 0;
	float fan_rpm;
	DSGPC_HW hw;

	//FAN1
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = 0xff;
	DEBUGMSG("Fan1 Current Duty = 0x%X\r\n",hw.u08_fanduty);
	res = GPC_CTLCODE(GPC_FAN_HW_0, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
	{
		printf("Set Fan Duty Error\n");
		return RESULT_FAIL;
	}
	sleep(2);
	//get current period speed
	hw.u32_subtarget = FAN_HW_CURRENT_PERIOD;
	res = GPC_CTLCODE(GPC_FAN_HW_0, ACTION_GET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Get Fan Current Period Error\n");
	DEBUGMSG("FAN1 Current Period = 0x%X\r\n",hw.u08_fanperiod);
	fan_rpm = hw.u08_fanperiod;
	//calculate the fan RPM
	fan_rpm = (60000/(fan_rpm/10*2));
	DEBUGMSG("FAN1 RPM = %.0f\r\n",fan_rpm);
	if (fan_rpm > g_testargu.fan1_full_threshold)
		passcnt ++;
	DEBUGMSG("passcnt = %d\r\n",passcnt);
	fan_rpm = 0;
	sleep(1);

	//set half duty
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = g_testargu.fan1_duty;
	DEBUGMSG("FAN1 Current Duty = 0x%X\r\n",hw.u08_fanduty);
	res = GPC_CTLCODE(GPC_FAN_HW_0, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
	{
		printf("Set Fan Duty Error\n");
		return RESULT_FAIL;
	}
	sleep(1);
	//get current period speed
	hw.u32_subtarget = FAN_HW_CURRENT_PERIOD;
	res = GPC_CTLCODE(GPC_FAN_HW_0, ACTION_GET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Get Fan Current Period Error\n");
	DEBUGMSG("FAN1 Current Period = 0x%X\r\n",hw.u08_fanperiod);
	sleep(1);
	fan_rpm = hw.u08_fanperiod;
	//calculate the fan RPM
	fan_rpm = (60000/(fan_rpm/10*2));
	DEBUGMSG("FAN1 RPM  = %.0f\r\n",fan_rpm);
	if((fan_rpm > g_testargu.fan1_lower) && (fan_rpm < g_testargu.fan1_upper))
		passcnt ++;
	printf("passcnt = %d\r\n",passcnt);
	
	if (passcnt == 2)
		res1 = RESULT_PASS;
	else
		res1 = RESULT_FAIL;

	passcnt = 0;

	//FAN2
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = 0xff;
	DEBUGMSG("FAN2 Current Duty = 0x%X\r\n",hw.u08_fanduty);
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
	{
		printf("Set Fan Duty Error\n");
		return RESULT_FAIL;
	}
	sleep(2);
	//get current period speed
	hw.u32_subtarget = FAN_HW_CURRENT_PERIOD;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_GET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Get Fan Current Period Error\n");
	DEBUGMSG("Fan2 Current Period = 0x%X\r\n",hw.u08_fanperiod);
	fan_rpm = hw.u08_fanperiod;
	//calculate the fan RPM
	fan_rpm = (60000/(fan_rpm/10*2));
	DEBUGMSG("FAN2 RPM = %.0f\r\n",fan_rpm);
	if (fan_rpm > g_testargu.fan2_full_threshold)
		passcnt ++;
	DEBUGMSG("passcnt = %d\r\n",passcnt);
	fan_rpm = 0;
	
	sleep(2);
	//set half duty
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = g_testargu.fan2_duty;
	DEBUGMSG("FAN2 Current Duty = 0x%X\r\n",hw.u08_fanduty);
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
	{
		printf("Set Fan Duty Error\n");
		return RESULT_FAIL;
	}
	sleep(2);
	//get current period speed
	hw.u32_subtarget = FAN_HW_CURRENT_PERIOD;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_GET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Get Fan Current Period Error\n");
	DEBUGMSG("Fan2 Current Period = 0x%X\r\n",hw.u08_fanperiod);
	sleep(1);

	fan_rpm = hw.u08_fanperiod;
	//calculate the fan RPM
	fan_rpm = (60000/(fan_rpm/10*2));
	DEBUGMSG("FAN2 RPM  = %.0f\r\n",fan_rpm);
	if((fan_rpm > g_testargu.fan2_lower) && (fan_rpm < g_testargu.fan2_upper))
		passcnt ++;
	DEBUGMSG("passcnt = %d\r\n",passcnt);

	if (passcnt == 2)
		res2 = RESULT_PASS;
	else
		res2 = RESULT_FAIL;

	passcnt = 0;
	//FAN3
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = 0xff;
	DEBUGMSG("Fan3 Current Duty = 0x%X\r\n",hw.u08_fanduty);
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
	{
		printf("Set Fan Duty Error\n");
		return RESULT_FAIL;
	}
	sleep(2);
	//get current period speed
	hw.u32_subtarget = FAN_HW_CURRENT_PERIOD;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_GET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Get Fan Current Period Error\n");
	DEBUGMSG("FAN3 Current Period = 0x%X\r\n",hw.u08_fanperiod);
	fan_rpm = hw.u08_fanperiod;
	//calculate the fan RPM
	fan_rpm = (60000/(fan_rpm/10*2));
	DEBUGMSG("FAN3 RPM = %.0f\r\n",fan_rpm);
	if (fan_rpm > g_testargu.fan3_full_threshold)
		passcnt ++;
	DEBUGMSG("passcnt = %d\r\n",passcnt);
	fan_rpm = 0;
	sleep(1);

	//set half duty
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = g_testargu.fan3_duty;
	DEBUGMSG("FAN3 Current Duty = 0x%X\r\n",hw.u08_fanduty);
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
	{
		printf("Set Fan Duty Error\n");
		return RESULT_FAIL;
	}
	sleep(1);
	//get current period speed
	hw.u32_subtarget = FAN_HW_CURRENT_PERIOD;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_GET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Get Fan Current Period Error\n");
	DEBUGMSG("FAN3 Current Period = 0x%X\r\n",hw.u08_fanperiod);
	sleep(1);
	fan_rpm = hw.u08_fanperiod;
	//calculate the fan RPM
	fan_rpm = (60000/(fan_rpm/10*2));
	DEBUGMSG("FAN3 RPM  = %.0f\r\n",fan_rpm);
	if((fan_rpm > g_testargu.fan3_lower) && (fan_rpm < g_testargu.fan3_upper))
		passcnt ++;
	printf("passcnt = %d\r\n",passcnt);
	
	if (passcnt == 2)
		res3 = RESULT_PASS;
	else
		res3 = RESULT_FAIL;
	
	return RESULT_PASS;
}

//FAN1 sys gmb7055 - 4800RPM - 20160523 - only test full speed
int fun_T01020001()
{
	int res = RESULT_FAIL;
	int passcnt = 0;
	float fan_rpm;
	DSGPC_HW hw;

	//set maximum duty
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = 0xff;
	DEBUGMSG("Fan Current Duty = 0x%X\r\n",hw.u08_fanduty);
	res = GPC_CTLCODE(GPC_FAN_HW_0, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
	{
		printf("Set Fan Duty Error\n");
		return RESULT_FAIL;
	}
	sleep(2);
	//get current period speed
	hw.u32_subtarget = FAN_HW_CURRENT_PERIOD;
	res = GPC_CTLCODE(GPC_FAN_HW_0, ACTION_GET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Get Fan Current Period Error\n");
	DEBUGMSG("Fan Current Period = 0x%X\r\n",hw.u08_fanperiod);
	fan_rpm = hw.u08_fanperiod;
	//calculate the fan RPM
	fan_rpm = (60000/(fan_rpm/10*2));
	DEBUGMSG("fan_rpm = %.0f\r\n",fan_rpm);
	//if( fan_rpm > 3800)
	if (fan_rpm > g_testargu.fan1_full_threshold)
		passcnt ++;
	DEBUGMSG("passcnt = %d\r\n",passcnt);
	fan_rpm = 0;
	sleep(1);

	
	// //set half duty
	// hw.u32_subtarget = FAN_HW_DUTY;
	// hw.u08_fanduty = g_testargu.fan1_duty;
	// DEBUGMSG("Fan Current Duty = 0x%X\r\n",hw.u08_fanduty);
	// res = GPC_CTLCODE(GPC_FAN_HW_0, ACTION_SET, &hw);
	// if (res != GPC_FUN_RES_SUCC)
	// {
	// 	printf("Set Fan Duty Error\n");
	// 	return RESULT_FAIL;
	// }
	// sleep(1);
	// //get current period speed
	// hw.u32_subtarget = FAN_HW_CURRENT_PERIOD;
	// res = GPC_CTLCODE(GPC_FAN_HW_0, ACTION_GET, &hw);
	// if (res != GPC_FUN_RES_SUCC)
	// 	printf("Get Fan Current Period Error\n");
	// printf("Fan Current Period = 0x%X\r\n",hw.u08_fanperiod);
	// fan_rpm = hw.u08_fanperiod;
	// //calculate the fan RPM
	// fan_rpm = (60000/(fan_rpm/10*2));
	// printf("fan_rpm = %.0f\r\n",fan_rpm);
	// if((fan_rpm > g_testargu.fan1_lower) && (fan_rpm < g_testargu.fan1_upper))
	// 	passcnt ++;
	// printf("passcnt = %d\r\n",passcnt);
	

	if (passcnt == 1)
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

//FAN2 left gmb7055 - 5200RPM *fan2 & fan4 use the same register, manual test
int fun_T01020002()
{
	showjpeg(0,0,"./pics/800_600/fan_manual_1.jpg");
	sleep(3);

	return ask_reslut(0,0);
}

//FAN3	right gmb7055 5300RPM
int fun_T01020003()
{
	int res = RESULT_FAIL;
	int passcnt = 0;
	float fan_rpm;
	DSGPC_HW hw;

	//set maximum duty
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = 0xff;
	DEBUGMSG("Fan Current Duty = 0x%X\r\n",hw.u08_fanduty);
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
	{
		printf("Set Fan Duty Error\n");
		return RESULT_FAIL;
	}
	sleep(2);
	//get current period speed
	hw.u32_subtarget = FAN_HW_CURRENT_PERIOD;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_GET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Get Fan Current Period Error\n");
	DEBUGMSG("Fan Current Period = 0x%X\r\n",hw.u08_fanperiod);
	fan_rpm = hw.u08_fanperiod;
	//calculate the fan RPM
	fan_rpm = (60000/(fan_rpm/10*2));
	DEBUGMSG("fan_rpm = %.0f\r\n",fan_rpm);
	//if( fan_rpm > 3800)
	if (fan_rpm > g_testargu.fan3_full_threshold)
		passcnt ++;
	DEBUGMSG("passcnt = %d\r\n",passcnt);
	fan_rpm = 0;
	sleep(1);

	
	//set half duty
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = g_testargu.fan3_duty;
	DEBUGMSG("Fan Current Duty = 0x%X\r\n",hw.u08_fanduty);
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
	{
		printf("Set Fan Duty Error\n");
		return RESULT_FAIL;
	}
	sleep(1);
	//get current period speed
	hw.u32_subtarget = FAN_HW_CURRENT_PERIOD;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_GET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Get Fan Current Period Error\n");
	printf("Fan Current Period = 0x%X\r\n",hw.u08_fanperiod);
	fan_rpm = hw.u08_fanperiod;
	//calculate the fan RPM
	fan_rpm = (60000/(fan_rpm/10*2));
	printf("fan_rpm = %.0f\r\n",fan_rpm);
	if((fan_rpm > g_testargu.fan3_lower) && (fan_rpm < g_testargu.fan3_upper))
		passcnt ++;
	printf("passcnt = %d\r\n",passcnt);
	
	auto_final = 1;

	if (passcnt == 2)
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

//FAN4 gcb881c - 5200RPM
int fun_T01020004()
{
	int res = RESULT_FAIL;
	int passcnt = 0;
	float fan_rpm;
	DSGPC_HW hw;

	//set maximum duty
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = 0xff;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
	{
		printf("Set Fan Duty Error\n");
		return RESULT_FAIL; 
	}
	sleep(2);
	//get current period speed
	hw.u32_subtarget = FAN_HW_CURRENT_PERIOD;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_GET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Get Fan Current Period Error\n");
	printf("Fan Current Period = 0x%X\r\n",hw.u08_fanperiod);
	sleep(2);

	fan_rpm = hw.u08_fanperiod;
	//calculate the fan RPM
	fan_rpm = (60000/(fan_rpm/10*2));
	printf("fan_rpm = %.0f\r\n",fan_rpm);
	if( fan_rpm > 3800)
		passcnt ++;
	printf("passcnt = %d\r\n",passcnt);
	fan_rpm = 0;
		sleep(2);
	//set half duty
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = 0x10;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
	{
		printf("Set Fan Duty Error\n");
		return RESULT_FAIL; 
	}
	sleep(2);
	//get current period speed
	hw.u32_subtarget = FAN_HW_CURRENT_PERIOD;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_GET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Get Fan Current Period Error\n");
	printf("Fan Current Period = 0x%X\r\n",hw.u08_fanperiod);
	sleep(2);

	fan_rpm = hw.u08_fanperiod;	
	//calculate the fan RPM
	fan_rpm = (60000/(fan_rpm/10*2));
	printf("fan_rpm = %.0f\r\n",fan_rpm);
	if((fan_rpm > 2100 ) && (fan_rpm < 3500))
		passcnt ++;
	printf("passcnt = %d\r\n",passcnt);
	
	if (passcnt == 2)
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}


//DK 20160304
//PCIe 1x for gpc
int fun_T01019001()
{
	const char *fp= "./data/pci_chk";
	int res = RESULT_FAIL;
	system("lspci | grep 3800 > ./data/pci_chk 2>&1");
	if(FUN_SUCC == pci_detect01(7,"Lattice",fp))
		res = RESULT_PASS;
	else
		res = RESULT_FAIL;
	return res;
}

//SIO detect
int fun_T01022001()
{
	int buf=99;
	int res = RESULT_FAIL;
	iopl(3);
	//SENSOR_INFO info;

	//get_SIOinfo(&info);
	//SIO_HW_Init(SIO_CFG_ADDR1,SIO_DAT_ADDR1,0x60,0x61,5);

	outb(0x77,SIO_CFG_ADDR2);	//open
	outb(0x77,SIO_CFG_ADDR2);	//open
	usleep(100);
	outb(0x77,SIO_CFG_ADDR2);	//open
	outb(0x77,SIO_CFG_ADDR2);	//open
	outb(0x23,SIO_CFG_ADDR2);  	//switch vendor ID_1
	buf = inb(SIO_DAT_ADDR2);
	//printf("buf=%d,0x%X\r\n",buf,buf);
	
	if(buf == 25)
 		res = RESULT_PASS;
	else
		res = RESULT_FAIL;
	return res;
}


//HWaddr1
int fun_T01024001()
{
	int i;
	FILE *fp;
	char szHWaddr1[64];
	//char szHWaddr2[256];
	system("ifconfig | grep -o '[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]' > ./data/mac_addr");
	fp = fopen("./data/mac_addr","r+");
	fgets(szHWaddr1,64,fp);
	//fgets(szHWaddr2,256,fp);
	fclose(fp);

	//printf("\r\ng_testargu.hwaddr = %s\r\n", g_testargu.hwaddr1);
	//printf("\r\nszHWaddr1 = %s\r\n", szHWaddr1);
	//printf("\r\nszHWaddr2 = %s\r\n", szHWaddr2);
	for(i = 0; i < 8; i++)
	{
		//printf("g_testargu.hwaddr1[i] = %c szHWaddr1[i] = %c\r\n", g_testargu.hwaddr1[i], szHWaddr1[i]);
		if (g_testargu.hwaddr1[i] != szHWaddr1[i])
			return RESULT_FAIL;
	}
	//printf("\r\ng_testargu.hwaddr = %s\r\n", g_testargu.hwaddr);
	//printf("\r\nszHWaddr = %s\r\n", szHWaddr);
	
	//g_testargu.hwaddr

	return RESULT_PASS;
}


//HWaddr2
int fun_T01024002()
{
	int i;
	FILE *fp;
	char szHWaddr1[64];
	char szHWaddr2[64];
	system("ifconfig | grep -o '[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]' > ./data/mac_addr");
	fp = fopen("./data/mac_addr","r+");
	fgets(szHWaddr1,64,fp);
	fgets(szHWaddr2,64,fp);
	fclose(fp);

	//printf("\r\ng_testargu.hwaddr = %s\r\n", g_testargu.hwaddr2);
	//printf("\r\nszHWaddr2 = %s\r\n", szHWaddr2);
	//printf("\r\nszHWaddr2 = %s\r\n", szHWaddr2);
	for(i = 0; i < 8; i++)
	{
		//printf("g_testargu.hwaddr2[i] = %c szHWaddr2[i] = %c\r\n", g_testargu.hwaddr2[i], szHWaddr2[i]);
		if (g_testargu.hwaddr2[i] != szHWaddr2[i])
			return RESULT_FAIL;
	}
	//printf("\r\ng_testargu.hwaddr = %s\r\n", g_testargu.hwaddr);
	//printf("\r\nszHWaddr = %s\r\n", szHWaddr);
	
	//g_testargu.hwaddr

	return RESULT_PASS;
}


/*********************************************
*Sysytem RTC //DK 170614
* 1. check year is match between bios and os
* 2. get cmos 'sec' and verify it is step or not
*********************************************/
int fun_T01023001()
{
	int count=0,time1,time2,res = RESULT_PASS;
	char buf1[5],buf2[5];
	FILE *fp;

	//first check year is correctly.
	system("hwclock -r | grep -o '[0-9][0-9][0-9][0-9]' > ./data/sys_time_chk 2>&1");
	system("date | grep -o '[0-9][0-9][0-9][0-9]' > ./data/temp_time 2>&1");
	fp = fopen("./data/sys_time_chk","r+");
	fseek(fp,0,SEEK_SET);
	fgets(buf1,5,fp);
	fclose(fp);
	usleep(10000);
	fp = fopen("./data/temp_time","r+");
	fgets(buf2,5,fp);
	fclose(fp);
	usleep(10000);
	system("rm ./data/temp_time");
	DEBUGMSG("fun_T01023001() : buf1 = %s, buf2 = %s\r\n",buf1,buf2);
	time1 = atoi(buf1);
	time2 = atoi(buf2);
	if(time1 == time2)
	{
		count ++;
	}

	//get bios 'sec' and verify it
	system("hwclock -r | grep -o '[0-9][0-9]:[0-9][0-9]:[0-9][0-9]' > ./data/sys_time_chk 2>&1");
	fp = fopen("./data/sys_time_chk","r+");
	fseek(fp,6,SEEK_SET);
	fgets(buf1,3,fp);
	time1 = atoi(buf1);
	fclose(fp);
	usleep(10000);
	DEBUGMSG("fun_T01023001() : time1 sec = %d\r\n",time1);	
	sleep(2);
	//get 2nd time
	system("hwclock -r | grep -o '[0-9][0-9]:[0-9][0-9]:[0-9][0-9]' > ./data/sys_time_chk 2>&1");
	fp = fopen("./data/sys_time_chk","r+");
	fseek(fp,6,SEEK_SET);
	fgets(buf2,3,fp);
	time2 = atoi(buf2);
	fclose(fp);
	usleep(10000);
	DEBUGMSG("fun_T01023001() : time2 sec = %d\r\n",time2);
	system("rm ./data/sys_time_chk");

	if( ((time1 - time2) > 0) || ((time2 - time1) >0) )
	{
		count++;
	}
	if(count == 2)
		return RESULT_PASS;
	else
		return RESULT_FAIL;

}

/*************************************************************************************
	Gaming Feature 
*************************************************************************************/
void init_gameio()
{

}

void enable_all_int()
{

}

//SRAM
#define KBYTES	1024
int FillSRAM(const int bank,const int ksize,const char value)
{
	int i,res = FUN_SUCC;
	DSGPC_SRAM  sram;
	U08 buffer_w[KBYTES];

	memset(buffer_w,value,KBYTES);

    //fill in value
    for(i=0;i<ksize;i++)
    {
		sram.u32_subtarget = SRAM_BLOCK_RW;
		sram.u08_bank = bank;
		sram.u32_address = i*KBYTES;
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

int fun_TestSRAM(const int bank,const int ksize)	
{
	int i,j,res = RESULT_PASS;
	DSGPC_SRAM  sram;
	U08 buffer_w[KBYTES],buffer_r[KBYTES];

	FillSRAM(bank,ksize,0);

    //fill in 1K buffer to pattern
	for(i=0;i<KBYTES;i+=4)
	{
		buffer_w[i+0]=0x00;
		buffer_w[i+1]=0xFF;
		buffer_w[i+2]=0x55;
		buffer_w[i+3]=0xAA;
	}
 
    //fill in pattern
    for(i=0;i<ksize;i++)
    {
    	if((bank==0)&&(i==0)) //gpc reserve first 1k
    		continue;

		sram.u32_subtarget = SRAM_BLOCK_RW;
		sram.u08_bank = bank;
		sram.u32_address = i*KBYTES;
		sram.u32_size = KBYTES;
		sram.pu08_data = buffer_w;		
 		if(GPC_FUN_RES_SUCC != GPC_CTLCODE(GPC_SRAM,ACTION_SET,&sram))
		{
			res = RESULT_FAIL;
			goto END;
		}
	}
	//check all data of bank
    for(i=0;i<ksize;i++)
    {
    	if((bank==0)&&(i==0)) //gpc reserve first 1k
    		continue;
    		
    	memset(buffer_r,0,KBYTES);
		sram.u32_subtarget = SRAM_BLOCK_RW;
		sram.u08_bank = bank;
		sram.u32_address = i*KBYTES;
		sram.u32_size = KBYTES;
		sram.pu08_data = buffer_r;				
 		if(GPC_FUN_RES_SUCC != GPC_CTLCODE(GPC_SRAM,ACTION_GET,&sram))
		{
			res = RESULT_FAIL;
			goto END;
		}
	    //check data
		for(j=0;j<KBYTES;j+=4)
		{
			if( (buffer_r[j+0]!=0x00) ||
			    (buffer_r[j+1]!=0xFF) ||
				(buffer_r[j+2]!=0x55) ||
				(buffer_r[j+3]!=0xAA) )
			{
				res = RESULT_FAIL;
				goto END;
			}
		}			
	}

END:	
	return res;
}

int fun_T02006001()
{
	int i,result=RESULT_PASS;

	// DEBUGMSG("Test Bank0:");
	// if(FUN_FAIL == fun_TestSRAM(0,512))
	// {
	// 	DEBUGMSG("Fail\r\n");
	// 	return RESULT_FAIL;
	// }
	// DEBUGMSG("Pass\r\n");

	// DEBUGMSG("Test Bank1:");
	// if(FUN_FAIL == fun_TestSRAM(1,512))
	// {
	// 	DEBUGMSG("Fail\r\n");
	// 	return RESULT_FAIL;
	// }
	// DEBUGMSG("Pass\r\n");

	// DEBUGMSG("Test Bank2:");
	// if(FUN_FAIL == fun_TestSRAM(2,512))
	// {
	// 	DEBUGMSG("Fail\r\n");
	// 	return RESULT_FAIL;
	// }
	// DEBUGMSG("Pass\r\n");
	// return RESULT_PASS;

	
	// DEBUGMSG("fun_T02006001(),banknum=%d,banksize=%d\r\n",g_testargu.sram_banknum,g_testargu.sram_size);
	// if(FUN_FAIL == fun_TestSRAM(1,g_testargu.sram_size))
	// 	return RESULT_FAIL;
	// if(FUN_FAIL == fun_TestSRAM(2,g_testargu.sram_size/3))
	// 	return RESULT_FAIL;
	// if(FUN_FAIL == fun_TestSRAM(4,g_testargu.sram_size/3))
	// 	return RESULT_FAIL;

	DEBUGMSG("fun_T02006001(),banknum=%d,banksize=%d\r\n",g_testargu.sram_banknum,g_testargu.sram_size);
	for(i = 0; i < g_testargu.sram_banknum; i++)
	{
		result = fun_TestSRAM( 1 << i, g_testargu.sram_size);
		if(RESULT_FAIL == result)
		{
			DEBUGMSG("fun_T02006001,bank %d Fail\r\n",i);
		}	
	}

	return result;	

}

/*
//Timer A
int fun_T02007001()
{
	DSGPC_TIMER timer;
	struct timeval tvdiff;
	unsigned long long timediff;
	
	timer.u32_subtarget = TIMER_CFG_SWITCH;
	timer.u08_onoff = SET_ON;
	timer.u32_data =  TIMER_CFG_ENABLE;
	GPC_CTLCODE(GPC_CFG_TIMER_A,ACTION_SET,&timer);

	timer.u32_subtarget = TIMER_CFG_RESOLUTION;
	timer.u08_resolution = TIMER_RESOLUTION_MSEC;
	GPC_CTLCODE(GPC_CFG_TIMER_A,ACTION_SET,&timer);

	timer.u32_subtarget  = TIMER_COUNTER_NUMBER;
	timer.u16_counter = 100;
	GPC_CTLCODE(GPC_COUNTER_TIMER_A,ACTION_SET,&timer);

	gettimeofday(&g_tc.tv1,NULL);
	timer.u32_subtarget = TIMER_CFG_START;
	timer.u08_onoff = SET_ON;
	GPC_CTLCODE(GPC_CFG_TIMER_A,ACTION_SET,&timer);

	sem_wait(&g_tc.sem_finish);
	timersub(&g_tc.tv2,&g_tc.tv1,&tvdiff);
	timediff = tvdiff.tv_sec * 1000000 + tvdiff.tv_usec;

	//printf("timediff=%d",timediff);
	
	if(timediff< ((100+10)*1000) )
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

//Timer B
int fun_T02007002()
{
	DSGPC_TIMER timer;
	struct timeval tvdiff;
	unsigned long long timediff;
	
	timer.u32_subtarget = TIMER_CFG_ENABLE;
	timer.u08_onoff = SET_ON;
	timer.u32_data =  TIMER_CFG_ENABLE;
	GPC_CTLCODE(GPC_CFG_TIMER_B,ACTION_SET,&timer);

	timer.u32_subtarget = TIMER_CFG_RESOLUTION;
	timer.u08_resolution = TIMER_RESOLUTION_MSEC;
	GPC_CTLCODE(GPC_CFG_TIMER_B,ACTION_SET,&timer);

	timer.u32_subtarget  = TIMER_COUNTER_NUMBER;
	timer.u16_counter = 100;
	GPC_CTLCODE(GPC_COUNTER_TIMER_B,ACTION_SET,&timer);

	gettimeofday(&g_tc.tv1,NULL);
	timer.u32_subtarget = TIMER_CFG_START;
	timer.u08_onoff = SET_ON;
	GPC_CTLCODE(GPC_CFG_TIMER_B,ACTION_SET,&timer);

	sem_wait(&g_tc.sem_finish);
	timersub(&g_tc.tv2,&g_tc.tv1,&tvdiff);
	timediff = tvdiff.tv_sec * 1000000 + tvdiff.tv_usec;

	//printf("timediff=%d",timediff);
	
	if(timediff< ((100+10)*1000) )
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

//Timer C
int fun_T02007003()
{
	return RESULT_FAIL;
}

//Timer D
int fun_T02007004()
{
	return RESULT_FAIL;
}
*/

//CCTalk COM2->COM3
int fun_T02008001()
{
	return fun_TestUART_002("/dev/ttyS2","/dev/ttyS2");
}

//CCTalk2
int fun_T02008002()
{
	return RESULT_FAIL;
}

/*
//SPI ROM1
int fun_T02019001()
{
	int i;
	DSGPC_SPI gpc_spi;
	DSEXT_SPIMODULE_W25X w25;
	U08	wdata[4096],rdata[4096];

	gpc_spi.u32_subtarget = SPI_CFG_SWITCH;
	gpc_spi.u08_onoff     = SET_OFF;
	gpc_spi.u32_data      = SPI_CFG_SLAVE;
	GPC_CTLCODE(GPC_CFG_SPI_0, ACTION_SET, &gpc_spi);

	gpc_spi.u32_subtarget = SPI_CFG_SWITCH;
	gpc_spi.u08_onoff     = SET_ON;
	gpc_spi.u32_data      = SPI_CFG_ENABLE;
	GPC_CTLCODE(GPC_CFG_SPI_0, ACTION_SET, &gpc_spi);

	//write data
	srand(time(NULL));
	for(i=0;i<4096;i++)
	{
		wdata[i]=rand()%0xFF;
	}
	w25.u32_subtarget = SPIMODULE_W25X_DATA;
	w25.u32_addr = 0;
	w25.u32_size = 4096;
	w25.pu08_data = wdata;
	GPC_CTLCODE(EXT_SPIMODULE_W25X,ACTION_SET,&w25);
	
	//read data
	w25.u32_subtarget = SPIMODULE_W25X_DATA;
	w25.u32_addr = 0;
	w25.u32_size = 4096;
	w25.pu08_data = rdata;
	GPC_CTLCODE(EXT_SPIMODULE_W25X,ACTION_GET,&w25);

	DEBUGMSG("\r\nstart comparing data.....\r\n");
	for(i=0;i<4096;i++)
	{
		DEBUGMSG("[%02X:%02X] ",wdata[i],rdata[i]);
		if(wdata[i]!=rdata[i])
		{
			DEBUGMSG("Comparing result(Fail)!!\r\n");
			return RESULT_FAIL;
		}
	}

	DEBUGMSG("\r\nComparing result(Succ)!!\r\n");
	return RESULT_PASS;
}

//SPI ROM2
int fun_T02019002()
{
	return RESULT_FAIL;
}
*/

//IIC0
int fun_T02010001()
{
	return RESULT_FAIL;
}

//IIC1
int fun_T02010002()
{
	return RESULT_FAIL;
}

/*
//DIP SW
int fun_T02011001()
{
	DSGPC_PORT	     libport;
	int				 passcnt=0;
	int time=0,pos=0;

	while(!g_diaginfo.bFinalDipswitch)
	{
		usleep(10000);
		time+=10000;
		if(0==(time%500000))
		{
			pos=(pos+1)%8;
			showjpeg(0,0,PATH_WAIT_RESULT);		
			showjpeg(80*pos,400,PATH_WAIT_RESULT1);	
			time = 0;
		}
	}

	showjpeg(0,0,"./pics/800_600/askswitch_high.jpg");
	check_yes();
	libport.u32_subtarget = PORT_ACCESS;
	libport.u08_portnum = 'E';
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	DEBUGMSG("[Dip High]PORT E = 0x%02X\r\n",libport.u08_data);
	if(libport.u08_data==0xFF)
		passcnt++;
	libport.u32_subtarget = PORT_ACCESS;	
	libport.u08_portnum = 'F';
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	DEBUGMSG("[Dip High]PORT F = 0x%02X\r\n",libport.u08_data);	
	if(libport.u08_data==0xFF)
		passcnt++;
	
	showjpeg(0,0,"./pics/800_600/askswitch_low.jpg");
	check_yes();
	libport.u32_subtarget = PORT_ACCESS;	
	libport.u08_portnum = 'E';
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	DEBUGMSG("[Dip  Low]PORT E = 0x%02X\r\n",libport.u08_data);	
	if(libport.u08_data==0)
		passcnt++;
	libport.u32_subtarget = PORT_ACCESS;	
	libport.u08_portnum = 'F';
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	DEBUGMSG("[Dip  Low]PORT F = 0x%02X\r\n",libport.u08_data);		
	if(libport.u08_data==0)
		passcnt++;

	if(passcnt==4)
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}
*/

//Secure RTC
int fun_T02012001()
{
#if 1
	DSEIT_INFO eit_info_start,eit_info_end;
	int diff;

	eit_info_start.u32_subtarget = EIT_INFO_RTC;  
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eit_info_start);
	sleep(2);
	eit_info_end.u32_subtarget = EIT_INFO_RTC;  
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eit_info_end);

	diff = eit_info_end.time_rtc.totalsec-eit_info_start.time_rtc.totalsec;
	//DEBUGMSG("start sec=%d, end sec=%d, diff=%d\r\n",eit_info_start.time_rtc.totalsec,eit_info_end.time_rtc.totalsec,diff);
	//if((1<=diff)&&(diff<=2))
	if (diff > 1)
	{
		return RESULT_PASS;
	}
	
	return RESULT_FAIL;
#else	
	struct timeval	systime;
	DSEIT_INFO 		eit_info;
	U32				timediff;

	//because in T1 test station, we will setting RTC to system time,
	//At this moment we can get system timer and EIT RTC to compare.
	gettimeofday(&systime,NULL);
	eit_info.u32_subtarget = EIT_INFO_RTC;
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eit_info);
	DEBUGMSG("eit_info.time_rtc.totalsec=%d , systime.tv_sec=%d\r\n",eit_info.time_rtc.totalsec,systime.tv_sec);
	if(eit_info.time_rtc.totalsec >= systime.tv_sec)
		timediff = eit_info.time_rtc.totalsec - systime.tv_sec;
	else
		timediff = systime.tv_sec - eit_info.time_rtc.totalsec;

	if(timediff > 1)
		return RESULT_FAIL;
	else
		return RESULT_PASS;
#endif	
}


/*************************************************************************************
 ALL ITR UP   = 0x04(0000 0100)
 ALL ITR DOWN = 0x1f(0001 1111)
 
   LOG BOX(bit3) :   up = 0x17(0001 0111)
 BILL DOOR(bit0) :   up = 0x1e(0001 1110)
 CASH DOOR(bit4) :   up = 0x0f(0000 1111)
 MAIN DOOR(bit1) :   up = 0x1d(0001 1101)
*************************************************************************************/

//Intrusion 0(LOG BOX)
int fun_T02013003()
{
	//for GMB6020,we test all 3 intrusion in this function
	DSEIT_INFO	eitinfo;

	showjpeg(0,0,"./pics/gameio/itrup.jpg");
	check_yes();
	sleep(1);	
	eitinfo.u32_subtarget = EIT_INFO_ITRSTATUS;
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eitinfo);	
	sleep(1);
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eitinfo);
	//we get all itr up status value here 
	g_itrup = eitinfo.u08_itrstatus;
	g_itrup &= MASK_ITRVALUE;
	//DEBUGMSG("g_itrup=0x%x\r\n",g_itrup);

	
	showjpeg(0,0,"./pics/gameio/itrdown.jpg");	
	check_yes();
	//check itrstatus
	sleep(1);
	eitinfo.u32_subtarget = EIT_INFO_ITRSTATUS;
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eitinfo);
	sleep(1);	
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eitinfo);
	//we get all itr down status value here 
	g_itrdown = eitinfo.u08_itrstatus;
	g_itrdown &= MASK_ITRVALUE;	
	//DEBUGMSG("g_itrdown=0x%x\r\n",g_itrdown);

	//check LOGBOX up value
	if((g_itrup&MASK_ITRVALUE_LOGBOX)!=0)
 		return RESULT_FAIL;
	if((g_itrdown&MASK_ITRVALUE_LOGBOX)!=MASK_ITRVALUE_LOGBOX)
 		return RESULT_FAIL;

	return RESULT_PASS;
}

//Intrusion 1(BILL DOOR)
int fun_T02013004()
{
	//check LOGBOX up value
	if((g_itrup&MASK_ITRVALUE_BILLDOOR)!=0)
 		return RESULT_FAIL;
	if((g_itrdown&MASK_ITRVALUE_BILLDOOR)!=MASK_ITRVALUE_BILLDOOR)
 		return RESULT_FAIL;

	return RESULT_PASS;
}

//Intrusion 2(CASH DOOR)
int fun_T02013005()
{
	//check LOGBOX up value
	if((g_itrup&MASK_ITRVALUE_CASHDOOR)!=0)
 		return RESULT_FAIL;
	if((g_itrdown&MASK_ITRVALUE_CASHDOOR)!=MASK_ITRVALUE_CASHDOOR)
 		return RESULT_FAIL;

	return RESULT_PASS;
}

//Intrusion 3(MAIN DOOR)
int fun_T02013006()
{
	//check LOGBOX up value
	if((g_itrup&MASK_ITRVALUE_MAINDOOR)!=0)
 		return RESULT_FAIL;
	if((g_itrdown&MASK_ITRVALUE_MAINDOOR)!=MASK_ITRVALUE_MAINDOOR)
 		return RESULT_FAIL;

	return RESULT_PASS;
}

//Intrusion 4
int fun_T02013007()
{
	return RESULT_FAIL;
}

//Intrusion 5
int fun_T02013008()
{
	return RESULT_FAIL;
}

//Intrusion 6
int fun_T02013009()
{
	return RESULT_FAIL;
}

//Intrusion 7
int fun_T02013010()
{
	return RESULT_FAIL;
}

//Intrusion power
int fun_T02013001()
{
	return RESULT_FAIL;
}

//Intrusion power
int fun_T02013002()
{
	return RESULT_FAIL;
}

//Battery Voltage 1
int fun_T02014001()
{
	char szBuf[128];
	DSEIT_BATTERY eit_battery;

	eit_battery.u32_subtarget = EIT_BATTERY_VOLTAGE;
	EIT_CTLCODE(EIT_BATTERY1, ACTION_GET, &eit_battery);  
	DEBUGMSG("Battery 1 voltage= %d mV\r\n", eit_battery.u16_voltage*2);
	//return check_voltage_range(4300,eit_battery.u16_voltage,20.0);
	if( (eit_battery.u16_voltage*2 > g_testargu.battery_downval)&&(eit_battery.u16_voltage*2 < g_testargu.battery_upval) )
	//if( (eit_battery.u16_voltage > 1500)&&(eit_battery.u16_voltage < 4500) )
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

//Battery Voltage 2
int fun_T02014002()
{
	return RESULT_FAIL;
}

//+5VNG
int fun_T02015001()
{
	return RESULT_FAIL;
}

//+12VNG
int fun_T02015002()
{
	return RESULT_FAIL;
}

// for gpc 3800 GPIO test
int fun_TestPort_3800(unsigned int out_portname,int out_bit,unsigned int in_portname,int in_bit)
{
	DSGPC_PORT	     libport;
	int				 passcnt=0;
	unsigned int	 in_portbit,out_portbit,in_portnum,out_portnum;
	unsigned int	 in_portdata,out_portdata;

	in_portbit=BITSHIFT(in_bit);
	out_portbit=BITSHIFT(out_bit);
	out_portnum=BITSHIFT(out_portname);
	in_portnum=BITSHIFT(in_portname);
	//printf("in_portbit=%x, out_portbit=%x, in_portnum=%x, out_portnum=%x\r\n",in_portbit,out_portbit,in_portnum,out_portnum);
	DEBUGMSG("\r\nOUTPUT[%d].%d -> INPUT[%d].%d\r\n",out_portname,out_portbit,in_portname,in_portbit);
	
	//get original output port data
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = out_portnum;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	out_portdata =	libport.u08_data;
	
	//set output port data to high
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = out_portnum;
	libport.u08_data = out_portdata|out_portbit;
	//libport.u08_data = out_portbit;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_SET,&libport);
	DEBUGMSG("outputdata (0x%02X) to high (0x%02X)\r\n",out_portdata,libport.u08_data);
	usleep(100000);
	//verify input port data
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = in_portnum;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	in_portdata=libport.u08_data;
	//if((in_portdata&out_portbit)==0)
	//	passcnt++;
	DEBUGMSG("inputdata = 0x%02X,bitval=0x%02X,passcnt=%d\r\n",in_portdata,in_portdata&out_portbit,passcnt);
	if((in_portdata&in_portbit)==0)
		passcnt++;
	DEBUGMSG("inputdata=0x%02X , (in_portdata&in_portbit)=0x%02X , passcnt=%d\r\n",in_portdata,in_portdata&in_portbit,passcnt);

	//get original output port data
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = out_portnum;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	out_portdata =	libport.u08_data;

	//set output port data to low
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = out_portnum;
	libport.u08_data = out_portdata&~out_portbit;	
	//libport.u08_data = out_portbit;	
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_SET,&libport);
	DEBUGMSG("outputdata (0x%02X) to low (0x%02X)\r\n",out_portdata,libport.u08_data);
	usleep(100000);
	//verify input port data
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = in_portnum;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	in_portdata=libport.u08_data;
	//if((in_portdata&out_portbit)==out_portbit)
	//	passcnt++;
	DEBUGMSG("inputdata = 0x%02X,bitval=0x%02X,passcnt=%d\r\n",in_portdata,in_portdata&out_portbit,passcnt);
	if((in_portdata&in_portbit)==in_portbit)
		passcnt++;
	DEBUGMSG("inputdata=0x%02X , (in_portdata&in_portbit)=0x%02X , passcnt=%d\r\n",in_portdata,in_portdata&in_portbit,passcnt);

	if(passcnt==2)
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

/***************************************
*GPC 3800  TestPort
***************************************/


//Port C 7 -> Port A 0 GPC3800 
int fun_T02017001()
{	
	return 	fun_TestPort_3800(22,7,0,0);
}

//Port R 2 -> Port A 1
int fun_T02017002()
{	
	return 	fun_TestPort_3800(22,2,0,1);
}

//Port R 1 -> Port A 2
int fun_T02017003()
{
	return 	fun_TestPort_3800(22,1,0,2);
}

//Port R 0 -> Port A 3
int fun_T02017004()
{
	return 	fun_TestPort_3800(22,0,0,3);
}

//Port R 3 -> Port A 4
int fun_T02017005()
{
	return 	fun_TestPort_3800(22,3,0,4);
}

//Port R 4 -> Port A 5
int fun_T02017006()
{
	return 	fun_TestPort_3800(22,4,0,5);
}

//Port P 0 -> Port A 6
int fun_T02017007()
{
	return 	fun_TestPort_3800(20,0,0,6);
}

//Port P 1 -> Port A 7
int fun_T02017008()
{
	return 	fun_TestPort_3800(20,1,0,7);
}

//Port P 2 -> Port B 0
int fun_T02017009()
{	
	return 	fun_TestPort_3800(20,2,1,0);
}

//Port P 3 -> Port B 1 
int fun_T02017010()
{	
	return 	fun_TestPort_3800(20,3,1,1);
}

//Port P 4 -> Port C 1
int fun_T02017011()
{	
	return 	fun_TestPort_3800(20,4,2,1);
}

//Port P 5 -> Port B 2
int fun_T02017012()
{	
	return 	fun_TestPort_3800(20,5,1,2);
}

//Port Q 0 -> Port D 0
int fun_T02017013()
{	
	return 	fun_TestPort_3800(21,0,3,0);
}

//Port Q 1 -> Port D 2
int fun_T02017014()
{	
	return 	fun_TestPort_3800(21,1,3,2);
}

//Port Q 2 -> Port C 2
int fun_T02017015()
{	
	return 	fun_TestPort_3800(21,2,2,2);
}

//Port Q 3 -> Port B 5
int fun_T02017016()
{	
	return 	fun_TestPort_3800(21,3,1,5);
}

//Port R 6 -> Port C 3
int fun_T02017017()
{	
	return 	fun_TestPort_3800(22,6,2,3);
}

//Port Q 5 -> Port C 4
int fun_T02017018()
{	
	return 	fun_TestPort_3800(21,5,2,4);
}

//Port Q 6 -> Port B 6
int fun_T02017019()
{	
	return 	fun_TestPort_3800(21,6,1,6);
}


//Port Q 7 -> Port B 7
int fun_T02017020()
{	
	return 	fun_TestPort_3800(21,7,1,7);
}

//Port P 6 -> Port B 3
int fun_T02017021()
{	
	return 	fun_TestPort_3800(20,6,1,3);
}

//Port P 7 -> Port D 1
int fun_T02017022()
{	
	return 	fun_TestPort_3800(20,7,3,1);
}

//Port Q 0 -> Port C 0
int fun_T02017023()
{	
	return 	fun_TestPort_3800(21,0,2,0);
}

//Port Q 1 -> Port B 4
int fun_T02017024()
{	
	return 	fun_TestPort_3800(21,1,1,4);
}

//Port S 2 -> Port D 3
int fun_T02017025()
{	
	return 	fun_TestPort_3800(23,2,3,3);
}


//int fun_TestITR(char out_portname,U08 out_bit,U08 itrnum,U08 check_high,U08 check_low)
//DK 20160317 GPC3800
int fun_TestITR_3800(unsigned int out_portname,int out_bit,U08 itrnum,U08 itrbitnum)
{
	DSGPC_PORT	     libport;
	DSEIT_INFO		 eitinfo;
	int				 res,passcnt=0;
	U08			 	 itrbitval=0,out_portbit,out_portdata;
	unsigned int 	 out_portnum=0;

	out_portbit=BITSHIFT(out_bit);
	itrbitval=BITSHIFT(itrbitnum);
	out_portnum=BITSHIFT(out_portname);

	//DEBUGMSG("\r\nOUTPUT[%c].%d -> ITR[%d]\r\n",out_portname,out_bit,itrnum);
	
	//get original output port data
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = out_portnum;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	out_portdata =	libport.u08_data;
	
	//set output port data to high
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = out_portnum;
	libport.u08_data = out_portdata|out_portbit;	
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_SET,&libport);
	//DEBUGMSG("outputdata (0x%02X) to high (0x%02X)\r\n",out_portdata,libport.u08_data);
	sleep(1);	
	eitinfo.u32_subtarget = EIT_INFO_ITRSTATUS;
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eitinfo);	
	sleep(1);
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eitinfo);
	g_itrup = eitinfo.u08_itrstatus;
	//DEBUGMSG("g_itrup=0x%x,g_itrup&itrbitval=0x%x\r\n",g_itrup,g_itrup&itrbitval);
	//if(g_itrup==check_high)
	//	passcnt++;
	if((g_itrup&itrbitval&0x3F)==0)
		passcnt++;
	
	//get original output port data
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = out_portnum;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	out_portdata =	libport.u08_data;

	//set output port data to low
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = out_portnum;
	libport.u08_data = out_portdata&~out_portbit;	
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_SET,&libport);
	//DEBUGMSG("outputdata (0x%02X) to low (0x%02X)\r\n",out_portdata,libport.u08_data);
	sleep(1);
	eitinfo.u32_subtarget = EIT_INFO_ITRSTATUS;
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eitinfo);
	sleep(1);	
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eitinfo);
	g_itrdown = eitinfo.u08_itrstatus;
	//DEBUGMSG("g_itrdown=0x%x,g_itrdown&itrbitval=0x%x\r\n",g_itrdown,g_itrdown&itrbitval);
	//if(g_itrdown==check_low)
	//	passcnt++;
	if((g_itrdown&itrbitval&0x3F)==(itrbitval&0x3F))
		passcnt++;

	//DEBUGMSG("passcnt=%d\r\n",passcnt);
	if(passcnt==2)
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

//ITR1 -> Port P 7 //(0xfc->0xfe)
int fun_T02017026()
{	
	//return fun_TestITR('P',3,1,0x98,0x9c);
	return fun_TestITR_3800(20,7,1,1);
}

//ITR2 -> Port S 1
int fun_T02017027()
{	
	//return fun_TestITR('P',4,2,0x94,0x9c);
	return fun_TestITR_3800(23,1,2,2);
}

//ITR3 -> Port S 0
int fun_T02017028()
{	
	//return fun_TestITR('P',5,3,0x8c,0x9c);
	return fun_TestITR_3800(23,0,3,3);
}

//ITR4 -> Port Q 4
int fun_T02017029()
{	
	//return fun_TestITR('P',5,3,0x8c,0x9c);
	return fun_TestITR_3800(21,4,4,4);
}

//ITR5 -> Port R 5
int fun_T02017030()
{
	int res=RESULT_FAIL;
	//return fun_TestITR('P',5,3,0x8c,0x9c);
	res = fun_TestITR_3800(22,5,5,5);
	
	//g_diaginfo.bFinalDipswitch = true;
	auto_final = 1;
	return res;
}

//Fan LED 20160309 DK
int fun_T02022001()	
{
	int res=RESULT_FAIL;
	int i=0;
	DSGPC_HW hw;
	int time=0,pos=0;

	//auto_final = 1;
	
	while(!auto_final)
	{
		pos=(pos+1)%8;
		showjpeg(0,0,PATH_WAIT_RESULT); 	
		showjpeg(80*pos,400,PATH_WAIT_RESULT1);
		//printf("02013003:auto_final = %d\r\n",auto_final);
		usleep(90000);
	}
	
	//set all fan to maximum speed
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = 0xFF;
	res = GPC_CTLCODE(GPC_FAN_HW_0, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Set Fan Duty Error\n");
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = 0xFF;
	res = GPC_CTLCODE(GPC_FAN_HW_1, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Set Fan Duty Error\n");
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = 0xFF;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Set Fan Duty Error\n");
	hw.u32_subtarget = FAN_HW_DUTY;
	hw.u08_fanduty = 0xFF;
	res = GPC_CTLCODE(GPC_FAN_HW_3, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Set Fan Duty Error\n");
	usleep(1000);
	//Set all target period value appropriate to change LED status to "orange light"
	hw.u32_subtarget = FAN_HW_TARGET_PERIOD;
	//hw.u08_fanperiod = 0x20;
	hw.u08_fanperiod = 0x25;
	res = GPC_CTLCODE(GPC_FAN_HW_0, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Set target period Error\n");	
	hw.u32_subtarget = FAN_HW_TARGET_PERIOD;
	hw.u08_fanperiod = 0x25;
	res = GPC_CTLCODE(GPC_FAN_HW_1, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Set target period Error\n");
	hw.u32_subtarget = FAN_HW_TARGET_PERIOD;
	hw.u08_fanperiod = 0x25;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Set target period Error\n");
	hw.u32_subtarget = FAN_HW_TARGET_PERIOD;
	hw.u08_fanperiod = 0xA0;
	//hw.u08_fanperiod = 0x25;
	res = GPC_CTLCODE(GPC_FAN_HW_3, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Set target period Error\n");

	showjpeg(0,0,"./pics/800_600/fan_8led.jpg");
	sleep(4);

	res = ask_reslut(0,0);

	hw.u32_subtarget = FAN_HW_TARGET_PERIOD;
	hw.u08_fanperiod = 0x00;
	res = GPC_CTLCODE(GPC_FAN_HW_0, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Set target period Error\n");	
	hw.u32_subtarget = FAN_HW_TARGET_PERIOD;
	hw.u08_fanperiod = 0x00;
	res = GPC_CTLCODE(GPC_FAN_HW_1, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Set target period Error\n");
	hw.u32_subtarget = FAN_HW_TARGET_PERIOD;
	hw.u08_fanperiod = 0x00;
	res = GPC_CTLCODE(GPC_FAN_HW_2, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Set target period Error\n");
	hw.u32_subtarget = FAN_HW_TARGET_PERIOD;
	hw.u08_fanperiod = 0x00 ;
	res = GPC_CTLCODE(GPC_FAN_HW_3, ACTION_SET, &hw);
	if (res != GPC_FUN_RES_SUCC)
		printf("Set target period Error\n");

	return res;

}


////DK 20161014 for GPC version check
int fun_T02017033()
{

	long int rData_1,rData_2;
	iopl(3);
	
	outl(0x80040008,0xcf8);		//04:00.0
	usleep(100);
	rData_2 = inb(0xcfc);
	outl(0x80030008,0xcf8);		//03:00.0
	usleep(100);	
	rData_1 = inb(0xcfc);
	printf("rData_1 = 0x%02X, rData_2 = 0x%02X\r\n",rData_1,rData_2);
	iopl(0);

	char buf[3];
	int i;
	system(" lspci -n | grep '1573:3800' > ./data/gpc_rev");
	FILE *fp=fopen("./data/gpc_rev","r+");
	fseek(fp,29,SEEK_SET);
	fgets(buf,3,fp);
	fclose(fp);
	i = atoi(buf);
	//printf("i = %02d\r\n",i);

}


/*
// for gpc 2800 GPIO test
int fun_TestPort(char out_portname,U08 out_bit,char in_portname,U08 in_bit)
{
	DSGPC_PORT	     libport;
	int				 passcnt=0;
	U08			 	 in_portbit,out_portbit;
	U08				 in_portdata,out_portdata;

	in_portbit=BITSHIFT(in_bit);
	out_portbit=BITSHIFT(out_bit);

	DEBUGMSG("\r\nOUTPUT[%c].%d -> INPUT[%c].%d\r\n",out_portname,out_bit,in_portname,in_bit);

	//get original output port data
	libport.u32_subtarget = PORT_ACCESS;
	libport.u08_portnum = out_portname;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	out_portdata =	libport.u08_data;

	//set output port data to high
	libport.u32_subtarget = PORT_ACCESS;
	libport.u08_portnum = out_portname;
	libport.u08_data = out_portdata|out_portbit;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_SET,&libport);
	DEBUGMSG("outputdata (0x%02X) to high (0x%02X)\r\n",out_portdata,libport.u08_data);
	usleep(100000);
	//verify input port data
	libport.u32_subtarget = PORT_ACCESS;
	libport.u08_portnum = in_portname;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	in_portdata=libport.u08_data;
	//if((in_portdata&out_portbit)==0)
	//	passcnt++;
	//DEBUGMSG("inputdata = 0x%02X,bitval=0x%02X,passcnt=%d\r\n",in_portdata,in_portdata&out_portbit,passcnt);
	if((in_portdata&in_portbit)==0)
		passcnt++;
	DEBUGMSG("inputdata=0x%02X , (in_portdata&in_portbit)=0x%02X , passcnt=%d\r\n",in_portdata,in_portdata&in_portbit,passcnt);

	//get original output port data
	libport.u32_subtarget = PORT_ACCESS;
	libport.u08_portnum = out_portname;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	out_portdata =	libport.u08_data;

	//set output port data to low
	libport.u32_subtarget = PORT_ACCESS;
	libport.u08_portnum = out_portname;
	libport.u08_data = out_portdata&~out_portbit;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_SET,&libport);
	DEBUGMSG("outputdata (0x%02X) to low (0x%02X)\r\n",out_portdata,libport.u08_data);
	usleep(100000);
	//verify input port data
	libport.u32_subtarget = PORT_ACCESS;
	libport.u08_portnum = in_portname;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	in_portdata=libport.u08_data;
	//if((in_portdata&out_portbit)==out_portbit)
	//	passcnt++;
	//DEBUGMSG("inputdata = 0x%02X,bitval=0x%02X,passcnt=%d\r\n",in_portdata,in_portdata&out_portbit,passcnt);
	if((in_portdata&in_portbit)==in_portbit)
		passcnt++;
	DEBUGMSG("inputdata=0x%02X , (in_portdata&in_portbit)=0x%02X , passcnt=%d\r\n",in_portdata,in_portdata&in_portbit,passcnt);

	if(passcnt==2)
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}
*/

/***************************************
*GPC 2800  TestPort
***************************************/
/*
//Port R 7 -> Port A 0 GPC2800
int fun_T02017001()
{
	return 	fun_TestPort('R',7,'A',0);
}

//Port R 2 -> Port A 1
int fun_T02017002()
{
	return 	fun_TestPort('R',2,'A',1);
}

//Port R 1 -> Port A 2
int fun_T02017003()
{
	return 	fun_TestPort('R',1,'A',2);
}

//Port R 0 -> Port A 3
int fun_T02017004()
{
	return 	fun_TestPort('R',0,'A',3);
}

//Port R 3 -> Port A 4
int fun_T02017005()
{
	return 	fun_TestPort('R',3,'A',4);
}

//Port R 4 -> Port A 5
int fun_T02017006()
{
	return 	fun_TestPort('R',4,'A',5);
}

//Port P 0 -> Port A 6
int fun_T02017007()
{
	return 	fun_TestPort('P',0,'A',6);
}

//Port P 1 -> Port A 7
int fun_T02017008()
{
	return 	fun_TestPort('P',1,'A',7);
}

//Port P 2 -> Port B 0
int fun_T02017009()
{
	return 	fun_TestPort('P',2,'B',0);
}

//Port P 3 -> Port B 1
int fun_T02017010()
{
	return 	fun_TestPort('P',3,'B',1);
}

//Port P 4 -> Port C 1
int fun_T02017011()
{
	return 	fun_TestPort('P',4,'C',1);
}

//Port P 5 -> Port B 2
int fun_T02017011()
{
	return 	fun_TestPort('P',5,'B',2);
}

//Port Q 0 -> Port D 0
int fun_T02017013()
{
	return 	fun_TestPort('Q',0,'D',0);
}

//Port Q 1 -> Port D 2
int fun_T02017014()
{
	return 	fun_TestPort('Q',1,'D',2);
}

//Port Q 2 -> Port C 2
int fun_T02017015()
{
	return 	fun_TestPort('Q',2,'C',2);
}

//Port Q 3 -> Port B 5
int fun_T02017016()
{
	return 	fun_TestPort('Q',3,'B',5);
}

//Port R 6 -> Port C 3
int fun_T02017017()
{
	return 	fun_TestPort('R',6,'C',3);
}

//Port Q 5 -> Port C 4
int fun_T02017018()
{
	return 	fun_TestPort('Q',5,'C',4);
}

//Port Q 6 -> Port B 6
int fun_T02017019()
{
	return 	fun_TestPort('Q',6,'B',6);
}


//Port Q 7 -> Port B 7
int fun_T02017020()
{
	return 	fun_TestPort('Q',7,'B',7);
}

//Port P 6 -> Port B 3
int fun_T02017021()
{
	return 	fun_TestPort('P',6,'B',3);
}

//Port P 7 -> Port D 1
int fun_T02017022()
{
	return 	fun_TestPort('P',7,'D',1);
}

//Port Q 0 -> Port C 0
int fun_T02017023()
{
	return 	fun_TestPort('Q',0,'C',0);
}

//Port Q 1 -> Port B 4
int fun_T02017024()
{
	return 	fun_TestPort('Q',1,'B',4);


//Port S 2 -> Port D 3
int fun_T02017025()
{
	return 	fun_TestPort('S',2,'D',3);
}
*/

/*
//int fun_TestITR(char out_portname,U08 out_bit,U08 itrnum,U08 check_high,U08 check_low)
int fun_TestITR(char out_portname,U08 out_bit,U08 itrnum,U08 itrbitnum)
{
	DSGPC_PORT	     libport;
	DSEIT_INFO		 eitinfo;
	int				 passcnt=0;
	U08			 	 itrbitval=0,out_portbit,out_portdata;

	out_portbit=BITSHIFT(out_bit);
	itrbitval=BITSHIFT(itrbitnum);

	DEBUGMSG("\r\nOUTPUT[%c].%d -> ITR[%d]\r\n",out_portname,out_bit,itrnum);

	//get original output port data
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = out_portname;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	out_portdata =	libport.u08_data;

	//set output port data to high
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = out_portname;
	libport.u08_data = out_portdata|out_portbit;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_SET,&libport);
	DEBUGMSG("outputdata (0x%02X) to high (0x%02X)\r\n",out_portdata,libport.u08_data);
	sleep(1);
	eitinfo.u32_subtarget = EIT_INFO_ITRSTATUS;
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eitinfo);
	sleep(1);
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eitinfo);
	g_itrup = eitinfo.u08_itrstatus;
	DEBUGMSG("g_itrup=0x%x,g_itrup&itrbitval=0x%x\r\n",g_itrup,g_itrup&itrbitval);
	//if(g_itrup==check_high)
	//	passcnt++;
	if((g_itrup&itrbitval&0x3F)==0)
		passcnt++;

	//get original output port data
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = out_portname;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_GET,&libport);
	out_portdata =	libport.u08_data;

	//set output port data to low
	libport.u32_subtarget = PORT_ACCESS;
	libport.u32_portnum = out_portname;
	libport.u08_data = out_portdata&~out_portbit;
	GPC_CTLCODE(GPC_DATA_PORT,ACTION_SET,&libport);
	DEBUGMSG("outputdata (0x%02X) to low (0x%02X)\r\n",out_portdata,libport.u08_data);
	sleep(1);
	eitinfo.u32_subtarget = EIT_INFO_ITRSTATUS;
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eitinfo);
	sleep(1);
	EIT_CTLCODE(EIT_INFO, ACTION_GET, &eitinfo);
	g_itrdown = eitinfo.u08_itrstatus;
	DEBUGMSG("g_itrdown=0x%x,g_itrdown&itrbitval=0x%x\r\n",g_itrdown,g_itrdown&itrbitval);
	//if(g_itrdown==check_low)
	//	passcnt++;
	if((g_itrdown&itrbitval&0x3F)==(itrbitval&0x3F))
		passcnt++;

	DEBUGMSG("passcnt=%d\r\n",passcnt);
	if(passcnt==2)
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}
*/

