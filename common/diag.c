#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

//#include "Testboard_lib.h"
#include "../in_version.h"
#include "Diag.h"
#include "Result.h"
#include "TestTable.h"
#include "UI.h"
#include "debug.h"
#include "gpclib.h"
#include "eitlib.h"
#include "Attach.h"
#include "db.h"		//for return the flag or all test result


//**********************************************************
//**   Global variables
//**********************************************************
void FixedTest_callbackfun(U32 cb_type, U16 cb_src, U16 cb_data, void *context);

DIAGINFO			g_diaginfo;
DIAGINFO			g_diaginfo_sram;
//SQL_DB 				sql_db;

extern TEST_ACTION g_DiagAutoTest[];
extern TEST_ACTION g_DiagManuTest[];

U32	g_cent_x,g_cent_y;
int fResult_a = 0; //all test pass or not

int scanBarCode_keyin()
{
	char szSN1[32],szSN2[32];
	char szBuf[256];
start:
	dlg_clr_result();
	SETPOS(40,3);
	dialog_inputbox("1st BarCode","Please input the Bar-Code",10,40,0,false);
	strcpy(szSN1,RESULT);
	dlg_clr_result();
	SETPOS(40,13);
	dialog_inputbox("2nd BarCode","Please input the Bar-Code again",10,40,0,false);
	strcpy(szSN2,RESULT);
	if( strcmp(szSN1,szSN2) || (strlen(szSN1)==0) || (strlen(szSN2)==0))
	{
		OUTPUT("BarCode not marched, please do this again!");
		goto start;
	}
	else
	{
		strcpy(g_diaginfo.szBarCode,szSN1);
		OUTPUT("Pass!Press any key to start Test....");
	}

	//SETPOS_MAINWIN();

	return RESULT_PASS;
}

//without dialog.
int scanBarCode_keyin_NoDiag()
{
	int i,length1=0,length2=0;
	char szSN1[32],szSN2[32];
	
start:
	printf("INPUT SERIAL NUMBER:\r\n");
	scanf("%s",szSN1);
	//printf("szSN1 = %s\r\n", szSN1);
	printf("INPUT SERIAL AGAIN:\r\n");
	scanf("%s",szSN2);
	//printf("szSN2 = %s\r\n", szSN2);

	length1 = strlen(szSN1);
	length2 = strlen(szSN2);

	if(length1 != length2)
	{
		printf("BarCode not marched, please do this again!\r\n");
		sleep(3);
		system("clear");
		goto start;
	}
	for(i=0;i<length1;i++)
	{
		if(szSN1[i] != szSN2[i])
		{
			printf("BarCode not marched, please do this again!\r\n");
			sleep(3);
			system("clear");
			goto start;
		}
	}
	strcpy(g_diaginfo.szBarCode,szSN1);

	return RESULT_PASS;
}


int scanBarCode_RS232(int comnum)
{
	int fd;
	char szBuf[254];

	sprintf(szBuf,"/dev/ttyS%d",comnum);
	fd = open(szBuf, O_RDWR | O_NOCTTY | O_NDELAY);
	//fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		DEBUGMSG("open_port: Unable to open COM Port");
		return FUN_FAIL;
	}
	else
	{
		fcntl(fd, F_SETFL, 0);
	}
	initport(fd);
	//initport_raw(fd);

	//dialog_msgbox("BarCode","   Please input the Bar-Code",5,40,0);
	//printf("===============   SCAN   ===============\r\n");
	showjpeg(100,300,"./pics/800_600/scanbarcode.jpg");
	DEBUGMSG("\r\nInput Barcode\r\n");
	if (readport(fd,szBuf))
	{
		DEBUGMSG("read failed\n");
		//printf("read failed!!!!!\n");
		close(fd);
		return FUN_FAIL;
	}
	//printf("barcode from szBuf:%s\r\n",szBuf);
	strcpy(g_diaginfo.szBarCode,szBuf);
	DEBUGMSG("barcode from szBarCode:%s\r\n",g_diaginfo.szBarCode);
	//OUTPUT("Press any key to start Test....");
	close(fd);
	//sleep(10);
	return RESULT_PASS;
}

