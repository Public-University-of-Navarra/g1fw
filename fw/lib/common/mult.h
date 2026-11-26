#pragma once

#include <fw_errors.h>

// Multiplexor I2C TCA9548A (Adafruit 2717) en 0x70. Devuelven los codigos de fw_errors.h.
// Wire.begin() tiene que estar hecho antes de mult_begin().

int mult_begin(void);          // comprueba que responde y desactiva todos los canales
int mult_select(int channel);  // activa solo ese canal (0..7). Llamar antes de cada transferencia
int mult_disable_all(void);    // desactiva todos los canales
