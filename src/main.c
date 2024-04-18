#include <windows.h>
#include <xinput.h>

#include <stdio.h>

typedef DWORD(WINAPI *GetStateFunc)(DWORD, XINPUT_STATE*);
typedef DWORD(WINAPI *SetStateFunc)(DWORD, XINPUT_VIBRATION*);

GetStateFunc ogGetState = NULL;
SetStateFunc ogSetState = NULL;
HMODULE ogDLL = NULL;
HMODULE curDLL = NULL;

DWORD WINAPI XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState) {
	return ogGetState(dwUserIndex, pState);
}

DWORD WINAPI XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration) {
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
