#include <Arduino.h>

#define SMOKE_MQ2_PIN 34
#define SMOKE_LED_PIN 12
#define SMOKE_BUZZER_PIN 15

#define SMOKE_THRESHOLD 2000
#define SMOKE_CLEAR_THRESHOLD 1800
#define SMOKE_FLASH_MS 250UL
#define PRINT_MS 500UL

static bool alarmActive = false;
static bool simulatedSmoke = false;
static bool alarmOn = false;
static bool lastSensorDetectingSmoke = false;
static unsigned long lastFlash = 0;
static unsigned long lastPrint = 0;

void setAlarm(bool on) {
    digitalWrite(SMOKE_LED_PIN, on ? HIGH : LOW);
    digitalWrite(SMOKE_BUZZER_PIN, on ? HIGH : LOW);
    alarmOn = on;
}

void printAlarmStatus() {
    Serial.print(" alarm_active=");
    Serial.print(alarmActive ? "FLASHING" : "OFF");
    Serial.print(" blink_output_now=");
    Serial.print(alarmOn ? "ON" : "OFF");
    Serial.print(" led_pin_read=");
    Serial.print(digitalRead(SMOKE_LED_PIN) == HIGH ? "HIGH" : "LOW");
    Serial.print(" buzzer_pin_read=");
    Serial.println(digitalRead(SMOKE_BUZZER_PIN) == HIGH ? "HIGH" : "LOW");
}

void handleSerial() {
    if (!Serial.available()) {
        return;
    }

    char c = Serial.read();

    if (c == 's') {
        simulatedSmoke = !simulatedSmoke;
        Serial.print("[Smoke Test] simulated sensor smoke=");
        Serial.println(simulatedSmoke ? "ON" : "OFF");
    } else if (c == '1') {
        simulatedSmoke = true;
        alarmActive = true;
        setAlarm(true);
        lastFlash = millis();
        Serial.println("[Smoke Test] simulated sensor smoke=ON");
    } else if (c == '0') {
        simulatedSmoke = false;
        Serial.println("[Smoke Test] simulated sensor smoke=OFF");
    } else if (c == 'f') {
        int mq2 = analogRead(SMOKE_MQ2_PIN);
        bool realSmoke = mq2 > SMOKE_THRESHOLD;

        if (!simulatedSmoke && !realSmoke) {
            alarmActive = false;
            setAlarm(false);
            Serial.println("[Smoke Test] server OFF accepted: no smoke detected");
        } else {
            Serial.println("[Smoke Test] server OFF ignored: smoke is still detected");
        }
    } else if (c == 'h') {
        Serial.println("[Commands]");
        Serial.println("  s = toggle simulated sensor smoke");
        Serial.println("  1 = simulated sensor smoke ON");
        Serial.println("  0 = simulated sensor smoke OFF");
        Serial.println("  f = server response OFF, accepted only when smoke=OFF");
        Serial.println("  h = help");
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(SMOKE_MQ2_PIN, INPUT);
    pinMode(SMOKE_LED_PIN, OUTPUT);
    pinMode(SMOKE_BUZZER_PIN, OUTPUT);
    setAlarm(false);

    Serial.println("[Smoke Test] MQ2=GPIO34, LED=GPIO12, buzzer=GPIO15");
    Serial.println("[Commands] s=toggle sensor smoke, 1=sensor ON, 0=sensor OFF, f=server OFF, h=help");
}

void loop() {
    handleSerial();

    unsigned long now = millis();
    int mq2 = analogRead(SMOKE_MQ2_PIN);
    bool realSmoke = mq2 > SMOKE_THRESHOLD;
    bool sensorDetectingSmoke = simulatedSmoke || realSmoke;

    if (sensorDetectingSmoke && !lastSensorDetectingSmoke) {
        alarmActive = true;
        setAlarm(true);
        lastFlash = now;
    }

    if (alarmActive && now - lastFlash >= SMOKE_FLASH_MS) {
        setAlarm(!alarmOn);
        lastFlash = now;
    }

    if (now - lastPrint >= PRINT_MS) {
        lastPrint = now;
        Serial.print("[Smoke] mq2_raw=");
        Serial.print(mq2);
        Serial.print(" simulated=");
        Serial.print(simulatedSmoke ? "ON" : "OFF");
        Serial.print(" sensor_smoke=");
        Serial.print(sensorDetectingSmoke ? "ON" : "OFF");
        printAlarmStatus();
    }

    lastSensorDetectingSmoke = sensorDetectingSmoke;
}
