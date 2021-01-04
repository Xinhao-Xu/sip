#ifndef EITLIB_H_INCLUDED
#define EITLIB_H_INCLUDED

#ifdef	__cplusplus
extern "C" {
#endif	//__cplusplus

#include "bits.h"
#include "global.h"

//**********************************************************
//**********************************************************
//**   Defnation for infterface of communication for EIT
//**********************************************************
//**********************************************************
#define CMD_READ    1
#define CMD_WRITE   2

#define ACTION_SET 0x08
#define ACTION_GET 0x20

typedef int (*CUSTOMIZE_SMBUSCMD)(U08 cmd,U08 datalen,U08 *pData,U08 rw);
typedef int (*CUSTOMIZE_SMBUSINIT)();

enum
{
    EIT_INTERFACE_GPC = 1,   //use i2c interface of GPC
    EIT_INTERFACE_CUSTOM,
};

enum _tagEIT_TARGET
{
    TARGET_SECTION2_START = BOUND_EITSEC_START,
    EIT_LIB_SETTING,
    EIT_INFO,
    EIT_ITR,
    EIT_ITR0,
    EIT_ITR1,
    EIT_ITR2,
    EIT_ITR3,
    EIT_ITR4,
    EIT_ITR5,
    EIT_ITR6,
    EIT_BATTERY,
    EIT_BATTERY1,
    EIT_BATTERY2,
    TARGET_SECTION2_END = BOUND_EITSEC_END,
};

//Library setting
#define	  EIT_LIB_SETTING_INIT 		 	      BITSHIFT(BIT_01)
#define	  EIT_LIB_SETTING_DEINIT              BITSHIFT(BIT_02)
#define	  EIT_LIB_SETTING_VERSION             BITSHIFT(BIT_03)   // version of EIT API
#define	  EIT_LIB_SETTING_LASTERROR           BITSHIFT(BIT_04)

//EIT
#define   EIT_INFO_VERSION                    BITSHIFT(BIT_01)  // version of EIT fireware
#define   EIT_INFO_RTC                        BITSHIFT(BIT_02)
#define   EIT_INFO_PWRLOG                     BITSHIFT(BIT_03)
#define   EIT_INFO_ITRSTATUS                  BITSHIFT(BIT_04)

#define   EIT_ITR_LOG                         BITSHIFT(BIT_01)
#define   EIT_BATTERY_LOW_CHKPT               BITSHIFT(BIT_01)
#define   EIT_BATTERY_LOW_LOG                 BITSHIFT(BIT_02)
#define   EIT_BATTERY_VOLTAGE                 BITSHIFT(BIT_03)
#define   EIT_BATTERY_ESTPERCENT              BITSHIFT(BIT_04)
#define   EIT_BATTERY_TYPE                    BITSHIFT(BIT_05)
#define   EIT_ONEWIRE_RESET                   BITSHIFT(BIT_01)
#define   EIT_ONEWIRE_BLOCKDATA               BITSHIFT(BIT_02)
#define   EIT_HCS300_CTRL                     BITSHIFT(BIT_01)
#define   EIT_HCS300_CTRL_LAW                 BITSHIFT(BIT_02)

#define   EIT_HCS300_CHKPASS                  0
#define   EIT_HCS300_CHKFAIL                  1
#define   EIT_HCS300_MANU_LEN                 8
#define   EIT_HCS300_SN_LEN                   4
#define   EIT_HCS300_LAWDATA_WLEN             24
#define   EIT_HCS300_LAWDATA_RLEN             8

typedef struct _tagDSEIT_LIB_SETTING
{
	U32 								u32_subtarget;
	U32 								u32_libver;
	U32									u32_lasterr;
	U32                                 u32_interface;
	U08                                 u08_eittype;
	U08                                 u08_battype;	
	CUSTOMIZE_SMBUSCMD                  pfun_smbuscmd;
	CUSTOMIZE_SMBUSINIT                 pfun_smbusinit;
}DSEIT_LIB_SETTING,*PDSEIT_LIB_SETTING;

typedef struct _tagSTRUCT_TIME
{
    U32     totalsec;
	U08		level;
    char    asctime[64];
    int     sec;
    int     min;
    int     hour;
    int     day;
    int     mon;
    int     year;
}STRUCT_TIME,*PSTRUCT_TIME;

typedef struct _tagDSEIT_INFO
{
	U32 	    u32_subtarget;
    U08         u08_version;
    U08			u08_itrstatus;
    STRUCT_TIME time_rtc;
    STRUCT_TIME time_last1;
    STRUCT_TIME time_last2;
    STRUCT_TIME time_last3;
    STRUCT_TIME time_last4;
}DSEIT_INFO,*PDSEIT_INFO;

typedef struct _tagDSEIT_ITR
{
	U32 	    u32_subtarget;
    STRUCT_TIME time_last1;
    STRUCT_TIME time_last2;
    STRUCT_TIME time_last3;
    STRUCT_TIME time_last4;
}DSEIT_ITR,*PDSEIT_ITR;

typedef struct _tagDSEIT_BATTERY
{
	U32 	    u32_subtarget;
    U16         u16_lowcp;
    U16         u16_voltage;
    U08         u08_estpercent;
    U08         *pu08_type;
    STRUCT_TIME time_last1;
    STRUCT_TIME time_last2;
    STRUCT_TIME time_last3;
    STRUCT_TIME time_last4;
}DSEIT_BATTERY,*PDSEIT_BATTERY;

typedef struct _tagDSEIT_ONEWIRE
{
	U32 	    u32_subtarget;
    U08         u08_freq;
    U08         u08_datacnt;
    U08        *pu08_data;
}DSEIT_ONEWIRE,*PDSEIT_ONEWIRE;

typedef struct _tagDSEIT_HCS300
{
	U32 	    u32_subtarget;
    U08         pu08_manufacture[EIT_HCS300_MANU_LEN];
    U08         pu08_serialnum[EIT_HCS300_SN_LEN];
    U08         pu08_lawdata[EIT_HCS300_LAWDATA_WLEN];
    U16         u16_count;
    U16         u16_errrange;
    U08         u08_chkresult;
}DSEIT_HCS300,*PDSEIT_HCS300;

//**********************************************************
//**********************************************************
//**   Defination for Data type
//**********************************************************
//**********************************************************
enum
{
	TYPE_EIT020A = 1,
	TYPE_NONBUFFER,
	TYPE_BUFFERING,
};



//Battery Types
enum
{
	TYPE_BATTERY_CR2032 = 1,
};

//Logical Level Types
enum
{
	TYPE_LV_LOW = 0,
	TYPE_LV_HIGH,		
};
//**********************************************************
//**********************************************************
//**   Description for Functions
//**********************************************************
//**********************************************************
int	EIT_CTLCODE(U32 target, U32 action, void *lparam);

#ifdef	__cplusplus
}
#endif	//__cplusplus

#endif // EIT_H_INCLUDED
