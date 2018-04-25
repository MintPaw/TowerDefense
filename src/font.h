#pragma once

#define TEXT_MAX 2048

struct Text {
	float width;
	float height;
	float fieldWidth;
	float fieldHeight;
	char text[TEXT_MAX];

	Texture *tex;
};

struct TextProps {
	bool exists;
	float width;
	float height;
	char text[TEXT_MAX];
};

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

void initText(Text *tf, float fieldWidth, float fieldHeight);
BitmapFont *loadBitmapFontPath(const char *fntPath);
void drawText(Text *tf, BitmapFont *font, const char *text, ...);
