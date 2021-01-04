#ifndef __SIO_H_
#define __SIO_H_

#include <unistd.h> /* for libc5 */
#include <sys/io.h> /* for glibc */

typedef	unsigned char 		byte;
typedef unsigned long int 	dword;

#define	HIGH		1
#define	LOW		0

/*
typedef	struct _tagSIO_HWMON
{
	unsigned char	laddr;
	unsigned char	haddr;
}SIO_HWMON,*PSIO_HWMON;
*/

typedef struct _tagSIO_CTLOBJ
{
	struct
	{
		unsigned short	cfg_addr;
		unsigned short	dat_addr;
	}sio_ctl;

	struct
	{
		unsigned short	cfg_addr;
		unsigned short	dat_addr;
	}sio_hwmon;
	
}SIO_CTLOBJ,*PSIO_CTLOBJ;

extern SIO_CTLOBJ g_sioctl;

/////////////////////////////////////////////////////////////////////////////
#define SIO_CFG_ADDR1 (0x2E)
#define SIO_DAT_ADDR1 (SIO_CFG_ADDR1+1)
#define SIO_CFG_ADDR2 (0x4E)
#define SIO_DAT_ADDR2 (SIO_CFG_ADDR2+1)

#define SIO_CFG_ADDR (g_sioctl.sio_ctl.cfg_addr)
#define SIO_DAT_ADDR (g_sioctl.sio_ctl.dat_addr)
#define REG_SIO_SEL_LOGDEV 	0x07
#define REG_SIO_INIT_MAGICNUM	0x87
#define REG_SIO_EXIT_EFER	0xAA

#define CFG_SIO_REG(reg)		outb((reg),SIO_CFG_ADDR)
#define WRITE_SIO_DATA(data)		outb((data),SIO_DAT_ADDR)
#define READ_SIO_DATA()			inb(SIO_DAT_ADDR)

#define SET_SIO_REG_BIT_DATA(offset,data)	\
	{CFG_SIO_REG(offset);WRITE_SIO_DATA(READ_SIO_DATA()|(data));}
#define CLS_SIO_REG_BIT_DATA(offset,data)	\
	{CFG_SIO_REG(offset);WRITE_SIO_DATA(READ_SIO_DATA()&~(data));}
#define SELECT_SIO_LOGICAL_DEV(dev)	\
	{WRITE_SIO_REG_DATA(REG_SIO_SEL_LOGDEV,(dev));}
#define WRITE_SIO_REG_DATA(offset,data)	\
	{CFG_SIO_REG(offset);WRITE_SIO_DATA(data);}
#define READ_SIO_REG_DATA(offset) _Read_SIO_Reg_Data(offset)
#define SIO_SET_GPIO(cf,x,level)	_SIO_Set_GPIO(cf,GPIO_BIT_MASK_##x,level)
#define SIO_GET_GPIO(cf,x)	_SIO_Get_GPIO(cf,GPIO_BIT_MASK_##x)

byte _Read_SIO_Reg_Data(byte offset);
int _SIO_Get_GPIO(byte CF,byte mask);
void _SIO_Set_GPIO(byte CF,byte mask,int level);

///////////////////////////////////////////////////////////////////////////

#define SIO_HWMON_IDX_ADDR (g_sioctl.sio_hwmon.cfg_addr)
#define SIO_HWMON_DAT_ADDR (g_sioctl.sio_hwmon.dat_addr)

#define CFG_HWMON_REG(reg)		outb((reg),SIO_HWMON_IDX_ADDR)
#define WRITE_HWMON_DATA(data)		outb((data),SIO_HWMON_DAT_ADDR)
#define READ_HWMON_DATA()			inb(SIO_HWMON_DAT_ADDR)
/////////////////////////////////////////////////////////////////////////////

#define GPIO_BIT_MASK_10	0x01
#define GPIO_BIT_MASK_11	0x02
#define GPIO_BIT_MASK_12	0x04
#define GPIO_BIT_MASK_13	0x08
#define GPIO_BIT_MASK_14	0x10
#define GPIO_BIT_MASK_15	0x20
#define GPIO_BIT_MASK_16	0x40
#define GPIO_BIT_MASK_17	0x80
#define GPIO_BIT_MASK_20	0x01
#define GPIO_BIT_MASK_21	0x02
#define GPIO_BIT_MASK_22	0x04
#define GPIO_BIT_MASK_23	0x08
#define GPIO_BIT_MASK_24	0x10
#define GPIO_BIT_MASK_25	0x20
#define GPIO_BIT_MASK_26	0x40
#define GPIO_BIT_MASK_27	0x80
#define GPIO_BIT_MASK_30	0x01
#define GPIO_BIT_MASK_31	0x02
#define GPIO_BIT_MASK_32	0x04
#define GPIO_BIT_MASK_33	0x08
#define GPIO_BIT_MASK_34	0x10
#define GPIO_BIT_MASK_35	0x20
#define GPIO_BIT_MASK_36	0x40
#define GPIO_BIT_MASK_37	0x80

