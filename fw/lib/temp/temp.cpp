#include "temp.h"

#include <Arduino.h>

// Mock: no habla con la sonda. Da una lectura por segundo, como la sonda real,
// que sube de 21,0 a 23,0 C y vuelve a bajar en pasos de 0,1 C.
// Una de cada 20 lecturas falla con FW_ERR_BAD_DATA para probar el camino de error.

namespace {
const uint32_t PERIODO_MS = 1000;
const uint32_t CADA_ERROR = 20;

bool iniciado = false;
bool nueva = false;
float ultima_c = NAN;
uint32_t lecturas = 0;
uint32_t ultimo_ms = 0;
}  // namespace

int temp_begin() {
  iniciado = true;
  lecturas = 0;
  ultimo_ms = millis();
  return FW_ERR_OK;
}

int temp_update() {
  nueva = false;
  if (!iniciado) return FW_ERR_NOT_INIT;

  const uint32_t ahora = millis();
  if (ahora - ultimo_ms < PERIODO_MS) return FW_ERR_OK;  // todavia no toca
  ultimo_ms = ahora;

  const uint32_t n = lecturas++;
  if ((n + 1) % CADA_ERROR == 0) return FW_ERR_BAD_DATA;

  const uint32_t paso = n % 40;                            // 0..39
  const uint32_t decimas = paso <= 20 ? paso : 40 - paso;  // 0..20..1
  ultima_c = (210 + decimas) / 10.0f;
  nueva = true;
  return FW_ERR_OK;
}

bool temp_new_data() { return nueva; }

float temp_get_c() { return ultima_c; }
