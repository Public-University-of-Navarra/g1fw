#pragma once

#include <stdint.h>

// 0 = OK, otro número = error
int tof_inf_begin();
// Llamar en cada loop(); nunca espera
int tof_inf_update();
// true si el último update dejó una lectura nueva
bool tof_inf_new_data();
// Última distancia válida, en mm
uint16_t tof_inf_get_mm();
