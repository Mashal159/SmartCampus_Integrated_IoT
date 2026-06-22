#include "ApiClient.h"
#include "config.h"

#include <WiFi.h>
#include <HTTPClient.h>

/*
  ==============================================================================
  File: ApiClient.cpp

  Purpose:
  - Implements safe shared HTTP communication.
  - Uses gHttpMutex to avoid multiple FreeRTOS tasks using HTTPClient at the same time.
  ==============================================================================
*/

#define HTTP_TIMEOUT_MS 400UL
#define HTTP_MUTEX_WAIT_MS 250UL

/* =============================================================================
   HTTP GET Request
   ============================================================================= */

bool ApiClient::get(const char* url, String& response) {
    // Do not attempt API request if Wi-Fi is disconnected
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    // Lock HTTP resource
    bool locked = false;

    if (gHttpMutex) {
        locked = xSemaphoreTake(gHttpMutex, pdMS_TO_TICKS(HTTP_MUTEX_WAIT_MS)) == pdTRUE;
        if (!locked) {
            return false;
        }
    }

    HTTPClient http;
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT_MS);

    int code = http.GET();
    bool success = false;

    if (code == 200) {
        response = http.getString();
        success = true;
    }

    http.end();

    // Release HTTP resource
    if (locked) {
        xSemaphoreGive(gHttpMutex);
    }

    return success;
}

/* =============================================================================
   HTTP POST JSON Request
   ============================================================================= */

bool ApiClient::postJson(const char* url, const String& payload, int* statusCode) {
    // Do not attempt API request if Wi-Fi is disconnected
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }

    // Lock HTTP resource
    bool locked = false;

    if (gHttpMutex) {
        locked = xSemaphoreTake(gHttpMutex, pdMS_TO_TICKS(HTTP_MUTEX_WAIT_MS)) == pdTRUE;
        if (!locked) {
            if (statusCode) {
                *statusCode = -1;
            }
            return false;
        }
    }

    HTTPClient http;
    http.begin(url);
    http.setTimeout(HTTP_TIMEOUT_MS);
    http.addHeader("Content-Type", "application/json");

    int code = http.POST(payload);

    if (statusCode) {
        *statusCode = code;
    }

    http.end();

    // Release HTTP resource
    if (locked) {
        xSemaphoreGive(gHttpMutex);
    }

    return code == 200 || code == 201;
}
