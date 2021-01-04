#ifndef TEST_ITEMS_H_INCLUDED
#define TEST_ITEMS_H_INCLUDED

#include "utils.h"
#include "iniparser.h"

#ifdef	__cplusplus
extern "C" {
#endif	//__cplusplus

//**********************************************************
//**********************************************************
//**   Data structs
//**********************************************************
//**********************************************************
typedef struct _tagCPU_TEST
{
	char 			*szCPUInfo;
	unsigned int	speed;
	unsigned int	range;
}CPU_TEST,*PCPU_TEST;

typedef struct _tagMEM_TEST
{
	unsigned int	size;
	unsigned int	freq;
	unsigned int	locator;
}MEM_TEST,*PMEM_TEST;

typedef struct _tagFILE_TX
{
	char 	*src;
	char 	*dest;
	char 	*fname;
	int     size;
}FILE_TX,*PFILE_TX;

/*
typedef struct _tagTEST_ITEMS_INFO
{
	CPU_TEST	cpudata;	//struct for storing test parameter of cpu
	MEM_TEST	memdata;	//struct for storing test parameter of memory
	FILE_TX		file_tx;	//struct for storing test parameter of  file Tx
}TEST_ITEMS_INFO,*PTEST_ITEMS_INFO;
*/

//**********************************************************
//**********************************************************
//**   General Functions
//**********************************************************
//**********************************************************
int init_TestItem();
int deinit_TestItem();

int check_voltage_range(float std,float cur,float range/*percentage*/);

//**********************************************************
//**********************************************************
//**   Auto Test Functions
//**********************************************************
//**********************************************************

//All Methods for testing CPU
int fun_TestCPU_001(PCPU_TEST pInfo);
int fun_TestCPU_002(PCPU_TEST pInfo);

int fun_TestMemory_001(PMEM_TEST pInfo);

int fun_FileTransmit_001(PFILE_TX pInfo);

int fun_TestVoltage_001();

int fun_TestClock_001();

int fun_GPCTestSRAM_001(const int bank,const int ksize);
int fun_GPCTestSRAM_002(const int bank_num,const int ksize,const int bits);

int fun_TestLAN_001();
int fun_TestLAN_002();
int fun_TestLAN_003();
int fun_TestLAN_004();

//**********************************************************
//**********************************************************
//**   Manual Test Functions
//**********************************************************
//**********************************************************
int fun_TestVGA_001();
int fun_TestVGA_002(char *dev);
int fun_TestUART_001();


#ifdef	__cplusplus
}
#endif	//__cplusplus

#endif // EIT_H_INCLUDED

