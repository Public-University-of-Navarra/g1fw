#include <Arduino.h>
#include <fw_errors.h>

static void setup_imu() {}
static void loop_imu() {}

static void setup_tof_cf(){}
static void loop_tof_cf(){}

static void setup_tof_ci(){}
static void loop_tof_ci(){}

static void setup_altimeter(){}
static void loop_altimeter(){}

static void setup_temp(){}
static void loop_temp(){}

static void setup_CAM(){}
static void loop_CAM(){}

static void setup_I2C(){}
static void loop_I2C(){}

static void setup_mult(){}
static void loop_mult(){}

// ... one pair per component ...

void setup() {
  Serial.begin(115200);
  setup_imu();
  setup_tof_cf();
  setup_tof_ci();
  setup_altimeter();
  setup_temp();
  setup_CAM();
  setup_I2C();
  setup_mult();
  // ...
  Serial.println("FPF-LAB READY");   // must be the LAST line of setup()
}

void loop() {
  loop_imu();
  loop_tof_cf();
  loop_tof_ci();
  loop_altimeter();
  loop_temp();
  loop_CAM();
  loop_I2C();
  loop_mult();
  // ...
}