#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "sqlite3.h"
#include "result.h"
#include "debug.h"
#include "db.h"
#include "tcil.h"
#include "utils.h"
#include "diag.h"

//#define DEBUG

#ifdef DEBUG
#define DD(format,...)	printf("%s:%d: "format,__FILE__, __LINE__, ##__VA_ARGS__);		//Variety
#else
#define DD //
#endif

SQL_DB 				sql_db;
DIAGINFO			g_diaginfo;
TEST_ARGU			g_testargu;

extern TEST_ACTION g_DiagAutoTest[];
extern TEST_ACTION g_DiagManuTest[];



unsigned long int rand_int(int low, int hi)
{
	srand( (unsigned)time(NULL) );
    return (unsigned long int)((rand() / (RAND_MAX+1.0)) * (hi - low) + low);
}

int uploadCurrentResult()
{
	int res = 0;
	char chSys[1024] = {0};
	char chBarCode[256] = {0};

	//sprintf(chBarCode,"%s_%d.txt",g_diaginfo.szBarCode,g_testargu.sip_uuid);
	//sprintf(chBarCode,"%s.txt",g_diaginfo.szBarCode);

	//g_diaginfo.szFTPFileName

	res =  ftpUpload(g_diaginfo.szFTPFileName);

	if (res)
	{
		sprintf(chSys,"mv %s/%s %s",RECORD_PATH_NOTUPLOAD, g_diaginfo.szFTPFileName, RECORD_PATH_UPLOADED);
		//sprintf(chSys,"mv ./record/NotUpload/%s ./record/Uploaded/", szBuf);
		//printf("\n%s\n", chSys);
		system(chSys);
	}	
	return res;
}

