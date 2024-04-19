#include "script.h"

#include <windows.h>
#include <stdio.h>

typedef enum {
	TOKEN_PRESS, TOKEN_HOLD, TOKEN_RELEASE, TOKEN_TOGGLE, TOKEN_SPAM, TOKEN_IDLE, TOKEN_WAIT, TOKEN_LSTICK, TOKEN_RSTICK, TOKEN_END
} ScriptTokenType;

// LT/RT technically aren't buttons, but Frontiers treats them as such
typedef enum {
	SCRIPT_UP, SCRIPT_DOWN, SCRIPT_LEFT, SCRIPT_RIGHT, SCRIPT_START, SCRIPT_BACK, SCRIPT_LS, SCRIPT_RS, SCRIPT_LB, SCRIPT_RB, SCRIPT_LT, SCRIPT_RT, SCRIPT_A, SCRIPT_B, SCRIPT_X, SCRIPT_Y
} ScriptButton;

typedef union {
	i64 i;
	double f;
	void* p;
} ScriptLiteral;

typedef struct {
	ScriptLiteral v1;
	ScriptLiteral v2;
	ScriptTokenType type;
} ScriptToken;

void scriptTokenize(char* path, ScriptToken** toks, u32* tokCount) {
	FILE* f = fopen(path, "r");
	if (f == NULL) {
		fprintf(stderr, "Failed to open %s\n", path);
		exit(1);
	}

	fseek(f, 0, SEEK_END);
	i64 fSize = ftell(f);
	fseek(f, 0, SEEK_SET);
	u8* fData = (u8*)malloc((u64)fSize+1);
	fread(fData, 1, (u64)fSize, f);
	fclose(f);

	fData[fSize] = '\0';

	u8 tok[32];
	u8 curChar = 0;

	ScriptToken* tokens = malloc(sizeof(ScriptToken)*64);
	u32 allocTok = 64;
	u32 tokenCount = 0;

	ScriptToken curTok = {0};
	for (i64 i = 0; i < fSize; i++) {
		if (tokenCount >= allocTok) {
			allocTok += 64;
			tokens = realloc(tokens, sizeof(ScriptToken)*allocTok);
		}

		switch (fData[i]) {
			case '\r': continue;
			case '\n':
				tok[curChar] = '\0';
				if (strcmp((const char*)tok, "nodelay") != 0) {
					curTok.type = TOKEN_WAIT;
					curTok.v1.i = 1;
					tokens[tokenCount] = curTok;
					tokenCount += 1;
					curTok = (ScriptToken){0};
				} 
				if (strcmp((const char*)tok, "idle") == 0) {
					curTok.type = TOKEN_IDLE;
					tokens[tokenCount] = curTok;
					tokenCount += 1;
					curTok = (ScriptToken){0};
				}
				curChar = 0;
				break;
			case ' ':
				tok[curChar] = '\0';

				if (strcmp((const char*)tok, "press") == 0) {
					curTok.type = TOKEN_PRESS;
				} else if (strcmp((const char*)tok, "hold") == 0) {
					curTok.type = TOKEN_HOLD;
				} else if (strcmp((const char*)tok, "release") == 0) {
					curTok.type = TOKEN_RELEASE;
				} else if (strcmp((const char*)tok, "toggle") == 0) {
					curTok.type = TOKEN_TOGGLE;
				} else if (strcmp((const char*)tok, "spam") == 0) {
					curTok.type = TOKEN_SPAM;
				} else if (strcmp((const char*)tok, "idle") == 0) {
					curTok.type = TOKEN_IDLE;
				} else if (strcmp((const char*)tok, "wait") == 0) {
					curTok.type = TOKEN_WAIT;
					curTok.v1.i = strtoll((const char*)&fData[i+1], NULL, 10);
				} else if (strcmp((const char*)tok, "leftstick") == 0) {
					char* endptr = NULL;
					curTok.type = TOKEN_LSTICK;
					curTok.v1.f = strtod((const char*)&fData[i+1], &endptr);
					curTok.v2.f = strtod(endptr, NULL);
				} else if (strcmp((const char*)tok, "rightstick") == 0) {
					char* endptr = NULL;
					curTok.type = TOKEN_RSTICK;
					curTok.v1.f = strtod((const char*)&fData[i+i], &endptr);
					curTok.v2.f = strtod(endptr, NULL);
				}

				if (curTok.type < TOKEN_IDLE) {
					switch(fData[i+1]) {
						case 'A': curTok.v1.i = SCRIPT_A; break;
						case 'B': curTok.v1.i = SCRIPT_B; break;
						case 'X': curTok.v1.i = SCRIPT_X; break;
						case 'Y': curTok.v1.i = SCRIPT_Y; break;
						case 'U': curTok.v1.i = SCRIPT_UP; break;
						case 'D': curTok.v1.i = SCRIPT_DOWN; break;
						case 'L':
							if (fData[i+2] == 'T') {curTok.v1.i = SCRIPT_LT; break;}
							if (fData[i+2] == 'B') {curTok.v1.i = SCRIPT_LB; break;}
							curTok.v1.i = SCRIPT_LEFT; break;
						case 'R':
							if (fData[i+2] == 'T') {curTok.v1.i = SCRIPT_RT; break;}
							if (fData[i+2] == 'B') {curTok.v1.i = SCRIPT_RB; break;}
							curTok.v1.i = SCRIPT_RIGHT; break;
							break;
						case 'S':
							if (fData[i+2] == 'T') {curTok.v1.i = SCRIPT_START; break;}
							curTok.v1.i = SCRIPT_BACK; break;
					}
				}

				tokens[tokenCount] = curTok;
				curTok = (ScriptToken){0};
				tokenCount += 1;

				curChar = 0;
				continue;
			default:
				tok[curChar] = fData[i];
				curChar += 1;
		}

		if (curChar >= 32) {
			fprintf(stderr, "STRING TOO LONG\n");
		}
	}

	*toks = tokens;
	*tokCount = tokenCount;

	free(fData);
}

