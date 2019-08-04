#pragma once

#include <stdint.h>
#include <stddef.h>

// Simple linear alloactor.

typedef struct basic_lin_alloc basic_lin_alloc_t;

void basic_lin_alloc_init(basic_lin_alloc_t *la, void *buf, size_t buflen);

void *basic_lin_alloc(basic_lin_alloc_t *la, size_t size);

void *basic_lin_alloc_aligned(basic_lin_alloc_t *la, size_t size, size_t align);

// opaque:
struct basic_lin_alloc {
  uint8_t *head;
  uint8_t *start;
  size_t size;
};
