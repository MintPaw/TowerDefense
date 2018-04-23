#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>

#ifdef __linux__
#include <time.h>
#endif

#include "platform.h"

#define Assert(expr) engineAssert(expr, __FILE__, __LINE__)
#define GL_CORE
// #define GL_ES

#define KEY_LIMIT 500

void updateEvents();
void engineAssert(bool expr, const char *filename, int lineNum);
void platformSleep(int ms);

struct Platform {
	SDL_Window *sdlWindow;
	SDL_GLContext sdlContext;

	int windowWidth;
	int windowHeight;

	bool running;

	KeyState keys[KEY_LIMIT];
	int mouseX;
	int mouseY;
	bool mouseDown;
	bool mouseJustDown;

	int frameTime;
	float elapsed;
	float time;
	float timeScale;
};

Platform *platform;

void initPlatform() {
	log("Platform Init\n");

	platform = (Platform *)malloc(sizeof(Platform));
	memset(platform, 0, sizeof(Platform));
	platform->windowWidth = 1280;
	platform->windowHeight = 720;
	platform->timeScale = 1;

#ifdef _WIN32
	timeBeginPeriod(1);
#endif

	Assert(!SDL_Init(SDL_INIT_VIDEO));

#ifdef GL_CORE
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#endif
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	platform->sdlWindow = SDL_CreateWindow("Shmup", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, platform->windowWidth, platform->windowHeight, SDL_WINDOW_OPENGL);
	Assert(platform->sdlWindow);

	platform->sdlContext = SDL_GL_CreateContext(platform->sdlWindow);
	Assert(platform->sdlContext);

	Assert(!SDL_GL_SetSwapInterval(0));

	glewExperimental = GL_TRUE; 
	Assert(glewInit() == GLEW_OK);
}

void platformUpdateLoop(void (*updateCallbcak)()) {
	platform->running = true;
	while (platform->running) {
		platform->elapsed = 1/60.0 * platform->timeScale;

		int startTime = SDL_GetTicks();
		platform->time += platform->elapsed;

		updateEvents();
		updateCallbcak();

		platform->frameTime = SDL_GetTicks() - startTime;
		int sleepMs = ceilf(1.0/60.0*1000.0) - platform->frameTime;
		if (sleepMs > 0) platformSleep(sleepMs);
	}
}

void swapBuffers() {
	SDL_GL_SwapWindow(platform->sdlWindow);
}

void updateEvents() {
	platform->mouseJustDown = false;

	for (int i = 0; i < KEY_LIMIT; i++) {
		if (platform->keys[i] == KEY_JUST_PRESSED) platform->keys[i] = KEY_PRESSED;
		else if (platform->keys[i] == KEY_JUST_RELEASED) platform->keys[i] = KEY_RELEASED;
	}

	SDL_Event e;
	while (SDL_PollEvent(&e) != 0) {
		if (e.type == SDL_QUIT) {
			platform->running = false;
		} else if (e.type == SDL_KEYDOWN) {
			int key = e.key.keysym.sym;
			if (key == 27) platform->running = false;

			if (key >= 'a' && key <= 'z') key -= 'a'-'A';
			if (key > KEY_LIMIT) return;

			if (platform->keys[key] == KEY_RELEASED || platform->keys[key] == KEY_JUST_RELEASED) {
				platform->keys[key] = KEY_JUST_PRESSED;
			}	
		} else if (e.type == SDL_KEYUP) {
			int key = e.key.keysym.sym;
			if (key >= 'a' && key <= 'z') key -= 'a'-'A';
			if (key > KEY_LIMIT) return;

			if (platform->keys[key] == KEY_PRESSED || platform->keys[key] == KEY_JUST_PRESSED) {
				platform->keys[key] = KEY_JUST_RELEASED;
			}	
		}	else if (e.type == SDL_MOUSEMOTION) {
			SDL_GetMouseState(&platform->mouseX, &platform->mouseY);
		} else if (e.type == SDL_MOUSEBUTTONDOWN) {
			platform->mouseDown = true;
			platform->mouseJustDown = true;
		} else if (e.type == SDL_MOUSEBUTTONUP) {
			platform->mouseDown = false;
		} else if (e.type == SDL_MOUSEWHEEL) {
			// platformMouseWheel = e.wheel.y;
		}
	}
}

long readFile(const char *filename, void **storage) {
	FILE *filePtr = fopen(filename, "rb");
	if (!filePtr) return 0;

	fseek(filePtr, 0, SEEK_END);
	long fileSize = ftell(filePtr);
	fseek(filePtr, 0, SEEK_SET);  //same as rewind(f);

	char *str = (char *)malloc(fileSize + 1);
	fread(str, fileSize, 1, filePtr);
	fclose(filePtr);

	str[fileSize] = 0;

	*storage = str;

	return fileSize;
}

void log(const char *text, ...) {
	va_list argptr;
	va_start(argptr, text);
	vprintf(text, argptr);
	va_end(argptr);
}

void engineAssert(bool expr, const char *filename, int lineNum) {
	if (!expr) {
		printf("Assert failed at %s line %d\n", filename, lineNum);
		exit(1);
	}
}

void platformSleep(int ms) {
#ifdef _WIN32
	Sleep(ms);
#elif __linux__
	timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
#endif
}

void getNanoTime(NanoTime *time) {
#ifdef _WIN32
#elif __linux__
	timespec ts;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
	time->seconds = ts.tv_sec;
	time->nanos = ts.tv_nsec;
#endif
}
