#include "mult.h"

#include <Wire.h>

namespace {
const uint8_t MULT_ADDR = 0x70;

int escribir(uint8_t mascara) {
  Wire.beginTransmission(MULT_ADDR);
  Wire.write(mascara);
  return Wire.endTransmission() == 0 ? FW_ERR_OK : FW_ERR_TIMEOUT;
}
}  // namespace

int mult_begin(void) {
  Wire.beginTransmission(MULT_ADDR);
  if (Wire.endTransmission() != 0) return FW_ERR_NOT_FOUND;  // nadie contesta en 0x70
  return mult_disable_all();
}

int mult_select(int channel) {
  if (channel < 0 || channel > 7) return FW_ERR_BAD_PARAM;
  return escribir(static_cast<uint8_t>(1u << channel));
}

int mult_disable_all(void) { return escribir(0); }
