//
//  Timer.h
//  DoomFire
//
//  Created by Алексей Аверченко on 26/06/2019.
//  Copyright © 2019 a-square. All rights reserved.
//

#pragma once

#include "Ints.h"

enum {
    // need a lot of samples to counteract the huge variance
    // that I encountered running on a physical iPhone 7
    TIMER_NUM_SAMPLES = 60 * 60,  // 1 minute @ 60 Hz
};

void timerSystemInit(void);
double timerMillis(void);

typedef struct {
    double samples[TIMER_NUM_SAMPLES];
    const char* name;
    u32 index;
} Timer;

void timerInit(Timer*, const char* name);
void timerUpdate(Timer* timer, double sample);
void reportTimer(Timer* timer);
