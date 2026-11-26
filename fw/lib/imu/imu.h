#pragma once

#include <fw_errors.h>

// IMU (imu, ICM-40609-D). Devuelven los codigos de fw_errors.h.

typedef struct {
  float ax, ay, az;  // aceleracion en g
  float gx, gy, gz;  // velocidad de giro en grados/s
} imu_data_t;

int imu_begin();                     // 0 = OK, si no, codigo de error. Una vez en setup()
int imu_update();                    // en cada loop(); nunca espera. 0 tambien si no hay dato nuevo
bool imu_new_data();                 // true si el ultimo imu_update() leyo un dato nuevo
int imu_get_state(imu_data_t* out);  // copia el ultimo dato valido. FW_ERR_BAD_PARAM si out es NULL
