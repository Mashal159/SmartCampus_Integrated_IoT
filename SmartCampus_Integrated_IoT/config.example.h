#ifndef CONFIG_H
#define CONFIG_H

/*
  ==============================================================================
  Project: SmartCampus_Integrated_IoT
  File: config.example.h

  Copy this file to config.h and fill in private Wi-Fi credentials locally.
  Do not commit config.h to GitHub.
  ==============================================================================
*/

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/* =============================================================================
   Wi-Fi Configuration
   ============================================================================= */

#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define WIFI_CHANNEL  6

/* =============================================================================
   Smart Gate Pins
   ============================================================================= */

#define GATE_SERVO_PIN        18
#define GATE_US_TRIG          5
#define GATE_US_ECHO          19

/* =============================================================================
   Smart Garbage Pins
   ============================================================================= */

#define GARBAGE_US_TRIG       23
#define GARBAGE_US_ECHO       22

/* =============================================================================
   Smart Parking Pins
   ============================================================================= */

#define PARKING_US1_TRIG      21
#define PARKING_US1_ECHO      4
#define PARKING_US2_TRIG      26
#define PARKING_US2_ECHO      25

/* =============================================================================
   Smoke Detector Pins
   ============================================================================= */

#define SMOKE_MQ2_PIN         34
#define SMOKE_LED_PIN         12
#define SMOKE_BUZZER_PIN      15

/* =============================================================================
   Smart Irrigation Pins
   ============================================================================= */

#define IRRIGATION_MOISTURE   35
#define IRRIGATION_PUMP_RELAY 27
#define IRRIGATION_RELAY_ACTIVE_LOW true

/* =============================================================================
   Smart Lighting Pins
   ============================================================================= */

#define LIGHTING_US1_TRIG     13
#define LIGHTING_US1_ECHO     14
#define LIGHTING_US2_TRIG     32
#define LIGHTING_US2_ECHO     33
#define LIGHTING_LDR          36
#define LIGHTING_LED_PIN      2

/* =============================================================================
   API Endpoints
   ============================================================================= */

#define API_GATE_STATE        "http://sensors.smartcampus.engineer/gate/state?gate_id=G-1"
#define API_GARBAGE_POST      "http://sensors.smartcampus.engineer/garbage_level"
#define API_PARKING_POST      "http://sensors.smartcampus.engineer/parking_spots"
#define API_LIGHTING_POST     "http://sensors.smartcampus.engineer/lighting"
#define API_LIGHTING_STATE    "http://sensors.smartcampus.engineer/lighting/state?location=L-1"
#define API_FIRE_POST         "http://sensors.smartcampus.engineer/fire_alarm"
#define API_FIRE_STATE        "http://sensors.smartcampus.engineer/fire_alarm/state?fire_id=F-3"
#define API_GARDEN_POST       "http://sensors.smartcampus.engineer/garden"
#define API_GARDEN_STATE      "http://sensors.smartcampus.engineer/garden/state?garden_id=Garden-5"

/* =============================================================================
   Device Logical IDs
   ============================================================================= */

#define LIGHTING_LOCATION     "L-1"
#define FIRE_ID               "F-3"
#define GARDEN_ID             "Garden-5"

/* =============================================================================
   Shared FreeRTOS Mutexes
   ============================================================================= */

extern SemaphoreHandle_t gHttpMutex;
extern SemaphoreHandle_t gSerialMutex;

#endif

