#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED

#include "dialog.h"

#define SET_QUOTE(quote)	dialog_state.separate_str = quote;

#define POS_X 		dialog_vars.begin_x
#define POS_Y			dialog_vars.begin_y
#define RESULT		dialog_vars.input_result

#define SETPOS(x,y)         {POS_X=x;POS_Y=y;}
#define SETPOS_MAINWIN()	{POS_X=g_wpMainWin.posx;POS_Y=g_wpMainWin.posy;}
#define SETPOS_OUTPUTWIN()	{POS_X=g_wpOutputWin.posx;POS_Y=g_wpOutputWin.posy;}

#define SIZE_ARRAY(array)	(sizeof(array)/sizeof(array[0]))
#define ITEM_NUM_MENU(menu)	(sizeof(menu)/sizeof(char *)/2)
#define MENU_HEIGHT(wpMenu)	(wpMenu.height-6)
#define ITEM_NUM_FORM(form)	(sizeof(form)/sizeof(char *)/8)
#define ITEM_NUM_CHECKLIST(list)	(sizeof(list)/sizeof(char *)/3)

#define MAPCMDS(val)	 {val, #val }
#define MAPCMDS_GETVAL(cmds,str_idx) cmds[atoi(str_idx)].value
#define MAPCMDS_GETSTR(cmds,str_idx) cmds[atoi(str_idx)].str

#define NUM_WIDTH			50
#define NUM_HEIGHT			50

#define PATH_PICS			"./pics/"
#define PATH_800_600		"/800_600/"
#define PATH_NUM			"num/"

#define PIC_RES_WIDTH		800
#define PIC_RES_HEIGHT		600

#define PATH_NUM0			PATH_PICS PATH_NUM "0.jpg"
#define PATH_NUM1			PATH_PICS PATH_NUM "1.jpg"
#define PATH_NUM2			PATH_PICS PATH_NUM "2.jpg"
#define PATH_NUM3			PATH_PICS PATH_NUM "3.jpg"
#define PATH_NUM4			PATH_PICS PATH_NUM "4.jpg"
#define PATH_NUM5			PATH_PICS PATH_NUM "5.jpg"
#define PATH_NUM6			PATH_PICS PATH_NUM "6.jpg"
#define PATH_NUM7			PATH_PICS PATH_NUM "7.jpg"
#define PATH_NUM8			PATH_PICS PATH_NUM "8.jpg"
#define PATH_NUM9			PATH_PICS PATH_NUM "9.jpg"
#define NUM2PATH(n)			PATH_PICS PATH_NUM #n ".jpg"

#define PATH_RES_PASS		PATH_PICS PATH_800_600 "result_pass.jpg"
#define PATH_RES_FAIL		PATH_PICS PATH_800_600 "result_fail.jpg"
#define PATH_WAIT_RESULT    PATH_PICS PATH_800_600 "wait_result.jpg"
#define PATH_WAIT_RESULT1    PATH_PICS PATH_800_600 "wait_result1.jpg"

#define CLEAR()	system("clear")

typedef struct __tagWIN_POS
{
	unsigned char		posy;
	unsigned char		posx;
	unsigned char		height;
	unsigned char		width;
}WIN_POS;

#define OUTPUTFILE	"/tmp/output.out"
#define OUTFILE(format,args...) g_fpOutput = fopen(OUTPUTFILE,"w+");fprintf(g_fpOutput,format, ## args);fclose(g_fpOutput);
#define OUTPUT(format,args...) g_fpOutput = fopen(OUTPUTFILE,"w+");fprintf(g_fpOutput,format, ## args);fclose(g_fpOutput); PRINT_OUT();
#define PRINT_OUT()	SETPOS_OUTPUTWIN(); dialog_textbox("OUTPUT",OUTPUTFILE,g_wpOutputWin.height,g_wpOutputWin.width);SETPOS_MAINWIN();

static FILE 				*g_fpOutput = NULL;
static WIN_POS 			g_wpMainWin;
static WIN_POS 			g_wpOutputWin;

#else

extern static FILE 			*g_fpOutput;
extern static WIN_POS 			g_wpMainWin;
extern static WIN_POS 			g_wpOutputWin;
extern static DIAGINFO			g_diaginfo;
#endif
