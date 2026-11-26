#include "tof_front.h"

#include <Arduino.h>
#include <fw_errors.h>

namespace {

constexpr uint8_t kGridRows = 4;
constexpr uint8_t kGridCols = 4;
constexpr uint16_t kMinValidMm = 20;
constexpr uint16_t kMaxValidMm = 4000;
constexpr uint32_t kFrameIntervalMs = 33; // ~30 Hz
constexpr uint32_t kErrorEveryNFrames = 20;
constexpr uint32_t kNoTargetEveryNFrames = 7;

// Patrón base de escena interior (mm): pasillo/pared frontal con variación por
// zona
constexpr int16_t kInteriorOffsetsMm[kGridRows][kGridCols] = {
    {220, 140, 140, 220},
    {120, 0, -25, 110},
    {130, -15, -35, 120},
    {250, 160, 160, 250},
};

bool g_initialized = false;
bool g_has_valid_frame = false;
bool g_new_data = false;

uint32_t g_frame_count = 0;
uint32_t g_last_update_ms = 0;
uint32_t g_last_frame_ms = 0;

uint16_t g_zones_mm[kGridRows][kGridCols] = {};
uint16_t g_min_mm = TOF_FRONT_NO_TARGET;

uint16_t g_center_dist_mm = 1500;
int16_t g_approach_step_mm =
    -45; // Acercamiento progresivo hacia obstáculo frontal

bool is_valid_distance_mm(uint16_t mm) {
  return mm >= kMinValidMm && mm <= kMaxValidMm;
}

void recompute_min_mm() {
  uint16_t current_min = TOF_FRONT_NO_TARGET;
  for (uint8_t r = 0; r < kGridRows; ++r) {
    for (uint8_t c = 0; c < kGridCols; ++c) {
      uint16_t val = g_zones_mm[r][c];
      if (is_valid_distance_mm(val) && val < current_min) {
        current_min = val;
      }
    }
  }
  g_min_mm = current_min;
}

void generate_next_frame(uint32_t now_ms, uint32_t frame_num) {
  int32_t next_center =
      static_cast<int32_t>(g_center_dist_mm) + g_approach_step_mm;
  if (next_center <= 260) {
    next_center = 260;
    g_approach_step_mm = 60; // Simula maniobra/evasión alejándose del obstáculo
  } else if (next_center >= 1800) {
    next_center = 1800;
    g_approach_step_mm = -45; // Vuelve a avanzar en espacio interior
  }
  g_center_dist_mm = static_cast<uint16_t>(next_center);

  for (uint8_t r = 0; r < kGridRows; ++r) {
    for (uint8_t c = 0; c < kGridCols; ++c) {
      // Simula una esquina superior sin retorno válido en rango (ej.
      // techo/reflejo)
      if (r == 0 && c == 3 && (frame_num % kNoTargetEveryNFrames == 0)) {
        g_zones_mm[r][c] = TOF_FRONT_NO_TARGET;
        continue;
      }

      int32_t zone_mm =
          static_cast<int32_t>(g_center_dist_mm) + kInteriorOffsetsMm[r][c];
      if (zone_mm < static_cast<int32_t>(kMinValidMm)) {
        zone_mm = kMinValidMm;
      } else if (zone_mm > static_cast<int32_t>(kMaxValidMm)) {
        zone_mm = kMaxValidMm;
      }
      g_zones_mm[r][c] = static_cast<uint16_t>(zone_mm);
    }
  }

  recompute_min_mm();
  g_has_valid_frame = true;
  g_new_data = true;
  g_last_frame_ms = now_ms;
}

} // namespace

int tof_front_begin() {
  uint32_t now_ms = millis();
  g_initialized = true;
  g_has_valid_frame = false;
  g_new_data = false;
  g_frame_count = 0;
  g_last_update_ms = now_ms;
  g_last_frame_ms = now_ms;
  g_min_mm = TOF_FRONT_NO_TARGET;
  g_center_dist_mm = 1500;
  g_approach_step_mm = -45;

  for (uint8_t r = 0; r < kGridRows; ++r) {
    for (uint8_t c = 0; c < kGridCols; ++c) {
      g_zones_mm[r][c] = TOF_FRONT_NO_TARGET;
    }
  }

  return FW_ERR_OK;
}

int tof_front_update() {
  g_new_data = false;
  if (!g_initialized) {
    return FW_ERR_NOT_INIT;
  }

  uint32_t now_ms = millis();
  if ((now_ms - g_last_update_ms) >= kFrameIntervalMs || !g_has_valid_frame) {
    g_last_update_ms = now_ms;
    ++g_frame_count;

    if (g_frame_count % kErrorEveryNFrames == 0) {
      return FW_ERR_TIMEOUT;
    }

    generate_next_frame(now_ms, g_frame_count);
  }

  return FW_ERR_OK;
}

bool tof_front_new_data() { return g_new_data; }

uint16_t tof_front_get_min_mm() {
  if (!g_initialized || !g_has_valid_frame) {
    return TOF_FRONT_NO_TARGET;
  }
  return g_min_mm;
}

bool tof_front_obstacle(uint16_t threshold_mm) {
  uint16_t min_mm = tof_front_get_min_mm();
  if (min_mm == TOF_FRONT_NO_TARGET) {
    return false;
  }
  return min_mm < threshold_mm;
}

uint16_t tof_front_get_zone_mm(uint8_t row, uint8_t col) {
  if (!g_initialized || !g_has_valid_frame) {
    return TOF_FRONT_NO_TARGET;
  }
  if (row >= kGridRows || col >= kGridCols) {
    return TOF_FRONT_NO_TARGET;
  }
  return g_zones_mm[row][col];
}

uint32_t tof_front_get_age_ms() {
  if (!g_initialized || !g_has_valid_frame) {
    return UINT32_MAX;
  }
  return millis() - g_last_frame_ms;
}