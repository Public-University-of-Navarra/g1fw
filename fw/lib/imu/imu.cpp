typedef struct {
    float ax, ay, az;   // aceleración en g (1 g = 9,81 m/s², la aceleración de la gravedad)
    float gx, gy, gz;   // velocidad de giro en °/s
} imu_data_t;

int imu