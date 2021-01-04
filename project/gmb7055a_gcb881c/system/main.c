#include <stdlib.h>
#include <stdio.h>
#include "Diag.h"
#include "SIO.h"
#include "db.h"
//#include "Attach.h"

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

#if defined __UPLOADFTP__
	uploadFTP();
	return 0;
#endif
	
	
	HIDDEN_CURSES();
#if defined(__TEST_FUN__)

	//diag_main(argc, argv, 0);
	printf("Starting Test...\r\n");

	get_argument(argc,argv);
	init_Dialog();	
	init_Diag();

	res = fun_T01006001();
	printf("fun_T01006001 res = %d\r\n",res);
	res = fun_T01006002();
	printf("fun_T01006002 res = %d\r\n",res);

	res = fun_T01012002();
	printf("fun_T01012002 res = %d\r\n",res);
	res = fun_T01012003();
	printf("fun_T01012003 res = %d\r\n",res);
	res = fun_T01012004();
	printf("fun_T01012004 res = %d\r\n",res);

	res = fun_T01013002();
	printf("fun_T01013002 res = %d\r\n",res);
	res = fun_T01013003();
	printf("fun_T01013003 res = %d\r\n",res);
	res = fun_T01013004();
	printf("fun_T01013004 res = %d\r\n",res);

	deinit_Diag();	
	deinit_Dialog();


#else

	//the main function for diag application	
	diag_main(argc, argv, 0);
	// db_process();
	// db_export();
	// ftp_trans();	

#endif
	SHOW_CURSES();

	return 0;
}

