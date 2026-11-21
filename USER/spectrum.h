#ifndef __SPECTRUM_H
#define __SPECTRUM_H

#include "stm32f10x.h"

#define SPECTRUM_BAR_COUNT 16

void Spectrum_ComputeBars(const u8 samples[128], u8 bars[SPECTRUM_BAR_COUNT]);

#endif
