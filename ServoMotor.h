#ifndef SERVO_MOTOR_H
#define SERVO_MOTOR_H

/*
  ==============================================================================
  Canonical Driver: Servo Motor

  Purpose:
  - Controls servo using ESP32 LEDC PWM.
  - Used by Smart Gate module.
  ==============================================================================
*/

class ServoMotor {
private:
    int servoPin;
    int freq;
    int resolution;

    int angleToDuty(int angle);

public:
    ServoMotor(int pin);
    void begin();
    void moveToAngle(int angle);
};

#endif