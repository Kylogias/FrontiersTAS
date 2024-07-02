#include "ui_main.h"

#include <stdlib.h>
#include <stdio.h>

#define CNFG_IMPLEMENTATION
#include "rawdraw/rawdraw_sf.h"

#include "../tas.h"

char* uiLevelID;
double uiDelta;

void HandleKey(int keycode, int bDown) {

}

void HandleButton(int x, int y, int button, int bDown) {

}

void HandleMotion(int x, int y, int mask) {

}

int HandleDestroy() { return 0; }

void uiInit(void) {
	uiLevelID = NULL;
	uiDelta = 0.0;

	CNFGSetup("Frontiers TAS", 640, 480);
	CNFGBGColor = CNFGDialogColor = 0x000000FF;
	CNFGColor(0xFFFFFFFF);
}

void uiTick(void) {
	if (!CNFGHandleInput()) exit(0);

	CNFGClearFrame();

	char buf[512];
	if (uiLevelID == NULL) uiLevelID = "???";
	snprintf(buf, 512, "Level ID: %s\nDelta: %f", uiLevelID, uiDelta);
	CNFGDrawText(buf, 3);

	CNFGSwapBuffers();
}
