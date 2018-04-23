#include "profile.h"

void initProfiler(Profiler *profiler) {
	memset(profiler, 0, sizeof(Profiler));
}

void Profiler::reset() {
	Profiler *profiler = this;

	for (int i = 0; i < profiler->profilesNum; i++) {
		Profile *prof = &profiler->profiles[i];
		free(prof->name);
		memset(prof, 0, sizeof(Profile));
	}

	profiler->profilesNum = 0;
}

void Profiler::startProfile(const char *name) {
	Profiler *profiler = this;

	NanoTime time;

	Profile *prof = &profiler->profiles[profiler->profilesNum++];
	prof->name = stringClone(name);
	getNanoTime(&prof->startTime);
}

void Profiler::endProfile(const char *name) {
	Profiler *profiler = this;

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

float Profiler::getMsResult(const char *name) {
	Profiler *profiler = this;

	NanoTime time;
	profiler->getResult(name, &time);

	float ms = time.seconds*1000 + time.nanos/1000000.0;
	return ms;
}

void Profiler::getResult(const char *name, NanoTime *time) {
	Profiler *profiler = this;

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
	Profiler *profiler = this;

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
