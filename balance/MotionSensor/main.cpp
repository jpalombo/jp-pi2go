#include "balance.h"

int main() {
    balance_init();
    do{
	balance_loop();
    }while(1);
    balance_term();
    return 0;
}
