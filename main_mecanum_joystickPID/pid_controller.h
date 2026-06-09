// PID Controller untuk posisi (relatif ke robot) 
// dan headings/arah jalan, dan pid lain jika dibutuhkan
#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>

class PIDController {
private:
  float kp;
  float ki;
  float kd;
  
  float integral;
  float prevError;
  
  float outputMin;
  float outputMax;
  
  unsigned long lastTime;
  bool firstRun;
  
public:
  PIDController(float p, float i, float d) 
    : kp(p), ki(i), kd(d), integral(0), prevError(0),
      outputMin(-1e6), outputMax(1e6), lastTime(0), firstRun(true) {
  }
  
  void setGains(float p, float i, float d) {
    kp = p; // set kp ki kd dari object(p, i, d)
    ki = i;
    kd = d;
  }
  
  void setOutputLimits(float min, float max) {
    outputMin = min;
    outputMax = max;
  }
  
  float compute(float setpoint, float measurement) {
    unsigned long currentTime = millis();
    
    if (firstRun) {
      lastTime = currentTime;
      prevError = setpoint - measurement;
      firstRun = false;
      return 0;
    }
    
    float dt = (currentTime - lastTime) / 1000.0f; // convert to seconds
    lastTime = currentTime;
    
    if (dt <= 0) dt = 0.01f; // prevent division by zero
    
    // Calculate error
    float error = setpoint - measurement;
    
    // Proportional term
    float pTerm = kp * error;
    
    // Integral term
    integral += error * dt;
    float iTerm = ki * integral;
    
    // Derivative term
    float derivative = (error - prevError) / dt;
    float dTerm = kd * derivative;
    
    prevError = error;
    
    // Calculate output
    float output = pTerm + iTerm + dTerm;
    
    // Apply output limits
    if (output > outputMax) {
      output = outputMax;
      // Anti-windup: stop integrating if saturated
      integral -= error * dt;
    } else if (output < outputMin) {
      output = outputMin;
      // Anti-windup
      integral -= error * dt;
    }
    
    return output;
  }
  
  void reset() {
    integral = 0;
    prevError = 0;
    firstRun = true;
  }
};

#endif // PID_CONTROLLER_H
