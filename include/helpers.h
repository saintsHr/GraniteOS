#pragma once

#include <stdint.h>

inline void outb(uint16_t port, uint8_t val){
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port){
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void io_wait(void){
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

void wait(uint32_t cycles);
char* uint_to_str(uint32_t num);
int strcmp(const char* s1, const char* s2);