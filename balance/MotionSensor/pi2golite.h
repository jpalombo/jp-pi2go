/* 
 * File:   pi2golite.h
 * Author: John
 *
 * Created on 28 February 2016, 16:24
 */

#ifndef PI2GOLITE_H
#define PI2GOLITE_H

namespace pi2go {

    void init();
    void cleanup();
    void stop();
    void forward(int speed);
    void reverse(int speed);
    void spinLeft(int speed);
    void spinRight(int speed);
    void turnForward(int leftSpeed, int rightSpeed);
    void turnReverse(int leftSpeed, int rightSpeed);
    void go(int leftSpeed, int rightSpeed);
    void goBoth(int speed);
    void stopL();
    void stopR();
    void stepForward(int speed, int counts);
    void stepReverse(int speed, int counts);
    void stepSpinL(int speed, int counts);
    void stepSpinR(int speed, int counts);
    void LsetLED(int LED, int value);
    bool irLeft();
    bool irRight();
    bool irAll();
    bool irLeftLine();
    bool irRightLine();
    float getDistance();
    bool getSwitch();
}
#endif /* PI2GOLITE_H */

