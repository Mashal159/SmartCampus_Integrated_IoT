#include "Ultrasonic.h"

/*
  ==============================================================================
  Ultrasonic Sensor Driver Implementation

  Hardware:
  - HC-SR04 sensor.
  - Echo pin must be stepped down from 5V to 3.3V before ESP32 GPIO.
  - Measurement logic intentionally matches the proven function-test sketches.
  ==============================================================================
*/

#define ULTRASONIC_TIMEOUT_US 18000UL
#define ULTRASONIC_MAX_CM     300.0f

Ultrasonic::Ultrasonic(int trig, int echo)
    : trigPin(trig),
      echoPin(echo),
      echoDuration(0),
      echoReady(false) {

    pinMode(trigPin, OUTPUT);
    digitalWrite(trigPin, LOW);
    pinMode(echoPin, INPUT);
}

void Ultrasonic::trigger() {
    echoReady = false;
    echoDuration = 0;

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    echoDuration = pulseIn(echoPin, HIGH, ULTRASONIC_TIMEOUT_US);
    echoReady = true;
}

bool Ultrasonic::isReady() const {
    return echoReady;
}

float Ultrasonic::getDistance() {
    if (!echoReady || echoDuration == 0) {
        return -1.0f;
    }

    float distance = (float)echoDuration * 0.034f / 2.0f;

    if (distance <= 0.0f || distance > ULTRASONIC_MAX_CM) {
        return -1.0f;
    }

    return distance;
}

float Ultrasonic::readDistance() {
    trigger();
    return getDistance();
}
