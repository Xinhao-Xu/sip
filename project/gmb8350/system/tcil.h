#ifndef TCIL_H_INCLUDED
#define TCIL_H_INCLUDED

#include "bits.h"

#define BASE_ID			"gmb8350"
#define DB_PATH			"./record/database_gmb8350.db"
#define DB_EXPORT_PATH	"./record/gmb8350_"

#define FILE_SIPARGU	"sip.ini"

//#define SETKEY(sec,argu)	sec ":" argu
#define SETKEY(key,sec,argu)	sprintf(key,"%s:%s",sec,argu)

//arguments for section [HWINFO]
#define SEC_HWINFO			"DEFAULT"
#define ARGU_CPUNAME		"CPU_NAME"
#define ARGU_CPUSPEED		"CPU_SPEED"
#define ARGU_CPUSPEEDRANGE	"CPU_SPEED_RANGE"
#define ARGU_BARCODE_COM	"BARCODE_COM"
#define ARGU_DRAM_SIZE		"DRAM_SIZE"
#define ARGU_SRAM_BANKNUM	"SRAM_BANKNUM"
#define ARGU_SRAM_SIZE		"SRAM_SIZE"
#define ARGU_DIP_NUMS		"DIP_NUMS"
#define ARGU_EIT_BATUP		"EIT_BATUP"
#define ARGU_EIT_BATDW		"EIT_BATDW"
#define ARGU_FAN1_DUTY		"FAN1_DUTY"
#define ARGU_FAN2_DUTY		"FAN2_DUTY"
#define ARGU_FAN3_DUTY		"FAN3_DUTY"
#define ARGU_FAN1_RPM		"FAN1_RPM"
#define ARGU_FAN2_RPM		"FAN2_RPM"
#define ARGU_FAN3_RPM		"FAN3_RPM"
#define ARGU_AUDIO_HW		"AUDIO_HW"
#define ARGU_SIP_UUID		"SIP_UUID"


typedef struct _tagTEST_ARGU
{
	char	cpu_name[64];
	U08		barcode_comnum;
	U08     dip_nums;
	U32		sram_size;
	U08		sram_banknum;
	U32		dram_size;
	U32		battery_upval;
	U32		battery_downval;
	U08     fan1_duty;
	U08     fan2_duty;
	U08     fan3_duty;
	int		fan1_rpm;
	int		fan2_rpm;
	int		fan3_rpm;
	int		audio_hw;
	U32		sip_uuid;
}TEST_ARGU,*PTEST_ARGU;

extern TEST_ARGU	g_testargu;


#endif
