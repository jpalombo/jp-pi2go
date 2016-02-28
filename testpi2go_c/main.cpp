/* 
 * File:   main.cpp
 * Author: John
 *
 * Created on 28 February 2016, 19:13
 */

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <wiringPi.h>
#include "pi2golite.h"

int main(int argc, char** argv) {

    printf("metabot.py -h -d -i <ip address>\n");
    printf("  -h   help\n");
    printf("  -d   debug\n");
    printf("  -i   set local ip address\n\n");
    printf("keys : \n");
    printf("  'w, up-arrow      forward\n");
    printf("  z, down arrow    reverse\n");
    printf("  s, right arrow   spin right\n");
    printf("  d, left arrow    spin left\n");
    printf("  >, .             increase speed\n");
    printf("  <, ,             decrease speed\n");
    printf("  space            stop\n");
    printf("  i                IR light status\n");
    printf("  f                toggle front LED\n");
    printf("  r                toggle rear LED\n");
    printf("  u                ultrasound distance\n");
    printf("  t                switch status\n");
    printf("  1,2,..9          move forward n steps\n");
    printf("  End, ^C, q       exit\n\n");

    int frontLED = 0;
    int rearLED = 0;

    wiringPiSetupPhys();
    init();

    int speed = 30;
    char keyp;
    bool done = false;

    // main loop
    while (!done) {
        std::cin >> keyp;
        switch (keyp) {
            case 'w':
                go(speed, speed);
                printf("Forward %i\n", speed);
                break;
            case 'z':
                go(-speed, -speed);
                printf("Reverse %i\n", speed);
                break;
            case 's':
                go(speed, -speed);
                printf("Spin Right %i\n", speed);
                break;
            case 'a':
                go(-speed, speed);
                printf("Spin Left %i\n", speed);
                break;
            case '.':
            case '>':
                speed = std::min(100, speed + 10);
                printf("Speed+ %i\n", speed);
                break;
            case ',':
            case '<':
                speed = std::max(0, speed - 10);
                printf("Speed- %i\n", speed);
                break;
            case ' ':
                go(0, 0);
                printf("Stop\n");
                break;
            case 'i':
                printf("Left: %i\n", irLeft());
                printf("Right: %i\n", irRight());
                printf("Line left %i\n", irLeftLine());
                printf("Line right %i\n", irRightLine());
                break;
            case 'f':
                frontLED = (frontLED + 1) % 2;
                LsetLED(0, frontLED);
                printf("Toggle front LEDs\n");
                break;
            case 'r':
                rearLED = (rearLED + 1) % 2;
                LsetLED(1, rearLED);
                printf("Toggle rear LEDs\n");
                break;
            case 'u':
                printf("Distance: %i\n", getDistance());
                break;
            case 't':
                printf("Switch: %i\n", getSwitch());
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                printf("Step Forward: %c\n", keyp);
                stepForward(20, keyp - '0');
                break;
            case 'q':
                done = true;
                break;
        }
    }

    cleanup();
    return 0;
}


