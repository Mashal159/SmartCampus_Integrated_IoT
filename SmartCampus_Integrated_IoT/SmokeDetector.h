#ifndef SMOKE_DETECTOR_H
#define SMOKE_DETECTOR_H

/*
  ==============================================================================
  Module: Smoke Detector

  Public Interface:
  - setupSmokeDetector()
  - loopSmokeDetector()
  - test_SmokeDetector()

  Description:
  - Reads MQ2 smoke/gas sensor.
  - Posts alarm activation to server.
  - Polls server alarm state.
  - Drives LED and buzzer according to database state.
  ==============================================================================
*/

void setupSmokeDetector();
void loopSmokeDetector();
void test_SmokeDetector();

#endif