#pragma once

#include <stdint.h>
#include <stddef.h>

// Simple linear alloactor.

typedef struct lin_alloc lin_alloc_t;

void lin_alloc_init(lin_alloc_t *la, void *buf, size_t buflen);

void *lin_alloc(lin_alloc_t *la, size_t size);

void *lin_alloc_aligned(lin_alloc_t *la, size_t size, size_t align);

// opaque:
struct lin_alloc {
  uint8_t *head;
  uint8_t *start;
  size_t size;
};
