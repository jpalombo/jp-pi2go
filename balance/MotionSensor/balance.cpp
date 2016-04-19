/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <iostream>
#include <unistd.h>
#include <algorithm>
#include "sensor.h"
#include <wiringPi.h>
#include "pi2golite.h"
#include "Monitor.h"

Monitor * monitor;
// Global state variables that can be watched
int g_angle = 0;
int g_gyro = 0;
int g_speed = 0;
int g_speedint = 0;
int g_gyroint = 0;
int g_angleavg = 0;
int g_angleint = 0;
int g_gyrointerr = 0;
int g_gyrointerrint = 0;
int g_wheelcount = 0;
int g_wheelspeed = 0;

int gyrointoffset = 2220000;
int maxangle = 400;

int gyrointfactor = 3500000; //P
int wheelfactor = 300;     //I
int gyrofactor = 400000;     //D
int speedfactor = 10000;

int g_P_angle;
int g_I;
int g_D;
int g_P_speed;

int g_Kp_angle = -800;
int g_Ki = 200;
int g_Kd = -1000;
int g_Kp_speed = 800;

int loopspersec = 500;
int avgratio = 100;
int targetspeed = 0;

void balance_init()
{
    monitor = new Monitor();
    wiringPiSetupPhys ();
    pi2go::init();
    mpu_open();
    
    // Add variables to monitor
    //monitor->watch(&g_angle, "angle");
    //monitor->watch(&g_gyro, "gyro");
    //monitor->watch(&gyrointoffset, "gyro integral offset");
    //monitor->watch(&g_gyrointerr, "gyro integral err");
    //monitor->watch(&g_angleavg, "angle average");
    //monitor->watch(&g_angleint, "angle integral");
    monitor->watch(&g_speed, "wheel speed");
    //monitor->watch(&g_speedint, "speed int");
    //monitor->watch(&g_wheelcount, "wheelcount");
    //monitor->watch(&g_wheelspeed, "wheelspeed");
    monitor->watch(&g_P_angle, "P angle");
    monitor->watch(&g_P_speed, "P speed");
    monitor->watch(&g_I, "I");
    monitor->watch(&g_D, "D");
    //monitor->watch(&g_D2, "D2");
    
    monitor->control(&g_Kp_angle, "Kp angle", 5000, -5000);
    monitor->control(&g_Kp_speed, "Kp speed", 5000, -5000);
    monitor->control(&g_Ki, "Ki", 5000, -5000);
    monitor->control(&g_Kd, "Kd", 5000, -5000);
}

void balance_term() {
    pi2go::stop();
    delete monitor;
}

int timeloop(int loopcount) {
    static int lastloopcount = 0;
    static unsigned int lastsecs = 0;
    unsigned int thissecs; 
    
    thissecs = millis() >> 10;
    if (thissecs != lastsecs)
    {
        std::cout << "Loops per sec = " << loopcount - lastloopcount << std::endl;
        lastsecs = thissecs;
        lastloopcount = loopcount;
    }
}

int dir(int val) 
{
    if (val > 0)
        return 1;
    else if (val < 0)
        return -1;
    else
        return 0;
}

void balance_loop()
{
    static int loopcount = 0;
    static bool started = false;
    static int impulsecount = 0;
    static int lastgyrointegral = 0;
    static int lastangleint = 0;
    int newspeed;
    
    loopcount++;
    timeloop(loopcount);  // Print how many loops per second we are getting
    if ((loopcount % loopspersec) == 0) {
        // Here roughly once per second
        updateGyroTrim(dir(g_gyroint - lastgyrointegral));
        lastgyrointegral = g_gyroint;
        if (started) {
            updateAngleTrim(30 * dir(g_angleint - lastangleint));
            lastangleint = g_angleint;
        }
    }
    
    g_angle = sensorAngle();
    g_angleavg = ((g_angleavg * avgratio) + g_angle) / (avgratio + 1);
    if (started) {
        g_angleint += g_angle;
        //gyrointoffset += g_P;
    }
    g_gyro = sensorGyro();
    g_gyroint += (g_gyro >> 1);   // Divide by 2 to stop overflow on gyroint
    g_gyrointerr = g_gyroint - gyrointoffset;
    g_wheelcount = pi2go::wheelCount();
    g_wheelspeed = pi2go::speed();
    
    if (!started && abs(g_gyro) < 10 && abs(g_angle) > 10000) {
        g_gyroint = 0;  // Zero the gyroint when we are on the floor
    }
    
    if (!started && abs(g_angle) < 100) {
        started = true;
        std::cout << "started 1 " << g_angle << std::endl;
        g_angleint = 0;
        g_speedint = 0;
    }
   
    g_P_angle  = g_Kp_angle * g_gyrointerr / gyrointfactor;  //P angle
    g_P_speed = g_Kp_speed * g_wheelspeed / speedfactor;
    g_I  = g_Ki * g_wheelcount / wheelfactor;    //I
    g_D = g_Kd * g_gyro / gyrofactor;           //D
    
    newspeed = g_P_speed + g_P_angle + g_I + g_D;

    if (!started) {
        return;
    }
    
    /*
    if (abs(g_wheelcount > 10) && (dir(newspeed) * dir(g_wheelcount)) > 0) {
        // We are starting to drift and the speed is in the same direction as the drift
        // Add an impulse to reverse direction.  Do this by increasing
        // Motor speed for a short period
        newspeed = newspeed * 2;
        impulsecount = 10;
    }
    */
    
    // Add the average speed over the last half second.
    // Half second should include two wobbles, so is a decent long term average
    // Add it because that will tend to stop forward motion
    
    //g_speed += g_wheelcount;
    
    
    if (abs(newspeed) > 100){
        pi2go::stop();
        started = false;
        std::cout << "stopped 2" << std::endl;
        g_speed = 0;
        return;
    }
    
    g_speed = newspeed;
    g_speedint += g_speed;
    pi2go::goBoth(g_speed);
    
}