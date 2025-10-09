#pragma once

#include <stdbool.h>
#include <stdint.h>

extern const char scancode_to_ascii[128];

uint8_t read_keyboard_scancode();
char get_char_from_keyboard();
void keyboard_handler();