//
//  Timer.c
//  DoomFire
//
//  Created by Алексей Аверченко on 26/06/2019.
//  Copyright © 2019 a-square. All rights reserved.
//

#include "Timer.h"

#include <mach/mach_time.h>
#include <math.h>
#include <stdio.h>

static mach_timebase_info_data_t s_timebase_info;

void timerSystemInit(void) {
    mach_timebase_info(&s_timebase_info);
}

double timerMillis(void) {
    // https://stackoverflow.com/a/4753909
    return mach_absolute_time() * s_timebase_info.numer / (1000000.0 * s_timebase_info.denom);
}

void timerInit(Timer* timer, const char* name) {
    for (size_t i = 0; i < TIMER_NUM_SAMPLES; ++i) {
        timer->samples[i] = NAN;
    }
    timer->name = name;
    timer->index = 0;
}

void timerUpdate(Timer* timer, double sample) {
    timer->samples[timer->index] = sample;
    ++timer->index;

    if (timer->index == TIMER_NUM_SAMPLES) {
        timer->index = 0;
        reportTimer(timer);
    }
}

void reportTimer(Timer* timer) {
    double sum = 0;
    for (size_t i = 0; i < TIMER_NUM_SAMPLES; ++i) {
        sum += timer->samples[i];
    }
    double mean = sum / TIMER_NUM_SAMPLES;

    double varSum = 0;
    for (size_t i = 0; i < TIMER_NUM_SAMPLES; ++i) {
        double centeredSample = timer->samples[i] - mean;
        varSum += centeredSample * centeredSample;
    }
    double std = sqrt(varSum / (TIMER_NUM_SAMPLES - 1));

    fprintf(stderr, "%s: mean = %f ms, std = %f ms\n", timer->name, mean, std);
}
