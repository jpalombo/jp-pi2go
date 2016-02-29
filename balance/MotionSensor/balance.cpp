/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "sensor.h"
#include <wiringPi.h>
#include "pi2golite.h"

void balance_init()
{
    wiringPiSetupPhys ();
    pi2go::init();
    ms_open();
}

void timeloop() {
    static int loopcount = 0;
    static unsigned int lastsecs = 0;
    unsigned int thissecs; 
    
    thissecs = micros();
    thissecs >>= 20;
    loopcount++;
    
    if (thissecs != lastsecs)
    {
        lastsecs = thissecs;
        printf("Loops per sec = %i\n", loopcount);
        loopcount = 0;
    }
}

int maxangle = 300;

void balance_loop()
{
    // Stats on how many loops per second we are getting
    timeloop();
    
    int angle = angle_err();
    int gyro = gyro_err();
    
    printf("Angle Err: %i \tGyro Err: %i\n", angle, gyro);
    
    if (abs(angle) > maxangle) {
        // we're done
        pi2go::stop();
        return;
    }
    
    int speed = angle * 100 / maxangle;
    pi2go::goBoth(speed);
    
    delay(10);
}