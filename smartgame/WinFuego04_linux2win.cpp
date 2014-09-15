#include <windows.h>
#include <time.h>

#include "WinFuego04_linux2win.h"	


int sysconf(int a)
{
#ifdef _DEBUG
	if (a != _SC_CLK_TCK) throw (20);
#endif

	return (100);	// ticks per second
}


union WinFT
{
	FILETIME lohi;
	__int64	 lft;
};


int times(tms * pt)
{
	WinFT creaT, exitT, kernT, userT;

	pt->tms_cstime = 0;
	pt->tms_cutime = 0;								// Times for child processes

	GetProcessTimes(GetCurrentProcess(), &creaT.lohi, &exitT.lohi, &kernT.lohi, &userT.lohi);	// 100-nanosecond time units

	pt->tms_stime = (clock_t) (kernT.lft / 100000);	// Times in 10 microsecond seconds
	pt->tms_utime = (clock_t) (userT.lft / 100000);

	return ((clock_t) (creaT.lft / 100000));
}


int gettimeofday(timeval * tv, int zero)
{
#ifdef _DEBUG
	if (zero != 0) throw (20);
#endif

	SYSTEMTIME sysT;

	GetLocalTime(&sysT);

	tv->tv_sec  = (sysT.wHour*60 + sysT.wMinute)*60 + sysT.wSecond;
	tv->tv_usec = sysT.wMilliseconds*1000;

	return (0);
}


pid_t getpid(void)
{
	return (GetCurrentProcessId());
}
