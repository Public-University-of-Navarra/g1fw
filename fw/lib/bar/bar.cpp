
#include "bar.h"

#include <math.h>

#include "../common/fw_errors.h"

// Variables internas
static bool inicializado = false;
static bool calibrado = false;

static float altura = NAN;
static int contador = 0;

// Inicializacion
int bar_begin() {
  inicializado = true;
  calibrado = false;

  altura = NAN;
  contador = 0;

  return FW_ERR_OK;
}

// Actualizacion de los datos simulados
int bar_update() {
  if (!inicializado) {
    return FW_ERR_NOT_INIT;
  }

  contador++;

  // Simular error cada 20 llamadas
  if (contador % 20 == 0) {
    return FW_ERR_TIMEOUT;
  }

  // Primera medida: calibracion
  if (!calibrado) {
    altura = 0.0f;
    calibrado = true;

    return FW_ERR_OK;
  }

  // Simular pequenas variaciones de altura
  altura = ((contador % 5) - 2) * 0.01f;

  return FW_ERR_OK;
}

// Obtener la ultima altura valida
float bar_get_height_m() {
  return altura;
}

// Estado de calibracion
bool bar_is_calibrated() {
  return calibrado;
}
