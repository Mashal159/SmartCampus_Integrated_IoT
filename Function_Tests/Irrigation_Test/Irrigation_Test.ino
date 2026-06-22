#include <Arduino.h>

#define IRRIGATION_MOISTURE 35
#define IRRIGATION_PUMP_RELAY 27
#define IRRIGATION_RELAY_ACTIVE_LOW true

#define SOIL_ADC_DRY 4095
#define SOIL_ADC_WET 1200
#define IRRIGATION_LOW_THRESHOLD 40
#define IRRIGATION_HIGH_THRESHOLD 60
#define PRINT_MS 500UL

static String mode = "AUTO";
static bool pumpOn = false;
static unsigned long lastPrint = 0;

int readMoisturePercent() {
    int raw = analogRead(IRRIGATION_MOISTURE);
    int percentage = map(raw, SOIL_ADC_DRY, SOIL_ADC_WET, 0, 100);
    return constrain(percentage, 0, 100);
}

void setPump(bool on) {
    bool outputHigh = IRRIGATION_RELAY_ACTIVE_LOW ? !on : on;
    digitalWrite(IRRIGATION_PUMP_RELAY, outputHigh ? HIGH : LOW);
    pumpOn = on;
}

void applyAutoHysteresis(int moisture) {
    if (moisture < IRRIGATION_LOW_THRESHOLD && !pumpOn) {
        setPump(true);
    } else if (moisture > IRRIGATION_HIGH_THRESHOLD && pumpOn) {
        setPump(false);
    }
}

void handleSerial() {
    if (!Serial.available()) return;

    char c = Serial.read();
    if (c == 'a') mode = "AUTO";
    if (c == 'o') mode = "ON";
    if (c == 'f') mode = "OFF";

    Serial.print("[Irrigation Test] mode=");
    Serial.println(mode);
}

void setup() {
    Serial.begin(115200);
    pinMode(IRRIGATION_MOISTURE, INPUT);
    pinMode(IRRIGATION_PUMP_RELAY, OUTPUT);
    setPump(false);

    Serial.println("[Irrigation Test] moisture=GPIO35, relay=GPIO27");
    Serial.println("[Commands] a=AUTO, o=ON, f=OFF");
}

void loop() {
    handleSerial();

    unsigned long now = millis();
    int raw = analogRead(IRRIGATION_MOISTURE);
    int moisture = readMoisturePercent();

    if (mode == "ON") {
        setPump(true);
    } else if (mode == "OFF") {
        setPump(false);
    } else {
        applyAutoHysteresis(moisture);
    }

    if (now - lastPrint >= PRINT_MS) {
        lastPrint = now;
        Serial.print("[Irrigation] mode=");
        Serial.print(mode);
        Serial.print(" raw=");
        Serial.print(raw);
        Serial.print(" moisture_pct=");
        Serial.print(moisture);
        Serial.print(" low=");
        Serial.print(IRRIGATION_LOW_THRESHOLD);
        Serial.print(" high=");
        Serial.print(IRRIGATION_HIGH_THRESHOLD);
        Serial.print(" pump_gpio27=");
        Serial.println(pumpOn ? "ON" : "OFF");
    }
}