u32 scriptInputCount(ScriptToken* tokens, u32 tokCount) {
	u32 numInp = 0;
	for (u32 i = 0; i < tokCount; i++) {
		if (tokens[i].type == TOKEN_WAIT) numInp += tokens[i].v1.i;
	}
	return numInp;
}

void scriptInit(void) {}

void scriptParse(char* path, XINPUT_GAMEPAD** inpArray, u32* inpCount) {
	ScriptToken* tokens;
	u32 tokCount;
	scriptTokenize(path, &tokens, &tokCount);

	u32 numInp = scriptInputCount(tokens, tokCount);
	*inpCount = numInp;
	*inpArray = (XINPUT_GAMEPAD*)malloc(numInp * sizeof(XINPUT_GAMEPAD));

	u32 curInp = 0;
	XINPUT_GAMEPAD curGP = {0};

	u16 pressInp = 0, holdInp = 0, spamInpFront = 0, spamInpBack = 0;

	for (u32 i = 0; i < tokCount; i++) {
		switch (tokens[i].type) {
			case TOKEN_PRESS:
				pressInp |= 1 << tokens[i].v1.i;
				break;
			case TOKEN_HOLD:
				holdInp |= 1 << tokens[i].v1.i;
				break;
			case TOKEN_RELEASE:
				holdInp &= ~(1 << tokens[i].v1.i);
				spamInpFront &= ~(1 << tokens[i].v1.i);
				spamInpBack &= ~(1 << tokens[i].v1.i);
				break;
			case TOKEN_TOGGLE:
				holdInp ^= 1 << tokens[i].v1.i;
				break;
			case TOKEN_SPAM:
				spamInpFront |= 1 << tokens[i].v1.i;
				break;
			case TOKEN_IDLE:
				pressInp = holdInp = spamInpFront = spamInpBack = 0;
				curGP = (XINPUT_GAMEPAD){0};
				break;
			case TOKEN_WAIT:
				
				for (u32 j = 0; j < tokens[i].v1.i; j++) {
					curGP.wButtons = pressInp | holdInp | spamInpFront;
					curGP.bLeftTrigger = 255 * !!(curGP.wButtons & 0x0400);
					curGP.bRightTrigger = 255 * !!(curGP.wButtons & 0x0800);
					curGP.wButtons &= 0xf3ff;
					(*inpArray)[curInp] = curGP;
	
					curInp += 1;
					pressInp &= 0;
					u16 oldSpam = spamInpFront;
					spamInpFront = spamInpBack;
					spamInpBack = oldSpam;
				}
				break;
			case TOKEN_LSTICK:
				curGP.sThumbLX = (i16)(tokens[i].v1.f*32768);
				curGP.sThumbLY = (i16)(tokens[i].v2.f*32768);
				break;
			case TOKEN_RSTICK:
				curGP.sThumbRX = (i16)(tokens[i].v1.f*32768);
				curGP.sThumbRY = (i16)(tokens[i].v2.f*32768);
				break;
			case TOKEN_END:
				break;
		}
	}

	free(tokens);
}

i64 scriptTimestamp(char* path) {
	HANDLE hFile = CreateFileA(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	FILETIME ftWrite;
	GetFileTime(hFile, NULL, NULL, &ftWrite);
	CloseHandle(hFile);

	ULARGE_INTEGER ft = {.LowPart=ftWrite.dwLowDateTime, .HighPart=ftWrite.dwHighDateTime};
	i64 sec = ft.QuadPart / 10000000;
	return sec;
}
