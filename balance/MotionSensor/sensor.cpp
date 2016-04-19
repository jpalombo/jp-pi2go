#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "sensor.h"
#include <wiringPi.h>
#include "Monitor.h"

extern "C" {
#include "inv_mpu_lib/inv_mpu.h"
#include "inv_mpu_lib/inv_mpu_dmp_motion_driver.h"
}

// MPU control/status vars
unsigned long timestamp;

#define DIM 3
short accel[DIM]; // [x, y, z]           accel vector
short gyro[DIM];  // [x, y, z]           gyro vector
int gyrotrim = 200;
int acceltrim = 5000;

extern Monitor * monitor;

int mpu_open() {

    unsigned char devStatus; // return status after each device operation

    //monitor->watch(&gyrotrim, "gyrotrim");
    //monitor->watch(&acceltrim, "acceltrim");
    
    // initialize device
    printf("Initializing MPU...\n");
    if (mpu_init(NULL) != 0) {
        printf("MPU init failed!\n");
        return -1;
    }
    printf("Setting MPU sensors...\n");
    if (mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL) != 0) {
        printf("Failed to set sensors!\n");
        return -1;
    }
    printf("Setting GYRO sensitivity...\n");
    if (mpu_set_gyro_fsr(250) != 0) {
        printf("Failed to set gyro sensitivity!\n");
        return -1;
    }
    printf("Setting ACCEL sensitivity...\n");
    if (mpu_set_accel_fsr(2) != 0) {
        printf("Failed to set accel sensitivity!\n");
        return -1;
    }
    if (mpu_set_compass_sample_rate(10) != 0) {
        printf("Failed to set compass sample rate!\n");
        return -1;
    }
    // verify connection
    printf("Powering up MPU...\n");
    mpu_get_power_state(&devStatus);
    printf(devStatus ? "MPU6050 connection successful\n" : "MPU6050 connection failed %u\n", devStatus);

    // calibrating
    int gyrosum = 0;
    int count = 0;
    printf("Calibrating...\n");
    while (count < 300)
    {
        if (mpu_get_gyro_reg(gyro, &timestamp) == 0) {
            gyrosum += gyro[0];
            count++;
        }
        delay(1);
    }
    gyrotrim = gyrosum/count;
    printf("Gyro offset : %i\n", gyrotrim);
    
    
    printf("Done.\n");
    return 0;
}

int sensorAngle(){
    if (mpu_get_accel_reg(accel, &timestamp) == 0)
        return accel[2] - acceltrim;
    else
        return 0;
}

void updateAngleTrim(int offset) {
    acceltrim += offset;
}

int sensorGyro() {
    if (mpu_get_gyro_reg(gyro, &timestamp) == 0)
        return gyro[0] - gyrotrim;
    else
        return 0;
}

void updateGyroTrim(int offset) {
    gyrotrim += offset;
}

int wheelcount()
{
    
}

int ms_close() {
    return 0;
}
