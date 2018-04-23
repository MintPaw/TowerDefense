#pragma once

enum KeyState { KEY_RELEASED = 0, KEY_PRESSED, KEY_JUST_RELEASED, KEY_JUST_PRESSED };

struct NanoTime {
	unsigned int seconds;
	unsigned int nanos;

#ifdef _WIN32
	LARGE_INTEGER winFreq;
	LARGE_INTEGER time;
#endif
};

void initPlatform();
void platformUpdateLoop(void (*updateCallbcak)());
void log(const char *text, ...);
long readFile(const char *filename, void **storage);
void swapBuffers();
void getNanoTime(NanoTime *time);
