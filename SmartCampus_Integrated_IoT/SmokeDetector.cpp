#include "SmokeDetector.h"
#include "config.h"
#include "MQ2.h"
#include "ApiClient.h"
#include <ArduinoJson.h>

/*
  ==============================================================================
  Module: Smoke Detector

  Function:
  - Reads MQ2 analog value.
  - Detects smoke if sensor value exceeds threshold.
  - Posts fire alarm activation immediately when smoke is detected.
  - Polls database state to control LED and buzzer.
  - Flashes the LED and buzzer alarm outputs while smoke alarm is active.
  - Smoke system has faster checking because it is a safety-critical subsystem.

  Hardware:
  - MQ2 AO GPIO 34
  - Alarm LED GPIO 12
  - Active buzzer GPIO 15

  Safety:
  - MQ2 heater powered from 5V.
  - MQ2 analog output connected to GPIO 34.
  - GPIO 34 is ADC1 input-only.
  ==============================================================================
*/

/* =============================================================================
   Hardware Object
   ============================================================================= */

static MQ2 smokeSensor(SMOKE_MQ2_PIN);

/* =============================================================================
   Alarm State Variables
   ============================================================================= */

static bool lastPostedActive = false;
static bool dbActive = false;
static bool localSmokeActive = false;
static bool lastLocalSmokeActive = false;
static bool alarmActive = false;
static bool alarmOutputOn = false;
static bool alarmFlashActive = false;

/* =============================================================================
   Timing Variables
   ============================================================================= */

static unsigned long lastPoll = 0;
static unsigned long lastCheck = 0;
static unsigned long lastFlash = 0;

/* =============================================================================
   Timing and Threshold Constants

   SMOKE_CHECK_MS:
   - Reduced to 50ms so smoke detection reacts faster than other subsystems.

   SMOKE_POLL_MS:
   - Server state is polled separately from the flash timing. The alarm output
     is updated before server communication so network delay has less effect on
     LED/buzzer flashing.
   ============================================================================= */

#define SMOKE_THRESHOLD       2000
#define SMOKE_CLEAR_THRESHOLD 1800
#define SMOKE_POLL_MS         2000UL
#define SMOKE_CHECK_MS        50UL
#define SMOKE_FLASH_MS        250UL

/* =============================================================================
   Alarm Outputs
   ============================================================================= */

static void setAlarmOutput(bool on) {
    digitalWrite(SMOKE_LED_PIN, on ? HIGH : LOW);
    digitalWrite(SMOKE_BUZZER_PIN, on ? HIGH : LOW);
    alarmOutputOn = on;
}

static void updateAlarmFlash(bool active, unsigned long now) {
    if (!active) {
        if (alarmOutputOn) {
            setAlarmOutput(false);
        }
        alarmFlashActive = false;
        lastFlash = now;
        return;
    }

    if (!alarmFlashActive) {
        alarmFlashActive = true;
        setAlarmOutput(true);
        lastFlash = now;
        return;
    }

    if (now - lastFlash >= SMOKE_FLASH_MS) {
        setAlarmOutput(!alarmOutputOn);
        lastFlash = now;
    }
}

static void stopAlarmFromServer() {
    alarmActive = false;
    dbActive = false;
    alarmFlashActive = false;
    setAlarmOutput(false);
    lastPostedActive = false;
}

/* =============================================================================
   Fetch Alarm State from Server
   ============================================================================= */

static bool fetchAlarmState(bool& active) {
    String body;

    if (ApiClient::get(API_FIRE_STATE, body)) {
        StaticJsonDocument<128> doc;

        if (deserializeJson(doc, body) == DeserializationError::Ok) {
            active = doc["is_active"].as<bool>();
            return true;
        }
    }

    return false;
}

/* =============================================================================
   Post Fire Alarm Event

   Important:
   - This is called immediately when local smoke is detected.
   - It should not be called repeatedly while smoke is still present.
   ============================================================================= */

static void postFire(bool active) {
    String payload = String("{\"fire_id\":\"") + FIRE_ID +
                     "\",\"is_active\":" + (active ? "true" : "false") + "}";

    ApiClient::postJson(API_FIRE_POST, payload);
}

/* =============================================================================
   Setup Function
   ============================================================================= */

void setupSmokeDetector() {
    pinMode(SMOKE_LED_PIN, OUTPUT);
    pinMode(SMOKE_BUZZER_PIN, OUTPUT);

    setAlarmOutput(false);
}

/* =============================================================================
   Main Non-Blocking Loop Function

   Priority Logic:
   - This function is called by a high-priority FreeRTOS task.
   - Smoke sensor is checked every 50ms.
   - First smoke detection event is posted to server immediately.
   ============================================================================= */

void loopSmokeDetector() {
    unsigned long now = millis();

    if (now - lastCheck < SMOKE_CHECK_MS) {
        return;
    }

    lastCheck = now;

    int smokeValue = smokeSensor.read();

    if (smokeValue > SMOKE_THRESHOLD) {
        localSmokeActive = true;
    } else if (smokeValue < SMOKE_CLEAR_THRESHOLD) {
        localSmokeActive = false;
    }

    // Sensor detection starts/latches the alarm. Sensor clear alone does not
    // stop it; the server OFF response is required, like the working test sketch.
    if (localSmokeActive && !lastLocalSmokeActive) {
        alarmActive = true;
        setAlarmOutput(true);
        alarmFlashActive = true;
        lastFlash = now;

        if (!lastPostedActive) {
            postFire(true);
            lastPostedActive = true;
            dbActive = true;
        }
    }

    lastLocalSmokeActive = localSmokeActive;

    // Flash output before network communication. HTTP requests can block, so
    // doing this first keeps the alarm timing as regular as possible.
    updateAlarmFlash(alarmActive, now);

    // Poll server. Server ON can latch the alarm. Server OFF stops the alarm
    // only when the sensor is no longer detecting smoke.
    if (now - lastPoll >= SMOKE_POLL_MS) {
        bool serverActive = dbActive;

        if (fetchAlarmState(serverActive)) {
            dbActive = serverActive;

            if (serverActive) {
                alarmActive = true;
            } else {
                if (!localSmokeActive) {
                    stopAlarmFromServer();
                }
            }
        }

        lastPoll = now;
    }

}

/* =============================================================================
   Independent Test Function
   ============================================================================= */

void test_SmokeDetector() {
    setAlarmOutput(true);
}
