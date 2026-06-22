#ifndef SMART_IRRIGATION_H
#define SMART_IRRIGATION_H

/*
  ==============================================================================
  Module: Smart Irrigation

  Public Interface:
  - setupSmartIrrigation()
  - loopSmartIrrigation()
  - test_SmartIrrigation()

  Description:
  - Reads soil moisture percentage.
  - Controls pump/valve relay.
  - Supports AUTO, MANUAL_OPEN, and MANUAL_CLOSE from server.
  - Reports humidity and valve state to server.
  ==============================================================================
*/

void setupSmartIrrigation();
void loopSmartIrrigation();
void test_SmartIrrigation();

#endif