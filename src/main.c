#include <windows.h>
#include <xinput.h>

#include <stdio.h>
#include <time.h>

typedef DWORD(WINAPI *GetStateFunc)(DWORD, XINPUT_STATE*);
typedef DWORD(WINAPI *SetStateFunc)(DWORD, XINPUT_VIBRATION*);

GetStateFunc ogGetState = NULL;
SetStateFunc ogSetState = NULL;
HMODULE ogDLL = NULL;
HMODULE curDLL = NULL;

struct timespec getTS;
struct timespec setTS;

DWORD WINAPI XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState) {
	// Get the current time. Technically not portable code but it is with my compiler
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	// Calculate the nanosecond difference. If it is negative, add a billion and set the negative flag
	long nano = ts.tv_nsec - getTS.tv_nsec;
	BOOL nanNeg = FALSE;
	if (nano < 0) {
		nano += 1000000000;
		nanNeg = TRUE;
	}

	// Calculate the second difference, taking into account the negative flag, then print alongside the nanosecond difference
	printf("Get: %ld.%09ld\n", ts.tv_sec - getTS.tv_sec - nanNeg, nano);

	getTS = ts;
	return ogGetState(dwUserIndex, pState);
}

DWORD WINAPI XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration) {
	// Get the current time. Technically not portable code but it is with my compiler
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	// Calculate the nanosecond difference. If it is negative, add a billion and set the negative flag
	long nano = ts.tv_nsec - setTS.tv_nsec;
	BOOL nanNeg = FALSE;
	if (nano < 0) {
		nano += 1000000000;
		nanNeg = TRUE;
	}

	// Calculate the second difference, taking into account the negative flag, then print alongside the nanosecond difference
	printf("Set: %ld.%09ld\n", ts.tv_sec - setTS.tv_sec - nanNeg, nano);

	setTS = ts;
	return ogSetState(dwUserIndex, pVibration);	
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	char buf[PATH_MAX];

	// Warning Suppression
	lpReserved = (LPVOID)((LONGLONG)lpReserved + 1);

	// Store the DLL's handle
	curDLL = hModule;

	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			// Get the directory of the original DLL and load it
			GetSystemDirectory(buf, PATH_MAX);
			strcat(buf, "\\xinput9_1_0.dll");
			ogDLL = LoadLibraryA(buf);

			// Get the addresses of the original functions
			ogGetState = (GetStateFunc)GetProcAddress(ogDLL, "XInputGetState");
			ogSetState = (SetStateFunc)GetProcAddress(ogDLL, "XInputSetState");

			// Frontiers doesn't have a console. Open our own and redirect stdin/stdout/stderr
			AllocConsole();
			freopen("CONIN$", "r", stdin);
			freopen("CONOUT$", "w", stdout);
			freopen("CONOUT$", "w", stderr);
			break;
		default:
			break;
	}
	return TRUE;
}
