#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>

#include "Debug.h"
//#include "gpclib_type2.h"
//#include "eitlib.h"
#include "Diag.h"
#include "Result.h"
#include "FixedTest.h"
#include "Test_items.h"
#include "Utils.h"
//#include "extmode.h"
#include "UI.h"
#include "sio.h"

/*************************************************************************************
	General Items  
*************************************************************************************/

#undef ITEM_MAP
#define ITEM_MAP(item)	{item , ToString(item), {FIXLOG_TYPE_NOTEST,0,0}}

typedef struct _tagFIXEDTEST_RESULT
{
	int			type;   	//0:FIXLOG_TYPE_NOTEST, 1:FIXLOG_TYPE_TESTED
	U32			pass_num;
	U32			fail_num;
}FIXEDTEST_RESULT,*PFIXEDTEST_RESULT;

typedef struct _tagFIXEDTEST_LOG
{
    int					testid;			//identify number for test case
	char				strItem[16];	//Test case name like "T010001"
	FIXEDTEST_RESULT	Result;		//test result
}FIXEDTEST_LOG,*PFIXEDTEST_LOG;

FIXEDTEST_LOG g_FixedTestItems[] =
{
	/*
		Fixed Items
	*/	
	//System
	ITEM_MAP(T01002001),  //Product ID
	ITEM_MAP(T01003001),  //DRAM 
	ITEM_MAP(T01004001),  //IDE1
	ITEM_MAP(T01004002),  //IDE2
	ITEM_MAP(T01015001),  //SATA1
	ITEM_MAP(T01015002),  //SATA2
	ITEM_MAP(T01015003),  //SATA3
	ITEM_MAP(T01015004),  //SATA4
	ITEM_MAP(T01015005),  //OnBoard Storage1
	ITEM_MAP(T01015006),  //M.2-1
	ITEM_MAP(T01005001),  //Audio General(R+L)
	ITEM_MAP(T01005002),  //Audio Right
	ITEM_MAP(T01005003),  //Audio Left
	ITEM_MAP(T01005004),  //Audio Bass
	ITEM_MAP(T01006001),  //LAN1
	ITEM_MAP(T01006002),  //LAN2
	ITEM_MAP(T01008001),  //HDMI1
	ITEM_MAP(T01008002),  //HDMI2
	ITEM_MAP(T01009001),  //Vcore
	ITEM_MAP(T01010001),  //CPU Temp.
	ITEM_MAP(T01010002),  //System Temp.
	ITEM_MAP(T01009002),  //+12V
	ITEM_MAP(T01009003),  //+5V
	ITEM_MAP(T01009004),  //+3.3V
	ITEM_MAP(T01011001),  //PS2 Mouse
	ITEM_MAP(T01011002),  //PS2 Keyboard
	ITEM_MAP(T01012001),  //USB1
	ITEM_MAP(T01012002),  //USB2
	ITEM_MAP(T01012003),  //USB3
	ITEM_MAP(T01012004),  //USB4
	ITEM_MAP(T01012005),  //USB5
	ITEM_MAP(T01012006),  //USB6
	ITEM_MAP(T01013001),  //COM1
	ITEM_MAP(T01013002),  //COM2
	ITEM_MAP(T01013003),  //COM3
	ITEM_MAP(T01013004),  //COM4
	ITEM_MAP(T01013005),  //COM5
	ITEM_MAP(T01013006),  //COM6 
	ITEM_MAP(T01014001),  //GPIO-A	
	ITEM_MAP(T01014002),  //GPIO-B
	ITEM_MAP(T01014003),  //GPIO-C
	ITEM_MAP(T01014004),  //GPIO-D
	ITEM_MAP(T01016002),  //CF1	
	ITEM_MAP(T01017001),  //PCI1	
	ITEM_MAP(T01017002),  //PCI2
	ITEM_MAP(T01017003),  //Read RootSecu
	ITEM_MAP(T01023001),  //System RTC
    
	//Gaming Feature
	ITEM_MAP(T02001001),  //FW Ver.
	ITEM_MAP(T02002001),  //Meter1
	ITEM_MAP(T02002002),  //Meter2
	ITEM_MAP(T02002003),  //Meter3
	ITEM_MAP(T02002004),  //Meter4
	ITEM_MAP(T02002005),  //Meter5
	ITEM_MAP(T02002006),  //Meter6
	ITEM_MAP(T02002007),  //Meter7
	ITEM_MAP(T02002008),  //Meter8
	ITEM_MAP(T02003001),  //Coin Enable
	ITEM_MAP(T02003002),  //Coin A
	ITEM_MAP(T02003003),  //Coin B
	ITEM_MAP(T02004001),  //Bill enable
	ITEM_MAP(T02004002),  //Bill A
	ITEM_MAP(T02004003),  //Bill B
	ITEM_MAP(T02005001),  //Hopper Enable
	ITEM_MAP(T02005002),  //Hopper
	ITEM_MAP(T02006001),  //SRAM
	ITEM_MAP(T02007001),  //Timer A
	ITEM_MAP(T02005002),  //Timer B
	ITEM_MAP(T02005003),  //Timer C
	ITEM_MAP(T02005004),  //Timer D
	ITEM_MAP(T02008001),  //CCTalk 1
	ITEM_MAP(T02008002),  //CCTalk 2
	ITEM_MAP(T02019001),  //SPI Flash1
	ITEM_MAP(T02019002),  //SPI Flash2
	ITEM_MAP(T02010001),  //IIC0
	ITEM_MAP(T02010002),  //IIC1
	ITEM_MAP(T02012001),  //Secure RTC
	ITEM_MAP(T02013003),  //Intrusion 0
	ITEM_MAP(T02013004),  //Intrusion 1
	ITEM_MAP(T02013005),  //Intrusion 2
	ITEM_MAP(T02013006),  //Intrusion 3
	ITEM_MAP(T02013007),  //Intrusion 4
	ITEM_MAP(T02013008),  //Intrusion 5
	ITEM_MAP(T02013009),  //Intrusion 6
	ITEM_MAP(T02013010),  //Intrusion 7
	ITEM_MAP(T02013001),  //Intrusion power
	ITEM_MAP(T02013002),  //Intrusion alert
	ITEM_MAP(T02014001),  //Battery Voltage1
	ITEM_MAP(T02014002),  //Battery voltage 2
	ITEM_MAP(T02015001),  //+5VNG
	ITEM_MAP(T02015002),  //+12VNG
	ITEM_MAP(T02016001),  //HCS300
	ITEM_MAP(T02021001),  //ibutton
	ITEM_MAP(T02001001),  //AT88SA104
	ITEM_MAP(T02017001),  //GPIO-A
	ITEM_MAP(T02017002),  //GPIO-B
	ITEM_MAP(T02017003),  //GPIO-C
	ITEM_MAP(T02017004),  //GPIO-D
	ITEM_MAP(T02017005),
	ITEM_MAP(T02017006),
	ITEM_MAP(T02017007),
	ITEM_MAP(T02017008),
	ITEM_MAP(T02017009),
	ITEM_MAP(T02017010),
	ITEM_MAP(T02017011),
	ITEM_MAP(T02017012),
	ITEM_MAP(T02017013),
	ITEM_MAP(T02017014),
	ITEM_MAP(T02017015),
	ITEM_MAP(T02017016),
	ITEM_MAP(T02017017),
	ITEM_MAP(T02017018),
	ITEM_MAP(T02017019),
	ITEM_MAP(T02017020),
	ITEM_MAP(T02017021),
	ITEM_MAP(T02017022),
	ITEM_MAP(T02017023),
	ITEM_MAP(T02017024),
	ITEM_MAP(T02017025),
	ITEM_MAP(T02017026),
	ITEM_MAP(T02017027),
	ITEM_MAP(T02017028),
	ITEM_MAP(T02017029),
	ITEM_MAP(T02017030),
	ITEM_MAP(T02017031),
	ITEM_MAP(T02017032),
	ITEM_MAP(T02017033),
	ITEM_MAP(T02017034),
	ITEM_MAP(T02017035),
	ITEM_MAP(T02017036),
	ITEM_MAP(T02017037),
	ITEM_MAP(T02017038),
	ITEM_MAP(T02018001),  //Gaming I/O
//DK 20160304	
	ITEM_MAP(T01019001),
	ITEM_MAP(T01019002),
	ITEM_MAP(T01022001), 
	ITEM_MAP(T01020001),
	ITEM_MAP(T01020002),
	ITEM_MAP(T01020003),
	ITEM_MAP(T01020004),
};

