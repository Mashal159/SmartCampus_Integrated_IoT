#include <Arduino.h>

#define PARKING_US1_TRIG 21
#define PARKING_US1_ECHO 4
#define PARKING_US2_TRIG 26
#define PARKING_US2_ECHO 25
#define PRINT_MS 500UL

static unsigned long lastPrint = 0;

float readDistance(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 18000);
    if (duration == 0) return -1;

    float distance = duration * 0.034f / 2.0f;
    if (distance <= 0 || distance > 300) return -1;
    return distance;
}

void setup() {
    Serial.begin(115200);
    pinMode(PARKING_US1_TRIG, OUTPUT);
    pinMode(PARKING_US1_ECHO, INPUT);
    pinMode(PARKING_US2_TRIG, OUTPUT);
    pinMode(PARKING_US2_ECHO, INPUT);
    Serial.println("[Parking Test] ultrasonic1=(21,4), ultrasonic2=(26,25)");
}

void loop() {
    unsigned long now = millis();
    if (now - lastPrint < PRINT_MS) return;
    lastPrint = now;

    float d1 = readDistance(PARKING_US1_TRIG, PARKING_US1_ECHO);
    float d2 = readDistance(PARKING_US2_TRIG, PARKING_US2_ECHO);

    Serial.print("[Parking] P1_cm=");
    Serial.print(d1, 1);
    Serial.print(" P2_cm=");
    Serial.println(d2, 1);
}
