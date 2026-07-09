#pragma once

#include <stdint.h>

#define PAGE_PRESENT 0x1
#define PAGE_WRITE   0x2
#define PAGE_USER    0x4

typedef uint32_t page_entry_t;

typedef struct {
    page_entry_t entries[1024];
} page_directory_t;

typedef struct {
    page_entry_t entries[1024];
} page_table_t;