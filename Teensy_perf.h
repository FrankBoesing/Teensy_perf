/* This library code is placed under the GPL-v3.0 license
 * Copyright (c) 2020 Frank Bösing
 */

#if !defined(TeensyPerf)
#define TeensyPerf
#include "Arduino.h"

uint32_t measure(void (*func)(void));
uint32_t measure(void (*func)(void), void (*compensate)(void));
#endif
