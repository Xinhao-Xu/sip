#ifndef RESULT_H_INCLUDED
#define RESULT_H_INCLUDED

#ifdef	__cplusplus
extern "C" {
#endif	//__cplusplus

#define	RECORD_PATH				"./record"
#define	RESULT_PATH				RECORD_PATH "/result"
#define	ANSYS_PATH				RECORD_PATH "/ansys_table"
#define	OTHERREC_PATH			RECORD_PATH "/other_rec"

#define	UPLOADLIST				UTILS_TMPFOLDER "/uploadlist.txt"
#define RECORD_PATH_NOTUPLOAD	RECORD_PATH "/NotUpload"
#define RECORD_PATH_UPLOADED	RECORD_PATH "/Uploaded"

#define	STATISTIC_FILE			ANSYS_PATH "/StatisticTable"
#define	MACADDR_FILE			OTHERREC_PATH "/MacAddrTable"

#define	FIN_RECORD_PATH			RESULT_PATH
#define	FIN_RECORD_FILE			FIN_RECORD_PATH "/FinRecord"

#define RESULT_PASS 0
#define RESULT_FAIL 1

#define MEM_DATA_FILE					"./data/mem"
#define GRAPHIC_DEV_ID					"./data/pci_graphic_id"

#define FIXLOG_TYPE_NOTEST	0
#define FIXLOG_TYPE_TESTED	1

//defination for test result
typedef struct _tagRESULT
{
	int		resultID;
//	char	*pDescript;
}RESULT,*PRESULT;

#define RESULT_PASS  0
#define RESULT_FAIL  1

/*
	Define the result of general test.
	The result for test board should less than RESULT_RANGE_START

	Note: only Auto test need result. manual test only return pass or fail.
*/
#define RESULT_RANGE_START	2000

enum _tagResultList
{
	RESULT_CPU_FAIL=RESULT_RANGE_START,
	RESULT_MEMORY_FAIL,
	RESULT_VGA_FAIL,
	RESULT_UART_FAIL,	
};


PRESULT getResult(int result);
//char *getResultDescript(int result);
	
#ifdef	__cplusplus
}
#endif	//__cplusplus

#endif // EIT_H_INCLUDED

