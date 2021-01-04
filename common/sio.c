#include "SIO.h"

SIO_CTLOBJ g_sioctl;

byte g_Matrix_GPIO[3][8]={
	{GPIO_BIT_MASK_10,GPIO_BIT_MASK_11,GPIO_BIT_MASK_12,GPIO_BIT_MASK_13,
	GPIO_BIT_MASK_14,GPIO_BIT_MASK_15,GPIO_BIT_MASK_16,GPIO_BIT_MASK_17},
	{GPIO_BIT_MASK_20,GPIO_BIT_MASK_21,GPIO_BIT_MASK_22,GPIO_BIT_MASK_23,
	GPIO_BIT_MASK_24,GPIO_BIT_MASK_25,GPIO_BIT_MASK_26,GPIO_BIT_MASK_27},
	{GPIO_BIT_MASK_30,GPIO_BIT_MASK_31,GPIO_BIT_MASK_32,GPIO_BIT_MASK_33,
	GPIO_BIT_MASK_34,GPIO_BIT_MASK_35,GPIO_BIT_MASK_36,GPIO_BIT_MASK_37}};

/*
	Setting the directory of GPIO1x.
	where bMask is the mask bits of GPIO1x and
		1 will set the port to input and 0 will set the port to output.
		bInvert is the mask bits for setting invert atribute of GPIO2x and
		1 means value is inverted , 0 means value is the same as in register
*/
void GP1x_HW_Init(byte bMask,byte bInvert)
{
	int i;

	//
	// multifunction pin -> GPIO
	//
	SET_SIO_REG_BIT_DATA(0x2A, 0xFC);

	//
	// goto LD7
	//
	SELECT_SIO_LOGICAL_DEV(0x07);

	//
	// activate current Logic device
	//
	W83627HF_ACTIVE_LG();

	for(i=0;i<8;i++)
	{
		if(bMask&g_Matrix_GPIO[0][i])
		{
			W83627HF_SET_DIR_IN(g_Matrix_GPIO[0][i]);
		}
		else
		{
			W83627HF_SET_DIR_OUT(g_Matrix_GPIO[0][i]);
		}
		if(bInvert&g_Matrix_GPIO[0][i])
		{
			W83627HF_DIR_INVERT(g_Matrix_GPIO[0][i]);
		}
		else
		{
			W83627HF_DIR_NOINVERT(g_Matrix_GPIO[0][i]);
		}
	}
}

/*
	Setting the directory of GPIO2x.
	where bMask is the mask bits of GPIO2x and
		1 will set the port to input and 0 will set the port to output.
		bInvert is the mask bits for setting invert atribute of GPIO2x and
		1 means value is inverted , 0 means value is the same as in register
*/
void GP2x_HW_Init(byte bMask,byte bInvert)
{
	int i;

	//
	// multifunction pin -> GPIO
	//
	SET_SIO_REG_BIT_DATA(0x2A, 0x01);

	//
	// multifunction pin -> GPIO
	//
	SET_SIO_REG_BIT_DATA(0x2B, 0xFF);
	
	//
	// goto LD8
	//
	SELECT_SIO_LOGICAL_DEV(0x08);

	//
	// activate current Logic device
	//
	W83627HF_ACTIVE_LG();

	for(i=0;i<8;i++)
	{
		if(bMask&g_Matrix_GPIO[1][i])
		{
			W83627HF_SET_DIR_IN(g_Matrix_GPIO[1][i]);
		}
		else
		{
			W83627HF_SET_DIR_OUT(g_Matrix_GPIO[1][i]);
		}
		if(bInvert&g_Matrix_GPIO[1][i])
		{
			W83627HF_DIR_INVERT(g_Matrix_GPIO[1][i]);
		}
		else
		{
			W83627HF_DIR_NOINVERT(g_Matrix_GPIO[1][i]);
		}
	}
}


/*
	Setting the directory of GPIO3x.
	where bMask is the mask bits of GPIO3x and
		1 will set the port to input and 0 will set the port to output.
		bInvert is the mask bits for setting invert atribute of GPIO2x and
		1 means value is inverted , 0 means value is the same as in register
*/
void GP3x_HW_Init(byte bMask,byte bInvert)
{
	int i;

	//
	// multifunction pin -> GPIO
	//
	SET_SIO_REG_BIT_DATA(0x29, 0xFC);
	
	//
	// goto LD9
	//
	SELECT_SIO_LOGICAL_DEV(0x09);

	//
	// activate current Logic device
	//
	W83627HF_ACTIVE_LG();

	for(i=0;i<6;i++)
	{
		if(bMask&g_Matrix_GPIO[2][i])
		{
			W83627HF_SET_DIR_IN(g_Matrix_GPIO[2][i]);
		}
		else
		{
			W83627HF_SET_DIR_OUT(g_Matrix_GPIO[2][i]);
		}
		if(bInvert&g_Matrix_GPIO[2][i])
		{
			W83627HF_DIR_INVERT(g_Matrix_GPIO[2][i]);
		}
		else
		{
			W83627HF_DIR_NOINVERT(g_Matrix_GPIO[2][i]);
		}
	}
}

void SIO_HW_Init(unsigned char sio_cfgaddr,		//SIO config address
				unsigned char sio_dataddr,		//SIO data address
				unsigned char h_baddr,			//HWM high base address
				unsigned char l_baddr,			//HWM low base address
				unsigned char shift)			//shift address for HWM
{
	g_sioctl.sio_ctl.cfg_addr = sio_cfgaddr;
	g_sioctl.sio_ctl.dat_addr = sio_dataddr;	

	ioperm(g_sioctl.sio_ctl.cfg_addr,1,1);
	ioperm(g_sioctl.sio_ctl.dat_addr,1,1);
	CFG_SIO_REG(REG_SIO_INIT_MAGICNUM);
	CFG_SIO_REG(REG_SIO_INIT_MAGICNUM);
	SET_HWMON_ADDR(h_baddr,l_baddr,shift);
	ioperm(g_sioctl.sio_hwmon.cfg_addr,1,1);		
	ioperm(g_sioctl.sio_hwmon.dat_addr,1,1);		
}

void SIO_HW_DeInit()
{
	CFG_SIO_REG(REG_SIO_EXIT_EFER);
}

byte _Read_SIO_Reg_Data(byte offset) 
{
	CFG_SIO_REG(offset); 
	
	return READ_SIO_DATA(); 
}

int _SIO_Get_GPIO(byte CF,byte mask)
{
	byte data=READ_SIO_REG_DATA(CF);

	if(data & mask)
		return HIGH;
	else
		return LOW;
}

void _SIO_Set_GPIO(byte CF,byte mask,int level)
{
	if(level)
	{
		SET_SIO_REG_BIT_DATA(CF, mask);
	}
	else
	{
		CLS_SIO_REG_BIT_DATA(CF, mask);
	}
}

void SET_HWMON_ADDR(unsigned char h_baddr, unsigned char l_baddr,unsigned char shift)
{
	unsigned char laddr,haddr;
	CFG_SIO_REG(h_baddr);
	haddr = READ_SIO_DATA();
	CFG_SIO_REG(l_baddr);
	laddr = READ_SIO_DATA();
	g_sioctl.sio_hwmon.cfg_addr=(haddr<<8|laddr)+shift;
	g_sioctl.sio_hwmon.dat_addr = g_sioctl.sio_hwmon.cfg_addr+1;	
}

