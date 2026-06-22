#ifndef SMART_LIGHTING_H
#define SMART_LIGHTING_H

/*
  ==============================================================================
  Module: Smart Lighting

  Public Interface:
  - setupSmartLighting()
  - loopSmartLighting()
  - test_SmartLighting()

  Description:
  - Controls campus lighting using:
      1. Two ultrasonic sensors for presence detection
      2. One LDR sensor for darkness detection
      3. Three LED outputs
      4. Server mode control: AUTO / MANUAL_ON / MANUAL_OFF
  ==============================================================================
*/

void setupSmartLighting();
void loopSmartLighting();
void test_SmartLighting();

#endif