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
};

void initRenderer();
void clearRenderer();
void drawCircle(float x, float y, float radius, int colour);
Texture *uploadPngTexturePath(const char *path);
Texture *uploadPngTexture(void *data, int size);
Texture *uploadTexture(void *data, int width, int height);
void destroyTexture(Texture *tex);
void drawSprite(Texture *tex, float x, float y);
void defaultSpriteDef(SpriteDef *def);
void drawSpriteEx(SpriteDef *def);
void setCameraPosition(float x, float y);
