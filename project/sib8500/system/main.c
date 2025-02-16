#include <stdlib.h>
#include <stdio.h>
#include "Diag.h"
#include "SIO.h"
#include "db.h"
//#include "Attach.h"

DIAGINFO			g_diaginfo;

int main(int argc,char *argv[])
{
	long int i;
	int res=99;
	unsigned long int aa;
	char szBuf[512];	

	//handle argument '-v'
	if( (argc==2)&&(!strcmp(argv[1],"-v")) )
	{
		show_verson();
		exit(0);
	}
	//handle argument '-cplog'
	if( (argc==2)&&(!strcmp(argv[1],"-cplog")) )
	{
		cp_logfiles3(argv[0]);
		exit(0);
	}

	
	
	HIDDEN_CURSES();
#if defined(__TEST_FUN__)

	printf("Starting Test...\r\n");

	//fun_TestLAN_004("192.168.137.1","enp1s0");
	//fun_TestLAN_004("192.168.137.1","enp3s0");

	//sprintf(szBuf,"ping -c 3 -I %s %s",src_interface,dest_ip);
	//system("ping -c 3 -I enp1s0 192.168.137.1");
	//system("ping -c 3 -I enp3s0 192.168.137.1");

	get_argument(argc,argv);
	init_Dialog();
	init_Diag();
	

	res = fun_T01006001();
	printf("\r\n fun_T01006001 res = %d\r\n", res);
	
	res = fun_T01006002();
	printf("\r\n fun_T01006002 res = %d\r\n", res);

	/*
	res = fun_T01006001();
	printf("\r\n fun_T01006001 res = %d\r\n", res);
	
	res = fun_T01006002();
	printf("\r\n fun_T01006002 res = %d\r\n", res);
	
	res = fun_T01012003();
	printf("\r\n fun_T01012003 res = %d\r\n", res);

	res = fun_T01012004();
	printf("\r\n fun_T01012004 res = %d\r\n", res);

	res = fun_T01012005();
	printf("\r\n fun_T01012005 res = %d\r\n", res);
	
	res = fun_T01012006();
	printf("\r\n fun_T01012006 res = %d\r\n", res);
	*/

	
	//printf("========bbbbb=========\r\n");

	//VGA
	//fun_TestVGA_002("/dev/fb0");
	//fun_T01007001();
	//printf("fun_T01007001 = %d\r\n",res);


	//M.2
	//res = fun_T01015006();
	//printf("fun_T01015006 m.2 = %d\r\n",res);

	//Audio 4ch
	//res = fun_T01005002();
	//printf("fun_T01005002 Audio = %d\r\n",res);

	//LAN2
	//res = fun_T01006002();
	//printf("fun_T01006002 Lan 2 = %d\r\n",res);

	//res = fun_T01015006();
	//printf("fun_T01015006: m.2 test = %d\r\n",res);

	//scanBarCode_RS232(0);

	//Audio
	//res = fun_T01005002();
	//printf("audio = %d\r\n",res);
	//com
	//res = fun_T01013002();
	//printf("COM2 = %d\r\n",res);
	//usleep(30000);
	//res = fun_T01013003();
	//printf("COM3 = %d\r\n",res);
	//usleep(30000);
	//res = fun_T01013004();
	//printf("COM4 = %d\r\n",res);
	//usleep(30000);
	//res = fun_T01013005();	//use jump to switch com5 or com6, 0x02f0 or 0x02e0
	//printf("COM5 = %d\r\n",res);
	//usleep(30000);
	//res = fun_T01013006();
	//printf("COM6 = %d\r\n",res);

	//fan
	//res = fun_T01020001();
	//printf("fun_T01020001 = %d\r\n", res);

	//res = fun_T01020003();
	//printf("fun_T01020003 = %d\r\n", res);



/********************************************************
*
*	fan_test tool
*
********************************************************/


	//tcil_test(argc,argv);
	deinit_Diag();	
	deinit_Dialog();
#else
	//the main function for diag application	
	//configure_share_irq();	//for gm6080
	diag_main(argc,argv);
#if defined(__SERVER_V1__)
		db_process();
		db_export();
		ftp_trans();	
#endif

#endif
	SHOW_CURSES();

	return 0;
}

