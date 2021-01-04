#ifndef VERSION_H_INCLUDED
#define VERSION_H_INCLUDED

//**********************************************************
/*   History

O v0.0.1
¡E First Edition, Implement main architecture
¡E Implement function "fun_RunManuTest(int testid)". Used for run manual test with assigned Test ID. 
¡E Implement Test Board Library(only have architecture included)

O v0.0.2
¡E Implement test result record.
¡E Finish bar-code scan
¡E Remove Autotest.h and Manutest.h and remove all variables to Test_items.h
¡E modify some architecture for main
¡E Finish Auto test item: CPU
¡E Finish Manual test item: VGA
¡E Added struct "_tagTEST_ITEMS_INFO" for transmitting parameter to test function

O v0.0.3
¡E Remove the _tagRESULT.pDescript from Result.h , from now all test item only return RESULT_SUCC or RESULT_FAIL
¡E Finish Auto test item: File_TX, Memory
¡E Added function for get dmidecode information
¡E Added function "fun_TestCPU_002" and "fun_TestMemory_001'

O v0.1.0
¡E Change architecture
		Remove Autotest.c and Manutest.c replaced by TestPackage.c, the functions for each files are list below
		diag : main function to run the test process, also the place to define the test items.
		Test_items :  unit functions to provide ability to test something.
		TestPackage : The package(T101,T102¡K) of test items to provide diag to test
		Utils : To provide some utility to do something.
¡E Implemented all TestPackage (not all workable)

O V0.1.1
¡E Binding with GPC Library

O V0.2.0
¡E Binding with EIT Library
¡E Added SRAM test for testboard_lib

O V0.3.0 [Change architecture for output record]

O V0.4.0
¡E Split the test items to auto test and manual test items.
¡E Added test items fun_TestVGA_002() to test VGA by frame buffer.
¡E Modify Testboard_lib

O V0.5.0
¡E Modify : Change the test method for CMD_01~CMD_07 from use interrupt to use 

O V0.6.0
¡E Modify : Change the test method for CMD_01~CMD_07 from use polling to use interrupt
¡E Finish CMD_01 and CMD_02

O V0.7.0
¡E Shift all command by 1 and use CMD_00 for idle command.
¡E Finish : Cmd_01 ~ Cmd_08

O V0.8.0
¡E Added function 'output_FixedRecord' for output fixed test items result
¡E Bug Fix : fun_GPCTestSRAM_001 should not test first sector of bank0

O V0.8.1
¡E Finish CMD_01 ~ Cmd_09 for Testboard

O V0.9.0
¡E Modify Architecture : [Use TestTable.c for assigning test items]

O V0.10.0
¡E Implement jpeg show function

O V0.11.0
¡E Implement Super I/O function
¡E Show pictures for hits on test process.

O V0.12.0
¡E Added test items as following
		cctalk, sound, VccSRAM
¡E Fixed SIP without timeout.

O V0.13.0
¡E Added function "MAC Address Table"
¡E Added function "Detail information for Game I/O"
¡E Added Statistic tables
¡E Modify DRAM test item form 1G to 2G

O V1.0.0
¡EUpgrading the version from v0.13.0 to v1.0.0 

O V1.0.1
¡EModify the test method for RTC(system & eit)

O V1.0.2
¡E[Modify] remove the defination place of _TEST_FUN_ from source file to Makefile
¡E[Modify] Change the error range from 5% to 10% for test items(fun_T01009002,fun_T01009003,fun_T01009004,fun_T01009005)
¡E[Modify] Change the ping address from 192.168.137.1 to 192.168.5.1 for test item fun_T01006001

O V1.1.0
¡E[Added] Pure Gaming I/O support

O V1.1.1
¡E[Modify] Holding Gaming I/O test until dipswitch test finish. 
¡E[Modify] Change the test item T02018001 to use 'pureio'
¡E[Modify] Modify the function 'get_TestBoard_Result' from
           "for(i=CMD_01;i<=CMD_0D;i++)" to "for(i=0;i<CMD_TOTAL;i++)"
           for convenienting to return result when changing CMD_TOTAL number.
¡E[Modify] function "pureio" to output log for total counter of pass/fail times.
¡E[Added] Added usb0,1,2 test items

O V1.1.2
¡E[Modify] Enable test item T01004001(IDE0) and T01004002(IDE1)
¡E[Modify] fun_T01004001() and fun_T01004002() to test with randmon file

O V1.2.0
¡E[Changed] Changed the architecture of SIP to two folder [common] and [project]
¡E[Added] To support GMB6020(ODM/DLV) 

O V1.2.1
¡E[Changed] Change FixedTest.c to /common/FixedTest.c and /system/tcil.c(Test case implement layer)
            And implement the relationship of them.

O V1.2.2
¡E[Changed] Splitting diag.c to /common/diag.c and /system/main.c

O V1.3.0
¡E[Added] To support SIB7300

O V1.4.0
¡E[Added] To support SIB6310

O V1.5.0
¡E[Added] To support GMB6030
¡E[Added] Added 'Gaming test' and 'external test' architecture

O V1.5.1
¡E[Modify] To fit new common architecture for GMB6020

O V1.5.2
¡E[Modify] Removed and change some test method(ibutton,gaming I/O) for GMB6020

O V1.6.0
¡E[Added] To support GMB6730U

O V1.7.0
¡E[Added] To support GMB7030
¡E[Modify] /common/Utils.c
           1. to rearrange all sections for utility
           2. added section "String handle" to provide ability to handle string.
¡E[Added] inipaser to parse test argument from .ini file
¡E[Fixed] Bug fixed for g_testargu.dram_size
				  phenomenon: aftter setting value of g_testargu.dram_size in get_argument(),
				              It will be set to zero after exit function get_argument()
				  root cause: init_tcil() calling pthread_mutex_init(),after this will set
				              g_testargu.dram_size to zero
¡E[Fixed] Misjudgment of function fun_TestPort() in Tcil.c	
				  phenomenon: if use different output pin to test input bin it gets wrong test result.
				              ex. C1 -> R3
					root cause: the judgmental if((in_portdata&out_portbit)==0 
					            should replace with	if((in_portdata&in_portbit)==0
					            also the same with others.      
					            
O V1.8.0
¡E[Added] To support GMB6735
¡E[Modify] fun_TestMemory_001(PMEM_TEST pInfo)@/Common/Test_Items.c
           to test memory device's size if large than 1.9G     
¡E[Fixed] GMB6030
          1. To support inipaser
          2. adjust the locate of showing picture from (300,300) to (0,0) for function fun_check_itrstatus()
          3. adjust the locate of showing picture from (300,300) to (0,0) for function powerfailtest()

O V1.8.1
¡E[Added] Adding test item "smart card" for GMB6730U

O V1.8.2
¡E[Added] function "GetNetCardCount" in file "UTILs.c"
          where "GetNetCardCount" is used for getting numbers of Net Card.
¡E[Modify] function "getMacAddr" in file "UTILs.c"
					To get MacAddress even it didn't plug-in any cable.
					
O V1.8.3
¡E[Added] test item "T01012002(USB2)" for GMB6735

O V1.8.4
¡E[Fixed] Drawing way of Framebuffer for supporting unstandand screen resolution. 

O V1.9.0
¡E[Added] Supporting GMB6050. 

O V1.10.0
¡E[Added] Supporting GMB6795.
¡E[Added] Functions uart_loopback_directio() in util.c and 
          fun_TestUART_003() in Test_items() for testing UART directly.
          
O V1.11.0
¡E[Added] Supporting GMB120.
¡E[Modify] Adjusting the test method for SRAM

O V1.11.1
¡E[Modify] Adjusting the SRAM test method for GMB6735

O V1.12.0
¡E[Fixed] GMB6020:Fixed the test item fun_T02017001
¡E[Modify] GMB6020:Enable tradition mode when sip starting.
¡E[Modify] GMB6020: Added test item to test register offset + 0x70 bit 31
                    1:Fail, 0:Pass

O V1.13.4	20151008 - DK
¡E[Added] To support AGX30.	
¡E[Added] Create "Attah.h" file in separate project folder to support "FinRecord",
		  "OnBoard_Storage01", "RootSecu_Read", "UID_Record" function.
¡E[Modify][Test_items.c] fun_TestSnd_002: Modify the mplayer command.						
¡E[Added][FixedTest.h] fun_T02017031 to fun_T02017034, fun_T01015005, fun_T01017003,
                       fun_T02011002, fun_T02017035, to fun_T02017038.
¡E[Added][FixedTest.c] ITEM_MAP(T01015005)//OnBoard Storage1, ITEM_MAP(T01017003)//Read RootSecu				
					   ITEM_MAP(T02017005) to ITEM_MAP(T02017038).
¡E[Added][gmb6050][Environment] New hint pics for 3 pin dip switch test(800x600). 
¡E[Modify][gmb6050]Using new librarys for support UID relative functions.
¡E[Modify][diag.c]Supporting "FinRecord" function.
¡E[Modify][gmb6050][agx30][gmb6795]Allow tester use "CPR" relative command to copy record folder into specify usb device.

O V1.15.8 20151210 - DK
¡E[Added] To support gmb7070.
¡E[Added] To support gmb7735.
¡E[Modify][gmb6050] Change UID_record format.
¡E[Modify][Test_items.c] Modify fun_TestVGA_002 to support different multi monitor resolution mode.
¡E[Modify][diag.c][diag.h] Modify UID_record function.
¡E[Modify][showjpeg.c] Modify Function "showjpeg" for draw jpeg on multi monitor.

O V1.19.0 20160329 - DK
¡E[Added] To support gmb7055.
¡E[Added] To support glb7055.
¡E[Added] To support gcb881c.
¡E[Modify][showjpeg.c] Modify Function "showjpeg" to support draw on fb0 and fb1.
¡E[Modify][gmb6795] To support 4g DRAM test item.
¡E[Added] Implement GPC3800 test function.

O V1.19.1 20160513 - DK
¡E[Modify][glb7055] Modify test item "com5", "com6" to use function "directio_test_each_other".

O V1.22.2 20160615 - DK
¡E[Added][gmb6020] Added test function fun_T02017017 to read GPC I/O address for check GPC version code. Added record file "SVID" in "other_rec" folder.
¡E[Modify][gmb7055][glb7055][gcb881c] Change the test method to use ini file for indicate the fan_duty value. 
¡E[Added][gmb7055][glb7055][gcb881c] Create test progress without fan test item. USe it by keyin "2" in console.
¡E[Added][gmb6020] Added test function fun_TestMemory_003 at Attach.h for dram test.

O V1.23.2 20160706 - DK
¡E[Added] To support gmb6075.

O V1.24.2 20160815 - DK
¡E[Added] To support gmb7077.

O V1.24.3 20160817 - DK
¡E[Modify][gmb6030] Change Battery condition to 3900 mv.

O V1.24.4 20161116- DK
¡E[Modify][gmb7055] Change "FinRecord" Function in Attach.h to fit user request.




*/
//**********************************************************/ 

/*
    rule for version:
    1. added one to VER_EDITION for each time modify.
    2. added one to VER_SUB when each time added function.
    3. added one to VER_MAIN when each time chang the architecture.
*/

#define VER_MAIN			 8
#define VER_SUB         	15 
#define VER_EDITION			 1
#define VERSION					((VER_MAIN<<24)|(VER_SUB<<16)|(VER_EDITION))

#endif // VERSION_H_INCLUDED
