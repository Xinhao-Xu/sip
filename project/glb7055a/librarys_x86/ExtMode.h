#ifndef __EXTMODE_H__
#define  __EXTMODE_H__

#include "bits.h"
#include "global.h"

#undef _EXTERN
#ifdef MAIN_DEFINATION
    #define _EXTERN
#else
	#ifdef OS_WIN
		#define _EXTERN __declspec(dllexport)
	#else
    	#define _EXTERN extern
    #endif
#endif
#undef MAIN_DEFINATION

#ifdef	__cplusplus
extern "C" {
#endif	

#define EXTMODE_FILENAME		"./extmode.so"
#define FUNNAME_EXTMODE_HANDLER	"extmode_handler"
typedef int (*extmode_handler_t)(U32 target, U32 action, void *lparam);

#define SERIAL_EXTERN(num)			(BOUND_EXTSEC_START + (num)*(BOUND_DEVICE+1))
#define SERIAL_EXTFUN(dev,fun)			((dev)+(fun)*(BOUND_DEVFUN+1))
#define SERIAL_EXTNUM(devfun,num)		((devfun)+(num))
#define EXT_TARGET(devfun,num)			SERIAL_EXTNUM(devfun,num)
#define MASK_TARGET_FUN				0xFFFFFF00

//**********************************************************
//**********************************************************
//**   Defination for Target
//**********************************************************
//**********************************************************

#define EXT_SPIMODULE_W25X			SERIAL_EXTERN(1)

//**********************************************************
//**********************************************************
//**   Defination for SubTarget
//**********************************************************
//**********************************************************
#define SPIMODULE_W25X_DEVICE			SERIAL_EXTFUN(EXT_SPIMODULE_W25X,1)
#define SPIMODULE_W25X_DATA			SERIAL_EXTFUN(EXT_SPIMODULE_W25X,2)
#define SPIMODULE_W25X_ERASE_ALL		SERIAL_EXTFUN(EXT_SPIMODULE_W25X,3)
#define SPIMODULE_W25X_ERASE_BLOCK		SERIAL_EXTFUN(EXT_SPIMODULE_W25X,4)
#define SPIMODULE_W25X_ERASE_SECTOR		SERIAL_EXTFUN(EXT_SPIMODULE_W25X,5)

//**********************************************************
//**********************************************************
//**   Defination for data struct
//**********************************************************
//**********************************************************
typedef struct _tagDSEXT_SPIMODULE_W25X
{
	U32 	u32_subtarget;
	U32     u32_addr;
	U32	u32_size;
	U08	u08_onoff;
	U08    *pu08_data;	
}DSEXT_SPIMODULE_W25X,*PDSEXT_SPIMODULE_W25X;

//**********************************************************
//**********************************************************
//**   Description for Functions
//**********************************************************
//**********************************************************
_EXTERN int	extmode_handler(U32 target, U32 action, void *lparam);


#ifdef	__cplusplus
}
#endif

#endif

