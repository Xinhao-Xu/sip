#Compiler
CC = gcc

PRJ_SOBJ	=	${PRJSOURCES:.c=.so}
PRJ_OBJS	=	${PRJSOURCES:.c=.o}
PRJ_OBJS_M	=	${PRJSOURCES:.c=.mo}
PRJ_OBJS_N	=	${PRJSOURCES:.c=.no}
PRJ_OBJS_O	=	${PRJSOURCES:.c=.oo}

.SUFFIXES:
.SUFFIXES: .c .o .so .mo .no .oo

.c.so:
	$(CC) -fPIC -s -c $< -o $@
.c.o:
	$(CC) $(CFLAGS) $(INCLUDE) -c -w $< -o $@
#for keyin
.c.mo:
	$(CC) -c -O2 -w -static -DHAVE_NCURSES -I/usr/include/ncursesw -D__BARCODE_KEYIN__ -D__SERVER_V1__ $(INCLUDE) -c $< -o $@
#for no_server
.c.no:
	$(CC) -c -O2 -w -static -DHAVE_NCURSES -I/usr/include/ncursesw $(INCLUDE) -c $< -o $@
#for keyin_no_server
.c.oo:
	$(CC) -c -O2 -w -static -DHAVE_NCURSES -I/usr/include/ncursesw -D__BARCODE_KEYIN__ $(INCLUDE) -c $< -o $@

#Options for development
CFLAGD = -g -c -O2 -static

#Option for release
CFLAGR = -c -O2 -static

#Library function
AR = ar

#Library Options
ARFLAGS = q

SYSDIRS=`ls -d * | grep -v 'obj' | grep -v 'bin' | grep -v 'lib' | grep -v 'include' | grep -v '_svn'`
COMMDIRS=`ls -d ../../../common/* | grep -v 'obj' | grep -v 'bin' | grep -v 'lib' | grep -v 'include' | grep -v '_svn'`

#SUBDIRS=dir1 dir2 dir3

define make_subdir
 @for subdir in $(SYSDIRS) ; do \
 ( cd $$subdir && make $1) \
 done;
endef 

#-----------------------------------------------------------

#All of Source files

VERSION = 0.1
AP_NAME = diag_sib600p
AP_NAME_KEYIN = diag_sib600p_keyin 
AP_NO_SERVER = diag_sib600p_no_server
AP_KEYIN_NO_SERVER = diag_sib600p_keyin_no_server

#FLAG for chose version
CFLAGS+= $(CFLAGR)
//CFLAGS+= $(CFLAGD)

#if barcode is input by keyboard
#CFLAGS+= -D__BARCODE_KEYIN__

#Use record server or not
CFLAGS+= -D__SERVER_V1__

CFLAGS+=-DHAVE_NCURSES -I/usr/include/ncursesw


INCLUDE = -I. -I../../../dialog -I../librarys -I../testboard_lib_io -I../../../common -I../../../common/iniparser -I../../../common/sqlite
#LIBS = ../../../dialog/libdialog.a ../librarys/gpclib.a ../librarys/libeit.a ../librarys/extmode.a -lm -lncursesw -lpthread -ljpeg -lsqlite3 -ldl
#we use sqlite3.a to replace sqlite3.o
LIBS = ../../../dialog/libdialog.a ../librarys/gpclib.a ../librarys/libeit.a ../../../common/sqlite/sqlite3.a -lm -lncursesw -lpthread -ljpeg -lsqlite3 -ldl


PRJSOURCES = $(shell find $(COMMDIRS) -name '*.c') $(shell find $(SYSDIRS) -name '*.c')

#-----------------------------------------------------------

all:clean $(AP_NAME)

clean:
	rm -rf $(AP_NAME) $(AP_NO_SERVER) $(AP_NAME_KEYIN) $(AP_KEYIN_NO_SERVER) $(PRJ_OBJS) $(PRJ_OBJS_M) $(PRJ_OBJS_N) $(PRJ_OBJS_O)

clean_o:
	rm -rf $(PRJ_OBJS) $(PRJ_OBJS_M) $(PRJ_OBJS_N) $(PRJ_OBJS_O)

full:clean_o $(AP_NAME) $(AP_NO_SERVER) $(AP_NAME_KEYIN) $(AP_KEYIN_NO_SERVER)

fast: $(AP_NO_SERVER)

test:CFLAGS = -g -c -O2 -w -static -D__DEBUGMSG__ -D__TEST_FUN__ -DHAVE_NCURSES -I/usr/include/ncursesw
test:clean $(PRJ_OBJS)
#test:$(PRJ_OBJS)
	$(CC) -o test  $(PRJ_OBJS) $(LIBS)

debugmsg:CFLAGS = -c -O2 -static -D__DEBUGMSG__ -DHAVE_NCURSES -I/usr/include/ncursesw
debugmsg:clean $(PRJ_OBJS)
	$(CC) -o debugmsg  $(PRJ_OBJS) $(LIBS)

$(AP_NAME):$(PRJ_OBJS)
	$(CC) -o $(AP_NAME) $(PRJ_OBJS) $(LIBS)
#$(AP_NAME):$(PRJ_OBJS)
#	$(CC) -o $(AP_NAME) ${shell find *.o} $(LIBS)

$(AP_NAME_KEYIN):$(PRJ_OBJS_M)
	$(CC) -o $(AP_NAME_KEYIN) $(PRJ_OBJS_M) $(LIBS)

$(AP_NO_SERVER):$(PRJ_OBJS_N)
	$(CC) -o $(AP_NO_SERVER) $(PRJ_OBJS_N) $(LIBS)

$(AP_KEYIN_NO_SERVER):$(PRJ_OBJS_O)
	$(CC) -o $(AP_KEYIN_NO_SERVER) $(PRJ_OBJS_O) $(LIBS)


