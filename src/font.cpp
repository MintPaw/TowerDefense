#include "font.h"

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
			strncpy(font->pngPath, firstQuote + 1, nameLen);
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

	// printf("Font name: %s\n", font->name);
	// printf("Font size: %d\n", font->size);
	// printf("Font pngPath: %s\n", font->pngPath);

	free(fontData);
	return font;
}
