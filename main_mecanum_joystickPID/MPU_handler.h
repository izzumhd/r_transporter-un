#ifndef MPU6050_SENSOR_H
#define MPU6050_SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include "MPU6050_6Axis_MotionApps20.h"

class MPU6050Sensor {
private:
    MPU6050 mpu;

    bool dmpReady = false;
    uint8_t devStatus;
    uint8_t fifoBuffer[64];

    Quaternion q;
    VectorFloat gravity;
    float ypr[3];

    float yaw = 0;
    float pitch = 0;
    float roll = 0;

    float yawOffset = 0;

public:

    bool begin() {

        Wire.begin();
        Wire.setClock(400000);

        mpu.initialize();

        devStatus = mpu.dmpInitialize();

        mpu.setXAccelOffset(-2484);
        mpu.setYAccelOffset(564);
        mpu.setZAccelOffset(1182);

        mpu.setXGyroOffset(73);
        mpu.setYGyroOffset(34);
        mpu.setZGyroOffset(-75);

        if (devStatus != 0)
            return false;

        mpu.setDMPEnabled(true);
        dmpReady = true;

        return true;
    }

    bool update() {

        if (!dmpReady)
            return false;

        if (!mpu.dmpGetCurrentFIFOPacket(fifoBuffer))
            return false;

        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

        yaw   = ypr[0] * 180.0f / M_PI;
        pitch = ypr[1] * 180.0f / M_PI;
        roll  = ypr[2] * 180.0f / M_PI;

        return true;
    }

    float getYaw() {
        return yaw;
    }

    float getPitch() {
        return pitch;
    }

    float getRoll() {
        return roll;
    }

    void resetYaw() {
        yawOffset = yaw;
    }

    float getRelativeYaw() {
        return yaw - yawOffset;
    }
};

#endif