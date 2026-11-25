#pragma once

#include <fw_errors.h>

// Sensor de temperatura (temp). Devuelven los codigos de fw_errors.h.

int temp_begin();      // 0 = OK, si no, codigo de error. Una vez en setup()
int temp_update();     // en cada loop(); nunca espera. 0 tambien si aun no hay lectura nueva
bool temp_new_data();  // true si el ultimo temp_update() dejo una lectura nueva
float temp_get_c();    // ultima temperatura valida en C (NAN si todavia no hay ninguna)
