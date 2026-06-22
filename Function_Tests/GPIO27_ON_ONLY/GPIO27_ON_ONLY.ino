#include <Arduino.h>

#define IRRIGATION_PUMP_RELAY 27
#define RELAY_ACTIVE_LOW true

static bool relayOn = false;
static bool rawOutputHigh = false;

void setRelay(bool on) {
    bool outputHigh = RELAY_ACTIVE_LOW ? !on : on;
    digitalWrite(IRRIGATION_PUMP_RELAY, outputHigh ? HIGH : LOW);
    relayOn = on;
    rawOutputHigh = outputHigh;

    Serial.print("[GPIO27 Serial Test] GPIO 27 = ");
    Serial.println(relayOn ? "ON" : "OFF");
    Serial.print("[GPIO27 Serial Test] physical output = ");
    Serial.println(outputHigh ? "HIGH" : "LOW");
}

void setRawOutput(bool high) {
    digitalWrite(IRRIGATION_PUMP_RELAY, high ? HIGH : LOW);
    rawOutputHigh = high;

    Serial.print("[GPIO27 Serial Test] raw GPIO 27 = ");
    Serial.println(rawOutputHigh ? "HIGH" : "LOW");
}

void setup() {
    Serial.begin(115200);

    pinMode(IRRIGATION_PUMP_RELAY, OUTPUT);
    setRelay(false);

    Serial.println("[GPIO27 Serial Test] Commands:");
    Serial.println("  1 or o = ON");
    Serial.println("  0 or f = OFF");
    Serial.println("  t      = TOGGLE");
    Serial.println("  h      = raw GPIO HIGH");
    Serial.println("  l      = raw GPIO LOW");
}

void loop() {
    while (Serial.available()) {
        char command = Serial.read();

        if (command == '\r' || command == '\n' || command == ' ') {
            continue;
        }

        Serial.print("[GPIO27 Serial Test] received: ");
        Serial.println(command);

        if (command == '1' || command == 'o' || command == 'O') {
            setRelay(true);
        } else if (command == '0' || command == 'f' || command == 'F') {
            setRelay(false);
        } else if (command == 't' || command == 'T') {
            setRelay(!relayOn);
        } else if (command == 'h' || command == 'H') {
            setRawOutput(true);
        } else if (command == 'l' || command == 'L') {
            setRawOutput(false);
        } else {
            Serial.println("[GPIO27 Serial Test] unknown command");
        }
    }
}
