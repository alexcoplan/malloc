#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct better_lin_alloc better_lin_alloc_t;

void better_lin_alloc_init(better_lin_alloc_t *bla, void *buf, size_t len);

void *better_lin_alloc(better_lin_alloc_t *bla, size_t size, size_t align);

void *better_lin_realloc(better_lin_alloc_t *bla,
    void *orig, size_t size, size_t align);

struct better_lin_alloc {
  uint8_t *head;
  uint8_t *start;
  size_t size;
};
