#include <Arduino.h>
#include <fw_errors.h>
#include <Wire.h>

void setup_i2c() {
    Wire.begin(33, 32);
    // Llama aquí a la inicialización de tu multiplexor, por ejemplo:
    // mux.begin();
}

static void setup_imu() {}
static void loop_imu() {}

static void setup_tof_cf() {}
static void loop_tof_cf() {}

static void setup_tof_ci() {}
static void loop_tof_ci() {}

static void setup_altimeter() {}
static void loop_altimeter() {}

static void setup_temp() {}
static void loop_temp() {}

static void setup_cam() {}
static void loop_cam() {}

// ... one pair per component ...

void setup() {
  Serial.begin(115200);
  setup_i2c();
  setup_imu();
  setup_tof_cf();
  setup_tof_ci();
  setup_altimeter();
  setup_temp();
  setup_cam();
  // ...
  Serial.println("FPF-LAB READY");   // must be the LAST line of setup()
}

void loop() {
  loop_imu();
  loop_tof_cf();
  loop_tof_ci();
  loop_altimeter();
  loop_temp();
  loop_cam();
  // ...
}