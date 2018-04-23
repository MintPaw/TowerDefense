#include "profile.h"

void initProfiler(Profiler *profilerInstance) {
	profiler = profilerInstance;
	memset(profiler, 0, sizeof(Profiler));
}

void Profiler::startProfile(const char *name) {
	NanoTime time;

	Profile *prof = NULL;
	for (int i = 0; i < profiler->profilesNum; i++) {
		Profile *curProf = &profiler->profiles[i];
		if (streq(name, curProf->name)) {
			prof = curProf;

			for (int j = PROFILER_AVERAGE_FRAMES-1; j > 0; j--) prof->pastMs[j] = prof->pastMs[j-1];
			prof->pastMs[0] = profiler->getMsResult(name);
		}
	}

	if (!prof) {
		prof = &profiler->profiles[profiler->profilesNum++];
		prof->name = stringClone(name);
	}

	getNanoTime(&prof->startTime);
}

void Profiler::endProfile(const char *name) {
	Profile *prof;
	for (int i = 0; i < profiler->profilesNum; i++) {
		Profile *curProf = &profiler->profiles[i];
		if (streq(name, curProf->name)) {
			prof = curProf;
			break;
		}
	}

	getNanoTime(&prof->endTime);
}

float Profiler::getAverage(const char *name) {
	Profile *prof = NULL;
	for (int i = 0; i < profiler->profilesNum; i++) {
		Profile *curProf = &profiler->profiles[i];
		if (streq(name, curProf->name)) {
			prof = curProf;
			break;
		}
	}

	if (!prof) return 0;

	float sum = 0;
	for (int i = 0; i < PROFILER_AVERAGE_FRAMES; i++) sum += prof->pastMs[i];
	sum /= PROFILER_AVERAGE_FRAMES;
	return sum;
}

float Profiler::getMsResult(const char *name) {
	NanoTime time;
	profiler->getResult(name, &time);

	float ms = time.seconds*1000 + time.nanos/1000000.0;
	return ms;
}

void Profiler::getResult(const char *name, NanoTime *time) {
	for (int i = 0; i < profiler->profilesNum; i++) {
		Profile *prof = &profiler->profiles[i];
		if (streq(name, prof->name)) {
			if (prof->endTime.nanos - prof->startTime.nanos < 0) {
				time->seconds = prof->endTime.seconds - prof->startTime.seconds - 1;
				time->nanos = 1000000000 + prof->endTime.nanos - prof->startTime.nanos;
			} else {
				time->seconds = prof->endTime.seconds - prof->startTime.seconds;
				time->nanos = prof->endTime.nanos - prof->startTime.nanos;
			}
		}
	}
}

void Profiler::printAll() {
	for (int i = 0; i < profiler->profilesNum; i++) {
		Profile *prof = &profiler->profiles[i];
		printf("Profile: %s\nStart: %d, %d\n", prof->name, prof->startTime.seconds, prof->startTime.nanos);
		if (prof->endTime.seconds != 0 || prof->endTime.nanos != 0) {
		printf("End: %d, %d\n", prof->endTime.seconds, prof->endTime.nanos);
		} else {
			printf("Not finished\n");
		}
	}
}
