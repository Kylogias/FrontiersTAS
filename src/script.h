#ifndef FTS_SCRIPT_H
#define FTS_SCRIPT_H

#include <xinput.h>

#include "util.h"

void scriptInit(void);
void scriptParse(char* path, XINPUT_GAMEPAD** inpArray, u32* inpCount);
i64 scriptTimestamp(char* path);

#endif
