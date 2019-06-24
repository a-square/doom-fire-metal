//
//  FireBuffer.h
//  DoomFire
//
//  Created by Алексей Аверченко on 23/06/2019.
//  Copyright © 2019 a-square. All rights reserved.
//

#pragma once

#include "ShaderTypes.h"
#include <inttypes.h>

int32_t* createFireBuffer(uint32_t width, uint32_t height);
void destroyFireBuffer(int32_t* restrict buffer);
void updateFireBuffer(int32_t* restrict buffer, uint32_t width, uint32_t height);
