#ifndef ULTRASONIC_H
#define ULTRASONIC_H

/*
  ==============================================================================
  Canonical Driver: Ultrasonic Sensor

  Important:
  - Uses the same direct pulseIn() measurement logic proven in the isolated
    function tests.
  - trigger() performs one measurement and stores the result.
  - isReady() becomes true after trigger() completes.
  - getDistance() returns the stored distance in cm, or -1 if invalid.
  - readDistance() is a convenience wrapper: trigger() + getDistance().
  ==============================================================================
*/

#include <Arduino.h>

class Ultrasonic {
private:
    int trigPin;
    int echoPin;

    unsigned long echoDuration;
    bool echoReady;

public:
    Ultrasonic(int trig, int echo);
    void trigger();
    bool isReady() const;
    float getDistance();
    float readDistance();
};

#endif
