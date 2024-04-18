#include <windows.h>
#include <xinput.h>

#include <stdio.h>
#include <time.h>

#include "util.h"
#include "tas.h"

typedef DWORD(WINAPI *GetStateFunc)(DWORD, XINPUT_STATE*);
typedef DWORD(WINAPI *SetStateFunc)(DWORD, XINPUT_VIBRATION*);

GetStateFunc ogGetState = NULL;
SetStateFunc ogSetState = NULL;
HMODULE ogDLL = NULL;
HMODULE curDLL = NULL;

// For TASing keyboard/mouse
HWND gameWnd = NULL;

HINSTANCE exeAddr = NULL;

XINPUT_STATE lastGamepad[4] = {0};
DWORD lastGPCode[4] = {0};

BOOL hasSetup = FALSE;

FILE* logFile;

BOOL CALLBACK enumerateWindows(HWND hWnd, LPARAM lParam) {
	DWORD wndProcess = 0;
	GetWindowThreadProcessId(hWnd, (LPDWORD)(&wndProcess));
	if (wndProcess == lParam) {
		gameWnd = hWnd;
		return FALSE;
	}
	return TRUE;
}

void firstTimeSetup(void) {
	DWORD pid = GetCurrentProcessId();
	EnumWindows(enumerateWindows, pid);

	utilInit();
	tasInit();

	hasSetup = TRUE;
}

DWORD WINAPI XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState) {
	// The window is created by the first call to XInputGetState
	if (!hasSetup) firstTimeSetup();
	if (tasIsReady()) lastGPCode[dwUserIndex] = tasTick(dwUserIndex, &lastGamepad[dwUserIndex]);

	*pState = lastGamepad[dwUserIndex];
	return lastGPCode[dwUserIndex];
}

DWORD WINAPI XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration) {
	printf("XInputSetState was called! Returning original function.\n");
	return ogSetState(dwUserIndex, pVibration);	
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	char buf[PATH_MAX];

	// Warning Suppression
	lpReserved = (LPVOID)((LONGLONG)lpReserved + 1);

	// Store the DLL's handle
	curDLL = hModule;

	// Get the base address of the executable
	exeAddr = GetModuleHandle(NULL);

	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			// Get the directory of the original DLL and load it
			GetSystemDirectory(buf, PATH_MAX);
			strcat(buf, "\\xinput9_1_0.dll");
			ogDLL = LoadLibraryA(buf);

			// Get the addresses of the original functions
			// Technically there's more than just these two functions, but Frontiers doesn't use them
			ogGetState = (GetStateFunc)GetProcAddress(ogDLL, "XInputGetState");
			ogSetState = (SetStateFunc)GetProcAddress(ogDLL, "XInputSetState");

			// Frontiers doesn't have a console. Open our own and redirect stdin/stdout/stderr
			AllocConsole();
			freopen("CONIN$", "r", stdin);
			freopen("CONOUT$", "w+", stdout);
			freopen("CONOUT$", "w+", stderr);
			break;
		default:
			break;
	}
	return TRUE;
}