int fill_FixedItemLog(FILE *file)
{
	char szBuf[1024];
	int i,itemcnt;
	char         *pch;
	FIXEDTEST_LOG	filelog;
	
	itemcnt = ARRAYCNT(g_FixedTestItems);

	while(fgets(szBuf,1024,file))
	{
		pch = strtok (szBuf, ",");
  		strcpy(filelog.strItem,pch);
		pch = strtok (NULL, ",");
		filelog.Result.type = strtol(pch, NULL, 10);
		pch = strtok (NULL, ",");
		filelog.Result.pass_num = strtol(pch, NULL, 10);
		pch = strtok (NULL, ",");
		filelog.Result.fail_num = strtol(pch, NULL, 10);
		//DEBUGMSG("fill_FixedItemLog:filelog[%s,%d,%d,%d]\r\n",filelog.strItem,filelog.Result.type,filelog.Result.pass_num,filelog.Result.fail_num);
		
		for(i=0;i<itemcnt;i++)
		{
			if(!strcmp(g_FixedTestItems[i].strItem,filelog.strItem))
			{
				g_FixedTestItems[i].Result.type = filelog.Result.type;
				g_FixedTestItems[i].Result.pass_num = filelog.Result.pass_num;
				g_FixedTestItems[i].Result.fail_num = filelog.Result.fail_num;				
			}
		}
	}
}

