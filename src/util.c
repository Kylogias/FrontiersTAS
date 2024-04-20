#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include "util.h"

i64 perfFreq = 0;

typedef struct {
	char* name;
	void* loc;
	bool resolved;
} VTableListEntry;

VTableListEntry* vtList = NULL;
u64 vtCount = 0;

void utilInit(void) {
	// The performance counter is how you get time in Windows. The frequency is how many times a second it ticks
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	perfFreq = freq.QuadPart;
}

TimeValue getMonoTime(void) {
	LARGE_INTEGER pc;
	TimeValue tv;

	// Query the counter then divide by the frequency	
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

u64 getMemorySize(void) {
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(exeProc, &pmc, sizeof(pmc));
	return pmc.PagefileUsage;
}

void* sigScan(u8* bytes, u8* mask, u32 numBytes, void* start, u64 length, u64 align) {
	if ((u64)start % align) {
		start = (u8*)start + ((u64)start % align);
		length = length - ((u64)start % align);
	}
	for (u32 i = 0; i < length - numBytes; i+=align) {
		u32 j = 0;
		for (; j < numBytes; j++) {
			u8 b = *(u8*)((u64)start+i+j);
			bool checkFail = false;
			switch (mask[j]) {
				case '?': // Don't Care
					break;
				case 'X': // eXact
					if (bytes[j] != b) checkFail = true; break;
				case '_': // Not
					if (bytes[j] == b) checkFail = true; break;
				case '<': // Less than
					if (b >= bytes[j]) checkFail = true; break;
				case '>': // Greater than
					if (b <= bytes[j]) checkFail = true; break;
				default:
					checkFail = true;
			}
			if (checkFail) break;
		}
		if (j == numBytes) {
			return (void*)((u64)start + i);
		}
	}
	
	return NULL;
}

u64 countSigs(u8* bytes, u8* mask, u32 numBytes, void* start, u64 length, u64 align) {
	if ((u64)start % align) {
		start = (u8*)start + ((u64)start % align);
		length = length - ((u64)start % align);
	}
	u64 curSigs = 0;
	for (i64 i = 0; i < (i64)length - numBytes; i+=align) {
		void* newAddr = sigScan(bytes, mask, numBytes, (u8*)start+i, length-(u64)i, align);
		if (newAddr == NULL) return curSigs;
		i = (u8*)newAddr - (u8*)start;
		curSigs += 1;
	}
	return curSigs;
}

#define RTTILIMIT exeInfo.SizeOfImage

// Steps taken from autosplitter
void rttiInit(void) {
//	u64 memSize = getMemorySize();
	u64 sigCount = countSigs((u8[]){0x2e, 0x3f, 0x41, 0x56}, (u8*)"XXXX", 4, exeAddr, RTTILIMIT, 8);
	vtList = malloc(sizeof(VTableListEntry)*sigCount);
	vtCount = sigCount;

	void* addr = exeAddr;
	printf("RTTI count: %llu\n", sigCount);
	for (u64 i = 0; i < sigCount; i++) {
		addr = sigScan((u8[]){0x2e, 0x3f, 0x41, 0x56}, (u8*)"XXXX", 4, addr, RTTILIMIT, 8);
		if (addr == NULL) break;
		vtList[i].name = (char*)addr + 4;

		// Warning when converting to i32, so perform the effective operation ourselves with bitwise AND
		vtList[i].loc = (void*)(((u8*)addr - (u8*)exeAddr - 0x10) & 0xFFFFFFFF);
		vtList[i].resolved = false;
		addr = (u8*)addr + 4;
	}
}

void rttiResolve(char* name) {
	for (u64 i = 0; i < vtCount; i++) {
		if (strcmp(name, vtList[i].name) == 0) {
			if (vtList[i].resolved) return;
			u64 temp = (u64)sigScan((u8*)(&vtList[i].loc), (u8*)"XXXX", 4, exeAddr, RTTILIMIT, 4) - 0xC;
			if (temp == (u64)NULL) return;
			vtList[i].loc = (u8*)sigScan((u8*)(&temp), (u8*)"XXXXXXXX", 8, exeAddr, RTTILIMIT, 8) + 0x8;
			if (vtList[i].loc == NULL) return;
			vtList[i].resolved = true;
			return;
		}
	}
}

void* rttiFind(char* name) {
	for (u64 i = 0; i < vtCount; i++) {
		if (strcmp(name, vtList[i].name) == 0) {
			if (vtList[i].resolved) return vtList[i].loc;
		}
	}
	return NULL;
}
