#ifndef __LIBFPSENSORDEF_H__
#define __LIBFPSENSORDEF_H__

#include <stdint.h>

typedef enum {
	CAP_MODE_1,
	CAP_MODE_2,
} silk_capture_mode_t;

typedef enum {
	DPI_VALUE_1,
	DPI_VALUE_2,
} silk_dpi_values_t;

typedef enum {
    WHITE_LED, 
    GREEN_LED, 
    RED_LED, 
    WHITE_BLINK, 
    WHITE_BREATH, 
} led_type_t;

typedef enum {
	SILK_ENC_TYPE_NONE,
	SILK_ENC_TYPE_CBC,
	SILK_ENC_TYPE_ECB,
} silk_enc_type_t;

typedef struct {
	int enable_antifake;
	silk_dpi_values_t dpi_value;
	silk_capture_mode_t capture_mode;
	silk_enc_type_t enc_type;
} silk_image_params_t;

#endif /*__LIBFPSENSORDEF_H__ */
