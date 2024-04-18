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
	
	// Get the process of the window then check it against the Frontiers process
	// If they match, we have the window
	GetWindowThreadProcessId(hWnd, (LPDWORD)(&wndProcess));
	if (wndProcess == lParam) {
		gameWnd = hWnd;
		return FALSE;
	}
	return TRUE;
}

void firstTimeSetup(void) {
	// Loop through every window (calls enumerateWindows for each window)
	DWORD pid = GetCurrentProcessId();
	EnumWindows(enumerateWindows, pid);

	utilInit();
	tasInit();

	// Set the flag saying we shouldn't call this function again
	hasSetup = TRUE;
}

DWORD WINAPI XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState) {
	// By the time XInputGetState is first called, the window is created
	// Find it for future KB+M TAS support
	if (!hasSetup) firstTimeSetup();

	// If the TAS believes we are ready for the next input, tick the TAS, setting the gamepad error code and the gamepad state
	if (tasIsReady()) lastGPCode[dwUserIndex] = tasTick(dwUserIndex, &lastGamepad[dwUserIndex]);

	// Return the gamepad state and gamepad error code
	*pState = lastGamepad[dwUserIndex];
	return lastGPCode[dwUserIndex];
}

// This function isn't necessary for TAS, but Frontiers uses it (for example, Cross Slash)
// It just sets the rumble on the controller
DWORD WINAPI XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration) {
	printf("XInputSetState was called! Returning original function.\n");
	return ogSetState(dwUserIndex, pVibration);	
}

// Entry point. We start here (also see XInputGetState)
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
