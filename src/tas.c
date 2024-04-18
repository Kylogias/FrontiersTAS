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

void tasInit(void) {
	scriptInit();
	lastTime = getMonoTime();

	// Get the timestamp for reloading and parse it into the input array
	tasTimestamp = scriptTimestamp("fpsling.fts");
	scriptParse("fpsling.fts", &tasInputs, &tasInputCount);
}

// Just check if the time since last tick is more than 1/100 of a second.
// The game will not request inputs unless it is close to a new frame
bool tasIsReady(void) {
	TimeValue curTime = getMonoTime();

	return (getDelta(curTime, lastTime) > 0.01);
}

DWORD tasTick(DWORD dwUserIndex, XINPUT_STATE* p) {
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
		if (p->Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) tasRunning = true;

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
