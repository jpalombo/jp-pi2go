#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

int ms_open();
int ms_update();
int angle_err();
int gyro_err();
int ms_close();

#endif