int uploadFileCreate()
{
	int nEth = 0, i = 0, last_item;
	char chFTPFilePath[256] = {0};
	char chEth1[128] = {0}, chEth2[128] = {0};
	char chResult[1024] = {0}, chSummary[32] = {0};
	char chTime[64] = {0};
	char chFileTime[128] = {0};
	time_t tTime;
	struct tm *stTm;
	FILE *fp = NULL;
	//int nT2 = 0;
	//g_diaginfo.szBarCode

	
	//if (chBarCode[0] == 'g')
	//get time record
	(void) time(&tTime);
	stTm=localtime(&tTime);
	//DD("strptime gives:\n");
	DD("date: %02d%02d%02d\n",stTm->tm_year%100,stTm->tm_mon+1,stTm->tm_mday);
	DD("time: %02d%02d%02d\n",stTm->tm_hour,stTm->tm_min,stTm->tm_sec);
	sprintf(chTime, "20%02d/%02d/%02d_%02d:%02d:%02d",stTm->tm_year%100,stTm->tm_mon+1,stTm->tm_mday,stTm->tm_hour,stTm->tm_min, stTm->tm_sec );
	sprintf(chFileTime, "%02d%02d%02d_%02d%02d%02d",stTm->tm_year%100,stTm->tm_mon+1,stTm->tm_mday,stTm->tm_hour,stTm->tm_min, stTm->tm_sec );

	DD("start test time = %s\n", chTime);

	
//strcpy(chEth1, "N/A");
	nEth = GetNetCardCount();
	system("ifconfig | grep -o '[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]' > ./data/mac_addr");
	fp = fopen("./data/mac_addr","r+");
	fgets(chEth1,18,fp);
	if(nEth == 1)
	{
		strcpy(chEth2, "N/A");
	}
	if(nEth >= 2)
	{
		fseek(fp,18,SEEK_SET);
		fgets(chEth2, 18, fp);
		DD("db_process: szEth1 = %s\r\n",chEth2);

	}
	fclose(fp);

	//combine all test item result
	for(i = 0; i < g_diaginfo.autoItems; i++)
	{
		if(g_DiagAutoTest[i].resultlog.pResult == NULL)
		{
			strcat(chResult,"'N/A',");
			continue;
		}
		if(g_DiagAutoTest[i].resultlog.pResult->resultID == RESULT_PASS)
		{
			strcat(chResult,"'pass',");
		}
		else
		{
			strcat(chResult,"'fail',");
		}
	}
	DD("db_process: auto_test_buf = %s\r\n",chResult);
	
	if(sql_db.summary_result == 0)
	{
		strcat(chSummary,"PASS");
	}
	else
		strcat(chSummary,"FAIL");
	DD("db_process: sql_db.summary_result = %d, chSummary = %s\r\n",sql_db.summary_result,chSummary);
		
	for(i=0;i<g_diaginfo.manuItems;i++)
	{
		if(g_DiagManuTest[i].resultlog.pResult == NULL)
		{
			strcat(chResult,"N/A,");
			continue;
		}
		if(g_DiagManuTest[i].resultlog.pResult->resultID == RESULT_PASS)
		{
			strcat(chResult,"pass,");
		}
		else
		{
			strcat(chResult,"fail,");
		}
	}

	//total  maximum 66 item(T00 - T65), fill all last empty item to "N/A"
	last_item = 65 -(g_diaginfo.manuItems + g_diaginfo.autoItems);
	for(i=0;i<=last_item;i++)
	{
		if(i != last_item)
			strcat(chResult,"N/A,");
		else
			strcat(chResult,"N/A");	//follow sqlite command: the last item we remove comma
	}
	DD("db_process: all_test_buf = %s\r\n",chResult);

	while( (g_testargu.sip_uuid < 10000) || (g_testargu.sip_uuid > 99999) )
	{
		if (g_testargu.sip_uuid < 10000)
			g_testargu.sip_uuid *= 10;
		else if (g_testargu.sip_uuid > 99999)
			g_testargu.sip_uuid /= 10;
	}

	//sprintf(chBarCode,"%s/%s_%d.txt", RECORD_PATH_NOTUPLOAD,g_diaginfo.szBarCode,g_testargu.sip_uuid);
	//sprintf(chBarCode,"%s/%s.txt", RECORD_PATH_NOTUPLOAD,g_diaginfo.szBarCode);
	sprintf(chFTPFilePath,"%s/%s_%s.txt", RECORD_PATH_NOTUPLOAD, g_diaginfo.szBarCode, chFileTime);

	sprintf(g_diaginfo.szFTPFileName,"%s_%s.txt", g_diaginfo.szBarCode, chFileTime);

	fp = fopen(chFTPFilePath, "w");
	fprintf(fp, "T2,Date,SN,MAC1,MAC2,Summary,T00,T01,T02,T03,T04,T05,T06,T07,T08,T09,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20,T21,T22,T23,T24,T25,T26,T27,T28,T29,T30,T31,T32,T33,T34,T35,T36,T37,T38,T39,T40,T41,T42,T43,T44,T45,T46,T47,T48,T49,T50,T51,T52,T53,T54,T55,T56,T57,T58,T59,T60,T61,T62,T63,T64,T65\n");
	fprintf(fp, "T2,%s,%s,%s,%s,%s,%s", 
	chTime, g_diaginfo.szBarCode, chEth1, chEth2, chSummary, chResult);
	fprintf(fp, "\n");

	fclose(fp);



	//srcpy(chBarCode, g_diaginfo.szBarCode);
	//strlen
	return TRUE;
}