int output_TestCaseRecord(char *szBarCode,PTEST_ACTION autoitmes,int auto_num,PTEST_ACTION manuitems,int manu_num)
{
	char szBuf[256];
	FILE *file;
	int i,j,fixtest_num;

	fixtest_num = ARRAYCNT(g_FixedTestItems);
	for(i=0;i<auto_num;i++)
	{
		for(j=0;j<fixtest_num;j++)
		{
			if(autoitmes[i].resultlog.testid == g_FixedTestItems[j].testid)
			{
				if(autoitmes[i].resultlog.pResult->resultID == RESULT_FAIL)
				{
					sprintf(szBuf,"%s/%s",ANSYS_PATH,autoitmes[i].resultlog.strItem);
					file = fopen(szBuf,"r+");
					if(file == NULL)
					{
						file = fopen(szBuf,"w+");
					}
					fseek(file,0,SEEK_END);
					fprintf(file,"%s,",szBarCode);
					fclose(file);
				}
				break;	
			}
		}
	}
	for(i=0;i<manu_num;i++)
	{
		for(j=0;j<fixtest_num;j++)
		{
			if(manuitems[i].resultlog.testid == g_FixedTestItems[j].testid)
			{
				if(manuitems[i].resultlog.pResult->resultID == RESULT_FAIL)
				{
					sprintf(szBuf,"%s/%s",ANSYS_PATH,manuitems[i].resultlog.strItem);
					file = fopen(szBuf,"r+");
					if(file == NULL)
					{
						file = fopen(szBuf,"w+");
					}
					fseek(file,0,SEEK_END);
					fprintf(file,"%s,",szBarCode);
					fclose(file);
				}
				break;	
			}
		}
	}
	
	return RESULT_PASS;
}

int output_FixedRecord(PTEST_ACTION autoitmes,int auto_num,PTEST_ACTION manuitems,int manu_num)
{
	char szBuf[256];
	FILE *file;
	int i,j,fixtest_num;

	DEBUGMSG("output_FixedRecord:auto_num=%d, manu_num=%d\r\n",auto_num,manu_num);
	file = fopen(STATISTIC_FILE,"r+");
	if(file == NULL)
	{
		DEBUGMSG("Creating new 'fixedrec' file\r\n");
		file = fopen(STATISTIC_FILE,"w+");		
		fprintf(file,"TestCase     tested(1:yes,0:no)      Pass(Times)     Fail(Times)\r\n");
		fprintf(file,"---------------------------------------------------------------\r\n");
	}
	else
	{
		DEBUGMSG("Writing exist 'fixedrec' file\r\n");	
		fill_FixedItemLog(file);
	}

	fixtest_num = ARRAYCNT(g_FixedTestItems);
	for(i=0;i<auto_num;i++)
	{
		for(j=0;j<fixtest_num;j++)
		{
			if(autoitmes[i].resultlog.testid == g_FixedTestItems[j].testid)
			{
				g_FixedTestItems[j].Result.type = FIXLOG_TYPE_TESTED;
				if(autoitmes[i].resultlog.pResult->resultID == RESULT_PASS)
					g_FixedTestItems[j].Result.pass_num++;
				else
					g_FixedTestItems[j].Result.fail_num++;
				break;	
			}
		}
	}
	for(i=0;i<manu_num;i++)
	{
		for(j=0;j<fixtest_num;j++)
		{
			if(manuitems[i].resultlog.testid == g_FixedTestItems[j].testid)
			{
				g_FixedTestItems[j].Result.type = FIXLOG_TYPE_TESTED;
				if(manuitems[i].resultlog.pResult->resultID == RESULT_PASS)
					g_FixedTestItems[j].Result.pass_num++;
				else
					g_FixedTestItems[j].Result.fail_num++;
				break;	
			}
		}
	}

	fseek(file,0,SEEK_SET);
	for(i=0;i<fixtest_num;i++)
	{
		sprintf(szBuf,"%s,%d,%d,%d\r\n",g_FixedTestItems[i].strItem,g_FixedTestItems[i].Result.type,g_FixedTestItems[i].Result.pass_num,g_FixedTestItems[i].Result.fail_num);
		//DEBUGMSG(szBuf);
		fprintf(file,szBuf);
	}	
	fclose(file);
	
	return RESULT_PASS;
}

int init_FixedTest()
{
	init_tcil();

	return FUN_SUCC;
}

int deinit_FixedTest()
{
	deinit_tcil();

	return FUN_SUCC;
}

/*
void FixedTest_callbackfun(U32 cb_type, U16 cb_src, U16 cb_data, void *context)
{
	tcil_callbackfun(cb_type,cb_src,cb_data,context);
}
*/

