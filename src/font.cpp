#include "font.h"
#include "profile.h"

#define TEXT_MAX 2048

int getKern(BitmapFont *font, int first, int second);
BitmapCharDef *getCharDef(BitmapFont *font, int ch);

BitmapFont *loadBitmapFontPath(const char *fntPath) {
	BitmapFont *font = (BitmapFont *)malloc(sizeof(BitmapFont));
	memset(font, 0, sizeof(BitmapFont));

	char *fontData;
	readFile(fntPath, (void **)&fontData);
	// printf("Font data: %s\n", fontData);

	const char *delim;
	if (strstr(fontData, "\r\n")) delim = "\r\n";
	else delim = "\n";
	int delimNum = strlen(delim);

	char *lineStart = fontData;

	bool moreLines = true;
	for (int i = 0; moreLines; i++) {
		char *lineEnd = strstr(lineStart, delim);

		if (!lineEnd) {
			moreLines = false;
			lineEnd = &fontData[strlen(fontData)-1];
		}

		int lineLen = lineEnd-lineStart;
		char line[1024] = {};
		if (lineLen <= 0) break; // This was added to prevent crashing, I think it happens because readFile adds and extra \n

		memcpy(line, lineStart, lineLen);
		line[lineLen] = '\0';

		if (i == 0) {
			char *firstQuote = strstr(line, "\"");
			char *secondQuote = strstr(firstQuote + 1, "\"");
			font->name = (char *)malloc((secondQuote - firstQuote) + 1); // This +1 might not be needed because I'm skipping the first quote on the next line
			int nameLen = secondQuote - firstQuote - 1;
			strncpy(font->name, firstQuote + 1, nameLen);
			font->name[nameLen] = '\0';

			char *preSize = strstr(line, "size=") + 4;
			char *postSize = strstr(preSize, " ");
			char *sizeStr = (char *)malloc((postSize - preSize) + 1); // This +1 might not be needed because I'm skipping the first quote on the next line
			int sizeLen = postSize - preSize - 1;
			strncpy(sizeStr, preSize + 1, sizeLen);
			sizeStr[sizeLen] = '\0';
			font->size = atoi(sizeStr);
		}

		if (i == 1) {
			sscanf(line, "common lineHeight=%d base=%*d scaleW=%*d scaleH=%*d pages=%*d packed=%*d alphaChnl=%*d redChnl=%*d greenChnl=%*d blueChnl=%*d", &font->lineHeight);
		}

		if (i == 2) {
			char *firstQuote = strstr(line, "\"");
			char *secondQuote = strstr(firstQuote + 1, "\"");
			font->pngPath = (char *)malloc((secondQuote - firstQuote) + 1); // This +1 might not be needed because I'm skipping the first quote on the next line
			int pathLen = secondQuote - firstQuote - 1;
			strncpy(font->pngPath, firstQuote + 1, pathLen);
			font->pngPath[pathLen] = '\0';
		}

		if (strstr(line, "chars count=")) {
			int chars;
			sscanf(line, "chars count=%d", &chars);
			font->charDefs = (BitmapCharDef *)malloc(chars * sizeof(BitmapCharDef));
		}

		if (strstr(line, "kernings count=")) {
			int kerns;
			sscanf(line, "kernings count=%d", &kerns);
			font->kernDefs = (BitmapKernDef *)malloc(kerns * sizeof(BitmapKernDef));
		}

		if (strstr(line, "char id")) {
			BitmapCharDef *charDef = &font->charDefs[font->charDefsNum++];

			sscanf(line, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%*d  chnl=%*d", 
				&charDef->id, &charDef->x, &charDef->y, &charDef->width, &charDef->height, &charDef->xoff, &charDef->yoff, &charDef->xadvance
			);
		}

		if (strstr(line, "kerning first")) {
			BitmapKernDef *kernDef = &font->kernDefs[font->kernDefsNum++];
			sscanf(line, "kerning first=%d  second=%d  amount=%d", &kernDef->first, &kernDef->second, &kernDef->amount);
		}

		lineStart = lineEnd + delimNum;
	}

	font->texture = uploadPngTexturePath(font->pngPath);

	// printf("Font name: %s\n", font->name);
	// printf("Font size: %d\n", font->size);
	// printf("Font pngPath: %s\n", font->pngPath);

	free(fontData);
	return font;
}

