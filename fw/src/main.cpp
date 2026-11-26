#include <Arduino.h>
#include <Wire.h>
#include <fw_errors.h>
#include <mult.h>

static void setup_i2c() { Wire.begin(33, 32); }
static void setup_mult() {
  const int err = mult_begin();
  if (err != FW_ERR_OK) Serial.printf("[mult] ERR code=%d\n", err);
}
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
  Serial.println("FPF-LAB READY");  // must be the LAST line of setup()
}

void loop() {
  loop_mult();
  loop_imu();
  loop_tof_front();
  loop_tof_inf();
  loop_bar();
  loop_temp();
  loop_camera();
  // ...
}
