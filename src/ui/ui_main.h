#ifndef FTS_UIMAIN_H
#define FTS_UIMAIN_H

#include "../primdefs.h"

extern char* uiLevelID;
extern double uiDelta;

typedef union {
	struct {
		u8 b, g, r, a;
	};
	u32 c;
	u8 m[4];
} Color;

void uiInit(void);
void uiTick(void);

#endif
