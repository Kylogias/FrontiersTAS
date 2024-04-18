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

	tasTimestamp = scriptTimestamp("fpsling.fts");
	scriptParse("fpsling.fts", &tasInputs, &tasInputCount);
}

bool tasIsReady(void) {
	TimeValue curTime = getMonoTime();

	return (getDelta(curTime, lastTime) > 0.01);
}

DWORD tasTick(DWORD dwUserIndex, XINPUT_STATE* p) {
	TimeValue curTime = getMonoTime();
	printf("Delta: %f\n", getDelta(curTime, lastTime));
	lastTime = curTime;

	if (!tasRunning) {
		DWORD ret = ogGetState(dwUserIndex, p);
		curPacket = p->dwPacketNumber;
		p->dwPacketNumber = (DWORD)(curPacket + packetOffset);

		if (p->Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) tasRunning = true;

		i64 newTimestamp = scriptTimestamp("fpsling.fts");
		if (newTimestamp != tasTimestamp) {
			free(tasInputs);
			scriptParse("fpsling.fts", &tasInputs, &tasInputCount);
			tasTimestamp = newTimestamp;
		}

		return ret;
	}

	dwUserIndex = dwUserIndex + 1;
	p->dwPacketNumber = (DWORD)(curPacket + packetOffset);

	p->Gamepad = tasInputs[tasCurInput];
	tasCurInput += 1;
	if (tasCurInput == tasInputCount) {
		tasRunning = false;
		tasCurInput = 0;
	}

	return ERROR_SUCCESS;
}
