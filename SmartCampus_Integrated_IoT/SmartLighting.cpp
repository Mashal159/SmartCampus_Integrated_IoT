#include "SmartLighting.h"
#include "config.h"
#include "Ultrasonic.h"
#include "ApiClient.h"
#include <ArduinoJson.h>

/*
  ==============================================================================
  Module: Smart Lighting

  Function:
  - Polls lighting mode from server.
  - Supports AUTO, MANUAL_ON, and MANUAL_OFF.
  - In AUTO mode:
      LED turns ON only when:
        1. Area is dark
        2. Presence is detected by ultrasonic sensors
      Then it stays ON for a hold period.
  - Reports light level and LED state to server.

  Hardware:
  - Ultrasonic Sensor 1:
      TRIG GPIO 13
      ECHO GPIO 14

  - Ultrasonic Sensor 2:
      TRIG GPIO 32
      ECHO GPIO 33

  - LDR:
      AO GPIO 36
      GPIO 36 is ADC1 input-only and safe with Wi-Fi.

  - Shared lighting LED output GPIO 2

  Safety:
  - Each LED must have 220 ohm series resistor.
  - HC-SR04 ECHO lines must be stepped down from 5V to 3.3V.
  - Lighting ultrasonic readings use the canonical Ultrasonic driver, which
    matches the validated pulseIn() function-test logic.
  ==============================================================================
*/

/* =============================================================================
   Hardware Objects
   ============================================================================= */

static Ultrasonic lightUS1(LIGHTING_US1_TRIG, LIGHTING_US1_ECHO);
static Ultrasonic lightUS2(LIGHTING_US2_TRIG, LIGHTING_US2_ECHO);

/* =============================================================================
   Lighting Mode State
   ============================================================================= */

static String currentMode = "AUTO";

/* =============================================================================
   Timing Variables
   ============================================================================= */

static unsigned long lastPoll = 0;
static unsigned long lastReport = 0;
static unsigned long lampOffAt = 0;

/* =============================================================================
   Report State Variables
   ============================================================================= */

static bool ledState = false;
static bool lastReportedState = false;
static int lastReportedLight = -1;
static float lightDist1 = -1.0f;
static float lightDist2 = -1.0f;

/* =============================================================================
   Timing and Detection Constants
   ============================================================================= */

#define LIGHT_POLL_MS       3000UL
#define LIGHT_REPORT_MIN_MS 1000UL
#define LIGHT_HOLD_MS       5000UL
#define LIGHT_PRESENCE_CM   20.0f
#define LIGHT_DARK_LEVEL    100
#define LIGHT_DRIFT_LEVEL   200

/* =============================================================================
   Sensor Reading
   ============================================================================= */

static int readLightLevel() {
    return analogRead(LIGHTING_LDR);
}

static bool isDark() {
    return readLightLevel() < LIGHT_DARK_LEVEL;
}

/* =============================================================================
   LED Control
   ============================================================================= */

static void setAllLights(bool on) {
    digitalWrite(LIGHTING_LED_PIN, on ? HIGH : LOW);

    ledState = on;
}

/* =============================================================================
   Server Mode Polling
   ============================================================================= */

static void pollLightingMode() {
    String body;

    if (ApiClient::get(API_LIGHTING_STATE, body)) {
        StaticJsonDocument<128> doc;

        if (deserializeJson(doc, body) == DeserializationError::Ok &&
            doc["mode"].is<const char*>()) {
            String mode = String((const char*)doc["mode"]);
            if (mode == "AUTO" || mode == "MANUAL_ON" || mode == "MANUAL_OFF" ||
                mode == "ON" || mode == "OFF") {
                currentMode = mode;
            }
        }
    }
}

/* =============================================================================
   Server Reporting
   ============================================================================= */

static void reportLightingState(int lightLevel, bool isOn) {
    String payload = String("{\"location\":\"") + LIGHTING_LOCATION +
                     "\",\"light_level\":" + String(lightLevel) +
                     ",\"is_on\":" + (isOn ? "true" : "false") + "}";

    if (ApiClient::postJson(API_LIGHTING_POST, payload)) {
        lastReportedState = isOn;
        lastReportedLight = lightLevel;
    }
}

/* =============================================================================
   Setup Function
   ============================================================================= */

void setupSmartLighting() {
    pinMode(LIGHTING_LDR, INPUT);

    pinMode(LIGHTING_LED_PIN, OUTPUT);

    setAllLights(false);
}

/* =============================================================================
   Main Non-Blocking Loop Function
   ============================================================================= */

void loopSmartLighting() {
    unsigned long now = millis();

    // Poll server mode periodically
    if (now - lastPoll >= LIGHT_POLL_MS) {
        pollLightingMode();
        lastPoll = now;
    }

    bool desired = false;

    // Manual control from server
    if (currentMode == "MANUAL_ON" || currentMode == "ON") {
        desired = true;
    } else if (currentMode == "MANUAL_OFF" || currentMode == "OFF") {
        desired = false;
    } else {
        // AUTO mode: dark + presence required
        bool dark = isDark();

        if (dark) {
            lightDist1 = lightUS1.readDistance();
            lightDist2 = lightUS2.readDistance();

            bool near1 = lightDist1 > 0 && lightDist1 <= LIGHT_PRESENCE_CM;
            bool near2 = lightDist2 > 0 && lightDist2 <= LIGHT_PRESENCE_CM;
            bool present = near1 || near2;

            if (present) {
                lampOffAt = now + LIGHT_HOLD_MS;
            }
        }

        desired = now < lampOffAt;
    }

    // Apply LED state only when changed
    if (desired != ledState) {
        setAllLights(desired);
    }

    int lightLevel = readLightLevel();

    bool stateChanged = ledState != lastReportedState;
    bool reportWindowOpen = now - lastReport >= LIGHT_REPORT_MIN_MS;
    bool lightDrifted = lastReportedLight < 0 ||
                        abs(lightLevel - lastReportedLight) > LIGHT_DRIFT_LEVEL;

    // Report only when the state changes or the sensor reading drifts enough.
    if (stateChanged || (lightDrifted && reportWindowOpen)) {
        reportLightingState(lightLevel, ledState);
        lastReport = now;
    }
}

/* =============================================================================
   Independent Test Function
   ============================================================================= */

void test_SmartLighting() {
    setAllLights(true);
}
