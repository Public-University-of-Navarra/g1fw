#pragma once

#include <fw_errors.h>
#include <stdint.h>

// Camara (camera): orden que manda la DSP por UART tras procesar cada imagen.
// Devuelven los codigos de fw_errors.h.

typedef enum {
  CAMERA_QUIETO = 0,
  CAMERA_ARRIBA = 1,
  CAMERA_ABAJO = 2,
  CAMERA_IZQUIERDA = 3,
  CAMERA_DERECHA = 4,
  CAMERA_ADELANTE = 5,
  CAMERA_ATRAS = 6
} camera_cmd_t;

typedef struct {
  uint32_t t_ms;     // marca de tiempo de la DSP
  int64_t t_rx_us;   // esp_timer_get_time() al recibir la linea en el ESP32
  camera_cmd_t cmd;  // orden de la DSP
} camera_result_t;

int camera_begin();                           // UART con la DSP. Una vez en setup()
int camera_update();                          // en cada loop(); nunca espera
bool camera_new_data();                       // true si el ultimo update() dejo una orden nueva
int camera_get_result(camera_result_t* out);  // FW_ERR_BAD_PARAM si out es NULL
