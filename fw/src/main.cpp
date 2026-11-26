#include <Arduino.h>
#include <Wire.h>
#include <fw_errors.h>

// Tareas de FreeRTOS (ver "Ejecucion con FreeRTOS" en fw/README.md):
// - tarea_i2c: la unica que usa Wire y el multiplexor. Llama a los loop_X() de los sensores I2C
//   sin parar; cada libreria decide con millis() si le toca leer, asi que ninguna espera.
// - tarea_temp: la sonda 1-Wire, en el otro nucleo, porque no comparte bus con nadie.
// Cada componente solo rellena su setup_X() / loop_X(), igual que antes.

namespace {
const uint32_t I2C_HZ = 400000;  // maximo del TCA9548A
const uint32_t PILA = 8192;      // bytes de pila por tarea
const UBaseType_t PRIO_I2C = 3;  // las colisiones son lo urgente
const UBaseType_t PRIO_TEMP = 1;
const BaseType_t NUCLEO_I2C = 1;
const BaseType_t NUCLEO_TEMP = 0;
}  // namespace

static void setup_i2c() {
  Wire.begin(33, 32);
  Wire.setClock(I2C_HZ);
}
static void setup_mult() {}
static void loop_mult() {}

static void setup_imu() {}
static void loop_imu() {}

static void setup_tof_front() {}
static void loop_tof_front() {}

static void setup_tof_inf() {}
static void loop_tof_inf() {}

static void setup_bar() {}
static void loop_bar() {}

static void setup_temp() {}
static void loop_temp() {}

static void setup_camera() {}
static void loop_camera() {}

// ... one pair per component ...

static void tarea_i2c(void*) {
  for (;;) {
    // primero lo mas urgente: colision frontal e inferior
    loop_tof_front();
    loop_tof_inf();
    loop_imu();
    loop_bar();
    loop_mult();
    vTaskDelay(1);  // 1 ms: deja correr a las demas tareas y al watchdog
  }
}

static void tarea_temp(void*) {
  for (;;) {
    loop_temp();
    vTaskDelay(pdMS_TO_TICKS(50));  // la sonda tarda 750 ms en convertir; no hace falta mas
  }
}

void setup() {
  Serial.begin(115200);
  setup_i2c();
  setup_mult();
  setup_imu();
  setup_tof_front();
  setup_tof_inf();
  setup_bar();
  setup_temp();
  setup_camera();
  // ...
  xTaskCreatePinnedToCore(tarea_i2c, "i2c", PILA, NULL, PRIO_I2C, NULL, NUCLEO_I2C);
  xTaskCreatePinnedToCore(tarea_temp, "temp", PILA, NULL, PRIO_TEMP, NULL, NUCLEO_TEMP);
  Serial.println("FPF-LAB READY");  // must be the LAST line of setup()
}

void loop() {
  loop_camera();  // la camara sigue en loop() hasta que se cierre su interfaz con el DSP
  // ...
}
