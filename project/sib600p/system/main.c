#include <stdlib.h>
#include <stdio.h>
#include "diag.h"
#include "SIO.h"
#include "db.h"
//#include "Attach.h"

DIAGINFO			g_diaginfo;

int main(int argc,char *argv[])
{
	long int i = 0;
	int res = 99;
	int res1=99;
	int res2=99;
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
	init_Dialog();	
	init_Diag();

	FILE *fp = NULL;

	while (1)
	{
		printf("Count = %d \r\n",i);
		fp = fopen("LAN_TEST.txt","a");
		res1 = fun_T01006001();
		res2 = fun_T01006002();
		fprintf(fp,"Count : %d\r\nT01006001 : %d\r\nT01006002 : %d\r\n", i, res1, res2);
		if (res1 == 0 && res2 == 0)
		{
			fprintf(fp,"O\r\n\r\n");
		}
		else
		{
			fprintf(fp,"X\r\n\r\n");
		}
		fclose(fp);
		i++;
	}
	
	//fun_T01005001();

	// res = fun_T01013001();
	// printf("\r\n fun_T01013001 res = %d\r\n",res);
	// res = fun_T01013002();
	// printf("\r\n fun_T01013002 res = %d\r\n",res);
	// res = fun_T01013003();
	// printf("\r\n fun_T01013003 res = %d\r\n",res);
	// res = fun_T01013004();
	// printf("\r\n fun_T01013004 res = %d\r\n",res);
	// res = fun_T01013005();
	// printf("\r\n fun_T01013005 res = %d\r\n",res);
	// res = fun_T01013006();
	// printf("\r\n fun_T01013006 res = %d\r\n",res);
	// res = fun_T01013007();
	// printf("\r\n fun_T01013007 res = %d\r\n",res);
	// res = fun_T01013008();
	// printf("\r\n fun_T01013008 res = %d\r\n",res);
	// res = fun_T01013009();
	// printf("\r\n fun_T01013009 res = %d\r\n",res);
	// res = fun_T01013010();
	// printf("\r\n fun_T01013010 res = %d\r\n",res);
	scanBarCode_RS232(5);
	//scanBarCode_RS232(3);
	//scanBarCode_RS232(4);
	//scanBarCode_RS232(5);
	//scanBarCode_RS232(6);
	//scanBarCode_RS232(7);
	//scanBarCode_RS232(8);
	//scanBarCode_RS232(9);
	// res = fun_T01020001();
	// printf("res = %d\r\n", res);
	// res = fun_T01020002();
	// printf("res = %d\r\n", res);
	// res = fun_T01020003();
	// printf("res = %d\r\n", res);

	//scanBarCode_RS232(1);

	res = fun_T02014001();
	

	res = fun_T01013005();
	printf("res = %d\r\n", res);
	res = fun_T01013006();
	printf("res = %d\r\n", res);
	res = fun_T01024002();
	printf("res = %d\r\n", res);
	//res = fun_T02022002();
	//printf("res = %d\r\n", res);
	//fun_T02022001();
	//res = fun_T02006001();
	//printf("res = %d\r\n", res);
	//res = fun_T02017031();

	deinit_Diag();	
	deinit_Dialog();
#else

	//the main function for diag application	
	//configure_share_irq();	//for gm6080
	diag_main(argc,argv, 5);

#if defined(__SERVER_V1__)
		db_process();
		db_export();
		ftp_trans();	
#endif

#endif
	SHOW_CURSES();

	return 0;
}

