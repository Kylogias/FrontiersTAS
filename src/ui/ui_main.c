#include "ui_main.h"

#include <stdlib.h>
#include <stdio.h>

#define CNFG_IMPLEMENTATION
#include "rawdraw/rawdraw_sf.h"

#define NK_IMPLEMENTATION
#include "nkinc.h"

#include "../tas.h"

char* uiLevelID;
double uiDelta;

struct nk_context uiCtx;
struct nk_font* uiFont;
struct nk_font_atlas uiAtlas;
void* uiFontImage;
int uiFontX, uiFontY;

#define CIRCLE_VERTICES 32

void HandleKey(int keycode, int bDown) {

}

void HandleButton(int x, int y, int button, int bDown) {

}

void HandleMotion(int x, int y, int mask) {

}

int HandleDestroy() { return 0; }

void uiDraw(void) {
	const struct nk_command* cmd;
	nk_foreach(cmd, &uiCtx) {
		switch (cmd->type) {
			case NK_COMMAND_SCISSOR: break;
			case NK_COMMAND_CURVE: break;
			case NK_COMMAND_ARC: break;
			case NK_COMMAND_ARC_FILLED: break;
			case NK_COMMAND_IMAGE: break;
			case NK_COMMAND_CUSTOM: break;
			case NK_COMMAND_NOP: break;
			case NK_COMMAND_LINE: {
				const struct nk_command_line* tcmd = (struct nk_command_line*)cmd;
				CNFGColor((u32)((tcmd->color.a) | (tcmd->color.r << 8) | (tcmd->color.g << 16) | (tcmd->color.b << 24)));
				CNFGTackSegment(tcmd->begin.x, tcmd->begin.y, tcmd->end.x, tcmd->end.y);
			} break;
			case NK_COMMAND_RECT: {
				const struct nk_command_rect* tcmd = (struct nk_command_rect*)cmd;
				CNFGColor((u32)((tcmd->color.a) | (tcmd->color.r << 8) | (tcmd->color.g << 16) | (tcmd->color.b << 24)));
				CNFGDrawBox(tcmd->x, tcmd->y, (short)(tcmd->x+tcmd->w), (short)(tcmd->y+tcmd->h));
			} break;
			case NK_COMMAND_RECT_FILLED: {
				const struct nk_command_rect_filled* tcmd = (struct nk_command_rect_filled*)cmd;
				CNFGColor((u32)((tcmd->color.a) | (tcmd->color.r << 8) | (tcmd->color.g << 16) | (tcmd->color.b << 24)));
				CNFGTackRectangle(tcmd->x, tcmd->y, (short)(tcmd->x+tcmd->w), (short)(tcmd->y+tcmd->h));
			} break;
			case NK_COMMAND_RECT_MULTI_COLOR: {
				const struct nk_command_rect_multi_color* tcmd = (struct nk_command_rect_multi_color*)cmd;
				RDPoint points[3];

				CNFGColor((u32)((tcmd->left.a) | (tcmd->left.r << 8) | (tcmd->left.g << 16) | (tcmd->left.b << 24)));
				points[0] = (RDPoint){.x=tcmd->x, .y=tcmd->y};
				points[1] = (RDPoint){.x=tcmd->x, .y=(short)(tcmd->y+tcmd->h)};
				points[2] = (RDPoint){.x=tcmd->x+tcmd->w/2, .y=tcmd->y+tcmd->h/2};
				CNFGTackPoly(points, 3);

				CNFGColor((u32)((tcmd->right.a) | (tcmd->right.r << 8) | (tcmd->right.g << 16) | (tcmd->right.b << 24)));
				points[0] = (RDPoint){.x=(short)(tcmd->x+tcmd->w), .y=tcmd->y};
				points[1] = (RDPoint){.x=(short)(tcmd->x+tcmd->w), .y=(short)(tcmd->y+tcmd->h)};
				points[2] = (RDPoint){.x=tcmd->x+tcmd->w/2, .y=tcmd->y+tcmd->h/2};
				CNFGTackPoly(points, 3);

				CNFGColor((u32)((tcmd->bottom.a) | (tcmd->bottom.r << 8) | (tcmd->bottom.g << 16) | (tcmd->bottom.b << 24)));
				points[0] = (RDPoint){.x=tcmd->x, .y=(short)(tcmd->y+tcmd->h)};
				points[1] = (RDPoint){.x=(short)(tcmd->x+tcmd->w), .y=(short)(tcmd->y+tcmd->h)};
				points[2] = (RDPoint){.x=tcmd->x+tcmd->w/2, .y=tcmd->y+tcmd->h/2};
				CNFGTackPoly(points, 3);

				CNFGColor((u32)((tcmd->top.a) | (tcmd->top.r << 8) | (tcmd->top.g << 16) | (tcmd->top.b << 24)));
				points[0] = (RDPoint){.x=tcmd->x, .y=tcmd->y};
				points[1] = (RDPoint){.x=(short)(tcmd->x+tcmd->w), .y=tcmd->y};
				points[2] = (RDPoint){.x=tcmd->x+tcmd->w/2, .y=tcmd->y+tcmd->h/2};
				CNFGTackPoly(points, 3);
			} break;
			case NK_COMMAND_TRIANGLE: {
				const struct nk_command_triangle* tcmd = (struct nk_command_triangle*)cmd;
				CNFGColor((u32)((tcmd->color.a) | (tcmd->color.r << 8) | (tcmd->color.g << 16) | (tcmd->color.b << 24)));
				CNFGTackSegment(tcmd->a.x, tcmd->a.y, tcmd->b.x, tcmd->b.y);
				CNFGTackSegment(tcmd->b.x, tcmd->b.y, tcmd->c.x, tcmd->c.y);
				CNFGTackSegment(tcmd->c.x, tcmd->c.y, tcmd->a.x, tcmd->a.y);
			} break;
			case NK_COMMAND_TRIANGLE_FILLED: {
				const struct nk_command_triangle_filled* tcmd = (struct nk_command_triangle_filled*)cmd;
				CNFGColor((u32)((tcmd->color.a) | (tcmd->color.r << 8) | (tcmd->color.g << 16) | (tcmd->color.b << 24)));
				RDPoint points[3] = {
					{.x=tcmd->a.x, .y=tcmd->a.y},
					{.x=tcmd->b.x, .y=tcmd->b.y},
					{.x=tcmd->c.x, .y=tcmd->c.y}
				};
				CNFGTackPoly(points, 3);
			}
			case NK_COMMAND_CIRCLE: {
				const struct nk_command_circle* tcmd = (struct nk_command_circle*)cmd;
				CNFGColor((u32)((tcmd->color.a) | (tcmd->color.r << 8) | (tcmd->color.g << 16) | (tcmd->color.b << 24)));
				for (int i = 0; i < CIRCLE_VERTICES; i++) {
					CNFGTackSegment(tcmd->x, tcmd->y, (short)(tcmd->x + tcmd->w*cos(i*2*M_PI/CIRCLE_VERTICES))/2, (short)(tcmd->y + tcmd->h*sin(i*2*M_PI/CIRCLE_VERTICES))/2);
				}
			} break;
			case NK_COMMAND_CIRCLE_FILLED: {
				RDPoint points[CIRCLE_VERTICES];
				const struct nk_command_circle_filled* tcmd = (struct nk_command_circle_filled*)cmd;
				CNFGColor((u32)((tcmd->color.a) | (tcmd->color.r << 8) | (tcmd->color.g << 16) | (tcmd->color.b << 24)));
				for (int i = 0; i < CIRCLE_VERTICES; i++) {
					points[i] = (RDPoint){(short)(tcmd->x + tcmd->w*cos(i*2*M_PI/CIRCLE_VERTICES))/2, (short)(tcmd->y + tcmd->h*sin(i*2*M_PI/CIRCLE_VERTICES))/2};
				}
				CNFGTackPoly(points, CIRCLE_VERTICES);
			} break;
			case NK_COMMAND_POLYGON_FILLED: {
				const struct nk_command_polygon_filled* tcmd = (struct nk_command_polygon_filled*)cmd;
				CNFGColor((u32)((tcmd->color.a) | (tcmd->color.r << 8) | (tcmd->color.g << 16) | (tcmd->color.b << 24)));
				RDPoint points[tcmd->point_count];
				for (int i = 0; i < tcmd->point_count; i++) {
					points[i] = (RDPoint){tcmd->points[i].x, tcmd->points[i].y};
				}
				CNFGTackPoly(points, tcmd->point_count);
			} break;
			case NK_COMMAND_TEXT: {

			} break;
			case NK_COMMAND_POLYGON: {
				const struct nk_command_polygon* tcmd = (struct nk_command_polygon*)cmd;
				CNFGColor((u32)((tcmd->color.a) | (tcmd->color.r << 8) | (tcmd->color.g << 16) | (tcmd->color.b << 24)));
				CNFGTackSegment(tcmd->points[0].x, tcmd->points[0].y, tcmd->points[tcmd->point_count-1].x, tcmd->points[tcmd->point_count-1].y);
			}
			case NK_COMMAND_POLYLINE: {
				const struct nk_command_polyline* tcmd = (struct nk_command_polyline*)cmd;
				CNFGColor((u32)((tcmd->color.a) | (tcmd->color.r << 8) | (tcmd->color.g << 16) | (tcmd->color.b << 24)));
				for (int i = 0; i < tcmd->point_count-1; i++) {
					CNFGTackSegment(tcmd->points[i].x, tcmd->points[i].y, tcmd->points[i+1].x, tcmd->points[i+1].y);
				}
			} break;
		}
	}
}

