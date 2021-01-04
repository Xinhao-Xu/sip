#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/rtc.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>


#include "Debug.h"
#include "Result.h"
#include "Test_items.h"
#include "gpclib.h"
#include "eitlib.h"


int init_TestItem()
{
	initUtil();
}

int deinit_TestItem()
{
}

/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             CPU Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/

/*************************************************************************************
	Input the parameter struct _tagCPU_TEST and run the test.
	and all data is come from /proc/cpuinfo
	in which, _tagCPU_TEST.szCPUInfo should compare with "model name"
	          _tagCPU_TEST.speed should compare with "cpu MHz" 
	                and allow the error range between _tagCPU_TEST.range
*************************************************************************************/
int fun_TestCPU_001(PCPU_TEST pInfo)
{
	char szBuf[2048];
	char *pszProcInfo = "/proc/cpuinfo";
	char *pszCPUInfo = "/tmp/cpuinfo";
	FILE *file=NULL;
	int  res=RESULT_PASS;
	char *p;
	int speed;

	// step1. copy /proc/cpuinfo to /tmp
	sprintf(szBuf,"cp %s %s",pszProcInfo,pszCPUInfo);
	system(szBuf);
	file = fopen(pszCPUInfo,"r");
	if(file!=NULL)
	{
		while(!feof(file))
		{
			fgets(szBuf,2048,file);
			if(strstr(szBuf,"model name"))
			{
				if(!strstr(szBuf,pInfo->szCPUInfo))
					res = RESULT_FAIL;
			}
			if(p=strstr(szBuf,"cpu MHz"))
			{
				p=strstr(szBuf,":");
				p++;
				while(*p==' ')
					p++;
				speed = strtol(p,NULL,10);
				if( (speed>=(pInfo->speed + pInfo->range)) ||
					(speed<=(pInfo->speed - pInfo->range)))
				{
					res = RESULT_FAIL;
				}
			}
		}
	}
	fclose(file);

	return res;
}

/*************************************************************************************
	Input the parameter struct _tagCPU_TEST and run the test.
	and all data is come from get_dmiinfo() of Utils.c
	in which, "_tagCPU_TEST.szCPUInfo" should compare with "_tagDMIINFO._tagDMIINFO_CPU.ver"
	          "_tagCPU_TEST.spee"d should compare with  "_tagDMIINFO._tagDMIINFO_CPU.curSpeed"
	                and allow the error range between "_tagCPU_TEST.range"
*************************************************************************************/
int fun_TestCPU_002(PCPU_TEST pInfo)
{
	DMIINFO	dmi_info;
	int speed;
	char *s=NULL;
	
	get_dmiinfo(&dmi_info);
	//printf("dmi_info.cpuinfo.ver = %s\r\n",dmi_info.cpuinfo.ver);
	//comparing the version of cpu
	s=strstr(dmi_info.cpuinfo.ver,pInfo->szCPUInfo);
	if(!s)
	{
		DEBUGMSG("fun_TestCPU_002: cpu version not compared\r\n");
		DEBUGMSG("s=%s                 (%s)!=(%s)\r\n",s,pInfo->szCPUInfo,pInfo->szCPUInfo);
		return RESULT_FAIL;
	}
/*	
	//comparing the speed of cpu
	speed = dmi_info.cpuinfo.curSpeed;
	if( (speed>=(pInfo->speed + pInfo->range)) ||
		(speed<=(pInfo->speed - pInfo->range)))
	{
		DEBUGMSG("fun_TestCPU_002: speed not compared\r\n");	
		return RESULT_FAIL;
	}
*/
	return RESULT_PASS;
}

/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             Sound Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/
int fun_TestSnd_001(char *file)
{
	wav_play(file);
}

int fun_TestSnd_002(int audio_select, char *file)
{
	char szBuf[256];

	DEBUGMSG("audio_select = %d\r\n",audio_select);
	if(audio_select == 0)
	{
		sprintf(szBuf,"mplayer -ao alsa:device=hw=0.0 %s > /dev/null 2>&1",file);
		system(szBuf);
	}
	if(audio_select == 1)
	{
		sprintf(szBuf,"mplayer -ao alsa:device=hw=1.0 %s > /dev/null 2>&1",file);
		system(szBuf);
	}
	//if(audio_select == NULL)
	//{
	//	sprintf(szBuf,"mplayer -ao alsa:device=hw=0.0 %s > /dev/null 2>&1",file);
	//	system(szBuf);
	//}
}

