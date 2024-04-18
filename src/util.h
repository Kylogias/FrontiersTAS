#ifndef FTS_UTIL_H
#define FTS_UTIL_H

#include "primdefs.h"

typedef struct {
	i64 sec;
	i32 nano;
} TimeValue;

void utilInit(void);
TimeValue getMonoTime(void);
double getDelta(TimeValue cur, TimeValue last);

#endif
