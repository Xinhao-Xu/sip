#ifndef __GPCLIB_H__
#define  __GPCLIB_H__

#include "bits.h"
#include "global.h"

#ifdef	__cplusplus
extern "C" {
#endif	//__cplusplus

//**********************************************************
//**********************************************************
//**   Defination for CallBack Function Parameter
//**********************************************************
//**********************************************************

#define HIGH  1
#define LOW   0

#define TRUE  1
#define FALSE 0

typedef void (*int_callback_fun_t)(U32 cb_type , U32 cb_src , U32 cb_srcbit , U32 cb_data , void *context);

//GPC Sction
#define SERIAL_DEVICE(num)			(BOUND_GPCSEC_START + (num)*(BOUND_DEVICE+1))
#define SERIAL_DEVFUN(dev,fun)			((dev)+(fun)*(BOUND_DEVFUN+1))
#define SERIAL_DEVNUM(devfun,num)		((devfun)+(num))

//This series must be the same with _tagTYPE_TARGET in global.h
#define GPC_DEV_LIB			SERIAL_DEVICE(GET_TARGET_TYPEVAL(TYPE_LIBRARY))
#define GPC_DEV_HW			SERIAL_DEVICE(GET_TARGET_TYPEVAL(TYPE_HARDWARE))
#define	GPC_DEV_SRAM		SERIAL_DEVICE(GET_TARGET_TYPEVAL(TYPE_SRAM))
#define	GPC_DEV_PORT		SERIAL_DEVICE(GET_TARGET_TYPEVAL(TYPE_PORT))
#define	GPC_DEV_IIC			SERIAL_DEVICE(GET_TARGET_TYPEVAL(TYPE_IIC))
#define	GPC_DEV_SMBUS		SERIAL_DEVICE(GET_TARGET_TYPEVAL(TYPE_SMBUS))

//Device Functions(DEVFUN)
#define GPC_LIB				SERIAL_DEVFUN(GPC_DEV_LIB,1)
#define GPC_FAN_HW			SERIAL_DEVFUN(GPC_DEV_HW,1)
#define	GPC_SRAM			SERIAL_DEVFUN(GPC_DEV_SRAM,1)
#define	GPC_CFG_PORT		SERIAL_DEVFUN(GPC_DEV_PORT,1)
#define	GPC_DATA_PORT		SERIAL_DEVFUN(GPC_DEV_PORT,2)
#define	GPC_CFG_IIC			SERIAL_DEVFUN(GPC_DEV_IIC,1)
#define	GPC_CMD_IIC			SERIAL_DEVFUN(GPC_DEV_IIC,2)
#define	GPC_CFG_SMBUS		SERIAL_DEVFUN(GPC_DEV_SMBUS,1)
#define	GPC_CMD_SMBUS		SERIAL_DEVFUN(GPC_DEV_SMBUS,2)

//Device Number
#define GPC_FAN_HW_0		SERIAL_DEVNUM(GPC_FAN_HW,1)
#define GPC_FAN_HW_1		SERIAL_DEVNUM(GPC_FAN_HW,2)
#define GPC_FAN_HW_2		SERIAL_DEVNUM(GPC_FAN_HW,3)
#define GPC_FAN_HW_3		SERIAL_DEVNUM(GPC_FAN_HW,4)

enum	_tagCB_SRC
{
    CB_SRC_HW_FAN0 = 1,
    CB_SRC_HW_FAN1,
    CB_SRC_HW_FAN2,
    CB_SRC_HW_FAN3,
}; 

enum	_tagCB_DATA
{
    CB_DATA_JAM = 1,
    CB_DATA_EMPTY,
    CB_DATA_COUNTING,
    CB_DATA_FISHING,
    CB_DATA_FULL,    
    CB_DATA_OVERPAY,   
    CB_DATA_COUNT_DONE,
    CB_DATA_SIGNAL_HIGH,
    CB_DATA_SIGNAL_LOW,
    CB_DATA_PRESENT,
    CB_DATA_NOTPRESENT,
	CB_DATA_FAN_HEALTHY = 1,
	CB_DATA_FAN_BROKEN,
	CB_DATA_FAN_OLD,
};

//**********************************************************
//**********************************************************
//**   Defination for Action
//**********************************************************
//**********************************************************
#define ACTION_SET 0x08
#define ACTION_GET 0x20

enum
{
	SET_ON = 1,
	SET_OFF,
	GET_ON,
	GET_OFF,
	SET_INPUT,
	SET_OUTPUT,
	GET_INPUT,
	GET_OUTPUT,
	SET_DATA,
};

//**********************************************************
//**********************************************************
//**   Defination for SubTarget
//**********************************************************
//**********************************************************
//Library setting
#define	  GPC_LIB_INIT 		 	          	  BITSHIFT(BIT_00)
#define	  GPC_LIB_CALLBACKFUN             	  BITSHIFT(BIT_01)
#define	  GPC_LIB_DEINIT                  	  BITSHIFT(BIT_02)
#define	  GPC_LIB_VERSION                 	  BITSHIFT(BIT_03)
#define	  GPC_LIB_LASTERROR              	  BITSHIFT(BIT_04)

//Hardware FAN Control
#define   FAN_HW_STATUS						  BITSHIFT(BIT_31)
#define   FAN_HW_DUTY						  BITSHIFT(BIT_30)
#define   FAN_HW_TARGET_PERIOD				  BITSHIFT(BIT_29)
#define   FAN_HW_CURRENT_PERIOD				  BITSHIFT(BIT_28)

//SRAM access
#define   SRAM_BANK_SELECT					  BITSHIFT(BIT_31)
#define	  SRAM_BYTE_RW                        BITSHIFT(BIT_30)
#define	  SRAM_WORD_RW                    	  BITSHIFT(BIT_29)
#define	  SRAM_DWORD_RW                       BITSHIFT(BIT_28)
#define	  SRAM_BLOCK_RW                       BITSHIFT(BIT_27)

#define	  SRAM_ACCESS                         BITSHIFT(BIT_01) 
#define	  SRAM_RW                             BITSHIFT(BIT_02)

//PORT
#define   PORT_ACCESS                  		  BITSHIFT(BIT_31)
#define   PORT_BIT_ACCESS                  	  BITSHIFT(BIT_30)
#define   PORT_ACTIVE                 		  BITSHIFT(BIT_29)
#define   PORT_DEBOUNCE                       BITSHIFT(BIT_28)
//for compatible with old gpc2800
#define   PORT_INTMASK                 		  BITSHIFT(BIT_29)

//IIC
#define   IIC_CFG_HOST_ENB                    BITSHIFT(BIT_31)
#define   IIC_CFG_CLK_EN                      BITSHIFT(BIT_30)
#define   IIC_CFG_SPEED                       BITSHIFT(BIT_29)

//SMBUS
#define   SMBUS_CFG_HOST_ENB                  BITSHIFT(BIT_31)
#define   SMBUS_CFG_CLK_EN                    BITSHIFT(BIT_30)
#define   SMBUS_CFG_SPEED                     BITSHIFT(BIT_29)
#define   SMBUS_CMD_QUICK                     BITSHIFT(BIT_31)
#define   SMBUS_CMD_BYTE					  BITSHIFT(BIT_30)
#define   SMBUS_CMD_BLOCK					  BITSHIFT(BIT_29)

//**********************************************************
//**********************************************************
//**   Defination for data struct
//**********************************************************
//**********************************************************

//Library setting
typedef struct _tagDSGPC_LIB
{
	U32 				u32_subtarget;
	U08         u08_type;
	int_callback_fun_t	        int_callback_fun;
	U32 				u32_libver;
	U32				  u32_lasterr;
	void				*void_pDev;
}DSGPC_LIB,*PDSGPC_LIB;

//Hardware
typedef struct _tagDSGPC_HW
{
	U32 				u32_subtarget;
	U08					u08_fanperiod;
	U08					u08_fanduty;
	U08					u08_fanstatus;
}DSGPC_HW, *PDSGPC_HW;


//SRAM
#define   SRAM_BANK_0						  BITSHIFT(BIT_00)
#define   SRAM_BANK_1						  BITSHIFT(BIT_01)
#define   SRAM_BANK_2						  BITSHIFT(BIT_02)

typedef struct _tagDSGPC_SRAM
{
	U32 	 u32_subtarget;
	//U32		 u32_offset;
	U32		 u32_address;
	U32		 u32_size;
	U08		 u08_bank;
	U08		 u08_db;
	U16		 u16_dw;
	U32		 u32_dd;
	U08	    *pu08_data;
}DSGPC_SRAM,*PDSGPC_SRAM; 

//PORT
//INPUT PORTs
#define PORT_00								  BITSHIFT(BIT_00)
#define PORT_01								  BITSHIFT(BIT_01)
#define PORT_02								  BITSHIFT(BIT_02)
#define PORT_03								  BITSHIFT(BIT_03)
#define PORT_04								  BITSHIFT(BIT_04)
#define PORT_05								  BITSHIFT(BIT_05)
#define PORT_06								  BITSHIFT(BIT_06)
#define PORT_07								  BITSHIFT(BIT_07)
#define PORT_08								  BITSHIFT(BIT_08)
#define PORT_09								  BITSHIFT(BIT_09)
#define PORT_10								  BITSHIFT(BIT_10)
#define PORT_11								  BITSHIFT(BIT_11)
#define PORT_12								  BITSHIFT(BIT_12)
#define PORT_13								  BITSHIFT(BIT_13)

//OUTPUT PORTs
#define PORT_20								  BITSHIFT(BIT_20)
#define PORT_21								  BITSHIFT(BIT_21)
#define PORT_22								  BITSHIFT(BIT_22)
#define PORT_23								  BITSHIFT(BIT_23)
#define PORT_24								  BITSHIFT(BIT_24)
#define PORT_25								  BITSHIFT(BIT_25)
#define PORT_26								  BITSHIFT(BIT_26)
#define PORT_27								  BITSHIFT(BIT_27)
#define PORT_28								  BITSHIFT(BIT_28)
#define PORT_29								  BITSHIFT(BIT_29)
#define PORT_30								  BITSHIFT(BIT_30)
#define PORT_31								  BITSHIFT(BIT_31)

#define   INPUTPORT_00   PORT_00
#define   INPUTPORT_01   PORT_01
#define   INPUTPORT_02   PORT_02
#define   INPUTPORT_03   PORT_03
#define   INPUTPORT_04   PORT_04
#define   INPUTPORT_05   PORT_05
#define   INPUTPORT_06   PORT_06
#define   INPUTPORT_07   PORT_07
#define   INPUTPORT_08   PORT_08
#define   INPUTPORT_09   PORT_09
#define   INPUTPORT_10   PORT_10
#define   INPUTPORT_11   PORT_11
#define   INPUTPORT_12   PORT_12
#define   INPUTPORT_13   PORT_13

#define   OUTPUTPORT_00  PORT_20
#define   OUTPUTPORT_01  PORT_21
#define   OUTPUTPORT_02  PORT_22
#define   OUTPUTPORT_03  PORT_23
#define   OUTPUTPORT_04  PORT_24
#define   OUTPUTPORT_05  PORT_25
#define   OUTPUTPORT_06  PORT_26
#define   OUTPUTPORT_07  PORT_27
#define   OUTPUTPORT_08  PORT_28
#define   OUTPUTPORT_09  PORT_29
#define   OUTPUTPORT_10  PORT_30
#define   OUTPUTPORT_11  PORT_31

//PORT BIT
#define PORT_BIT00								  BITSHIFT(BIT_00)
#define PORT_BIT01								  BITSHIFT(BIT_01)
#define PORT_BIT02								  BITSHIFT(BIT_02)
#define PORT_BIT03								  BITSHIFT(BIT_03)
#define PORT_BIT04								  BITSHIFT(BIT_04)
#define PORT_BIT05								  BITSHIFT(BIT_05)
#define PORT_BIT06								  BITSHIFT(BIT_06)
#define PORT_BIT07								  BITSHIFT(BIT_07)

//PORT DATA
#define PORT_DATA_0								0
#define PORT_DATA_1								1
#define PORT_DATA_HIGH							PORT_DATA_1
#define PORT_DATA_LOW							PORT_DATA_0

typedef struct _tagDSGPC_PORT
{
	U32 	u32_subtarget;
	U32		u32_portnum;
	U32		u32_portbit;
	U08     u08_onoff;
	U08		u08_debounce;
	U08		u08_data;
}DSGPC_PORT,*PDSGPC_PORT;

//IIC
#define IIC_CMD_START		0x02
#define IIC_CMD_STOP		0x03
#define IIC_CMD_TX			0x00
#define IIC_CMD_RX			0x01

enum
{
	IIC_SPEED_10K=0,
	IIC_SPEED_20K,
	IIC_SPEED_40K,
	IIC_SPEED_80K,
};

typedef struct _tagDSGPC_IIC
{
	U32 	u32_subtarget;
	U08		u08_onoff;
	U08     u08_speed;
	U08		u08_cmdlen;
	U08     u08_datalen;
	U08	    *pu08_data;
}DSGPC_IIC,*PDSGPC_IIC;


//SMBUS
typedef struct _tagDSGPC_SMBUS
{
	U32 	u32_subtarget;
	U08		u08_onoff;
	U08     u08_speed;
	U08     u08_datalen;
	U08	    *pu08_data;
	U08		u08_command;
	U08     u08_slaveaddr;
}DSGPC_SMBUS,*PDSGPC_SMBUS;

//**********************************************************
//**********************************************************
//**   Description for Functions
//**********************************************************
//**********************************************************
int	GPC_CTLCODE(U32 target, U32 action, void *lparam);

#ifdef	__cplusplus
}
#endif	//__cplusplus

#endif
