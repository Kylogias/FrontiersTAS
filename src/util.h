#ifndef FTS_UTIL_H
#define FTS_UTIL_H

#include "primdefs.h"

#include <windows.h>
#include <xinput.h>

typedef DWORD(WINAPI *GetStateFunc)(DWORD, XINPUT_STATE*);
typedef DWORD(WINAPI *SetStateFunc)(DWORD, XINPUT_VIBRATION*);

extern GetStateFunc ogGetState;
extern SetStateFunc ogSetState;
extern HMODULE ogDLL;
extern HMODULE curDLL;

// For TASing keyboard/mouse
extern HWND gameWnd;

extern HINSTANCE exeAddr;

typedef struct {
	i64 sec;
	i32 nano;
} TimeValue;

void utilInit(void);
TimeValue getMonoTime(void);
double getDelta(TimeValue cur, TimeValue last);

#endif