int ftpUpload(char *chUploadFile)
{
	FILE *fp = NULL;
	int nIsBarcode = 0;
	char chBarcode[64] = {0};
	int nSerial = 23;

	strcpy(chBarcode, chUploadFile);
	if (strlen(chBarcode) > (nSerial + 5))
		nIsBarcode++;
	if (nIsBarcode)
		chBarcode[strlen(chBarcode) - nSerial] = 0;
		//chBarcode[strlen(chBarcode) - 15] = 0;


	if( access( "/root/diag/ftp.log", F_OK ) == 0 )
		system("rm /root/diag/ftp.log");

	if( access( "uploader.sh", F_OK ) == 0 )
		system("rm uploader.sh");

	fp = fopen("uploader.sh", "w+");
	fprintf(fp,"open -u tester,1 114.32.153.15\n");
	fseek(fp,0,SEEK_END);
	fprintf(fp,"set ftp:passive-mode on\n"); 
	fseek(fp,0,SEEK_END);	
	fprintf(fp,"set ftp:list-options -a\n"); //hidden lftp console log
	fseek(fp,0,SEEK_END);	
	if (nIsBarcode)
	{
		fprintf(fp,"mkdir %s\n", chBarcode);
		fseek(fp,0,SEEK_END);
		fprintf(fp,"cd %s\n", chBarcode);
		fseek(fp,0,SEEK_END);
		fprintf(fp,"cd %s\n", chBarcode);
		fseek(fp,0,SEEK_END);
		fprintf(fp,"cd %s\n", chBarcode);
		fseek(fp,0,SEEK_END);
	}
	else
	{
		fprintf(fp,"mkdir test\n");
		fseek(fp,0,SEEK_END);
		fprintf(fp,"cd test\n");
		fseek(fp,0,SEEK_END);
		fprintf(fp,"cd test\n");
		fseek(fp,0,SEEK_END);
		fprintf(fp,"cd test\n");
		fseek(fp,0,SEEK_END);
	}
	//fprintf(fp,"put ./record/NotUpload/%s\n", chUploadFile);
	fprintf(fp,"put %s/%s\n", RECORD_PATH_NOTUPLOAD, chUploadFile);
	fseek(fp,0,SEEK_END);
	//fprintf(fp,"put ./record/NotUpload/%s\n", chUploadFile);
	//fprintf(fp,"put %s/%s\n", RECORD_PATH_NOTUPLOAD, chUploadFile);
	// fseek(fp,0,SEEK_END);	
	// fprintf(fp,"put %s\n",sql_db.exp_name);	
	//fprintf(fp,"repeat -c 5 -d 1 put %s\n",sql_db.exp_name);		
	//fseek(fp,0,SEEK_END);			
	fprintf(fp,"bye\n");	

	close(fp);	

	//sprintf(szbuf,"lftp -f sip_%s_uploader.sh",BASE_ID);
	//system("touch touch.txt");
	system("lftp -f uploader.sh > /dev/null 2>&1");

	if( access( "/root/diag/ftp.log", F_OK ) == 0 ) {
		return TRUE;
    // file exists
	} else {
		return FALSE;
    // file doesn't exist
	}
}

int ftp_trans()
{
	int ret=0;
	int szbuf[254];
	FILE *fp;

	DD("ftp_trans:\r\n");
	
	//auto generate sip_module_name_upload.sh
	sprintf(szbuf,"sip_%s_uploader.sh",BASE_ID);
	fp = fopen(szbuf,"r+");
	if(fp == NULL)
	{
		fp = fopen(szbuf,"w+");	
		fprintf(fp,"open -u tester,1 114.32.153.15\n");
		fseek(fp,0,SEEK_END);
		fprintf(fp,"set ftp:passive-mode on\n");
		fseek(fp,0,SEEK_END);	
		fprintf(fp,"cd sip_record\n");
		fseek(fp,0,SEEK_END);
		fprintf(fp,"put %s\n",sql_db.exp_name);
		fseek(fp,0,SEEK_END);	
		fprintf(fp,"put %s\n",sql_db.exp_name);	
		//fprintf(fp,"repeat -c 5 -d 1 put %s\n",sql_db.exp_name);		
		fseek(fp,0,SEEK_END);			
		fprintf(fp,"bye\n");		
	}
	close(fp);
	//20170314 
	ret = system("ifconfig eth0 down > /dev/null 2>&1");
	sleep(1);
	ret = system("ifconfig eth1 down > /dev/null 2>&1");
	sleep(1);
	ret = system("ifconfig eth2 down > /dev/null 2>&1");
	sleep(1);
	ret = system("ifconfig rename2 down > /dev/null 2>&1");
	printf("\n================ please wait ================\r\n");
	sleep(5);
	sprintf(szbuf,"lftp -f sip_%s_uploader.sh",BASE_ID);
	ret = system(szbuf);

	ret = system("ifconfig eth0 up > /dev/null 2>&1");
	sleep(1);
	ret = system("ifconfig eth1 up > /dev/null 2>&1");
	sleep(1);
	ret = system("ifconfig eth2 up > /dev/null 2>&1");
	sleep(1);
	ret = system("ifconfig rename2 down > /dev/null 2>&1");
	sleep(3);
	printf("\n=================== done ====================\r\n");	

	return 0;
}


static int CB_export(void *handle, int argc, char **argv, char **azColName)
{
    FILE *f = handle;
    int i;
    const char *sep = "";

	DD("CB_export:\r\n");
	
/*
    for (i=0;i<argc;i++)
    {
    	fprintf(f, "%s,",azColName[i]);
    }	
    fprintf(f, "\n");
*/
	fseek(f,0,SEEK_END);

    for (i=0;i<argc;i++)
	{
    	//fprintf(f, "%s\"%s\"", sep, argv[i]);
    	fprintf(f, "%s%s",sep,argv[i]);
    	sep = ",";
    }
    fprintf(f, "\n");
    return 0;
}


