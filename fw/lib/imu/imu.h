#pragma once

typedef struct {
    float ax, ay, az;   // aceleración en g (1 g = 9,81 m/s², la aceleración de la gravedad)
    float gx, gy, gz;   // velocidad de giro en °/s
} imu_data_t;

int  imu_begin();                     // 0 = OK, si no, código de error. Una vez en setup()
int  imu_update();                    // en cada loop(); nunca espera. 0 también si no hay dato nuevo
bool imu_new_data();                  // true si el último imu_update() leyó un dato nuevo
int  imu_get_state(imu_data_t *out);  // copia en *out el último dato válido. FW_ERR_BAD_PARAM si out es NULL
