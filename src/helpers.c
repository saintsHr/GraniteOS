#include "helpers.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182 

volatile uint32_t ticks = 0;

void wait(uint64_t cycles){
    for (uint64_t i = 0; i < cycles; i++){
        asm("nop");
    }
}