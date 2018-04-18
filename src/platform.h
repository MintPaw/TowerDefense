#pragma once

enum KeyState { KEY_RELEASED = 0, KEY_PRESSED, KEY_JUST_RELEASED, KEY_JUST_PRESSED };

void initPlatform();
void platformUpdateLoop(void (*updateCallbcak)());
void log(const char *text, ...);
long readFile(const char *filename, void **storage);
void swapBuffers();
