#include "tof_inf.h"

#include <Arduino.h>

#include "../common/fw_errors.h"

// Constantes del mock
static const uint32_t MOCK_PERIOD_MS = 33;  // ~30 Hz
static const uint16_t MOCK_MIN_MM = 500;
static const uint16_t MOCK_MAX_MM = 600;
static const uint16_t MOCK_STEP_MM = 2;
// Error cada 20 lecturas simuladas
static const uint32_t MOCK_ERR_EVERY_N = 20;

// Estado
static bool s_initialized = false;
static bool s_new_data = false;
static uint16_t s_distance_mm = MOCK_MIN_MM;  // última lectura válida
static int8_t s_direction = 1;
static uint32_t s_last_ms = 0;
static uint32_t s_call_count = 0;

int tof_inf_begin() {
  s_initialized = true;
  s_new_data = false;
  s_distance_mm = MOCK_MIN_MM;
  s_direction = 1;
  s_call_count = 0;
  s_last_ms = millis();
  return FW_ERR_OK;
}

int tof_inf_update() {
  s_new_data = false;

  if (!s_initialized) {
    return FW_ERR_NOT_INIT;
  }

  uint32_t now = millis();
  if ((uint32_t)(now - s_last_ms) < MOCK_PERIOD_MS) {
    return FW_ERR_OK;  // aún no toca lectura nueva
  }
  s_last_ms = now;
  s_call_count++;

  // Error simulado: se conserva la última lectura válida
  if ((s_call_count % MOCK_ERR_EVERY_N) == 0) {
    return FW_ERR_BAD_DATA;
  }

  // Valor que sube y baja lentamente entre MIN y MAX
  int32_t next = (int32_t)s_distance_mm + (int32_t)s_direction * MOCK_STEP_MM;
  if (next >= MOCK_MAX_MM) {
    next = MOCK_MAX_MM;
    s_direction = -1;
  }
  if (next <= MOCK_MIN_MM) {
    next = MOCK_MIN_MM;
    s_direction = 1;
  }
  s_distance_mm = (uint16_t)next;

  s_new_data = true;
  return FW_ERR_OK;
}

bool tof_inf_new_data() { return s_new_data; }

uint16_t tof_inf_get_mm() { return s_distance_mm; }
