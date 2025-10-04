#pragma once

#include <stdint.h>

void move_cursor(uint8_t x, uint8_t y);
void putEntry(char ch, char color);
void print(char* srt, char color);