#pragma once

struct Texture {
	GLuint textureId;
	int width;
	int height;
};

struct SpriteDef {
	Texture *tex;
	Point pos;
	Point scrollFactor;
	int tint;
	float rotation;
	Point pivot;
	float alpha;
};
void initRenderer();
void clearRenderer();

void drawCircle(float x, float y, float radius, int colour);
void drawRect(float x, float y, float width, float height, int colour);

Texture *uploadPngTexturePath(const char *path);
Texture *uploadPngTexture(void *data, int size);
Texture *uploadTexture(void *data, int width, int height);

void destroyTexture(Texture *tex);
void drawSprite(Texture *tex, float x, float y);
void defaultSpriteDef(SpriteDef *def);
void drawSpriteEx(SpriteDef *def);
void drawTextureToTexture(Texture *srcTex, Texture *destTex, int x, int y, int width, int height, int dx=0, int dy=0, int tint=0x00000000, float scaleX=1, float scaleY=1, bool bleed=false);

void clearTexture(Texture *tex);
void setCameraExtents(float x, float y, float width, float height);
void setCameraPosition(float x, float y);
