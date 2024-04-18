#include "tas.h"

#include <windows.h>
#include <stdio.h>

#include "util.h"

TimeValue lastTime;

u64 curPacket = 0;
bool pressA = false;

void tasInit(void) {
	lastTime = getMonoTime();
}

bool tasIsReady(void) {
	TimeValue curTime = getMonoTime();

	return (getDelta(curTime, lastTime) > 0.01);
}

DWORD tasTick(DWORD dwUserIndex, XINPUT_STATE* p) {
	dwUserIndex = dwUserIndex + 1;
	
	TimeValue curTime = getMonoTime();
	printf("Delta: %f\n", getDelta(curTime, lastTime));
	lastTime = curTime;
	//ogGetState(dwUserIndex, pState);

	curPacket += 1;
	p->Gamepad.bLeftTrigger = 0;
	p->Gamepad.bRightTrigger = 0;
	p->Gamepad.sThumbLX = 0;
	p->Gamepad.sThumbLY = 0;
	p->Gamepad.sThumbRX = 0;
	p->Gamepad.sThumbRY = 0;
	if (pressA) p->Gamepad.wButtons = XINPUT_GAMEPAD_A;
	else        p->Gamepad.wButtons = 0;
	pressA = !pressA;

	return ERROR_SUCCESS;
}
