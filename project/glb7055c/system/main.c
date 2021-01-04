#include <stdlib.h>
#include <stdio.h>
#include "diag.h"
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

	//diag_main(argc,argv, 0);

	// 	db_process();
	// 	db_export();
	// 	ftp_trans();	

	//diag_main(argc,argv, 0);

	
	
	HIDDEN_CURSES();
#if defined(__TEST_FUN__)

	// printf("Starting Test...\r\n");

	// get_argument(argc,argv);
	// init_Dialog();	
	// init_Diag();

	// //diag_main(argc,argv, 0);

	// // res = fun_T01020001();
	// // printf("res = %d\r\n", res);
	// // res = fun_T01020002();
	// // printf("res = %d\r\n", res);
	// // res = fun_T01020003();
	// // printf("res = %d\r\n", res);

	// //scanBarCode_RS232(0);

	


	
	// res = fun_T01013001();
	// printf("res = %d\r\n", res);
	// res = fun_T01013002();
	// printf("res = %d\r\n", res);
	// res = fun_T01013003();
	// printf("res = %d\r\n", res);
	// res = fun_T01013004();
	// printf("res = %d\r\n", res);
	// //res = fun_T02022002();
	// //printf("res = %d\r\n", res);
	// //fun_T02022001();
	// //res = fun_T02006001();
	// //printf("res = %d\r\n", res);
	// //res = fun_T02017031();

	// deinit_Diag();	
	// deinit_Dialog();
#else

	//the main function for diag application	
	//configure_share_irq();	//for gm6080
	diag_main(argc,argv, 0);
	//db_process();

#if defined(__SERVER_V1__)
		//db_process();
		//db_export();
		//ftp_trans();	
#endif

#endif
	SHOW_CURSES();

	return 0;
}

