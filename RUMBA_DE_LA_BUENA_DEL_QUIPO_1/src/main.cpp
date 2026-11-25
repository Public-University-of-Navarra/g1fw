#include <Arduino.h>
#include <fw_errors.h>

static void setupImu() {}
static void loopImu() {}

static void setupTOF_cf(){}
static void loopTOF_cf(){}

static void setupvl53l1x(){}
static void loopvl53l1x(){}

static void setupaltimeter(){}
static void loopaltimeter(){}

static void setupDS18820(){}
static void loopDS18820(){}
// ... one pair per component ...

void setup() {
  Serial.begin(115200);
  setupImu();
  setupTOF_cf();
  setupvl53l1x();
  setupaltimeter();
  setupDS18820();
  // ...
  Serial.println("FPF-LAB READY");   // must be the LAST line of setup()
}

void loop() {
  loopImu();
  loopTOF_cf();
  loopvl53l1x();
  loopaltimeter();
  loopDS18820();
  // ...
}