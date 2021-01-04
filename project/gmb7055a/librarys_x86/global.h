#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

//**********************************************************
//**********************************************************
//**   Defination for Function result return
//**********************************************************
//**********************************************************
#define GPC_FUN_RES_SUCC	0
#define GPC_FUN_RES_FAIL	1

#define EIT_FUN_RES_SUCC	0
#define EIT_FUN_RES_FAIL	1

//**********************************************************
//**********************************************************
//**   Defination for Traget sections
//**
//**	Target =    IC + Device + DevFun + Devnum
//**	       =0x  AA     BB      CC		DD
//**********************************************************
//**********************************************************

#define MASK_IC				0xFF000000
#define MASK_DEVICE			0x00FF0000
#define MASK_DEVFUN			0x0000FF00
#define MASK_DEVNUM			0x000000FF

#define BITSHIFT_IC			24
#define BITSHIFT_DEVICE		16
#define BITSHIFT_DEVFUN		8

#define SECTION_IC_RESERVE	0x01000000
#define SECTION_IC_GPC		0x02000000
#define SECTION_IC_EIT		0x03000000
#define SECTION_IC_EXTERN	0x04000000

#define BOUND_IC			0x00FFFFFF
#define BOUND_DEVICE		0x0000FFFF
#define BOUND_DEVFUN		0x000000FF

#define BOUND_REVSEC_START	(SECTION_IC_RESERVE)
#define BOUND_REVSEC_END	(SECTION_IC_RESERVE+BOUND_IC)
#define BOUND_GPCSEC_START	(SECTION_IC_GPC)
#define BOUND_GPCSEC_END	(SECTION_IC_GPC+BOUND_IC)
#define BOUND_EITSEC_START	(SECTION_IC_EIT)
#define BOUND_EITSEC_END	(SECTION_IC_EIT+BOUND_IC)
#define BOUND_EXTSEC_START	(SECTION_IC_EXTERN)
#define BOUND_EXTSEC_END	(SECTION_IC_EXTERN+BOUND_IC)

//**********************************************************
//**********************************************************
//**   Defination for Error Message Return
//**********************************************************
//**********************************************************
#define MSG_MASK_TARGET			0xFFFF0000
#define MSG_MASK_REASON			0x0000FFFF
#define MSG_TARGET_SHIFT			16
#define MSG_REASON_SHIFT			0

#define MSG_RESULT(target,reason)	(target|reason)
#define MSG_TARGET(msg)			((msg<<MSG_TARGET_SHIFT)&MSG_MASK_TARGET)
#define MSG_REASON(msg)			((msg<<MSG_REASON_SHIFT)&MSG_MASK_REASON)

#define GET_MSG_TARGET(msg)		((msg&MSG_MASK_TARGET)>>MSG_TARGET_SHIFT)
#define GET_MSG_REASON(msg)		((msg&MSG_MASK_REASON)>>MSG_REASON_SHIFT)

//used for system error message type of MSG_TARGET(msg)
//General MSG_TARGET(msg) is begin from 0x1FF
enum _tagTYPE_SYSTEM
{
	TYPE_GPC_CTLCODE	= 100,
	TYPE_EIT_CTLCODE,
	TYPE_EIT,
	TYPE_EXTERN,	
	TYPE_UNKNOWN,
};

//Note: this series must be the same with GPC_DEV_XXX
#define GET_TARGET_TYPEVAL(type)	(type-TYPE_TARGET_START)

enum _tagTYPE_TARGET
{
	/*
	   TYPE_INTERRUPT is not export used for user, and should avoid to appear in this table.
	   or detect_funtype() in Main.c(gpc20) will get wrong TYPE.
	   */
	//TYPE_INTERRUPT,
	TYPE_TARGET_START = 0x200,
	TYPE_LIBRARY,
	TYPE_HARDWARE,
	TYPE_SRAM,
	TYPE_IIC,
	TYPE_SMBUS,
	TYPE_METER,
	TYPE_HOPPER,
	TYPE_COIN_IN_SIGNAL,
	TYPE_BILL_IN_SIGNAL,
	TYPE_KEYLOCK,
	TYPE_TIMER,
	TYPE_LAMP,
	TYPE_DIPSWITCH,
	TYPE_BUTTON,
	TYPE_PORT,
	TYPE_SPI,
	TYPE_ONEWIRE,
	TYPE_ROOTSECU_BASIC,
	TYPE_RNG,
};

enum _tagERR_REASON
{
	//Common
	ERROR_PARAMETER = 0x0F,		        /* error parameter format */
	ERROR_IOCTL,						/* call ioctl() error */
	ERROR_RUNTIME,						/* runtime error */

	//Library Setting
	ERROR_NOTINITED,					/* Libary not initiated */
	ERROR_CREATEIST,                    /* Create IST service error */
	ERROR_OPENDEVICE,					/* can't open device */
	ERROR_REGISTER_CALLBACKFUN,         /* fail to register call back function */
	ERROR_CREATEFIFO,					/* Can't create FIFO server */
	ERROR_GPC_NOTSUPPORT,				/* Can't find supported GPC */
	ERROR_OPENLIBFILE,					/* Fail to load library file */
	ERROR_FINDFUN,						/* Fail to find functions */

	//SRAM
	ERROR_SRAM_OUTRANGE,                /* SRAM Read/Write out of range */

	//IIC
	ERROR_IIC_TIMEROUT,

	//SM-BUS
	ERROR_SMBUS_TIMEOUT,

	//SPI
	ERROR_SPI_TX_TIMEROUT,
	ERROR_SPI_RX_TIMEROUT,
};

#endif // ERROR_H_INCLUDED
