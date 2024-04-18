#include <windows.h>
#include "util.h"

i64 perfFreq = 0;

void utilInit(void) {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	perfFreq = freq.QuadPart;
}

TimeValue getMonoTime(void) {
	LARGE_INTEGER pc;
	TimeValue tv;
	
	QueryPerformanceCounter(&pc);
	i64 iPC = pc.QuadPart;
	tv.sec = iPC / perfFreq;
	tv.nano = (i32)((((double)iPC / (double)perfFreq)-(double)tv.sec)*1000000000);

	return tv;
}

double getDelta(TimeValue cur, TimeValue last) {
	i32 nano = cur.nano - last.nano;
	i64 sec = cur.sec - last.sec;
	if (nano < 0) {
		nano += 1000000000;
		sec -= 1;
	}

	return ((double)nano / 1000000000) + (double)sec;
}
