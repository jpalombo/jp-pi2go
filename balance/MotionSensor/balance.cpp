/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <stdio.h>
#include <unistd.h>
#include "sensor.h"
#include <wiringPi.h>

void balance_init()
{
    wiringPiSetupPhys ();
    ms_open();
}

void balance_loop()
{
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
    ms_update();
    delay(10);
}