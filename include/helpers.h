#pragma once
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1"
                      :
                      : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0"
                      : "=a"(result)
                      : "Nd"(port));
    return result;
}

static inline void io_wait(void) {
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

static inline void wait(uint32_t cycles) {
    if (cycles == 0) return;

    for (volatile uint32_t i = 0; i < cycles; i++) {
        __asm__ __volatile__("nop");
    }
}

static inline char* uint_to_str(uint32_t num){
    static char str[11];
    char *p = &str[10];
    *p = '\0';

    if (num == 0){
        *--p = '0';
        return p;
    }

    while (num > 0){
        *--p = '0' + (num % 10);
        num /= 10;
    }

    return p;
}

static inline int strcmp(const char* s1, const char* s2){
    while (*s1 == *s2){
        if (*s1 == '\0')
            return 0;
        s1++;
        s2++;
    }

    return (unsigned char)*s1 - (unsigned char)*s2;
}
