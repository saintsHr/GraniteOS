#include <stdint.h>
#include "helpers.h"
#include "terminal.h"

uint8_t term_x = 0;
uint8_t term_y = 0;

#define VGA_MEMORY 0xB8000
volatile uint8_t* vga = (volatile uint8_t*) VGA_MEMORY;

void move_cursor(uint8_t x, uint8_t y){
    uint16_t pos = y * 80 + x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));

    term_x = x;
    term_y = y;
}

inline void putEntry(char ch, char color){
    uint16_t offset = (term_y * 80 + term_x) * 2;
    vga[offset] = ch;
    vga[offset + 1] = color;
}

void print(char* str, char color){
    for (int i = 0;; i++){
        if (str[i] == '\0') break;
        if (str[i] == '\n'){
            term_y++;
            term_x = 0;
            continue;
        }

        if (term_x >= 80){
            term_y++;
            term_x = 0;
        }
        if (term_y >= 25){
            term_y = 0;
            term_x = 0;
            clear(0x00);
        }

        putEntry(str[i], color);
        term_x++;
        move_cursor(term_x, term_y);
    }
}

void clear(char color){
    for (uint8_t i = 0; i < 26; i++){
        for (uint8_t j = 0; j < 80; j++){
            putEntry(' ', color);
            term_x = j;
        }
        term_y = i;
    }
    term_x = 0;
    term_y = 0;
    move_cursor(0, 0);
}