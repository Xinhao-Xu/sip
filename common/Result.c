#include <stdlib.h>
#include <stdio.h>
#include "Result.h"
//#include "Testboard_lib.h"

#define SIZE_ARRY(x) (sizeof(x)/sizeof(x[0]))

//This Table include the result descript for general test and testboard test.
/*
RESULT g_ResultTable[]=
{
	{RESULT_PASS,"Pass Test"},

	//Test Board return
	{MSG_COIN_FIAL,"coin test fail"},  

	//General Return
	{RESULT_CPU_FAIL,"cpu test fail"}, //valuse start from RESULT_RANGE_START
	{RESULT_MEMORY_FAIL,"memory test fail"},		
	{RESULT_VGA_FAIL,"vga test fail"},		
	{RESULT_UART_FAIL,"uart test fail"},		
};
*/
RESULT g_ResultTable[]={RESULT_PASS,RESULT_FAIL};

PRESULT getResult(int result)
{
	int i;
	PRESULT	pResult=NULL;
	
	for(i=0;i<SIZE_ARRY(g_ResultTable);i++)
	{
		if(result == g_ResultTable[i].resultID)
			pResult = &g_ResultTable[i];
	}

	return pResult;
}

/*
char *getResultDescript(int result)
{
	return g_ResultTable[0].pDescript;
}
*/
