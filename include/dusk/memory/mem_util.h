#pragma once

#include <stdint.h>

void set_memory(void* destiny, uint8_t value, uint32_t size);

uint32_t align_up(uint32_t addr, uint32_t align);
uint32_t align_down(uint32_t addr, uint32_t align);