//function to combine all item for command
int db_combine_item()
{
	int array[100];
	int buf[512];
	int buf2[512];
	int item =50;
	int i=0;

	DD("db_combine_item:\r\n");
	
	memset(buf2,NULL,512);
	for(i=0;i<=item;i++)
	{	
		memset(buf,0,10);
		sprintf(buf, ",T%02d",i);
		DD("db_combine_item: buf = %s\r\n",buf);
		strcat(buf2,buf);
		DD("db_combine_item: buf2 = %s\r\n",buf2);
	}
	return 0;
}

/********************************************************
*
* 2011.12.05 output txt fotmat for user collect data
*
********************************************************/

int db_export()
{
	char *error_report = NULL;
	sqlite3 *db = NULL;
	char *sqlCmd;
	FILE *fp;
	char namebuf[64];
	
	DD("db_export:\r\n");
	
	memset(namebuf,NULL,64);
	sprintf(namebuf,"%s%d.txt",DB_EXPORT_PATH,g_testargu.sip_uuid);
	DD("namebuf = %s\r\n",namebuf);
	strcpy(sql_db.exp_name,namebuf);
	DD("sql_db.exp_name = %s\r\n",sql_db.exp_name );

	
	fp = fopen(namebuf,"w+");
	//if(fp == NULL)
	//{
		//DD("Creating new 'export table' file\r\n");
		//fp = fopen(namebuf,"w+");
		fprintf(fp,"T2,Date,SN,MAC1,MAC2,Summary,T00,T01,T02,T03,T04,T05,T06,T07,T08,T09,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20,T21,T22,T23,T24,T25,T26,T27,T28,T29,T30,T31,T32,T33,T34,T35,T36,T37,T38,T39,T40,T41,T42,T43,T44,T45,T46,T47,T48,T49,T50,T51,T52,T53,T54,T55,T56,T57,T58,T59,T60,T61,T62,T63,T64,T65\n");
	//}
	
	if( sqlite3_open( DB_PATH , &db ) != SQLITE_OK )
	{
		fprintf( stderr , "\t> Cannot open databases: %s\n" , sqlite3_errmsg(db) );
		sqlite3_close( db );
		exit(1);
	}

	sqlCmd = "SELECT * FROM GENERAL";
    if( sqlite3_exec( db, sqlCmd, CB_export, fp, &error_report ) != SQLITE_OK )
    {
        fprintf( stderr , "\t> SELECT, Error: %s\n", error_report );
        sqlite3_close(db);
        exit(1);
    }

	fclose(fp);
	sqlite3_close(db);
	return 0;
}

/********************************************************
*
* 2011.12.02 create database and store test record
*
********************************************************/

