#include "helpers.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43
#define PIT_FREQUENCY 1193182 

volatile uint32_t ticks = 0;

void wait(uint32_t cycles){
    for (uint32_t i = 0; i < cycles; i++){
        asm("nop");
    }
}

#include <stdint.h>

#include <stdint.h>

char* uint_to_str(uint32_t num){
    static char str[12];
    char buffer[11];
    int i = 0;

    if (num == 0){
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    while (num > 0){
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    int j = 0;
    while (i > 0){
        str[j++] = buffer[--i];
    }
    str[j] = '\0';

    return str;
}

int strcmp(const char* s1, const char* s2){
    while (*s1 && (*s1 == *s2)){
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}