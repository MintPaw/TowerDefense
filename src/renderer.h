#pragma once

void initRenderer();
void clearRenderer();
void drawCircle(float x, float y, float radius, int colour);
int uploadPngTexturePath(char *path);
int uploadPngTexture(void *data, int size);
int uploadTexture(void *data, int width, int height);
