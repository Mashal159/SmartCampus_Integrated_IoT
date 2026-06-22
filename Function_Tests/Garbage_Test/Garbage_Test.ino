#include <Arduino.h>

#define GARBAGE_US_TRIG 23
#define GARBAGE_US_ECHO 22
#define PRINT_MS 500UL

static unsigned long lastPrint = 0;

float readDistance() {
    digitalWrite(GARBAGE_US_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(GARBAGE_US_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(GARBAGE_US_TRIG, LOW);

    long duration = pulseIn(GARBAGE_US_ECHO, HIGH, 18000);
    if (duration == 0) return -1;

    float distance = duration * 0.034f / 2.0f;
    if (distance <= 0 || distance > 300) return -1;
    return distance;
}

void setup() {
    Serial.begin(115200);
    pinMode(GARBAGE_US_TRIG, OUTPUT);
    pinMode(GARBAGE_US_ECHO, INPUT);
    Serial.println("[Garbage Test] ultrasonic=(23,22)");
}

void loop() {
    unsigned long now = millis();
    if (now - lastPrint < PRINT_MS) return;
    lastPrint = now;

    float distance = readDistance();
    Serial.print("[Garbage] distance_cm=");
    Serial.println(distance, 1);
}
