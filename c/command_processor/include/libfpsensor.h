#ifndef __LIBFPSENSOR_H__
#define __LIBFPSENSOR_H__

#include "libfpsensordef.h"

void sensorFree(void);
int sensorInit(void);
void *sensorOpen(int);
int sensorClose(void *handle);

int sensorCapture(void *handle, uint8_t *buf, uint32_t size);

int sensorSetImageParams(void *handle, silk_image_params_t *img_params);

int sensorLedOn(void *handle, led_type_t led, uint32_t time);

int sensorAppSlkSetMode(void *handle, uint32_t mode);


int sensorSetBufferPatternSize(void *handle, uint32_t size);


int sensorSetBufferPattern(void *handle, uint32_t pattern);


int sensorGetPatternBuffer(void *handle, uint8_t *buf, uint32_t size);

#endif /* __LIBFPSENSOR_H__ */
