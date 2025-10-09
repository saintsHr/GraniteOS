#include "keyboard.h"
#include "helpers.h"
#include "shell.h"
#include "terminal.h"
#include "colors.h"
#include <stdbool.h>

const char scancode_to_ascii[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

bool keyboard_data_ready(){
    return inb(0x64) & 0x01;
}

uint8_t read_keyboard_scancode() {
    while (!keyboard_data_ready());
    uint8_t code = inb(0x60);
    io_wait();
    return code;
}

char get_char_from_keyboard() {
    uint8_t code = read_keyboard_scancode();
    if (code & 0x80) return 0;
    return scancode_to_ascii[code];
}

void keyboard_handler() {
    char c = get_char_from_keyboard();
    if (!c) return;

    if (c == '\n'){
        term_y++;
        term_x = 0;
        prompt();
        if (term_y >= 25) {
            term_y = 24;
            clear(VGA_LGRAY);
            prompt();
        }
        move_cursor(term_x, term_y);
    }
    else if (c == '\b') {
        if (term_x > 9) {
            term_x--;
        }
        putEntry(' ', VGA_BLACK);
        move_cursor(term_x, term_y);
    }
    else {
        putEntry(c, VGA_LGRAY);
        term_x++;
        if (term_x >= 80) {
            term_x = 0;
            term_y++;
        }
        if (term_y >= 25) {
            term_y = 0;
            clear(0x00);
        }
        move_cursor(term_x, term_y);
    }
}