//
//  FireBuffer.h
//  DoomFire
//
//  Created by Алексей Аверченко on 23/06/2019.
//  Copyright © 2019 a-square. All rights reserved.
//

#pragma once

#include "Ints.h"

i32* createFireBuffer(u32 width, u32 height);
void destroyFireBuffer(i32* buffer);

void updateFireBufferUnrolled(i32* restrict buffer, u32 width, u32 height);

#define updateFireBuffer updateFireBufferUnrolled