void drawText(Texture *tex, BitmapFont *font, const char *text, TextProps *props, ...) {
	char *realText = (char *)malloc(TEXT_MAX);

	va_list argptr;
	va_start(argptr, props);
	vsprintf(realText, text, argptr);
	va_end(argptr);

	text = (char *)realText;

	clearTexture(tex);

	Rect charRects[TEXT_MAX];
	int lineBreaks[TEXT_MAX];
	int charRectsNum = 0;
	int lineBreaksNum = 0;

	Point cursor;
	cursor.setTo(0, 0);
	char prevChar = -1;

	float textWidth = 0;
	float textHeight = 0;

	int textLen = strlen(text);

	for (int i = 0; i < textLen; i++) {
		// printf("Looking at char #%d %c\n", i, spr->text[i]);
		if (text[i] == '\n') {
			cursor.x = 0;
			cursor.y += font->lineHeight;
			textHeight += font->lineHeight;
			lineBreaks[lineBreaksNum++] = i;
			continue;
		}

		if (prevChar == 32) {
			const char *nextBreakPtr = strpbrk(&text[i], " \n"); //@cleanup change to strstr?
			int nextBreakIndex;
			if (nextBreakPtr != NULL) nextBreakIndex = i + (nextBreakPtr - &text[i]);
			else nextBreakIndex = textLen;

			int prevWordChar = -1;
			float wordWidth = 0;
			// printf("Going from %d to %d\n", i, nextBreakIndex);
			for (int j = i; j < nextBreakIndex; j++) {
				char curWordChar = text[i];
				BitmapCharDef *def = getCharDef(font, curWordChar);
				if (!def) continue;

				wordWidth += def->width + def->xoff + def->xadvance + getKern(font, prevWordChar, curWordChar);
				prevWordChar = curWordChar;
			}

			if (cursor.x + wordWidth > tex->width) {
				cursor.x = 0;
				cursor.y += font->lineHeight;
				textHeight += font->lineHeight;
			}

			lineBreaks[lineBreaksNum++] = i;
		}

		if (text[i] == 0) continue; // New line removal makes this happen. //@cleanup Really?

		cursor.x += round(getKern(font, prevChar, text[i]));

		BitmapCharDef *charDef;

		/////
		// This first branch is for the double hyphen character in unicode
		/////
		if (text[i] == -30 && i < textLen+3) {
			if (text[i+1] == -128 && text[i+2] == -108) {
				charDef = getCharDef(font, 8212);
			}
		} else {
			charDef = getCharDef(font, text[i]);
		}

		if (!charDef) continue;
		int charX = cursor.x + charDef->xoff;
		int charY = cursor.y + charDef->yoff;

		if (text[i] != ' ') {
			drawTextureToTexture(font->texture, tex, charDef->x, charDef->y, charDef->width, charDef->height, charX, charY);
		}

		charRects[charRectsNum++].setTo(charX, charY, charDef->width, charDef->height);

		textWidth = fmax((charX+charDef->width), textWidth);
		cursor.x += charDef->xadvance;
		prevChar = text[i];
	}

	textHeight += font->lineHeight;

	if (props) {
		props->width = textWidth;
		props->height = textHeight;
	}

	free(realText);
}

BitmapCharDef *getCharDef(BitmapFont *font, int ch) {
	for (int i = 0; i < font->charDefsNum; i++) {
		if (font->charDefs[i].id == ch)
			return &font->charDefs[i];
	}

	return NULL;
}

int getKern(BitmapFont *font, int first, int second) {
	for (int i = 0; i < font->kernDefsNum; i++)
		if (font->kernDefs[i].first == first && font->kernDefs[i].second == second)
			return font->kernDefs[i].amount;

	return 0;
}
