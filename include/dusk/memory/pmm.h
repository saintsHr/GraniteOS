#pragma once

#include "dusk/multiboot.h"
#include <stdint.h>

#define PMM_PAGE_SIZE (4 * 1024) // 4KiB

void pmm_init(const struct multiboot_info* mbi);

void* pmm_alloc_page(void);
void pmm_free_page(void* addr);