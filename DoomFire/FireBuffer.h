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

uint32_t* createFireBuffer(uint32_t width, uint32_t height);
void destroyFireBuffer(uint32_t* restrict buffer);
void updateFireBuffer(uint32_t* restrict buffer, uint32_t width, uint32_t height);
