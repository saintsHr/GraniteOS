#include "dusk/memory/util.h"
#include <stdint.h>
#include <stddef.h>

void set_memory(void* destiny, uint8_t value, uint32_t size) {
	if (destiny == NULL) return;

	for (uint32_t i = 0; i < size; i++) {
		((uint8_t*)destiny)[i] = value;
	}
}