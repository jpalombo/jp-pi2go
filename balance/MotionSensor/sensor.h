#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

int mpu_open();
int ms_update();
int sensorAngle();
int sensorGyro();
void updateGyroTrim(int offset);
void updateAngleTrim(int offset);
int ms_close();

#endif
