#include "imu.h"

#include <Arduino.h>

// Mock: no habla con el sensor. Da 10 lecturas por segundo de un dron quieto que se balancea:
// az cerca de 1 g y giros pequenos que cambian de signo.
// Una de cada 50 lecturas falla con FW_ERR_BAD_DATA para probar el camino de error.

namespace {
const uint32_t PERIODO_MS = 100;
const uint32_t CADA_ERROR = 50;

bool iniciado = false;
bool nueva = false;
bool hay_dato = false;
imu_data_t ultimo = {0, 0, 0, 0, 0, 0};
uint32_t lecturas = 0;
uint32_t ultimo_ms = 0;
}  // namespace

int imu_begin() {
  iniciado = true;
  hay_dato = false;
  lecturas = 0;
  ultimo_ms = millis();
  return FW_ERR_OK;
}

int imu_update() {
  nueva = false;
  if (!iniciado) return FW_ERR_NOT_INIT;

  const uint32_t ahora = millis();
  if (ahora - ultimo_ms < PERIODO_MS) return FW_ERR_OK;  // todavia no toca
  ultimo_ms = ahora;

  const uint32_t n = lecturas++;
  if ((n + 1) % CADA_ERROR == 0) return FW_ERR_BAD_DATA;

  const int32_t paso = static_cast<int32_t>(n % 20) - 10;  // -10..9
  ultimo.ax = paso / 100.0f;                               // -0,10..0,09 g
  ultimo.ay = -paso / 100.0f;
  ultimo.az = 1.0f;
  ultimo.gx = paso * 2.0f;  // -20..18 grados/s
  ultimo.gy = -paso * 2.0f;
  ultimo.gz = 0.0f;
  hay_dato = true;
  nueva = true;
  return FW_ERR_OK;
}

bool imu_new_data() { return nueva; }

int imu_get_state(imu_data_t *out) {
  if (out == NULL) return FW_ERR_BAD_PARAM;
  if (!hay_dato) return FW_ERR_NO_DATA;
  *out = ultimo;
  return FW_ERR_OK;
}
