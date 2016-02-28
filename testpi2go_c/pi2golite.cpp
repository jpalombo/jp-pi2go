/* 
 * File:   pi2golite.cpp
 * Author: John
 *
 * Created on 28 February 2016, 16:24
 */

//include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <wiringPi.h>
#include <softPwm.h>
#include "pi2golite.h"

// Pins 24, 26 Left Motor
// Pins 19, 21 Right Motor
#define L1 26
#define L2 24
#define R1 19
#define R2 21

//=======
// Define obstacle sensors and line sensors
#define irFL 7       // Front Left obstacle sensor
#define irFR 11      // Front Right obstacle sensor
#define lineRight 13 // Right Line sensor
#define lineLeft 12  // Left Line sensor

// Define GPIO pins for Front/rear LEDs on Pi2Go-Lite
#define frontLED 15
#define rearLED 16

// Define Sonar Pin (same pin for both Ping and Echo
#define sonar 8

// Define pins for switch (different on each version)
#define Lswitch 23

// Global variables for wheel sensor counting
bool running = true;
int countL = 0;
int countR = 0;

int leftCount = 0;
int rightCount = 0;
int lastL = 0;
int lastR = 0;

pthread_t threadC;
void * wheelCount(void * param);

//======================================================================
// General Functions
//
// init(). Initialises GPIO pins, switches motors and LEDs Off, etc

void init() {

    //use physical pin numbering
    //wiringPiSetupPhys ()

    //set up digital line detectors as inputs
    pinMode(lineRight, INPUT); // Right line sensor
    pinMode(lineLeft, INPUT); // Left line sensor

    //Set up IR obstacle sensors as inputs
    pinMode(irFL, INPUT); // Left obstacle sensor
    pinMode(irFR, INPUT); // Right obstacle sensor

    //use pwm on inputs so motors don't go too fast
    pinMode(L1, PWM_OUTPUT);
    if (softPwmCreate(L1, 0, 100) != 0)
        printf("Failed to init PWM: %i", errno);
    pinMode(L2, PWM_OUTPUT);
    if (softPwmCreate(L2, 0, 100) != 0)
        printf("Failed to init PWM: %i", errno);
    pinMode(R1, PWM_OUTPUT);
    if (softPwmCreate(R1, 0, 100) != 0)
        printf("Failed to init PWM: %i", errno);
    pinMode(R2, PWM_OUTPUT);
    if (softPwmCreate(R2, 0, 100) != 0)
        printf("Failed to init PWM: %i", errno);

    //set up Pi2Go-Lite White LEDs as outputs
    pinMode(frontLED, OUTPUT);
    digitalWrite(frontLED, 1);    // switch front LEDs off as they come on by default
    pinMode(rearLED, OUTPUT);
    digitalWrite(rearLED, 1);    // switch rear LEDs off as they come on by default

    //set switch as input with pullup
    pinMode(Lswitch, INPUT);
    pullUpDnControl (Lswitch, PUD_UP); 

    // initialise wheel counters
    running = true;
    pthread_create(&threadC, NULL, wheelCount, NULL );
}

// cleanup(). Sets all motors and LEDs off and sets GPIO to standard values
void cleanup(){
    running = false;
    stop();
    delay(100);
    pthread_join(threadC, NULL);
}

// End of General Functions
//======================================================================


//======================================================================
// Motor Functions
// (both versions)
//
// stop(): Stops both motors
void stop(){
    softPwmWrite (L1, 0);
    softPwmWrite (L2, 0);
    softPwmWrite (R1, 0);
    softPwmWrite (R2, 0);
}

// forward(speed): Sets both motors to move forward at speed. 0 <= speed <= 100
void forward(int speed){
    softPwmWrite (L1, speed);
    softPwmWrite (L2, 0);
    softPwmWrite (R1, speed);
    softPwmWrite (R2, 0);
}

