#include "SmartGate.h"
#include "config.h"
#include "ServoMotor.h"
#include "Ultrasonic.h"
#include "ApiClient.h"
#include <ArduinoJson.h>

/*
  ==============================================================================
  Module: Smart Gate

  Function:
  - Polls server for open command.
  - Opens servo gate when server state changes to open=true.
  - Keeps gate open for a fixed time.
  - Uses ultrasonic sensor while gate is open to detect objects.
  - Extends open time if object is detected under gate.

  Hardware:
  - Servo on GPIO 18.
  - Ultrasonic trigger on GPIO 5.
  - Ultrasonic echo on GPIO 19.
  - Ultrasonic reading uses the canonical pulseIn()-based driver.
  ==============================================================================
*/

/* =============================================================================
   Hardware Objects
   ============================================================================= */

static ServoMotor gateServo(GATE_SERVO_PIN);
static Ultrasonic gateSensor(GATE_US_TRIG, GATE_US_ECHO);

/* =============================================================================
   Gate State Variables
   ============================================================================= */

static bool gateOpen = false;
static bool serverWasOpen = false;
static bool lastServerOpen = false;

static unsigned long gateCloseTime = 0;
static unsigned long lastPollTime = 0;
static unsigned long lastSensorCheck = 0;
static float lastGateDistance = -1.0f;

/* =============================================================================
   Gate Constants
   ============================================================================= */

#define GATE_ANGLE_CLOSED 0
#define GATE_ANGLE_OPEN   90
#define GATE_OPEN_TIME_MS 10000UL
#define GATE_POLL_MS      2000UL
#define GATE_SENSOR_MS    1000UL
#define GATE_OBJECT_CM    25.0f

/* =============================================================================
   Internal Gate Control Functions
   ============================================================================= */

static void openGate() {
    gateServo.moveToAngle(GATE_ANGLE_OPEN);
    gateOpen = true;

    gateCloseTime = millis() + GATE_OPEN_TIME_MS;
    lastSensorCheck = millis();
}

static void closeGate() {
    gateServo.moveToAngle(GATE_ANGLE_CLOSED);
    gateOpen = false;
}

static bool fetchGateOpen(bool& serverOpen) {
    String body;

    if (!ApiClient::get(API_GATE_STATE, body)) {
        return false;
    }

    StaticJsonDocument<128> doc;

    if (deserializeJson(doc, body) != DeserializationError::Ok) {
        return false;
    }

    if (!doc["open"].is<bool>()) {
        return false;
    }

    serverOpen = doc["open"].as<bool>();
    return true;
}

/* =============================================================================
   Setup Function
   ============================================================================= */

void setupSmartGate() {
    gateServo.begin();
    gateServo.moveToAngle(GATE_ANGLE_CLOSED);
}

/* =============================================================================
   Main Non-Blocking Loop Function
   ============================================================================= */

void loopSmartGate() {
    unsigned long now = millis();

    // Poll server for gate state
    if (now - lastPollTime >= GATE_POLL_MS) {
        bool serverOpen = lastServerOpen;
        bool pollOk = fetchGateOpen(serverOpen);

        if (pollOk) {
            // Server command acts like button edge trigger
            if (serverOpen && !serverWasOpen && !gateOpen) {
                openGate();
            }

            serverWasOpen = serverOpen;
            lastServerOpen = serverOpen;
        }

        lastPollTime = now;
    }

    // Check ultrasonic only while gate is open
    if (gateOpen && now - lastSensorCheck >= GATE_SENSOR_MS) {
        float distance = gateSensor.readDistance();
        lastGateDistance = distance;

        if (distance > 0 && distance <= GATE_OBJECT_CM) {
            gateCloseTime = now + GATE_OPEN_TIME_MS;
        }

        lastSensorCheck = now;
    }

    // Close gate after timer expires
    if (gateOpen && now >= gateCloseTime) {
        closeGate();
    }
}

/* =============================================================================
   Independent Test Function
   ============================================================================= */

void test_SmartGate() {
    openGate();
}
