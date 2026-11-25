#include "imu.h"
#include "mult.h"
#include <Wire.h>


#define SDA_PIN 21
#define SCL_PIN 22
#define MULT_ADDR 0x76

typedef struct {
    float ax, ay, az;   // aceleración en g (1 g = 9,81 m/s², la aceleración de la gravedad)
    float gx, gy, gz;   // velocidad de giro en °/s
} imu_data_t;

const int IMU_CHANNEL=  5; //Canal del multiplexor correspondiente a la imu.

int imu_begin(){
    mult_select (IMU_CHANNEL);
}

int imu_update(){

}

bool imu_new_data(){

}

int imu_get_state(imu_data_t *out){

}