int scanBarCode_USB()
{
	int fd;
	char szBuf[256];

	//dialog_msgbox("BarCode","   Please input the Bar-Code",5,40,0);
	showjpeg(100,300,"./pics/800_600/scanbarcode.jpg");

	scanf("%s", szBuf);
	strcpy(g_diaginfo.szBarCode,szBuf);
	printf("barcode from szBuf:%s\r\n",szBuf);

	return RESULT_PASS;
}


/**********************************************************
	Print FinRecord  2015.05.06 - 2016.11.15 ver003
**********************************************************/

int output_FinRecord(char *szBarCode, int fResult)
{
	char szFile[256];
	char szBuf[512],szMac[128];
	int i=0,j=0,snlen=0;						//i=original file pointer,j=szBarCode strlen;
	FILE *file;
	int ethcnt,timebuf[30];
	char tbuf[30];
	struct tm *tm_ptr,timestruct;
	time_t the_time;

	(void) time(&the_time);
	tm_ptr=localtime(&the_time);
	//printf("strptime gives:\n");
	//printf("date: %02d%02d%02d\n",tm_ptr->tm_year%100,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
	//printf("time: %02d%02d%02d\n",tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
	sprintf(timebuf, "20%02d/%02d/%02d_%02d:%02d:%02d",tm_ptr->tm_year%100,tm_ptr->tm_mon+1,tm_ptr->tm_mday,tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
	//printf("start test time = %s\n", timebuf);

	//printf("start test time = %s\n", timebuf);

	//second way to record time
	//system("date > ./data/record_time");
	//FILE *fp = fopen("./data/record_time","r+");
	//fgets(tbuf,(sizeof(tbuf)-1),fp);
	//printf("tbuf = %s\r\n",tbuf);

	snlen=strlen(szBarCode);
	file = fopen(FIN_RECORD_FILE,"r+");
	if(file == NULL)
	{
		DEBUGMSG("Creating new 'FinRecord' file\r\n");
		file = fopen(FIN_RECORD_FILE,"w+");
		ethcnt=GetNetCardCount()-1;		//-1 for usb wifi
		//system("clear");
		//printf("total net card = %d\r\n",ethcnt);
		//sleep(3);
		if(ethcnt == 1)
			fprintf(file,"T2,Date/Time,S/N,MAC1,Summary,'001,'002,'003,'004,'005,'006,'007,'008,'009,'010,'011,'012,'013,'014,'015,'016,'017,'018,'019,'020,'021,'022,'023,'024,'025,'026,'027,'028,'029,'030,'031,'032,'033,'034,'035,'036,'037,'038,'039,'040,'041,'042,'043,'044,'045,'046,'047,'048,'049,'050\r\n");
		else
			fprintf(file,"T2,Date/Time,S/N,MAC1,MAC2,Summary,'001,'002,'003,'004,'005,'006,'007,'008,'009,'010,'011,'012,'013,'014,'015,'016,'017,'018,'019,'020,'021,'022,'023,'024,'025,'026,'027,'028,'029,'030,'031,'032,'033,'034,'035,'036,'037,'038,'039,'040,'041,'042,'043,'044,'045,'046,'047,'048,'049,'050\r\n");
	}
	fseek(file,0,SEEK_END);
	getMacAddr2(szMac);
	//fprintf(file,"T2,%s,%s,%s",tbuf,g_diaginfo.szBarCode,szMac);
	fprintf(file,"T2,%s,%s,%s,",timebuf,g_diaginfo.szBarCode,szMac);
	//printf("Finrecord: %s\r\n",szMac);
	fseek(file,0,SEEK_END);

	if(fResult == Pass)
	{
		fseek(file,0,SEEK_END);
		fprintf(file,"PASS,");
	}
	if(fResult == Fail)
	{
		fseek(file,0,SEEK_END);
		fprintf(file,"FAIL,");
	}

	//start to fill content: autotest item
	for(i=0;i<g_diaginfo.autoItems;i++)
	{
		if(g_DiagAutoTest[i].resultlog.pResult == NULL)
		{
			fprintf(file,"N/A,");
			continue;
		}
		if(g_DiagAutoTest[i].resultlog.pResult->resultID == RESULT_PASS)
		{
			fprintf(file,"Pass,");
		}
		else
		{
			fprintf(file,"Fail,");
		}
	}
	//start to fill content: manualtest item
	fseek(file,0,SEEK_END);
	for(i=0;i<g_diaginfo.manuItems;i++)
	{
		if(g_DiagManuTest[i].resultlog.pResult == NULL)
		{
			fprintf(file,"N/A,");
			continue;
		}
		if(g_DiagManuTest[i].resultlog.pResult->resultID == RESULT_PASS)
		{
			fprintf(file,"Pass,");
		}
		else
		{
			fprintf(file,"Fail,");
		}
	}
	fseek(file,0,SEEK_END);
	fprintf(file,"\r\n");

	//fclose(fp);
	fclose(file);


/*
    if((i=Replaceword(20, szBarCode, NULL, FIN_RECORD_FILE, NULL)) >0)
    {
		//printf("output i = %d\n",i);
		//printf("Replace function -- szBarCode = %s\n",szBarCode);
        if(fResult == Pass);
        {
            fseek(file,(i+25),SEEK_SET);			//"i+x" depend on format
            fprintf(file,"Pass");
        }
        if(fResult == Fail)
        {
            fseek(file,(i+25),SEEK_SET);
            fprintf(file,"Fail");
        }
        fclose(file);
    }
	else
	{
		i=(18-snlen);							// 18 = limit of sn strlen
		//printf("enter no match loop:\n");
        if(fResult == Pass)
        {
            fseek(file,0,SEEK_END);
            fprintf(file,"  %s",szBarCode);
			for(j=0;(j<(26-snlen));j++)			// 26 = 18 + 8" "
			{
				fseek(file,0,SEEK_END);
				fprintf(file," ");
			}
			fseek(file,0,SEEK_END);
            fprintf(file,"Pass\r\n");

        }
        if(fResult == Fail)
        {
            fseek(file,0,SEEK_END);
            fprintf(file,"  %s",szBarCode);
			for(j=0;(j<(26-snlen));j++)
			{
				fseek(file,0,SEEK_END);
				fprintf(file," ");
			}
			fseek(file,0,SEEK_END);
            fprintf(file,"Fail\r\n");
        }
        fclose(file);
	}
*/
	return FUN_SUCC;
}

int output_MacAddrTable(char *szBarCode)
{
	char szFile[256];
	char szBuf[128];
	FILE *file;

	file = fopen(MACADDR_FILE,"r+");
	if(file == NULL)
	{
		DEBUGMSG("Creating new 'MacAddrTable' file\r\n");
		file = fopen(MACADDR_FILE,"w+");
		fprintf(file,"     SN               MAC Address    \r\n");
		fprintf(file,"-------------------------------------\r\n");
	}

	fseek(file,0,SEEK_END);
	//getMacAddr(szBuf);
	getMacAddr2(szBuf);
	//printf("mac buf = %s\r\n",szBuf);
	fprintf(file,"  %s          %s\r\n",szBarCode,szBuf);
	//memset(szBuf,0,128);
	fclose(file);
}

int outputRecord()
{
	char szBuf[256],szMac[512];
	FILE *file;
	int i,cnt,res=RESULT_PASS,resgame;
	int pos_x=0,pos_y=150;
	//int *resTab;

	CLEAR();

	//output MacAddress Table
	output_MacAddrTable(g_diaginfo.szBarCode);

	//output TestCase item record
	output_TestCaseRecord(g_diaginfo.szBarCode,g_DiagAutoTest,g_diaginfo.autoItems,g_DiagManuTest,g_diaginfo.manuItems);

	//output fixed test item record
	output_FixedRecord(g_DiagAutoTest,g_diaginfo.autoItems,g_DiagManuTest,g_diaginfo.manuItems);

	sprintf(szBuf,"%s/%s",RESULT_PATH,g_diaginfo.szBarCode);
	file = fopen(szBuf,"w+");

	//output MacAddress
	fprintf(file,"     SN               MAC Address    \r\n");
	fprintf(file,"-------------------------------------\r\n");
	getMacAddr2(szMac);
	fprintf(file,"  %s          %s\r\n",g_diaginfo.szBarCode,szMac);


	//recording auto test items
	fprintf(file,"\r\n Auto Test Items    \r\n");
	fprintf(file,"-------------------------------------\r\n");
	for(i=0;i<g_diaginfo.autoItems;i++)
	{
		if(g_DiagAutoTest[i].resultlog.pResult == NULL)
		{
			fprintf(file,"%03d,%s,Fail,Test item not implemented\r\n",g_DiagAutoTest[i].id,g_DiagAutoTest[i].resultlog.strItem);
			continue;
		}
		if(g_DiagAutoTest[i].resultlog.pResult->resultID == RESULT_PASS)
		{
			fprintf(file,"%03d,%s,Pass\r\n",g_DiagAutoTest[i].id,g_DiagAutoTest[i].resultlog.strItem);
		}
		else
		{
			fprintf(file,"%03d,%s,Fail\r\n",g_DiagAutoTest[i].id,g_DiagAutoTest[i].resultlog.strItem);
			res = RESULT_FAIL;
			fResult_a = 1;
		}
	}

	//recording manual test items
	fprintf(file,"\r\n Manual Test Items    \r\n");
	fprintf(file,"-------------------------------------\r\n");
	for(i=0;i<g_diaginfo.manuItems;i++)
	{
		if(g_DiagManuTest[i].resultlog.pResult == NULL)
		{
			fprintf(file,"%03d,%s,Fail,Test item not implemented\r\n",g_DiagManuTest[i].id,g_DiagManuTest[i].resultlog.strItem);
			continue;
		}
		if(g_DiagManuTest[i].resultlog.pResult->resultID == RESULT_PASS)
		{
			fprintf(file,"%03d,%s,Pass\r\n",g_DiagManuTest[i].id,g_DiagManuTest[i].resultlog.strItem);
		}
		else
		{
			fprintf(file,"%03d,%s,Fail\r\n",g_DiagManuTest[i].id,g_DiagManuTest[i].resultlog.strItem);
			res = RESULT_FAIL;
			fResult_a = 1;
		}
	}
	sql_db.summary_result = fResult_a;
	output_FinRecord(g_diaginfo.szBarCode,fResult_a);
	//output_UID_Record(g_diaginfo.szBarCode,&librootsecu,fResult_a);


/*
	//output GAME I/O result
	cnt=get_TestBoard_Result(NULL);
	resTab = malloc(sizeof(int)*(cnt+1));
	resgame = get_TestBoard_Result(resTab);
	fprintf(file,"\r\n Game I/O Result    \r\n");
	fprintf(file,"------------------------------------------------\r\n");
	fprintf(file,"TestBoard,Result(%s)\r\n",resgame==FUN_FAIL?"Fail":"Pass");
	for(i=0;i<cnt;i++)
	{
		fprintf(file,"CMD_%02d[%s]:%s\r\n",i+1,resTab[i]==RESULT_PASS?"Pass":"Fail",strary_res[i]);
	}

	free(resTab);
*/
	fclose(file);

	//display result
	if(res == RESULT_PASS)
	{
		showjpeg(0,0,PATH_RES_PASS);
	}
	else
	{
		showjpeg(0,0,PATH_RES_FAIL);
		//check_yes();
		for(i=0;i<g_diaginfo.autoItems;i++)
		{
			if(g_DiagAutoTest[i].resultlog.pResult->resultID == RESULT_FAIL)
			{
				display_num(pos_x,pos_y,g_DiagAutoTest[i].id,3);
				pos_x+=NUM_WIDTH*3+NUM_WIDTH;
				if(pos_x>(NUM_WIDTH*4*3))
				{
					pos_x=0;
					pos_y+=NUM_HEIGHT;
				}
			}
		}
		pos_x=0;
		pos_y+=NUM_HEIGHT;
		for(i=0;i<g_diaginfo.manuItems;i++)
		{
			if(g_DiagManuTest[i].resultlog.pResult->resultID == RESULT_FAIL)
			{
				display_num(pos_x,pos_y,g_DiagManuTest[i].id,3);
				pos_x+=NUM_WIDTH*3+NUM_WIDTH;
				if(pos_x>(NUM_WIDTH*4*3))
				{
					pos_x=0;
					pos_y+=NUM_HEIGHT;
				}
			}
		}

	}

	return res;
}

int show_verson()
{
	//get versions
	g_diaginfo.ver_main = VER_MAIN;
	g_diaginfo.ver_sub = VER_SUB;
	g_diaginfo.ver_edition = VER_EDITION;

	printf("\r\n\r\nVersion(%d.%d.%d) : Main:%d , Sub:%d , Edition:%d\r\n\r\n",g_diaginfo.ver_main,g_diaginfo.ver_sub,g_diaginfo.ver_edition,g_diaginfo.ver_main,g_diaginfo.ver_sub,g_diaginfo.ver_edition);
}

//copy log files to all storages
int cp_logfiles1(char *szProgPath)
{
	int i,res=RESULT_FAIL;
	char szBuf[2048],szUSBDev[256],resolved_path[2048];
	char *path_end;
	FILE *fp;

	strcpy(resolved_path,szProgPath);
	path_end = strrchr (resolved_path, '/');
	*path_end = '\0';

	//mount all devices
	mountAllStorage("sd","/mnt/sipdevs",11,0xFF);

	fp = popen("mount","r");
	if(fp!=NULL)
	{
		for(i=0;i<11;i++)
		{
			sprintf(szUSBDev,"sd%c1",'a'+i);
			sprintf(szBuf,"/mnt/sipdevs/%s%s",szUSBDev,szProgPath);
			printf("file:%s\r\n",szBuf);
			if(!access(szBuf,F_OK)) //find file
				continue;

			while(!feof(fp))
			{
				fgets(szBuf,2048,fp);
				DEBUGMSG("line:%s    dev:%s\r\n",szBuf,szUSBDev);
				if(strstr(szBuf,szUSBDev))
				{
					goto founddev;
				}
			}
		}
	}

founddev:
	sprintf(szBuf,"cp -r %s/record /mnt/sipdevs/%s > /dev/null 2>&1",resolved_path,szUSBDev);
	system(szBuf);

	sleep(5);

	//umount all devices
	unmountAllStorage("sd","/mnt/sipdevs",11);

	return res;
}

//copy log files to assign device
int cp_logfiles2(char *szProgPath,char *devname)
{
	int i,res=RESULT_FAIL;
	char szBuf[2048],resolved_path[2048];
	char *path_end;

	strcpy(resolved_path,szProgPath);
	path_end = strrchr (resolved_path, '/');
	*path_end = '\0';

	//mount device to /mnt/sipdevs
	sprintf(szBuf,"mount %s /mnt/sipdevs > /dev/null 2>&1",devname);
	//printf(szBuf);
	system(szBuf);

	//copy log files to mount point
	sprintf(szBuf,"cp -r %s/record /mnt/sipdevs > /dev/null 2>&1",resolved_path);
	//printf(szBuf);
	system(szBuf);

	sleep(5);

	//umount /mnt/sipdevs
	sprintf(szBuf,"umount /mnt/sipdevs > /dev/null 2>&1");
	//printf(szBuf);
	system(szBuf);
}

init_Dialog()
{
	FILE *input = stdin;
	char	szBuf[256];

	dialog_state.output = stderr;
	dialog_vars.begin_set = true;
	dialog_vars.keep_window = true;
	dialog_vars.item_help = false;
	dialog_vars.dlg_clear_screen = true;
	dialog_vars.separate_output = false;

	//init_dialog(input, dialog_state.output);

	//create record folder
	sprintf(szBuf,"mkdir -p %s",RECORD_PATH);
	system(szBuf);

	g_wpMainWin.posx = 0;
	g_wpMainWin.posy = 0;
	g_wpMainWin.width = COLS;
	g_wpMainWin.height = LINES;

	g_wpOutputWin.posx = 0;
	//g_wpOutputWin.posy = g_wpMainWin.posy+g_wpMainWin.height+1;
	g_wpOutputWin.posy = LINES-10;
	g_wpOutputWin.width = COLS;
	g_wpOutputWin.height = 10;

    initscr();
    cbreak();
    nonl();
	noecho();
    intrflush(stdscr,FALSE);
    keypad(stdscr,TRUE);
    refresh();
}

deinit_Dialog()
{
	endwin();
	dlg_clear();
	//end_dialog();
}

int init_Diag()
{

	int i;
	PTEST_ACTION	pTestItem = NULL;
/*
	DSEIT_LIB_SETTING eitlib;
	DSGPC_LIB		  gpclib;
	DSEIT_INFO		  eit_info;

	//initiate gpc library
	gpclib.u32_subtarget = GPC_LIB_INIT;
	gpclib.u08_type = TYPE_GPC2010_GM;
	gpclib.void_pDev = "/dev/gpcdrv";
	if(GPC_CTLCODE(GPC_LIB,ACTION_SET,&gpclib) != GPC_FUN_RES_SUCC)
	{
		printf("GPC LIBRARY INIT FAIL\r\n");
		return MSG_LIBRARY_INIT_FAIL;
	}

    //Setting CallBack Function
    gpclib.u32_subtarget = GPC_LIB_CALLBACKFUN;
    gpclib.int_callback_fun = FixedTest_callbackfun;
    if(GPC_CTLCODE(GPC_LIB,ACTION_SET,&gpclib) != GPC_FUN_RES_SUCC)
    {
    	printf("Set Callback function error!\r\n");
        return MSG_SETCALLBACK_FAIL;
	}
*/
/*
	//initiate eit library
	eitlib.u32_subtarget = EIT_LIB_SETTING_INIT;
	eitlib.u32_interface = EIT_INTERFACE_GPC;
	eitlib.u08_eittype   = TYPE_EIT020A;
	eitlib.u08_battype   = TYPE_BATTERY_CR2032;
	if(EIT_CTLCODE(EIT_LIB_SETTING, ACTION_SET, &eitlib) != EIT_FUN_RES_SUCC)
	{
		printf("EIT LIBRARY INIT FAIL\r\n");
		return MSG_LIBRARY_INIT_FAIL;
	}
*/
//#ifndef __TEST_FUN__
//	init_Dialog();
//#endif

	
	printf("init_TestItem start\r\n");
	init_TestItem();
	printf("init_TestItem end\r\n");

	//Initiating fixed test items
	printf("init_FixedTest start\r\n");
	init_FixedTest();
	printf("init_FixedTest end\r\n");
	//not use for sib6330
	get_fbres(&g_diaginfo.xres,&g_diaginfo.yres,&g_diaginfo.bits_per_pixel);
	g_cent_x = (g_diaginfo.xres/2)-(PIC_RES_WIDTH/2);
	g_cent_y = (g_diaginfo.yres/2)-(PIC_RES_HEIGHT/2);
	
	//caculating total Test Items and malloc memory for RESULT Log
	g_diaginfo.autoItems = getAutoTestNums();
	g_diaginfo.manuItems = getManuTestNums();
	g_diaginfo.totalItems = g_diaginfo.autoItems+g_diaginfo.manuItems;
	//g_diaginfo.pResultLogs = (PRESULT_LOG)malloc(g_diaginfo.totalItems * sizeof(RESULT_LOG));

	g_diaginfo.bFinalAuto = FALSE;
	g_diaginfo.bFinalManu = FALSE;
	g_diaginfo.bStopUpload = FALSE;
	//Fill in id for all test items(auto+manual)
	for(i=0;i<g_diaginfo.autoItems;i++)
	{
		pTestItem = &g_DiagAutoTest[i];
		pTestItem->id = i;
	}
	for(i=0;i<g_diaginfo.manuItems;i++)
	{
		pTestItem = &g_DiagManuTest[i];
		pTestItem->id = i+g_diaginfo.autoItems;
	}
}

int deinit_Diag()
{
	//free(g_diaginfo.pResultLogs);
	deinit_FixedTest();
//#ifndef __TEST_FUN__
//	deinit_Dialog();
//#endif
}

int fun_DiagAutoTest(PTEST_ACTION pTestItem,int count)
{
	int i,result;
	TEST_CASE_FUN 	pFun;

	for(i=0;i<count;i++)
	{
		/*
		if(i==count-1)
		{
			while(!g_diaginfo.bFinalManu)
			{
				usleep(1000);
			}
		}
		*/
		pFun = pTestItem[i].pfn_testfun;
		result = (*pFun)();
		pTestItem[i].resultlog.pResult = (PRESULT)getResult(result);
	}
	g_diaginfo.bFinalAuto = TRUE;
}

int fun_DiagManuTest(PTEST_ACTION pTestItem,int count)
{
	int i,result;
	TEST_CASE_FUN 	pFun;

	for(i=0;i<count;i++)
	{
		pFun = pTestItem[i].pfn_testfun;
		result = (*pFun)();
		pTestItem[i].resultlog.pResult = (PRESULT)getResult(result);
	}
	g_diaginfo.bFinalManu = TRUE;
}


void *autoTestThread(void *arg)
{
	fun_DiagAutoTest(g_DiagAutoTest,g_diaginfo.autoItems);
}

void *ftpUploadThread(void *arg)
{
	char szBuf[1024];
	char chSys[1024];
	FILE *fp = NULL;
	int res = 0;
	sprintf(chSys,"ls %s > ./data/uploadlist.txt",RECORD_PATH_NOTUPLOAD);
	system(chSys);

	fp = fopen("./data/uploadlist.txt", "r");

	while( fgets(szBuf,1024,fp) != NULL)
	{
		szBuf[strlen(szBuf) -1] = 0;
		if (g_diaginfo.bStopUpload)
			break;
		res = ftpUpload(szBuf);
		//if (!(res = ftpUpload(szBuf)))
			//if (!(res = ftpUpload(szBuf)))
			//	if (!(res = ftpUpload(szBuf)))
			//	{}

		if (res)
		{
			sprintf(chSys,"mv %s/%s %s",RECORD_PATH_NOTUPLOAD, szBuf, RECORD_PATH_UPLOADED);
			//sprintf(chSys,"mv ./record/NotUpload/%s ./record/Uploaded/", szBuf);
			printf("\n%s\n", chSys);
			system(chSys);
		}		
	}
	//system("rm ./data/uploadlist.txt");
	fclose(fp);
}


// 

void uploadFTP()
{
	char szBuf[1024];
	char chSys[1024];
	FILE *fp = NULL;
	int res = 0;
	sprintf(chSys,"ls %s > ./data/uploadlist.txt",RECORD_PATH_NOTUPLOAD);
	system(chSys);

	fp = fopen("./data/uploadlist.txt", "r");

	while( fgets(szBuf,1024,fp) != NULL)
	{
		szBuf[strlen(szBuf) -1] = 0;
		if (g_diaginfo.bStopUpload)
			break;
		res = ftpUpload(szBuf);
		//if (!(res = ftpUpload(szBuf)))
			//if (!(res = ftpUpload(szBuf)))
			//	if (!(res = ftpUpload(szBuf)))
			//	{}

		if (res)
		{
			sprintf(chSys,"mv %s/%s %s",RECORD_PATH_NOTUPLOAD, szBuf, RECORD_PATH_UPLOADED);
			//sprintf(chSys,"mv ./record/NotUpload/%s ./record/Uploaded/", szBuf);
			printf("\n%s\n", chSys);
			system(chSys);
		}		
	}
	//system("rm ./data/uploadlist.txt");
	fclose(fp);
}

/*
int SetCmd(U32 mask, U32 value)
{
	DSGPC_LAMP	lamp;

	lamp.u32_subtarget = LAMP_DATA_SWITCH;
	lamp.u08_onoff = GET_ON;
	GPC_CTLCODE(GPC_DATA_LAMP,ACTION_GET,&lamp);

	lamp.u32_subtarget = LAMP_DATA_SWITCH;
	lamp.u08_onoff = SET_ON;
	lamp.u32_data = (lamp.u32_data&mask)|value;
	GPC_CTLCODE(GPC_DATA_LAMP,ACTION_SET,&lamp);

	DEBUGMSG("SetCmd_SETON::u32_data=0x%08X\r\n",lamp.u32_data);
	return 0;
}
*/

int diag_main(int argc,char *argv[], int nBarcodeCom)
{
	int time=0,pos=0,file=0;
	struct timeval	systime;
	DSEIT_INFO eit_info;

	g_diaginfo.bFinalDipswitch = false;

	get_argument(argc,argv);
#if !defined(__NODIALOG__)
	//init_Dialog();
#endif
	init_Diag();

#if defined(__GAMING_TEST__)
	Init_gaming_test(argc,argv);
#endif

#if defined(__EXTERN_TEST__)
	Init_extern_test(argc,argv);
#endif

/*
	//get system timer and set it to EIT RTC
	gettimeofday(&systime,NULL);
	eit_info.u32_subtarget = EIT_INFO_RTC;
	eit_info.time_rtc.totalsec = systime.tv_sec;
	EIT_CTLCODE(EIT_INFO, ACTION_SET, &eit_info);
*/
	CLEAR();
	//dialog_msgbox("","",g_wpMainWin.height,g_wpMainWin.width,0);
#if defined(__BARCODE_KEYIN__)
	//scanBarCode_keyin();
	scanBarCode_keyin_NoDiag();	//for sib6330 temp use.
	//After keyin then init dialog.
	init_Dialog();
#elif defined(__BARCODE_USB__)
	scanBarCode_USB();

#else
	//init dialog before scan.
	init_Dialog();
printf("scanBarCode_RS232");
	scanBarCode_RS232(nBarcodeCom);

#endif
	CLEAR();

#if defined(__SERVER_V1__)
	//Launch ftp upload
	if(pthread_create(&g_diaginfo.ftpUpload_thread_id,NULL,ftpUploadThread,NULL))
    {
       printf("create ftpUpload thread error!!/r/n");
       return 1;
    }
#endif

	//Launch Auto test
    if(pthread_create(&g_diaginfo.autotest_thread_id,NULL,autoTestThread,NULL))
    {
       printf("create autotest thread error!!/r/n");
       return 1;
    }
	
	//Launch Manual Test
	fun_DiagManuTest(g_DiagManuTest,g_diaginfo.manuItems);

	//wait for autotest finish
	
	showjpeg(0,0,PATH_WAIT_RESULT);
	while(!g_diaginfo.bFinalAuto)
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
	

	//output result
	outputRecord();

	uploadFileCreate();

#if defined(__SERVER_V1__)
	//wait upload thread
	g_diaginfo.bStopUpload = TRUE;
	//pthread_kill(g_diaginfo.ftpUpload_thread_id, SIGUSR1);
	pthread_join (g_diaginfo.ftpUpload_thread_id, NULL);
	uploadCurrentResult();
#endif

	

#if defined(__GAMING_TEST__)
	//gaming_test_main();
#endif

#if defined(__EXTERN_TEST__)
	//extern_test_main();
#endif

#if defined(__GAMING_TEST__)
	//output_gaming_test_result();
#endif

#if defined(__EXTERN_TEST__)
	//output_extern_test_result();
#endif

#if defined(__EXTERN_TEST__)
	Deinit_extern_test();
#endif

#if defined(__GAMING_TEST__)
	Deinit_gaming_test();
#endif

	//de-initization
	deinit_Diag();
#if !defined(__NODIALOG__)
	deinit_Dialog();
#endif

	return 0;
}

