#ifndef FTS_TAS_H
#define FTS_TAS_H

#include <xinput.h>
#include "util.h"

extern bool tasRunning;

void tasInit(void);
bool tasIsReady(void);
DWORD tasTick(DWORD dwUserIndex, XINPUT_STATE* p);

#endif
