#ifndef FTS_UTIL_H
#define FTS_UTIL_H

#include "primdefs.h"

#include <windows.h>
#include <psapi.h>
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
extern HANDLE exeProc;
extern MODULEINFO exeInfo;

typedef struct {
	i64 sec;
	i32 nano;
} TimeValue;

void utilInit(void);
TimeValue getMonoTime(void);
double getDelta(TimeValue cur, TimeValue last);

u64 getMemorySize(void);
void* sigScan(u8* bytes, u8* mask, u32 numBytes, void* start, u64 length, u64 align);
u64 countSigs(u8* bytes, u8* mask, u32 numBytes, void* start, u64 length, u64 align);

void rttiInit(void);
void rttiResolve(char* name);
void* rttiFind(char* name);

#endif
