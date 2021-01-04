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

	get_argument(argc,argv);
	//init_Dialog();	
	printf("========aaaaa=========\r\n");
	init_Diag();

	printf("========bbbbb=========\r\n");

	//VGA T01007001
	res = fun_T02006001();
	printf("fun_T02006001 return %d\r\n\r\n", res);
	//printf("T01007001: VGA test = %d\r\n",res);


	//res = fun_T01015006();
	//printf("fun_T01015006: m.2 test = %d\r\n",res);

	//scanBarCode_RS232(0);
	
	//res = fun_T02006001();
	//printf("T02006001: SRAM test = %d\r\n",res);

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
#if 0
	printf("\r\n=============================================\r\n");	
	//fan1		
	res = fun_T01020001();
	printf("FAN 1 - CN18 = %d\r\n",res);
	//fan2		
	printf("=============================================\r\n");
	res = fun_T01020003();
	printf("FAN 2 - CN17 = %d\r\n",res);
	//fan3
	printf("=============================================\r\n");
	res = fun_T01020004();
	printf("FAN 3 - CN19 = %d\r\n",res);
	printf("=============================================\r\n");
#endif

	//tcil_test(argc,argv);
	deinit_Diag();	
	//deinit_Dialog();

#else

	//the main function for diag application	
	configure_share_irq();	//for gm6080
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

