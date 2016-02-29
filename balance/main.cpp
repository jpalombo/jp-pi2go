#include "MotionSensor/balance.h"

#define delay_ms(a) usleep(a*1000)

int main() {
    balance_init();
    do{
	balance_loop();
    }while(1);
    return 0;
}
