#include <math.h>
#include "fixed.h"
#include "sincos.h"
#include "logger.h"

void sincos_init(void)
{
	int i;

	logmsg(LOG_ALL, "calculating sin/cos lut...\n");

	for(i=0; i<SINLUT_SIZE; i++) {
		float angle = 2.0 * M_PI * ((float)i / (float)SINLUT_SIZE);
		float val = sin(angle);
		sinlut[i] = (int16_t)(val * SINLUT_SCALE);
	}
}

int16_t sin_int(int16_t norm_angle)
{
	norm_angle %= SINLUT_SIZE;
	if(norm_angle < 0) {
		norm_angle += SINLUT_SIZE;
	}
	return sinlut[norm_angle];
}

int16_t cos_int(int16_t norm_angle)
{
	return sin_int(norm_angle + SINLUT_SIZE / 4);
}

int32_t sin_x16(int32_t radians)
{
	int32_t na = x16div(radians, M_PI_X16 * 2);
	return (sin_int((na * SINLUT_SIZE) >> 16) << 16) / SINLUT_SCALE;
}

int32_t cos_x16(int32_t radians)
{
	int32_t na = x16div(radians, M_PI_X16 * 2);
	return (cos_int((na * SINLUT_SIZE) >> 16) << 16) / SINLUT_SCALE;
}