void uiInit(void) {
	uiLevelID = NULL;
	uiDelta = 0.0;

	CNFGSetup("Frontiers TAS", 640, 480);
	CNFGBGColor = CNFGDialogColor = 0x000000FF;
	CNFGColor(0xFFFFFFFF);

	nk_font_atlas_init_default(&uiAtlas);
	nk_font_atlas_begin(&uiAtlas);
	uiFont = nk_font_atlas_add_default(&uiAtlas, 12, NULL);
	uiFontImage = (void*)nk_font_atlas_bake(&uiAtlas, &uiFontX, &uiFontY, NK_FONT_ATLAS_RGBA32);
	nk_font_atlas_end(&uiAtlas, nk_handle_id(0), NULL);

	nk_init_default(&uiCtx, &uiFont->handle);
}

void uiTick(void) {
	nk_input_begin(&uiCtx);
	if (!CNFGHandleInput()) exit(0);
	nk_input_end(&uiCtx);

	if (nk_begin(&uiCtx, "Frontiers TAS Tools", nk_rect(0, 0, 640, 480), 0)) {
		
	}
	nk_end(&uiCtx);

	CNFGClearFrame();

	char buf[512];
	if (uiLevelID == NULL) uiLevelID = "???";
	snprintf(buf, 512, "Level ID: %s\nDelta: %f", uiLevelID, uiDelta);
	CNFGDrawText(buf, 3);


//	uiDraw();

	CNFGSwapBuffers();
	nk_clear(&uiCtx);
}
