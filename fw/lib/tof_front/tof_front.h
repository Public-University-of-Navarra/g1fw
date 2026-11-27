#pragma once

#include <stdbool.h>
#include <stdint.h>

#define TOF_FRONT_NO_TARGET 0xFFFF  // marcador de "nada en rango"

#ifdef __cplusplus
extern "C" {
#endif

int tof_front_begin();
int tof_front_update();
bool tof_front_new_data();
uint16_t tof_front_get_min_mm();
bool tof_front_obstacle(uint16_t threshold_mm);
uint16_t tof_front_get_zone_mm(uint8_t row, uint8_t col);
uint32_t tof_front_get_age_ms();

#ifdef __cplusplus
}
#endif