#define W83627HF_GP_SET(gpnum,level)	SIO_SET_GPIO(0xF1,gpnum,level)
#define W83627HF_GP_GET(gpnum)	SIO_GET_GPIO(0xF1,gpnum)

//GPIO 1x
#define SIO_SET_GP10(level) W83627HF_GP_SET(10,level)
#define SIO_SET_GP11(level) W83627HF_GP_SET(11,level)
#define SIO_SET_GP12(level) W83627HF_GP_SET(12,level)
#define SIO_SET_GP13(level) W83627HF_GP_SET(13,level)
#define SIO_SET_GP14(level) W83627HF_GP_SET(14,level)
#define SIO_SET_GP15(level) W83627HF_GP_SET(15,level)
#define SIO_SET_GP16(level) W83627HF_GP_SET(16,level)
#define SIO_SET_GP17(level) W83627HF_GP_SET(17,level)
#define SIO_GET_GP10() W83627HF_GP_GET(10)
#define SIO_GET_GP11() W83627HF_GP_GET(11)
#define SIO_GET_GP12() W83627HF_GP_GET(12)
#define SIO_GET_GP13() W83627HF_GP_GET(13)
#define SIO_GET_GP14() W83627HF_GP_GET(14)
#define SIO_GET_GP15() W83627HF_GP_GET(15)
#define SIO_GET_GP16() W83627HF_GP_GET(16)
#define SIO_GET_GP17() W83627HF_GP_GET(17)

//GPIO 2x
#define SIO_SET_GP20(level) W83627HF_GP_SET(20,level)
#define SIO_SET_GP21(level) W83627HF_GP_SET(21,level)
#define SIO_SET_GP22(level) W83627HF_GP_SET(22,level)
#define SIO_SET_GP23(level) W83627HF_GP_SET(23,level)
#define SIO_SET_GP24(level) W83627HF_GP_SET(24,level)
#define SIO_SET_GP25(level) W83627HF_GP_SET(25,level)
#define SIO_SET_GP26(level) W83627HF_GP_SET(26,level)
#define SIO_SET_GP27(level) W83627HF_GP_SET(27,level)
#define SIO_GET_GP20() W83627HF_GP_GET(20)
#define SIO_GET_GP21() W83627HF_GP_GET(21)
#define SIO_GET_GP22() W83627HF_GP_GET(22)
#define SIO_GET_GP23() W83627HF_GP_GET(23)
#define SIO_GET_GP24() W83627HF_GP_GET(24)
#define SIO_GET_GP25() W83627HF_GP_GET(25)
#define SIO_GET_GP26() W83627HF_GP_GET(26)
#define SIO_GET_GP27() W83627HF_GP_GET(27)

//GPIO 3x
#define SIO_SET_GP30(level) W83627HF_GP_SET(30,level)
#define SIO_SET_GP31(level) W83627HF_GP_SET(31,level)
#define SIO_SET_GP32(level) W83627HF_GP_SET(32,level)
#define SIO_SET_GP33(level) W83627HF_GP_SET(33,level)
#define SIO_SET_GP34(level) W83627HF_GP_SET(34,level)
#define SIO_SET_GP35(level) W83627HF_GP_SET(35,level)
#define SIO_SET_GP36(level) W83627HF_GP_SET(36,level)
#define SIO_SET_GP37(level) W83627HF_GP_SET(37,level)
#define SIO_GET_GP30() W83627HF_GP_GET(30)
#define SIO_GET_GP31() W83627HF_GP_GET(31)
#define SIO_GET_GP32() W83627HF_GP_GET(32)
#define SIO_GET_GP33() W83627HF_GP_GET(33)
#define SIO_GET_GP34() W83627HF_GP_GET(34)
#define SIO_GET_GP35() W83627HF_GP_GET(35)
#define SIO_GET_GP36() W83627HF_GP_GET(36)
#define SIO_GET_GP37() W83627HF_GP_GET(37)

#define W83627HF_SET_DIR_OUT(mask)	CLS_SIO_REG_BIT_DATA(0xF0,mask)
#define W83627HF_SET_DIR_IN(mask)	SET_SIO_REG_BIT_DATA(0xF0,mask)
#define W83627HF_DIR_NOINVERT(mask)	CLS_SIO_REG_BIT_DATA(0xF2,mask)
#define W83627HF_DIR_INVERT(mask)	SET_SIO_REG_BIT_DATA(0xF2,mask)
#define W83627HF_ACTIVE_LG()	SET_SIO_REG_BIT_DATA(0x30, 0x01)

///////////////////////////////////////////////////////////////////////////

void SIO_HW_Init(unsigned char sio_cfgaddr,unsigned char sio_dataddr,
			unsigned char h_baddr, unsigned char l_baddr,unsigned char shift);
void SIO_HW_DeInit();
void GP1x_HW_Init(byte bMask,byte bInvert);
void GP2x_HW_Init(byte bMask,byte bInvert);
void GP3x_HW_Init(byte bMask,byte bInvert);
void SET_HWMON_ADDR(unsigned char h_baddr, unsigned char l_baddr,unsigned char shift);
#endif
