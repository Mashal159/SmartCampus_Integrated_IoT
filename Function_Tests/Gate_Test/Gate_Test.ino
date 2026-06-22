#include <Arduino.h>

#define GATE_SERVO_PIN 18
#define GATE_US_TRIG 5
#define GATE_US_ECHO 19

#define GATE_ANGLE_CLOSED 0
#define GATE_ANGLE_OPEN 90
#define GATE_OPEN_TIME_MS 5000UL
#define GATE_OBJECT_CM 25.0f
#define PRINT_MS 500UL

static bool gateOpen = false;
static unsigned long closeAt = 0;
static unsigned long lastPrint = 0;

int angleToDuty(int angle) {
    angle = constrain(angle, 0, 180);
    int pulseWidthUs = map(angle, 0, 180, 500, 2500);
    int maxDuty = (1 << 16) - 1;
    return (pulseWidthUs * maxDuty) / 20000;
}

void moveServo(int angle) {
    ledcWrite(GATE_SERVO_PIN, angleToDuty(angle));
}

float readDistance() {
    digitalWrite(GATE_US_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(GATE_US_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(GATE_US_TRIG, LOW);

    long duration = pulseIn(GATE_US_ECHO, HIGH, 18000);
    if (duration == 0) return -1;

    float distance = duration * 0.034f / 2.0f;
    if (distance <= 0 || distance > 300) return -1;
    return distance;
}

void openGate() {
    moveServo(GATE_ANGLE_OPEN);
    gateOpen = true;
    closeAt = millis() + GATE_OPEN_TIME_MS;
}

void closeGate() {
    moveServo(GATE_ANGLE_CLOSED);
    gateOpen = false;
}

void handleSerial() {
    if (!Serial.available()) return;

    char c = Serial.read();
    if (c == 'o') openGate();
    if (c == 'c') closeGate();
}

void setup() {
    Serial.begin(115200);
    pinMode(GATE_US_TRIG, OUTPUT);
    pinMode(GATE_US_ECHO, INPUT);
    ledcAttach(GATE_SERVO_PIN, 50, 16);
    closeGate();

    Serial.println("[Gate Test] Servo=GPIO18, ultrasonic=(5,19)");
    Serial.println("[Commands] o=open, c=close");
}

void loop() {
    handleSerial();

    unsigned long now = millis();
    float distance = readDistance();

    if (gateOpen && distance > 0 && distance <= GATE_OBJECT_CM) {
        closeAt = now + GATE_OPEN_TIME_MS;
    }

    if (gateOpen && now >= closeAt) {
        closeGate();
    }

    if (now - lastPrint >= PRINT_MS) {
        lastPrint = now;
        Serial.print("[Gate] open=");
        Serial.print(gateOpen ? "ON" : "OFF");
        Serial.print(" distance_cm=");
        Serial.print(distance, 1);
        Serial.print(" close_in_ms=");
        Serial.println(gateOpen ? (long)(closeAt - now) : 0);
    }
}