int db_process()
{
	int i,last_item,res,ethcnt,ret,buf[1024],buf1[10];
    char *error_report = NULL;
    sqlite3 *db = NULL;
	char sqlCmd1[1024],*sqlCmd,*szBuf = NULL;
	FILE *file, *fp;
	char tbuf[30],szMac[128];
	char szEth[128],szEth1[128];
	struct tm *tm_ptr,timestruct;
	time_t the_time;

	DD("db_process:\r\n");

	memset(buf,NULL,1024);
	memset(buf1,NULL,10);

	//get time record
	(void) time(&the_time);
	tm_ptr=localtime(&the_time);
	//DD("strptime gives:\n");
	DD("date: %02d%02d%02d\n",tm_ptr->tm_year%100,tm_ptr->tm_mon+1,tm_ptr->tm_mday);
	DD("time: %02d%02d%02d\n",tm_ptr->tm_hour,tm_ptr->tm_min,tm_ptr->tm_sec);
	sprintf(tbuf, "20%02d/%02d/%02d_%02d:%02d:%02d",tm_ptr->tm_year%100,tm_ptr->tm_mon+1,tm_ptr->tm_mday,tm_ptr->tm_hour,tm_ptr->tm_min, tm_ptr->tm_sec );
	DD("start test time = %s\n", tbuf);

/* format2 ex: Wed Mar 8 02:5:16 GMT 2017
	ret = system("date > ./data/record_time");
	fp = fopen("./data/record_time","r+");
	fgets(tbuf,(sizeof(tbuf)-1),fp);
	DD("db_process: tbuf = %s\r\n",tbuf);
	fclose(fp);
*/	
	//get mac
	ethcnt=GetNetCardCount();
	system("ifconfig | grep -o '[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]:[0-9|a-z][0-9|a-z]' > ./data/mac_addr");
	fp = fopen("./data/mac_addr","r+");
	fgets(szEth,18,fp);
	DD("db_process: szEth = %s\r\n",szEth);

	if(ethcnt == 1)
	{
		*szBuf = "N/A";
		strcpy(szEth1,szBuf);
	}
	if(ethcnt >= 2)
	{
		fseek(fp,18,SEEK_SET);
		fgets(szEth1,18,fp);
		DD("db_process: szEth1 = %s\r\n",szEth1);
	}
	fclose(fp);

	//combine all test item result
	for(i=0;i<g_diaginfo.autoItems;i++)
	{
		if(g_DiagAutoTest[i].resultlog.pResult == NULL)
		{
			strcat(buf,"'N/A',");
			continue;
		}
		if(g_DiagAutoTest[i].resultlog.pResult->resultID == RESULT_PASS)
		{
			strcat(buf,"'pass',");
		}
		else
		{
			strcat(buf,"'fail',");
		}
	}
	DD("db_process: auto_test_buf = %s\r\n",buf);
	
	if(sql_db.summary_result == 0)
	{
		strcat(buf1,"'PASS',");
	}
	else
		strcat(buf1,"'FAIL',");
	DD("db_process: sql_db.summary_result = %d, buf1 = %s\r\n",sql_db.summary_result,buf1);
		
	for(i=0;i<g_diaginfo.manuItems;i++)
	{
		if(g_DiagManuTest[i].resultlog.pResult == NULL)
		{
			strcat(buf,"'N/A',");
			continue;
		}
		if(g_DiagManuTest[i].resultlog.pResult->resultID == RESULT_PASS)
		{
			strcat(buf,"'pass',");
		}
		else
		{
			strcat(buf,"'fail',");
		}
	}

	//total  maximum 66 item(T00 - T65), fill all last empty item to "N/A"
	last_item = 65 -(g_diaginfo.manuItems + g_diaginfo.autoItems);
	for(i=0;i<=last_item;i++)
	{
		if(i != last_item)
			strcat(buf,"'N/A',");
		else
			strcat(buf,"'N/A'");	//follow sqlite command: the last item we remove comma
	}
	DD("db_process: all_test_buf = %s\r\n",buf);
	

	if( sqlite3_open( DB_PATH , &db ) != SQLITE_OK )
	{
		fprintf( stderr , "\t> Cannot open databases: %s\n" , sqlite3_errmsg(db) );
		sqlite3_close( db );
		exit(1);
	}

	sqlCmd = "CREATE TABLE IF NOT EXISTS GENERAL(T2, Date, SN, MAC1, MAC2, Summary, T00, T01, T02, T03, T04, T05, T06, T07, T08, T09, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29 ,T30, T31, T32, T33, T34, T35, T36, T37, T38, T39, T40, T41, T42, T43, T44, T45, T46, T47, T48, T49, T50, T51, T52, T53, T54, T55, T56, T57, T58, T59, T60, T61, T62, T63, T64, T65);";
    if( sqlite3_exec( db, sqlCmd, NULL, NULL, &error_report ) != SQLITE_OK )
    {
        fprintf( stderr , "\t> CMD_CREATE_TABLE, Error: %s\n", error_report );
        sqlite3_close(db);
        exit(1);
    }

	sprintf(sqlCmd1, "INSERT INTO GENERAL VALUES ('T2','%s','%s','%s','%s',%s%s);",tbuf,g_diaginfo.szBarCode,szEth,szEth1,buf1,buf);
	DD("db_process: sqlCmd1 = %s\r\n",sqlCmd1);
	if( sqlite3_exec( db, sqlCmd1, NULL, NULL, &error_report ) != SQLITE_OK )
	{
		fprintf( stderr , "\t> CMD_INSERT, Error: %s\n", error_report );
		sqlite3_close(db);
		exit(1);
	}

	sqlite3_close(db);
	return 0;
}


