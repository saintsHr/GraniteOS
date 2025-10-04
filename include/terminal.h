#pragma once

#include <stdint.h>

extern uint8_t term_x;
extern uint8_t term_y;

void move_cursor(uint8_t x, uint8_t y);
void putEntry(char ch, char color);
void print(char* srt, char color);
void clear(char color);