#pragma once

#include <Arduino.h>

namespace Disaster {

// Future CanSat integration boundary. No transport or sensor implementation yet.
struct CanSatSample {
  float altitudeMeters = 0;
  float temperatureC = 0;
  float pressureHpa = 0;
  uint32_t timestampMs = 0;
};

class CanSatInterface {
 public:
  bool begin() { return false; }
  bool available() const { return false; }
  bool readSample(CanSatSample&) { return false; }
};

} // namespace Disaster