// reverse(speed): Sets both motors to reverse at speed. 0 <= speed <= 100
void reverse(int speed){
    softPwmWrite (L1, 0);
    softPwmWrite (L2, speed);
    softPwmWrite (R1, 0);
    softPwmWrite (R2, speed);
}

// spinLeft(speed): Sets motors to turn opposite directions at speed. 0 <= speed <= 100
void spinLeft(int speed){
    softPwmWrite (L1, 0);
    softPwmWrite (L2, speed);
    softPwmWrite (R1, speed);
    softPwmWrite (R2, 0);
}

// spinRight(speed): Sets motors to turn opposite directions at speed. 0 <= speed <= 100
void spinRight(int speed){
    softPwmWrite (L1, speed);
    softPwmWrite (L2, 0);
    softPwmWrite (R1, 0);
    softPwmWrite (R2, speed);
}

// turnForward(leftSpeed, rightSpeed): Moves forwards in an arc by setting different speeds. 0 <= leftSpeed,rightSpeed <= 100
void turnForward(int leftSpeed, int rightSpeed){
    softPwmWrite (L1, leftSpeed);
    softPwmWrite (L2, 0);
    softPwmWrite (R1, rightSpeed);
    softPwmWrite (R2, 0);
}

// turnReverse(leftSpeed, rightSpeed): Moves backwards in an arc by setting different speeds. 0 <= leftSpeed,rightSpeed <= 100
void turnReverse(int leftSpeed, int rightSpeed){
    softPwmWrite (L1, 0);
    softPwmWrite (L2, leftSpeed);
    softPwmWrite (R1, 0);
    softPwmWrite (R2, rightSpeed);
}

// go(leftSpeed, rightSpeed): controls motors in both directions independently using different positive/negative speeds. -100<= leftSpeed,rightSpeed <= 100
void go(int leftSpeed, int rightSpeed){
    if (leftSpeed<0){
        softPwmWrite (L1, 0);
        softPwmWrite (L2, -leftSpeed);
    }
    else {
        softPwmWrite (L1, leftSpeed);
        softPwmWrite (L2, 0);
    }
    if (rightSpeed<0) {
        softPwmWrite (R1, 0);
        softPwmWrite (R2, -rightSpeed);
    }
    else {
        softPwmWrite (R1, rightSpeed);
        softPwmWrite (R2, 0);
    }
}

// go(speed): controls motors in both directions together with positive/negative speed parameter. -100<= speed <= 100
void goBoth(int speed){
    if (speed<0)
        reverse(-speed);
    else
        forward(speed);
}

// End of Motor Functions
//======================================================================


//======================================================================
// Wheel Sensor Functions
// (Pi2Go-Lite only)

void stopL(){
     softPwmWrite (L1, 0);
     softPwmWrite (L2, 0);
}

void stopR(){
     softPwmWrite (R1, 0);
     softPwmWrite (R2, 0);
}

void * wheelCount(void * param){
    int inval;
    int lastValidL = 2;
    int lastValidR = 2;
    int lastL = digitalRead(lineLeft);
    int lastR = digitalRead(lineRight);
    while (running) {
        delay(2);
        inval = digitalRead(lineLeft);
        if (inval == lastL && inval != lastValidL) {
            countL++;
            lastValidL = inval;
        }
        lastL = inval;
        inval = digitalRead(lineRight);
        if (inval == lastR and inval != lastValidR) {
            countR++;
            lastValidR = inval;
        }
        lastR = inval;
    }
}

// stepForward(speed, steps): Moves forward specified number of counts, then stops
void stepForward(int speed, int counts) {
    countL = 0;
    countR = 0;
    bool runL = true;
    bool runR = true;
    turnForward(speed, speed);
    while (runL || runR) {
        delay(2);
        if (countL >= counts) {
            stopL();
            runL = false;
        }
        if (countR >= counts) {
            stopR();
            runR = false;
        }
    }
}

