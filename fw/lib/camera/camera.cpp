#include "camera.h"

#include <Arduino.h>
#include <string.h>

// Mock: no hay DSP. Publica un frame en escala de grises de 160x120 por segundo,
// un degradado que se desplaza en cada frame.
// Uno de cada 20 frames falla con FW_ERR_BAD_DATA para probar el camino de error.

namespace {
const uint32_t PERIODO_MS = 1000;
const uint32_t CADA_ERROR = 20;
const uint32_t ANCHO = 160;
const uint32_t ALTO = 120;
const size_t BYTES = ANCHO * ALTO;

uint8_t frame[BYTES];  // bufer fijo, sin malloc
bool iniciado = false;
bool hay_frame = false;
uint32_t frames = 0;
uint32_t ultimo_ms = 0;
}  // namespace

int camera_begin() {
  iniciado = true;
  hay_frame = false;
  frames = 0;
  ultimo_ms = millis();
  return FW_ERR_OK;
}

int camera_update() {
  if (!iniciado) return FW_ERR_NOT_INIT;

  const uint32_t ahora = millis();
  if (ahora - ultimo_ms < PERIODO_MS) return FW_ERR_OK;  // todavia no toca
  ultimo_ms = ahora;

  const uint32_t n = frames++;
  if ((n + 1) % CADA_ERROR == 0) return FW_ERR_BAD_DATA;  // se conserva el frame anterior

  for (size_t i = 0; i < BYTES; i++) frame[i] = static_cast<uint8_t>((i % ANCHO) + n);
  hay_frame = true;
  return FW_ERR_OK;
}

int camera_get_frame(uint8_t* buffer, size_t buffer_size) {
  if (buffer == NULL || buffer_size < BYTES) return FW_ERR_BAD_PARAM;
  if (!hay_frame) return FW_ERR_NO_DATA;
  memcpy(buffer, frame, BYTES);
  return FW_ERR_OK;
}

int camera_get_frame_size(size_t& size) {
  if (!hay_frame) return FW_ERR_NO_DATA;
  size = BYTES;
  return FW_ERR_OK;
}

int camera_get_resolution(uint32_t& width, uint32_t& height) {
  if (!hay_frame) return FW_ERR_NO_DATA;
  width = ANCHO;
  height = ALTO;
  return FW_ERR_OK;
}

int camera_get_signal_status(bool& signal_detected) {
  if (!iniciado) return FW_ERR_NOT_INIT;
  signal_detected = true;
  return FW_ERR_OK;
}
