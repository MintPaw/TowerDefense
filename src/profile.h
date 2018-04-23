#pragma once

#define PROFILES_MAX 128

struct Profile {
	char *name;
	NanoTime startTime;
	NanoTime endTime;
	NanoTime timeTaken;
};

struct Profiler {
	Profile profiles[PROFILES_MAX];
	int profilesNum;

	void startProfile(const char *name);
	void endProfile(const char *name);
	void reset();
	float getMsResult(const char *name);
	void getResult(const char *name, NanoTime *time);
	void printAll();
};

Profiler *profiler;

void initProfiler(Profiler *profilerInstance);
