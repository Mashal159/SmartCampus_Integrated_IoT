#include <Arduino.h>
#include <WiFi.h>

#include "config.h"

#include "SmartGate.h"
#include "SmartGarbage.h"
#include "SmartParking.h"
#include "SmartLighting.h"
#include "SmokeDetector.h"
#include "SmartIrrigation.h"

/*
  ==============================================================================
  Project: SmartCampus_Integrated_IoT

  Main File:
  - SmartCampus_Integrated_IoT.ino

  Function:
  - Initializes all Smart Campus subsystems.
  - Starts Wi-Fi connection management.
  - Creates one FreeRTOS task per subsystem.
  - Keeps all systems running concurrently without blocking each other.

  Important Safety Update:
  - Smoke Detector task has higher priority than other subsystem tasks.
  - This allows smoke detection to react faster because it is a fire safety system.

  Subsystems:
  1. Smart Gate
  2. Smart Garbage
  3. Smart Parking
  4. Smart Lighting
  5. Smoke Detector
  6. Smart Irrigation
  ==============================================================================
*/

/* =============================================================================
   Global Shared Mutexes
   ============================================================================= */

SemaphoreHandle_t gHttpMutex;
SemaphoreHandle_t gSerialMutex;

/* =============================================================================
   Wi-Fi Reconnection State
   ============================================================================= */

static unsigned long lastWifiAttempt = 0;
static bool wifiWasConnected = false;

#define WIFI_RETRY_MS 5000UL
#define TASK_DELAY_MS 100UL
#define SMOKE_TASK_DELAY_MS 10UL

static void printSerialLine(const String& message) {
    bool locked = false;

    if (gSerialMutex) {
        locked = xSemaphoreTake(gSerialMutex, pdMS_TO_TICKS(100)) == pdTRUE;
        if (!locked) {
            return;
        }
    }

    Serial.println(message);

    if (locked) {
        xSemaphoreGive(gSerialMutex);
    }
}

/* =============================================================================
   Non-Blocking Wi-Fi Reconnection Manager
   ============================================================================= */

void connectWiFiNonBlocking() {
    if (WiFi.status() == WL_CONNECTED) {
        if (!wifiWasConnected) {
            wifiWasConnected = true;
            printSerialLine(String("[WiFi] Connected, IP: ") + WiFi.localIP().toString());
        }
        return;
    }

    wifiWasConnected = false;

    unsigned long now = millis();

    if (now - lastWifiAttempt < WIFI_RETRY_MS) {
        return;
    }

    lastWifiAttempt = now;

    WiFi.disconnect(false);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

    printSerialLine("[WiFi] Reconnect attempt...");
}

/* =============================================================================
   FreeRTOS Task: Wi-Fi Manager
   ============================================================================= */

void wifiTask(void* pv) {
    while (true) {
        connectWiFiNonBlocking();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* =============================================================================
   FreeRTOS Task: Smart Gate
   ============================================================================= */

void gateTask(void* pv) {
    while (true) {
        loopSmartGate();
        vTaskDelay(pdMS_TO_TICKS(TASK_DELAY_MS));
    }
}

/* =============================================================================
   FreeRTOS Task: Smart Garbage
   ============================================================================= */

void garbageTask(void* pv) {
    while (true) {
        loopSmartGarbage();
        vTaskDelay(pdMS_TO_TICKS(TASK_DELAY_MS));
    }
}

/* =============================================================================
   FreeRTOS Task: Smart Parking
   ============================================================================= */

void parkingTask(void* pv) {
    while (true) {
        loopSmartParking();
        vTaskDelay(pdMS_TO_TICKS(TASK_DELAY_MS));
    }
}

/* =============================================================================
   FreeRTOS Task: Smart Lighting
   ============================================================================= */

void lightingTask(void* pv) {
    while (true) {
        loopSmartLighting();
        vTaskDelay(pdMS_TO_TICKS(TASK_DELAY_MS));
    }
}

/* =============================================================================
   FreeRTOS Task: Smoke Detector

   Priority:
   - This task is created with priority 3.
   - Other subsystem tasks use priority 1.
   - This gives smoke detection faster CPU access.
   ============================================================================= */

void smokeTask(void* pv) {
    while (true) {
        loopSmokeDetector();
        vTaskDelay(pdMS_TO_TICKS(SMOKE_TASK_DELAY_MS));
    }
}

/* =============================================================================
   FreeRTOS Task: Smart Irrigation
   ============================================================================= */

void irrigationTask(void* pv) {
    while (true) {
        loopSmartIrrigation();
        vTaskDelay(pdMS_TO_TICKS(TASK_DELAY_MS));
    }
}

static void createTaskChecked(TaskFunction_t task,
                              const char* name,
                              uint32_t stackSize,
                              UBaseType_t priority,
                              BaseType_t core) {
    BaseType_t result = xTaskCreatePinnedToCore(
        task,
        name,
        stackSize,
        NULL,
        priority,
        NULL,
        core
    );

    if (result != pdPASS) {
        Serial.print("[System] Failed to create task: ");
        Serial.println(name);
    }
}

/* =============================================================================
   Arduino Setup
   ============================================================================= */

void setup() {
    Serial.begin(115200);

    // Create mutexes before starting tasks
    gHttpMutex = xSemaphoreCreateMutex();
    gSerialMutex = xSemaphoreCreateMutex();

    // Start Wi-Fi in station mode
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

    // Initialize all subsystem hardware
    setupSmartGate();
    setupSmartGarbage();
    setupSmartParking();
    setupSmartLighting();
    setupSmokeDetector();
    setupSmartIrrigation();

    /*
      Task Priority Plan:
      - WiFi Task:        priority 2
      - Smoke Task:       priority 3  highest priority because it is safety-critical
      - Other Modules:    priority 1
    */

    createTaskChecked(wifiTask,       "WiFi",       4096, 2, 0);

    createTaskChecked(gateTask,       "Gate",       4096, 1, 1);
    createTaskChecked(garbageTask,    "Garbage",    4096, 1, 1);
    createTaskChecked(parkingTask,    "Parking",    4096, 1, 1);
    createTaskChecked(lightingTask,   "Lighting",   4096, 1, 1);
    createTaskChecked(irrigationTask, "Irrigation", 4096, 1, 1);

    // Smoke task gets higher priority and larger stack
    createTaskChecked(smokeTask,      "Smoke",      6144, 3, 1);

    Serial.println("[System] Smart Campus Integrated IoT System Started");
    Serial.println("[System] Smoke Detector Task Running With High Priority");
}

/* =============================================================================
   Arduino Main Loop

   Note:
   - Subsystems are handled by FreeRTOS tasks.
   - Main loop stays idle.
   ============================================================================= */

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
