#pragma once

struct BitmapCharDef {
	int id, x, y, width, height, xoff, yoff, xadvance;
};
struct BitmapKernDef {
	int first, second, amount;
};

struct BitmapFont {
	char *name;
	int size;
	int lineHeight;
	char *pngPath;
	BitmapCharDef *charDefs;
	BitmapKernDef *kernDefs;
	int charDefsNum;
	int kernDefsNum;

	Texture *texture;
};

BitmapFont *loadBitmapFontPath(const char *fntPath);
