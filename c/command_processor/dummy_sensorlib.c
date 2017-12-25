#include <stdio.h>

#include "libfpsensordef.h"
#include "libfpsensor.h"

int sensorClose(void *handle)
{
	return 0;
}

void sensorFree(void)
{
}

int sensorInit(void)
{
	return 0;
}

void *sensorOpen(int a)
{
	return NULL;
}

int sensorCapture(void *handle, uint8_t *buf, uint32_t size)
{
	return 0;
}

int sensorSetImageParams(void *handle, silk_image_params_t *img_params)
{
	return 0;
}

int sensorLedOn(void *handle, led_type_t led, uint32_t time)
{
	return 0;
}

int sensorAppSlkSetMode(void *handle, uint32_t mode)
{
	return 0;
}

int sensorSetBufferPatternSize(void *handle, uint32_t size)
{
	return 0;
}

int sensorSetBufferPattern(void *handle, uint32_t pattern)
{
	return 0;
}

int sensorGetPatternBuffer(void *handle, uint8_t *buf, uint32_t size)
{
	return 0;
}


