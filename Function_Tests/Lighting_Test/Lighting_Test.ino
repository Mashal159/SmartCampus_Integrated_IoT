#include <Arduino.h>

#define LIGHTING_US1_TRIG 13
#define LIGHTING_US1_ECHO 14
#define LIGHTING_US2_TRIG 32
#define LIGHTING_US2_ECHO 33
#define LIGHTING_LDR 36
#define LIGHTING_LED_PIN 2

#define LIGHT_DARK_LEVEL 100
#define LIGHT_PRESENCE_CM 20.0f
#define LIGHT_HOLD_MS 5000UL
#define PRINT_MS 500UL

static String mode = "AUTO";
static bool ledOn = false;
static unsigned long lampOffAt = 0;
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

void setLight(bool on) {
    digitalWrite(LIGHTING_LED_PIN, on ? HIGH : LOW);
    ledOn = on;
}

void handleSerial() {
    if (!Serial.available()) return;

    char c = Serial.read();
    if (c == 'a') mode = "AUTO";
    if (c == 'o') mode = "ON";
    if (c == 'f') mode = "OFF";

    Serial.print("[Lighting Test] mode=");
    Serial.println(mode);
}

void setup() {
    Serial.begin(115200);
    pinMode(LIGHTING_US1_TRIG, OUTPUT);
    pinMode(LIGHTING_US1_ECHO, INPUT);
    pinMode(LIGHTING_US2_TRIG, OUTPUT);
    pinMode(LIGHTING_US2_ECHO, INPUT);
    pinMode(LIGHTING_LDR, INPUT);
    pinMode(LIGHTING_LED_PIN, OUTPUT);
    setLight(false);

    Serial.println("[Lighting Test] LEDs=GPIO2, LDR=GPIO36, US=(13,14),(32,33)");
    Serial.println("[Commands] a=AUTO, o=ON, f=OFF");
}

void loop() {
    handleSerial();

    unsigned long now = millis();
    int lightRaw = analogRead(LIGHTING_LDR);
    bool dark = lightRaw < LIGHT_DARK_LEVEL;
    float d1 = -1;
    float d2 = -1;
    bool presence = false;

    if (mode == "ON") {
        setLight(true);
    } else if (mode == "OFF") {
        setLight(false);
    } else {
        if (dark) {
            d1 = readDistance(LIGHTING_US1_TRIG, LIGHTING_US1_ECHO);
            d2 = readDistance(LIGHTING_US2_TRIG, LIGHTING_US2_ECHO);
            presence = (d1 > 0 && d1 <= LIGHT_PRESENCE_CM) ||
                       (d2 > 0 && d2 <= LIGHT_PRESENCE_CM);

            if (presence) {
                lampOffAt = now + LIGHT_HOLD_MS;
            }
        }

        setLight(now < lampOffAt);
    }

    if (now - lastPrint >= PRINT_MS) {
        lastPrint = now;
        Serial.print("[Lighting] mode=");
        Serial.print(mode);
        Serial.print(" light_raw=");
        Serial.print(lightRaw);
        Serial.print(" dark=");
        Serial.print(dark ? "ON" : "OFF");
        Serial.print(" d1_cm=");
        Serial.print(d1, 1);
        Serial.print(" d2_cm=");
        Serial.print(d2, 1);
        Serial.print(" presence=");
        Serial.print(presence ? "ON" : "OFF");
        Serial.print(" led_gpio2=");
        Serial.println(ledOn ? "ON" : "OFF");
    }
}
