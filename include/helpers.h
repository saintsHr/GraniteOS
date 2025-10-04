#pragma once

#include <stdint.h>

inline void outb(uint16_t port, uint8_t val){
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void wait(uint32_t cycles);
char* uint_to_str(uint32_t num);