#pragma once

#include <fw_errors.h>
#include <stddef.h>
#include <stdint.h>

// Camara (camera). Todas devuelven los codigos de fw_errors.h.

int camera_begin();   // inicializa la interfaz y los buferes. Una vez en setup()
int camera_update();  // en cada loop(); nunca espera. 0 tambien si no hay frame nuevo
int camera_get_frame(uint8_t* buffer, size_t buffer_size);  // copia el ultimo frame completo
int camera_get_frame_size(size_t& size);                    // bytes del ultimo frame
int camera_get_resolution(uint32_t& width, uint32_t& height);
int camera_get_signal_status(bool& signal_detected);
