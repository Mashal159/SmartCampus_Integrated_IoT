#include "SmartIrrigation.h"
#include "config.h"
#include "MoistureSensor.h"
#include "ApiClient.h"
#include <ArduinoJson.h>

/*
  ==============================================================================
  Module: Smart Irrigation

  Function:
  - Reads soil moisture percentage using SoilMoisture driver.
  - Polls server for irrigation mode.
  - Controls relay/pump according to:
      MANUAL_OPEN  -> pump ON
      MANUAL_CLOSE -> pump OFF
      AUTO         -> pump ON below low threshold and OFF above high threshold
  - Reports humidity and valve/pump state when changed.

  Hardware:
  - Soil moisture AO GPIO 35
  - Relay signal GPIO 27

  Safety:
  - GPIO 35 is ADC1 input-only.
  - Relay module should be powered from 5V.
  - Pump must use external power, not ESP32 power.
  ==============================================================================
*/

/* =============================================================================
   Hardware Object
   ============================================================================= */

static SoilMoisture soil(IRRIGATION_MOISTURE);

/* =============================================================================
   Irrigation State Variables
   ============================================================================= */

static int lastHumidity = -100;
static bool valveOpen = false;
static bool lastValve = false;
static bool relayOutputInitialized = false;
static bool lastRelayLogicalState = false;
static String currentMode = "AUTO";

/* =============================================================================
   Timing Variables
   ============================================================================= */

static unsigned long lastPoll = 0;
static unsigned long lastCheck = 0;

/* =============================================================================
   Timing and Threshold Constants
   ============================================================================= */

#define IRRIGATION_LOW_THRESHOLD  40
#define IRRIGATION_HIGH_THRESHOLD 60
#define IRRIGATION_POLL_MS       2000UL
#define IRRIGATION_CHECK_MS      1000UL

struct MoistureReading {
    int raw;
    int humidity;
};

/* =============================================================================
   Hysteresis Pump Control
   ============================================================================= */

static void applyAutoHysteresis(int humidity) {
    if (humidity < IRRIGATION_LOW_THRESHOLD && !valveOpen) {
        valveOpen = true;
    } else if (humidity > IRRIGATION_HIGH_THRESHOLD && valveOpen) {
        valveOpen = false;
    }
}

static void setPumpRelay(bool on) {
    if (relayOutputInitialized && on == lastRelayLogicalState) {
        return;
    }

    bool outputHigh = IRRIGATION_RELAY_ACTIVE_LOW ? !on : on;
    digitalWrite(IRRIGATION_PUMP_RELAY, outputHigh ? HIGH : LOW);

    lastRelayLogicalState = on;
    relayOutputInitialized = true;
}

static MoistureReading readStableMoisture() {
    long rawTotal = 0;
    long humidityTotal = 0;

    for (int i = 0; i < 5; i++) {
        rawTotal += soil.read();
        humidityTotal += soil.readPercentage();
        delay(5);
    }

    MoistureReading reading;
    reading.raw = rawTotal / 5;
    reading.humidity = humidityTotal / 5;
    return reading;
}

/* =============================================================================
   Fetch Irrigation Mode from Server
   ============================================================================= */

static bool fetchMode(String& mode) {
    String body;

    if (!ApiClient::get(API_GARDEN_STATE, body)) {
        return false;
    }

    StaticJsonDocument<128> doc;

    if (deserializeJson(doc, body) != DeserializationError::Ok) {
        return false;
    }

    if (!doc["mode"].is<const char*>()) {
        return false;
    }

    String receivedMode = String((const char*)doc["mode"]);

    if (receivedMode != "AUTO" &&
        receivedMode != "MANUAL_OPEN" &&
        receivedMode != "MANUAL_CLOSE" &&
        receivedMode != "ON" &&
        receivedMode != "OFF") {
        return false;
    }

    mode = receivedMode;
    return true;
}

/* =============================================================================
   Post Irrigation Reading to Server
   ============================================================================= */

static void postReading(int humidity, bool valve) {
    String payload = String("{\"garden_id\":\"") + GARDEN_ID +
                     "\",\"humidity\":" + String(humidity) +
                     ",\"valve_open\":" + (valve ? "true" : "false") + "}";

    ApiClient::postJson(API_GARDEN_POST, payload);
}

/* =============================================================================
   Setup Function
   ============================================================================= */

void setupSmartIrrigation() {
    pinMode(IRRIGATION_PUMP_RELAY, OUTPUT);
    setPumpRelay(false);
}

/* =============================================================================
   Main Non-Blocking Loop Function
   ============================================================================= */

void loopSmartIrrigation() {
    unsigned long now = millis();

    if (now - lastCheck < IRRIGATION_CHECK_MS) {
        return;
    }

    lastCheck = now;

    MoistureReading moisture = readStableMoisture();
    int humidity = moisture.humidity;

    // Poll server mode periodically
    if (now - lastPoll >= IRRIGATION_POLL_MS) {
        String mode;

        if (fetchMode(mode)) {
            currentMode = mode;
        }

        lastPoll = now;
    }

    if (currentMode == "MANUAL_OPEN" || currentMode == "ON") {
        valveOpen = true;
    } else if (currentMode == "MANUAL_CLOSE" || currentMode == "OFF") {
        valveOpen = false;
    } else {
        applyAutoHysteresis(humidity);
    }

    setPumpRelay(valveOpen);

    // Report only when humidity changes significantly or valve state changes
    if (abs(humidity - lastHumidity) > 10 || valveOpen != lastValve) {
        postReading(humidity, valveOpen);

        lastHumidity = humidity;
        lastValve = valveOpen;
    }
}

/* =============================================================================
   Independent Test Function
   ============================================================================= */

void test_SmartIrrigation() {
    setPumpRelay(true);
}
