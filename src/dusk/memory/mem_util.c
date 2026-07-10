#include "dusk/memory/mem_util.h"
#include <stdint.h>
#include <stddef.h>

void set_memory(void* destiny, uint8_t value, uint32_t size) {
	if (destiny == NULL) return;

	for (uint32_t i = 0; i < size; i++) {
		((uint8_t*)destiny)[i] = value;
	}
}

inline uint32_t align_up(uint32_t addr, uint32_t align) {
	return (addr + (align - 1)) & ~(align - 1);
}

inline uint32_t align_down(uint32_t addr, uint32_t align) {
	return addr & ~(align - 1);
}