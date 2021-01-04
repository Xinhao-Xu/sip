#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

//#define __DEBUGMSG__

#ifdef __DEBUGMSG__

	#define DEBUGMSG(format,args...) printf(format, ## args)

#else

	#define DEBUGMSG(format,args...)
	
#endif

#endif
