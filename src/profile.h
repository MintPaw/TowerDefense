#pragma once

#define PROFILES_MAX 128
#define PROFILER_AVERAGE_FRAMES 30

struct Profile {
	char *name;
	NanoTime startTime;
	NanoTime endTime;

	float pastMs[PROFILER_AVERAGE_FRAMES];
};

struct Profiler {
	Profile profiles[PROFILES_MAX];
	int profilesNum;

	void startProfile(const char *name);
	void endProfile(const char *name);
	float getMsResult(const char *name);
	float getAverage(const char *name);
	void getResult(const char *name, NanoTime *time);
	void printAll();
};

Profiler *profiler;

void initProfiler(Profiler *profilerInstance);
