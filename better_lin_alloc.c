#include "better_lin_alloc.h"
#include "safe_printf.h"

#include <string.h>
#include <assert.h>

void better_lin_alloc_init(better_lin_alloc_t *bla, void *buf, size_t buflen)
{
  *bla = (struct better_lin_alloc){
    .start = buf,
    .size = buflen,
  };
}

struct bla_meta {
  uint64_t block_size;
};

void *better_lin_alloc(better_lin_alloc_t *bla, size_t size, size_t align)
{
  struct bla_meta meta;
  size_t avail;
  uint8_t *dst;

  if (!bla->head) {
    dst = bla->start;
  } else {
    memcpy(&meta, bla->head, sizeof(meta));
    dst = bla->head + sizeof(meta) + meta.block_size;
  }

  avail = bla->start + bla->size - dst;

  size_t align_fix = 0;
  uint8_t *block_dst = dst + sizeof(meta);

  const size_t rem = align ? (((uintptr_t)block_dst) % align) : 0;
  if (rem) {
    align_fix = (align - rem);
    dst += align_fix;
    block_dst += align_fix;
  }

  assert(bla->start <= dst && dst <= bla->start + bla->size);

  const size_t required = size + sizeof(meta) + align_fix;
  if (avail < required)
    return NULL;

  bla->head = dst;
  meta.block_size = size;
  memcpy(dst, &meta, sizeof(meta));
  return block_dst;
}

size_t better_lin_size(better_lin_alloc_t *bla, const void *ptr)
{
  struct bla_meta meta;
  const uint8_t *p8 = ptr;

  assert(bla->head);
  assert(bla->start < p8 && p8 < bla->start + bla->size);
  memcpy(&meta, p8 - sizeof(meta), sizeof(meta));
  return meta.block_size;
}


void *better_lin_realloc(better_lin_alloc_t *bla,
    void *orig, size_t size, size_t align)
{
  if (!orig) {
    return better_lin_alloc(bla, size, align);
  }

  const size_t block_size = better_lin_size(bla, orig);

  if (size <= block_size) {
    return orig;
  }

  void *fresh = better_lin_alloc(bla, size, align);
  memcpy(fresh, orig, block_size);
  return fresh;
}
