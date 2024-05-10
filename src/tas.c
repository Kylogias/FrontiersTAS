#include "tas.h"

#include <windows.h>
#include <stdio.h>

#include "ui/ui_main.h"

#include "util.h"

#include "script.h"

TimeValue lastTime;

u64 curPacket = 0;
u64 packetOffset = 0;

bool tasRunning = false;
XINPUT_GAMEPAD* tasInputs = NULL;
u32 tasInputCount = 0;
u32 tasCurInput = 0;
i64 tasTimestamp = 0;

void* he2BaseAddress = NULL;

// Code from autosplitter
bool tasCheckLoad(void) {
	intptr_t _base = (intptr_t)(*(u64*)he2BaseAddress);
	if (_base != 0) {
		intptr_t APPLICATION = (intptr_t)(*(u64*)(_base + 0x80));
		if (APPLICATION != 0) {
//			printf("APPLICATION\n");
			u8 ApplicationSequenceCount = *(u8*)(_base + 0x80 + 0x8);
			if (ApplicationSequenceCount > 0 && ApplicationSequenceCount < 8) {
				u8 array[64] = {0};
				memcpy(array, (void*)APPLICATION, ApplicationSequenceCount*8);
				u64 ASERTTI = (u64)rttiFind("ApplicationSequenceExtension@game@app@@");
				intptr_t ASE = 0;
				for (u8 i = 0; i < ApplicationSequenceCount; i++) {
					intptr_t InstanceAddress = (intptr_t)(array + i*8);
//					printf("InstanceAddress[%i]: %p ASERTTI: %p\n", i, **(u64**)InstanceAddress, ASERTTI);
					if ((**(u64**)(InstanceAddress)) == ASERTTI) {
						ASE = *(intptr_t*)InstanceAddress;
					}
				}
				if (ASE != 0) {
//					printf("ASE\n");
					uiLevelID = (char*)ASE + 0xA0;
					intptr_t GAMEMODE = (intptr_t)(*(u64*)(ASE + 0x78));
					if (GAMEMODE == 0) return true;
					intptr_t GME = (intptr_t)(*(u64*)(GAMEMODE + 0xB0));
					if (GME != 0) {
//						printf("GME\n");
//						printf("LVLID 2: %s\n", GAMEMODE + 0x100);
						u8 GMECount = *(u8*)(GAMEMODE + 0xB0 + 0x8);
						printf("GMECount: %i\n", GMECount);
					} else return true;
				}
			}
		}
	}
	return false;
}

void tasInit(void) {
	uiInit();
	scriptInit();
	lastTime = getMonoTime();

	// Get the timestamp for reloading and parse it into the input array
	tasTimestamp = scriptTimestamp("fpsling.fts");
	scriptParse("fpsling.fts", &tasInputs, &tasInputCount);

	u64 memSize = exeInfo.SizeOfImage;
	printf("Num Sigs: %i\n", countSigs((u8[]){0xe8, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x8b, 0x78, 0x70}, (u8*)"X????XXXX", 9, exeAddr, memSize, 1));
	he2BaseAddress = (u8*)sigScan((u8[]){0xe8, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x8b, 0x78, 0x70}, (u8*)"X????XXXX", 9, exeAddr, memSize, 1) + 1;
	if (he2BaseAddress == NULL) {
		printf("Failed to find HE2 base address\n");
	}
	intptr_t tempAddr = (intptr_t)he2BaseAddress + *((i32*)he2BaseAddress) + 0x4 + 0x3;
	he2BaseAddress = (u8*)tempAddr + *((i32*)tempAddr) + 0x4;

	rttiInit();
	rttiResolve("ApplicationSequenceExtension@game@app@@");
}

// Just check if the time since last tick is more than 1/100 of a second.
// The game will not request inputs unless it is close to a new frame
bool tasIsReady(void) {
	TimeValue curTime = getMonoTime();

	return (getDelta(curTime, lastTime) > 0.01) && (!tasCheckLoad());
}

DWORD tasTick(DWORD dwUserIndex, XINPUT_STATE* p) {

	// Print time since last tick
	TimeValue curTime = getMonoTime();
	uiDelta = getDelta(curTime, lastTime);
	lastTime = curTime;

	uiTick();

	// Allow the user to use the gamepad if the TAS isn't using it
	if (!tasRunning) {
		// Get the state of the gamepad (and ensure the packet number is correct)
		DWORD ret = ogGetState(dwUserIndex, p);
		curPacket = p->dwPacketNumber;
		p->dwPacketNumber = (DWORD)(curPacket + packetOffset);

		// Code to switch on the TAS. Currently pressing the left stick will activate it
		if (p->Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) {
			tasRunning = true;
		}

		// Reloading code. If the timestamp changes, reparse the file
		i64 newTimestamp = scriptTimestamp("fpsling.fts");
		if (newTimestamp != tasTimestamp) {
			free(tasInputs);
			scriptParse("fpsling.fts", &tasInputs, &tasInputCount);
			tasTimestamp = newTimestamp;
		}

		return ret;
	}

	// Warning suppresion
	dwUserIndex = dwUserIndex + 1;

	// Set the packet number
	packetOffset += 1;
	p->dwPacketNumber = (DWORD)(curPacket + packetOffset);

	// Loop through the inputs and return them
	p->Gamepad = tasInputs[tasCurInput];
	tasCurInput += 1;

	// End the TAS if we finished the last input
	if (tasCurInput == tasInputCount) {
		tasRunning = false;
		tasCurInput = 0;
	}

	return ERROR_SUCCESS;
}
