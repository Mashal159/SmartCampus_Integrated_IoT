#include "SmartParking.h"
#include "config.h"
#include "Ultrasonic.h"
#include "ApiClient.h"

/*
  ==============================================================================
  Module: Smart Parking

  Function:
  - Uses two ultrasonic sensors to monitor parking spots.
  - Sends distance updates to server when values change significantly.

  Hardware:
  - Parking spot 1 ultrasonic:
      TRIG GPIO 21
      ECHO GPIO 4

  - Parking spot 2 ultrasonic:
      TRIG GPIO 26
      ECHO GPIO 25

  - Ultrasonic readings use the canonical pulseIn()-based driver.
  ==============================================================================
*/

/* =============================================================================
   Hardware Objects
   ============================================================================= */

static Ultrasonic parkingSensors[] = {
    Ultrasonic(PARKING_US1_TRIG, PARKING_US1_ECHO),
    Ultrasonic(PARKING_US2_TRIG, PARKING_US2_ECHO)
};

/* =============================================================================
   State Variables
   ============================================================================= */

static float lastDistances[2] = {-100, -100};
static unsigned long lastCheck = 0;

/* =============================================================================
   Timing and Threshold Constants
   ============================================================================= */

#define PARKING_CHECK_MS 200UL
#define PARKING_DRIFT_CM 5.0f

/* =============================================================================
   Setup Function
   ============================================================================= */

void setupSmartParking() {
    // Ultrasonic pin initialization is handled by the driver.
}

/* =============================================================================
   Main Non-Blocking Loop Function
   ============================================================================= */

void loopSmartParking() {
    unsigned long now = millis();

    if (now - lastCheck < PARKING_CHECK_MS) {
        return;
    }

    lastCheck = now;

    for (int i = 0; i < 2; i++) {
        float distance = parkingSensors[i].readDistance();

        if (distance < 0) {
            continue;
        }

        // Send update only if distance changed meaningfully
        if (abs(distance - lastDistances[i]) > PARKING_DRIFT_CM) {
            String payload = "{\"spot_name\":\"P-" + String(i + 1) +
                             "\",\"distance\":" + String(distance, 1) + "}";

            bool sent = ApiClient::postJson(API_PARKING_POST, payload);

            if (sent) {
                lastDistances[i] = distance;
            }
        }
    }
}

/* =============================================================================
   Independent Test Function
   ============================================================================= */

void test_SmartParking() {
    for (int i = 0; i < 2; i++) {
        Serial.println(parkingSensors[i].readDistance());
    }
}
