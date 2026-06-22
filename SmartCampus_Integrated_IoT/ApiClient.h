#ifndef API_CLIENT_H
#define API_CLIENT_H

/*
  ==============================================================================
  Project: SmartCampus_Integrated_IoT
  File: ApiClient.h

  Purpose:
  - Shared HTTP API wrapper.
  - Prevents duplicated HTTPClient code inside every subsystem.
  - Provides reusable GET and POST JSON methods.
  ==============================================================================
*/

#include <Arduino.h>

class ApiClient {
public:
    /*
      Performs HTTP GET request.

      Parameters:
      - url: target API endpoint
      - response: returned server response body

      Returns:
      - true if HTTP 200 received
      - false otherwise
    */
    static bool get(const char* url, String& response);

    /*
      Performs HTTP POST request with JSON body.

      Parameters:
      - url: target API endpoint
      - payload: JSON string payload
      - statusCode: optional pointer to store HTTP response code

      Returns:
      - true if HTTP 200 or 201 received
      - false otherwise
    */
    static bool postJson(const char* url, const String& payload, int* statusCode = nullptr);
};

#endif