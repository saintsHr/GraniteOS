#include "terminal.h"
#include "helpers.h"

void kernel_main(void) {
    char t = 0x01;
    while (1){
        wait(10000000);
        clear(0x00);
        print(" Hello from Granite!", t);
        t++;
        if (t > 0x0F) t = 0x01;
    }
}