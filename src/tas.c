#include "tas.h"

#include <windows.h>
#include <stdio.h>

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

bool firstTime = true;

// Code from autosplitter
bool tasCheckLoad(void) {
	//findVTable("HELLOWORLD");
	firstTime = false;
	if (he2BaseAddress != NULL) {
		void* gameBase = *(void**)he2BaseAddress;
		void* app = *(u8**)((u64)gameBase + 0x80);
		if (app != NULL) {
			u8 count = *(u8*)((u64)he2BaseAddress + 0x88);
			printf("A COUNT: %i\n", count);
//			void* appRTTI = rttiFind("ApplicationSequenceExtension@game@app@@");
			void** appArr = *(void***)(app);

			// Highly doubt I can assume this is the correct ASE.
			// If you can fix the commented code, I'd appreciate it
			void* ASE = appArr[0];
//			for (u8 i = 0; i < count; i++) {
//				printf("SEQ (APP+i: %p APP[i]: %p RTTI: %p)\n", appArr+i, **((void******)appArr[i]), appRTTI);
//				if (((void**)app)[i] == appRTTI) {
//					ASE = (void**)app + i;
//					break;
//				}
//			}
			if (ASE != NULL) {
//				printf("ASE\n");
				void* gmode = *(void**)((u64)ASE + 0x78);
				void* GME = *(void**)((u64)gmode + 0xB0);
				if (GME != NULL) {
					u8 extCount = *(u8*)((u64)gmode + 0xB8);
					printf("G COUNT: %i\n", extCount);

					// According to the autosplitter, no game mode extensions means no loading (when not on the main menu)
					return extCount == 0;
				}
			}
		}
	}
	return false;
}

void tasInit(void) {
	scriptInit();
	lastTime = getMonoTime();

	// Get the timestamp for reloading and parse it into the input array
	tasTimestamp = scriptTimestamp("fpsling.fts");
	scriptParse("fpsling.fts", &tasInputs, &tasInputCount);

	u64 memSize = getMemorySize();
	he2BaseAddress = (u8*)sigScan((u8[]){0xe8, 0x00, 0x00, 0x00, 0x00, 0x4c, 0x8b, 0x78, 0x70}, (u8*)"X????XXXX", 9, exeAddr, memSize, 1) + 1;
	if (he2BaseAddress == NULL) {
		printf("Failed to find HE2 base address\n");
	}
	void* tempAddr = (u8*)he2BaseAddress + *((i32*)he2BaseAddress) + 0x4 + 0x3;
	he2BaseAddress = (u8*)tempAddr + *((i32*)tempAddr) + 0x4;

	rttiInit();
	rttiResolve("ApplicationSequenceExtension@game@app@@");
}

// Just check if the time since last tick is more than 1/100 of a second.
// The game will not request inputs unless it is close to a new frame
bool tasIsReady(void) {
	TimeValue curTime = getMonoTime();

	return (getDelta(curTime, lastTime) > 0.01); // && (!tasCheckLoad());
}

DWORD tasTick(DWORD dwUserIndex, XINPUT_STATE* p) {
	if (firstTime) tasCheckLoad();
	
	// Print time since last tick
	TimeValue curTime = getMonoTime();
	printf("Delta: %f\n", getDelta(curTime, lastTime));
	lastTime = curTime;

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