// stepReverse(speed, steps): Moves backward specified number of counts, then stops
void stepReverse(int speed, int counts) {
    countL = 0;
    countR = 0;
    bool runL = true;
    bool runR = true;
    turnReverse(speed, speed);
    while (runL || runR) {
        delay(2);
        if (countL >= counts) {
            stopL();
            runL = false;
        }
        if (countR >= counts) {
            stopR();
            runR = false;
        }
    }
}

// stepSpinL(speed, steps): Spins left specified number of counts, then stops
void stepSpinL(int speed, int counts) {
    countL = 0;
    countR = 0;
    spinLeft(speed);
    while (countL<counts || countR<counts) {
        delay(2);
        if (countL >= counts)
            stopL();
        if (countR >= counts)
            stopR();
    }
}

// stepSpinR(speed, steps): Spins right specified number of counts, then stops
void stepSpinR(int speed, int counts) {
    countL = 0;
    countR = 0;;
    spinRight(speed);
    while (countL<counts or countR<counts) {
        delay(2);
        if (countL >= counts)
            stopL();
        if (countR >= counts)
            stopR();
    }
}

// End of Motor Functions
//======================================================================


//======================================================================
// White LED Functions
//
// LsetLED(LED, value): Sets the LED specified to OFF == 0 or ON == 1
// TODO: take value from 0 to 100 and use as percentage PWM value
void LsetLED (int LED, int value){
    value = (value + 1)%2;
    if (LED == 0)
        digitalWrite (frontLED, value);
    else
        digitalWrite (rearLED, value);
}

// LsetAllLEDs(value): Sets both LEDs to OFF == 0 or ON == 1

// End of White LED Functions
//======================================================================


//======================================================================
// IR Sensor Functions
//
// irLeft(): Returns state of Left IR Obstacle sensor
bool irLeft(){
    if (digitalRead(irFL)==0)
        return true;
    else
        return false;
}

// irRight(): Returns state of Right IR Obstacle sensor
bool irRight() {
    if (digitalRead(irFR)==0)
        return true;
    else
        return false;
}

// irAll(): Returns true if any of the Obstacle sensors are triggered
bool irAll(){
    if (digitalRead(irFL)==0 || digitalRead(irFR)==0)
        return true;
    else
        return false;
}

// irLeftLine(): Returns state of Left IR Line sensor
bool irLeftLine() {
    if (digitalRead(lineLeft)==0)
        return true;
    else
        return false;
}

// irRightLine(): Returns state of Right IR Line sensor
bool irRightLine() {
    if (digitalRead(lineRight)==0)
        return true;
    else
        return false;
}

// End of IR Sensor Functions
//======================================================================


//======================================================================
// UltraSonic Functions
//
// getDistance(). Returns the distance in cm to the nearest reflecting object. 0 == no object
// (Both versions)
//
float getDistance() {
    pinMode(sonar, OUTPUT);
    // Send 10us pulse to trigger
    digitalWrite(sonar, true);
    delayMicroseconds(10);
    digitalWrite(sonar, false);
    unsigned int start = micros();
    unsigned int count = micros();
    pinMode(sonar,INPUT);
    while (digitalRead(sonar)==0 && micros()- count< 100000)
        start = micros ();
    count = micros();
    unsigned int stop = count;
    while (digitalRead(sonar)==1 && micros() - count<100000) 
        stop = micros();
    // Calculate pulse length
    unsigned int elapsed = stop-start;
    // Distance pulse travelled in that time is time
    // multiplied by the speed of sound 34000(cm/s) divided by 2
    float distance = elapsed * 17000 / 1000000;
    return distance;
}

// End of UltraSonic Functions    
//======================================================================


//======================================================================
// Switch Functions
// 
// getSwitch(). Returns the value of the tact switch: True==pressed
bool getSwitch() {
    return (digitalRead(Lswitch) == 0);
}
//
// End of switch functions
//======================================================================
