#include <windows.h>
#include <time.h>


// Const

#define _SC_CLK_TCK 0


// Types

typedef int pid_t;

struct tms {
	clock_t tms_utime;
	clock_t tms_stime;
	clock_t tms_cutime;
	clock_t tms_cstime;
};


// Functions 

int	  sysconf	   (int a);
int	  times		   (tms * pt);
int	  gettimeofday (timeval * tv, int zero);
pid_t getpid	   (void);