int fun_TestSnd_003(unsigned int card,unsigned int dev,char *file)
{
	char szBuf[256];

	sprintf(szBuf,"aplay -D plughw:%d,%d %s > /dev/null 2>&1",card,dev,file);
	system(szBuf);
}


/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             Networking Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/
int fun_TestLAN_001()
{
	struct ifaddrs *ifap, *ifa;
	int rv;

	if ((rv=getifaddrs(&ifap))==-1)
	{ 
		perror("getifaddrs");
		exit(-1);
	}
	
	if (ifap == NULL)
	{ 
		DEBUGMSG("Null pointer returned\n");
		exit(-1);
	}
	
	for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next)
	{ 
		DEBUGMSG("Interface %s\r\n", ifa->ifa_name);
		DEBUGMSG("Interface address family: %d\r\n",
		ifa->ifa_addr->sa_family);
	}
	
	freeifaddrs(ifap);
}

#define MAX_IFS 64
int fun_TestLAN_002()
{
    struct ifreq *ifr, *ifend;
    struct ifreq ifreq;
    struct ifconf ifc;
    struct ifreq ifs[MAX_IFS];
    int SockFD;


    SockFD = socket(AF_INET, SOCK_DGRAM, 0);


    ifc.ifc_len = sizeof(ifs);
    ifc.ifc_req = ifs;
    if (ioctl(SockFD, SIOCGIFCONF, &ifc) < 0)
    {
        DEBUGMSG("ioctl(SIOCGIFCONF): %m\r\n");
        return 0;
    }


    ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));
    for (ifr = ifc.ifc_req; ifr < ifend; ifr++)
    {
    	if(!strcmp(ifr->ifr_name,"lo"))
    		continue;
    		
        if (ifr->ifr_addr.sa_family == AF_INET)
        {
            strncpy(ifreq.ifr_name, ifr->ifr_name,sizeof(ifreq.ifr_name));
            if (ioctl (SockFD, SIOCGIFHWADDR, &ifreq) < 0)
            {
              DEBUGMSG("SIOCGIFHWADDR(%s): %m\r\n", ifreq.ifr_name);
              return 0;
            }

			DEBUGMSG("Device %s -> Ethernet %02x:%02x:%02x:%02x:%02x:%02x\r\n", ifreq.ifr_name,
			(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[0],
			(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[1],
			(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[2],
			(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[3],
			(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[4],
			(int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[5]);
	      
			if(0==((int)((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[0])&&
			  0==((int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[1])&&
			  0==((int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[2])&&
			  0==((int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[3])&&
			  0==((int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[4])&&
			  0==((int) ((unsigned char *) &ifreq.ifr_hwaddr.sa_data)[5]))
			{
				DEBUGMSG("fun_TestNet_002 : RESULT_FAIL\r\n");
				return RESULT_FAIL;
			}
 		}
    }

	DEBUGMSG("fun_TestNet_002 : RESULT_PASS\r\n");
    return RESULT_PASS;
}

int fun_TestLAN_003(char *ip)
{
	if(FUN_SUCC == ping(ip))
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

/*************************************************************************************
  Using shell command 'ping' with specificy interface

  argument:
	dest_ip: destination ip for ping test
	src_interface: source interface like 'eth0'
*************************************************************************************/
int fun_TestLAN_004(char *dest_ip,char *src_interface)
{
	int res=RESULT_FAIL;
	char szBuf[256];
	FILE *fp;
	
	//sprintf(szBuf,"ping -c 3 -I %s %s  > /dev/null 2>&1",src_interface,dest_ip);
	sprintf(szBuf,"ping -c 3 -I %s %s",src_interface,dest_ip);
	//sprintf(szBuf,"ping -c 3 -I %s %s > /dev/null 2>&1",src_interface,dest_ip);
	system(szBuf);
	
	fp = popen(szBuf,"r");
	fgets(szBuf,256,fp);
	fgets(szBuf,256,fp);
	pclose(fp);
	
	if(strstr(szBuf,"ttl="))
		res=RESULT_PASS;
	else
		res=RESULT_FAIL;
		
	return res;
}


/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             Memory Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/
int fun_TestMemory_001(PMEM_TEST pInfo)
{
	DMIINFO	dmi_info;
	int speed;
	
	get_dmiinfo(&dmi_info);
	DEBUGMSG("pInfo->locator =%d\r\n",pInfo->locator);
	DEBUGMSG("fun_TestMemory_001(),dmi_info.meminfo.size=%d,pInfo->size=%d\r\n",dmi_info.meminfo.size,pInfo->size);
	DEBUGMSG("DK - - - TestMemory_001(),dmi_info.meminfo.size=%d,dmi_info2.meminfo.size=%d\r\n",dmi_info.meminfo.size,dmi_info.meminfo2.size);

	if(pInfo->locator==1)
	{
		//if(dmi_info.meminfo.size != pInfo->size)
		//	return RESULT_FAIL;
		//if(dmi_info.meminfo.size < pInfo->size)
		//	return RESULT_FAIL;

		//DK 20160626 temp		
		if( (dmi_info.meminfo.size + dmi_info.meminfo2.size) != pInfo->size )
			return RESULT_FAIL;
	}
	else if(pInfo->locator==2)
	{
		if(dmi_info.meminfo2.size != pInfo->size)
			return RESULT_FAIL;
	}
	
	return RESULT_PASS;
}

//DK - 201606116 dram
int fun_TestMemory_003(int  dram_size)
{
	char buf[16];
	char szBuf[256];	
	int i=0;
	DEBUGMSG("dram_size =%d\r\n",dram_size);
	sprintf(szBuf,"free -m | grep -o '[0-9]**[0-9]' > %s",MEM_DATA_FILE);
	system(szBuf);
	
	FILE *fp=fopen(MEM_DATA_FILE,"r+");
	
	fseek(fp,0,SEEK_SET);
	
	fgets(buf,16,fp);
	//printf("TestMemory_003: buf = %s %c %c %c %c\r\n",&buf,buf[0],buf[1],buf[2],buf[3]);
	fclose(fp);
	i = atoi(buf);
	DEBUGMSG("fun_TestMemory_003: free -m = %d\r\n",i);

	//printf("\r\ndram_size = %d\r\n",  dram_size);
	//printf("\r\i = %d\r\n",  i);

	if(dram_size >= 8192)
	{
		if( (i < dram_size) && (i > (dram_size-1300)) )
			return RESULT_PASS;
		else
			return RESULT_FAIL;
	}
	else if(dram_size == 1024)
	{
		if( (i < dram_size) && (i > (dram_size-400)) )
			return RESULT_PASS;
		else
			return RESULT_FAIL;
	}
	else
	{
		if( (i < dram_size) && (i > (dram_size-1000)) )
			return RESULT_PASS;
		else
			return RESULT_FAIL;
	}
	
}



/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             Storage Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/

int fun_MountStorage_001(char *dev,char *mntpoint)
{
	FILE_TX fileinfo;
	USBINFO usbinfo;
	char szBuf[256];
	char *tmpfile,*tmpdir,*tmpname;
	FILE *tmpfp;
	int res=RESULT_FAIL;;
	
	//mount usb storage
	sprintf(szBuf,"mount %s %s > /dev/null 2>&1",dev,mntpoint);
	system(szBuf);

	tmpfile = tmpnam(NULL);
	tmpdir = "/tmp";
	tmpname = tmpfile+5;
	tmpfp = open( tmpfile, O_CREAT ,0777);
	close(tmpfp);
	
	//Starting File_Tx test
	fileinfo.src = tmpdir;
	fileinfo.fname = tmpname;
	fileinfo.dest = mntpoint;

	fun_FileTransmit_001(&fileinfo);

	//umount usb storage
	sprintf(szBuf,"umount %s > /dev/null 2>&1",mntpoint);
	system(szBuf);

	//del test file to avoid usb storage didn't be mount, but we still 
	//  can find file exist on /mnt/usbx folder.
	sprintf(szBuf,"rm -rf %s/%s > /dev/null 2>&1",mntpoint,tmpname);
	system(szBuf);

	//mount usb storage again
	sprintf(szBuf,"mount %s %s > /dev/null 2>&1",dev,mntpoint);
	system(szBuf);

	sprintf(szBuf,"%s/%s",fileinfo.dest,fileinfo.fname);
	if(access(szBuf,F_OK))
		res=FUN_FAIL;
	else
		res=FUN_SUCC;

	sprintf(szBuf,"rm -rf /tmp/%s > /dev/null 2>&1",tmpname);
	system(szBuf);
	sprintf(szBuf,"rm -rf %s/%s > /dev/null 2>&1",mntpoint,tmpname);
	system(szBuf);

	//umount usb storage
	sprintf(szBuf,"umount %s > /dev/null 2>&1",mntpoint);
	system(szBuf);

	return res;
}

/*************************************************************************************
	Search all mounted device if exist verify file
	
	dest_folder: Destination folder mount all devices already
	    vfname:  file name for verify, ex. vf_sata1,vf_usb1
*************************************************************************************/
int fun_MountStorage_002(char *dest_folder,char *vfname)
{
	int res = RESULT_FAIL;

	res=FindFile(dest_folder,vfname);

	if(res==FUN_SUCC)
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

/*************************************************************************************
	This function will test file transmit from sources to destination.
*************************************************************************************/
int fun_FileTransmit_001(PFILE_TX pInfo)
{
	char szSrc[256];
	char szDest[256];
	char szCMD[1024];
	FILE *file;
	int  res=RESULT_PASS;

	if(access(pInfo->dest,F_OK))
	{
		sprintf(szCMD,"mkdir %s > /dev/null 2>&1",pInfo->dest);
	}

	sprintf(szSrc,"%s/%s",pInfo->src,pInfo->fname);
	sprintf(szDest,"%s/%s",pInfo->dest,pInfo->fname);
	if(access(szDest,F_OK))
	{
		sprintf(szCMD,"rm -rf %s > /dev/null 2>&1",szDest);
		system(szCMD);
	}

	sprintf(szCMD,"cp %s %s > /dev/null 2>&1",szSrc,szDest);
	system(szCMD);
	//check file if exist
	if(access(szDest,F_OK))
	{
		res=RESULT_FAIL;
	}
	
	return res;
}


/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             VGA Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/
 
int fun_TestVGA_001()
{
	int i,count=8;

	for(i=0;i<count;i++)
		printf("\33[1;32;41m                                                                                                         \33[0m\r\n");

	for(i=0;i<count;i++)
		printf("\33[1;32;44m                                                                                                         \33[0m\r\n");

	for(i=0;i<count;i++)
		printf("\33[1;32;47m                                                                                                         \33[0m\r\n");
		
	return RESULT_PASS;
}

int fun_TestVGA_002(char *dev)
{
 	int fbfd = 0,y_field=0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;
    int i, x = 0, y = 0;
    long int location = 0;

    // Open the file for reading and writing
    //fbfd = open("/dev/fb0", O_RDWR);
    fbfd = open(dev, O_RDWR);
    
    if (!fbfd) 
    {
        DEBUGMSG("Error: cannot open framebuffer device.\r\n");
        return 1;
    }
    //DEBUGMSG("The framebuffer device was opened successfully.\r\n");

    // get fix attrib
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) 
    {
        DEBUGMSG("Error reading fixed information.\r\n");
        return 1;
    }

    // get var attrib
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) 
    {
        DEBUGMSG("Error reading variable information.\r\n");
        return 1;
    }
    //DEBUGMSG("%dx%d, %dbpp\n", vinfo.xres_virtual, vinfo.yres_virtual, vinfo.bits_per_pixel);
	//Calc size for whole screen(Unit:byte),used for mapping to userspace
	//screensize = (vinfo.xres) * (vinfo.yres) * vinfo.bits_per_pixel / 8;
	screensize = (vinfo.xres_virtual) * (vinfo.yres_virtual) * vinfo.bits_per_pixel / 8;
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                       fbfd, 0);
    if ((int)fbp == -1) 
    {
        DEBUGMSG("Error: failed to map framebuffer device to memory.\r\n");
        return 1;
    }
    //DEBUGMSG("The framebuffer device was mapped to memory successfully.\r\n");
	//y_field = (vinfo.yres/3);
	//y_field = (vinfo.yres_virtual/3-3);
	y_field = (vinfo.yres_virtual/3-10);	//sib8500
	for(i = 0; i<100 ;i++)
	{
	    for (y = 0; y < y_field; y++)
	    {
			//for (x = 0; x < vinfo.xres; x++)
			for (x = 0; x < vinfo.xres_virtual; x++)
	        {

	            location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
	                       (y+vinfo.yoffset) * finfo.line_length;

	            if (vinfo.bits_per_pixel == 32)
	            {
	                	*(fbp + location) = 0xFF;
	                	*(fbp + location + 1) = 0;
	                	*(fbp + location + 2) = 0;
	                	*(fbp + location + 3) = 0;      // No transparency
	            }
			}
	    }
		
	    for (y = y_field; y < 2*y_field; y++)
	    {
			//for (x = 0; x < vinfo.xres; x++)
			for (x = 0; x < vinfo.xres_virtual; x++)
	        {
	            location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
	                       (y+vinfo.yoffset) * finfo.line_length;

	            if (vinfo.bits_per_pixel == 32) 
	        	{
	                *(fbp + location) = 0;
	                *(fbp + location + 1) = 0xFF;
	                *(fbp + location + 2) = 0;
	                *(fbp + location + 3) = 0;      // No transparency
	            }
	        }
	    }

	    for (y = 2*y_field; y < 3*y_field; y++)
	    {
			//for (x = 0; x < vinfo.xres; x++)
			for (x = 0; x < vinfo.xres_virtual; x++)
	        {
				location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
	                       (y+vinfo.yoffset) * finfo.line_length;

				if (vinfo.bits_per_pixel == 32) 
	            {
	               		*(fbp + location) = 0;
	                	*(fbp + location + 1) = 0;
	                	*(fbp + location + 2) = 0xFF;
	                	*(fbp + location + 3) = 0;      // No transparency
				}
				
	        }
	    }
	}

    munmap(fbp, screensize);
    close(fbfd);
    
    return 0;
}

/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             UART Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/

//loopback test
int fun_TestUART_001(char *dev)
{
	if(FUN_SUCC == uart_loopback_Test(dev))
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

//test each other
int fun_TestUART_002(char *dev1,char *dev2)
{
	if(FUN_SUCC == uart_each_other_Test(dev1,dev2))
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

int fun_TestUART_003(unsigned int port)
{
	if(FUN_SUCC == uart_loopback_directio(port))
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

// DK - loopback test - use for uart_loopback_Test_raw()
int fun_TestUART_004(char *dev)
{
	if(FUN_SUCC == uart_loopback_Test_raw(dev))
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             Power Voltage Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/
int check_voltage_range(float std,float cur,float range/*percentage*/)
{
	float up_range,down_range;

	up_range = (float)(std*(1+(range/100)));
	down_range = (float)(std*(1-(range/100)));

	if( (cur>up_range) || (cur<down_range) )
		return RESULT_FAIL;

	return RESULT_PASS;
}

/*
   Vcore: 10% error range
   other: 5% error range
*/
int fun_TestVoltage_001()
{
	SENSOR_INFO info;

	get_sensorinfo(&info);

	if(check_voltage_range(2.5,info.p2_5v,5.0))
		return RESULT_FAIL;
	if(check_voltage_range(1.2,info.vcore,10.0))
		return RESULT_FAIL;
	if(check_voltage_range(3.3,info.p3_3v,5.0))
		return RESULT_FAIL;
	if(check_voltage_range(5.0,info.p5v,5.0))
		return RESULT_FAIL;
	if(check_voltage_range(12.0,info.p12v,5.0))
		return RESULT_FAIL;
	if(check_voltage_range(3.3,info.vcc,5.0))
		return RESULT_FAIL;
	if(check_voltage_range(1.5,info.p1_5v,5.0))
		return RESULT_FAIL;
	if(check_voltage_range(1.8,info.p1_8v,5.0))
		return RESULT_FAIL;

	return RESULT_PASS;
}

/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             Fan Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/
int fun_TestFan_001(int fan_num,int rpm)
{
	SENSOR_INFO info;
	int			fanspeed;

	get_sensorinfo(&info);

	if(fan_num==1)
	{
	  fanspeed=info.fan1;
	}
	else if(fan_num==2)
	{
	  fanspeed=info.fan2;
	}

	if(fanspeed>rpm)
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}

int fun_TestFan_002()
{
	SENSOR_INFO info;
	int			fanspeed;

	get_sensorinfo(&info);
#if 0
	if(fan_num==1)
	{
	  fanspeed=info.fan1;
	}
	else if(fan_num==2)
	{
	  fanspeed=info.fan2;
	}
#endif
	printf("info.fan1 = %d, info.fan2 = %d\r\n",info.fan1, info.fan2);
	if( ((info.fan1 - info.fan2)>500) || ((info.fan2 - info.fan1)>500))
		return RESULT_PASS;
	else
		return RESULT_FAIL;
}


/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             Clock Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/

/*************************************************************************************
	The test case use timer to test clock and 
	will test the RTC and System Time by UTC
*************************************************************************************/
int fun_TestClock_001()
{
	STRUCT_TEST_TIME	rtc;
    time_t timep;
    struct tm *p;
	
	//get rtc
	get_RTC(&rtc,TYPE_PC_TIME);
	
    DEBUGMSG("RTC date/time(Local): %d/%d/%d %02d:%02d:%02d\r\n",
            rtc.day, rtc.mon + 1, rtc.year + 1900,
            rtc.hour, rtc.min, rtc.sec);		

    time(&timep);        
    p = gmtime(&timep);	
    DEBUGMSG("OS date/time(Local): %d/%d/%d %02d:%02d:%02d\r\n",
            p->tm_mday, p->tm_mon + 1, p->tm_year + 1900,
            p->tm_hour, p->tm_min, p->tm_sec);

	if(rtc.sec != p->tm_sec)
		return RESULT_FAIL;
	if(rtc.min != p->tm_min)
		return RESULT_FAIL;
	if(rtc.hour != p->tm_hour)
		return RESULT_FAIL;
	if(rtc.day != p->tm_mday)
		return RESULT_FAIL;
	if(rtc.mon != p->tm_mon)
		return RESULT_FAIL;
	if(rtc.year != p->tm_year)
		return RESULT_FAIL;

	return RESULT_PASS;	
}



/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             GPC Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             SRAM Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/

/*************************************************************************************
	fill pattern 0x00,0xFF,0x55,0xAA to all areas and checked
*************************************************************************************/
/*
#define KBYTES	1024
int fun_GPCTestSRAM_001(const int bank,const int ksize)
{
	int i,j,res = FUN_SUCC;
	DSGPC_SRAM  sram;
	U08 buffer_w[KBYTES],buffer_r[KBYTES];

	FillSRAM(bank,ksize,0);

    //fill in 1K buffer to pattern
	for(i=0;i<KBYTES;i+=4)
	{
		buffer_w[i+0]=0x00;
		buffer_w[i+1]=0xFF;
		buffer_w[i+2]=0x55;
		buffer_w[i+3]=0xAA;
	}

	//switch bank
	sram.u08_bank = bank;
    sram.u32_subtarget = SRAM_BANK_SELECT;
    //res = GPC_CTLCODE(GPC_SRAM,ACTION_SET,&sram);
	if(GPC_FUN_RES_SUCC != GPC_CTLCODE(GPC_SRAM,ACTION_SET,&sram))
	{
		DEBUGMSG("fun_GPCTestSRAM_001 switch bank fail!\r\n");
		res = FUN_FAIL;
		goto END;
	}

    //fill in pattern
    for(i=0;i<ksize;i++)
    {
    	if((bank==0)&&(i==0)) //gpc reserve first 1k
    		continue;
    
		sram.u32_subtarget = SRAM_RW;
		sram.u32_offset = i*KBYTES;
		sram.u32_size = KBYTES;
		sram.pu08_data = buffer_w;
		if(GPC_FUN_RES_SUCC != GPC_CTLCODE(GPC_SRAM,ACTION_SET,&sram))
		{
			DEBUGMSG("fun_GPCTestSRAM_001 write pattern fail!\r\n");
			res = FUN_FAIL;
			goto END;
		}
	}
	//check all data of bank
    for(i=0;i<ksize;i++)
    {
    	if((bank==0)&&(i==0)) //gpc reserve first 1k
    		continue;
    		
    	memset(buffer_r,0,KBYTES);
		sram.u32_subtarget = SRAM_RW;
		sram.u32_offset = i*KBYTES;
		sram.u32_size = KBYTES;
		sram.pu08_data = buffer_r;
		if(GPC_FUN_RES_SUCC != GPC_CTLCODE(GPC_SRAM,ACTION_GET,&sram))
		{
			DEBUGMSG("fun_GPCTestSRAM_001 read pattern fail!\r\n");		
			res = FUN_FAIL;
			goto END;
		}
	    //check data
		for(j=0;j<KBYTES;j+=4)
		{
			if( (buffer_r[j+0]!=0x00) ||
			    (buffer_r[j+1]!=0xFF) ||
				(buffer_r[j+2]!=0x55) ||
				(buffer_r[j+3]!=0xAA) )
			{
				DEBUGMSG("fun_GPCTestSRAM_001 check pattern fail!\r\n");
				DEBUGMSG("data = [%02X %02X %02X %02X]\r\n",buffer_r[j+0],buffer_r[j+1],buffer_r[j+2],buffer_r[j+3]);
				res = FUN_FAIL;
				goto END;
			}
		}			
	}

END:	
	DEBUGMSG("fun_GPCTestSRAM_001, res=%d\r\n",res);
	return res;
}
*/

/*************************************************************************************
	��SRAM address bus �C���ܴ��@��bit �洫���y �p�U�ҥ�
	���] address bus is 20 bit 
	1. Clear all SRAM data to 00 or FF 
	2. Insert data = 55 @ Address = 0000,0000,0000,000,0001 
	3. Scan data @ address = 0000,0000,0000,0000,0010, 
	                         0000,0000,0000,0000,0100
	                         0000,0000,0000,0000,1000 ~ 1000,0000,0000,0000,0000 ���Ҧ�data �@20�� 
	4. �T�{data ���F�]�w��}��J55�~ ��l���o��55 
	5. Clear data @ Address = 0000,0000,0000,000,0001
	6. �baddress �u �@���洫1 bit �� LSB �� MSB �� 20^20 �� �Y�i���� address �u���˥X
*************************************************************************************/
/*
#define VERIFY_VAL	0x55
int fun_GPCTestSRAM_002(const int bank_num,const int ksize,const int bits)
{
	int i,j,res = FUN_SUCC;
	DSGPC_SRAM  sram;
	U32			addr=0;
	U08			buffer;

	for(i=0;i<bank_num;i++)
	{
		if(FUN_FAIL == FillSRAM(i,ksize,0))
		{
			res = FUN_FAIL;		
			goto END;
		}
	}

	for(i=0;i<bits;i++)
	{
		addr=1<<i;
		buffer=VERIFY_VAL;
		
		sram.u32_subtarget = SRAM_RW;
		sram.u32_offset = addr;
		sram.u32_size = 1;
		sram.pu08_data = &buffer;
		if(GPC_FUN_RES_SUCC != GPC_CTLCODE(GPC_SRAM,ACTION_SET,&sram))
		{
			res = FUN_FAIL;
			goto END;
		}

		for(j=0;j<bits;j++)
		{
			if(j==i)
				continue;

			//Comparing VERIFY_VAL with others address;
			buffer=0;
			addr=1<<j;
			sram.u32_subtarget = SRAM_RW;
			sram.u32_offset = addr;
			sram.u32_size = 1;
			sram.pu08_data = &buffer;
			if(GPC_FUN_RES_SUCC != GPC_CTLCODE(GPC_SRAM,ACTION_GET,&sram))
			{
				res = FUN_FAIL;
				goto END;
			}
			if(VERIFY_VAL == buffer)
			{
				res = FUN_FAIL;
				goto END;
			}

			//recover data to 0
			buffer=0;
			addr=1<<i;
			sram.u32_subtarget = SRAM_RW;
			sram.u32_offset = addr;
			sram.u32_size = 1;
			sram.pu08_data = &buffer;
			if(GPC_FUN_RES_SUCC != GPC_CTLCODE(GPC_SRAM,ACTION_SET,&sram))
			{
				res = FUN_FAIL;
				goto END;
			}
		}
	}
	
END:
	return res;
}
*/

/*------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
                             PCI Test Items
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*
 *------------------------------------------------------------------------------------*/

/*************************************************************************************
  Using shell command 'lspci' to find out the PCI device 
     that specify by argument 'vid' and 'pid'
  
  argument:
	vid: vender id for PIC device
	pid: produce id for PIC device
*************************************************************************************/
int fun_TestPCI_001(char *vid,char *pid)
{
	int res=RESULT_FAIL;
	char szID[64],szBuf[1024];
	FILE *fp;
	char *p;

	sprintf(szID,"[%s:%s]",vid,pid);
	sprintf(szBuf,"lspci -nn -d %s:%s 2>&1");
	fp = popen(szBuf,"r");
	fgets(szBuf,1024,fp);
	pclose(fp);

	p=NULL;
	p=strstr(szBuf,szID);
	
	if(p)
		res=RESULT_PASS;
	else
		res=RESULT_FAIL;
		
	return res;
}

