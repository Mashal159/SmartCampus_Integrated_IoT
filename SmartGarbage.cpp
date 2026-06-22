#include "SmartGarbage.h"
#include "config.h"
#include "Ultrasonic.h"
#include "ApiClient.h"

/*
  ==============================================================================
  Module: Smart Garbage

  Function:
  - Measures garbage bin level using ultrasonic distance sensor.
  - Sends API update only when distance changes significantly.
  - Prevents unnecessary repeated server requests.

  Hardware:
  - Ultrasonic trigger on GPIO 23.
  - Ultrasonic echo on GPIO 22.
  - Ultrasonic reading uses the canonical Ultrasonic driver, which matches the
    validated pulseIn() function-test logic.
  ==============================================================================
*/

/* =============================================================================
   Hardware Object
   ============================================================================= */

static Ultrasonic garbageSensor(GARBAGE_US_TRIG, GARBAGE_US_ECHO);

/* =============================================================================
   State Variables
   ============================================================================= */

static float lastDistance = -100;
static unsigned long lastCheck = 0;

/* =============================================================================
   Timing and Threshold Constants
   ============================================================================= */

#define GARBAGE_CHECK_MS 200UL
#define GARBAGE_DRIFT_CM 5.0f

void setupSmartGarbage() {
    // Ultrasonic pin initialization is handled by the driver.
}

/* =============================================================================
   Main Non-Blocking Loop Function
   ============================================================================= */

void loopSmartGarbage() {
    unsigned long now = millis();

    if (now - lastCheck < GARBAGE_CHECK_MS) {
        return;
    }

    lastCheck = now;

    float distance = garbageSensor.readDistance();

    if (distance >= 0) {
        // Send update only if distance changed meaningfully
        if (abs(distance - lastDistance) > GARBAGE_DRIFT_CM) {
            String payload = "{\"garbage_name\":\"G-1\",\"garbage_level\":" +
                             String(distance, 1) + "}";

            bool sent = ApiClient::postJson(API_GARBAGE_POST, payload);

            if (sent) {
                lastDistance = distance;
            }
        }
    }
}

/* =============================================================================
   Independent Test Function
   ============================================================================= */

void test_SmartGarbage() {
    Serial.println(garbageSensor.readDistance());
}
