#include "camera.h"

#include <Arduino.h>
#include <esp_timer.h>

// Mock: no hay DSP. Da una orden cada 33 ms (30 imagenes/s) y va recorriendo las 7 ordenes,
// cada una durante 10 imagenes. Una de cada 50 llega mal (FW_ERR_BAD_DATA).

namespace {
const uint32_t PERIODO_MS = 33;
const uint32_t IMAGENES_POR_ORDEN = 10;
const uint32_t ORDENES = 7;
const uint32_t CADA_ERROR = 50;

bool iniciado = false;
bool nueva = false;
bool hay_dato = false;
camera_result_t ultimo = {0, 0, CAMERA_QUIETO};
uint32_t imagenes = 0;
uint32_t ultimo_ms = 0;
}  // namespace

int camera_begin() {
  iniciado = true;
  hay_dato = false;
  imagenes = 0;
  ultimo_ms = millis();
  return FW_ERR_OK;
}

int camera_update() {
  nueva = false;
  if (!iniciado) return FW_ERR_NOT_INIT;

  const uint32_t ahora = millis();
  if (ahora - ultimo_ms < PERIODO_MS) return FW_ERR_OK;  // todavia no toca
  ultimo_ms = ahora;

  const uint32_t n = imagenes++;
  if ((n + 1) % CADA_ERROR == 0) return FW_ERR_BAD_DATA;  // se conserva el resultado anterior

  ultimo.t_ms = ahora;
  ultimo.t_rx_us = esp_timer_get_time();
  ultimo.cmd = static_cast<camera_cmd_t>((n / IMAGENES_POR_ORDEN) % ORDENES);
  hay_dato = true;
  nueva = true;
  return FW_ERR_OK;
}

bool camera_new_data() { return nueva; }

int camera_get_result(camera_result_t* out) {
  if (out == NULL) return FW_ERR_BAD_PARAM;
  if (!hay_dato) return FW_ERR_NO_DATA;
  *out = ultimo;
  return FW_ERR_OK;
}
