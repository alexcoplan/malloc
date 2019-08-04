#include "lin_alloc.h"

#include <string.h>

struct la_meta {
  uint64_t block_size;
};

void lin_alloc_init(lin_alloc_t *la, void *buf, size_t buflen)
{
  *la = (struct lin_alloc){
    .start = buf,
    .size = buflen,
  };
}

void *lin_alloc_aligned(lin_alloc_t *la, size_t size, size_t align)
{
  struct la_meta meta;
  size_t avail;
  uint8_t *dst;

  if (!la->head) {
    dst = la->start;
  } else {
    memcpy(&meta, la->head, sizeof(meta));
    dst = la->head + meta.block_size;
  }
  avail = la->start + la->size - dst;

  size_t align_fix = 0;
  uint8_t *block_dst = dst + sizeof(struct la_meta);

  const size_t rem = align ? (((uintptr_t)block_dst) % align) : 0;
  if (rem) {
    align_fix = (align - rem);
    block_dst += align_fix;
  }

  const size_t required = size + sizeof(struct la_meta) + align_fix;
  if (avail < required)
    return NULL;

  la->head = dst;
  meta.block_size = required;
  memcpy(dst, &meta, sizeof(meta));
  return block_dst;
}

void *lin_alloc(lin_alloc_t *la, size_t size)
{
  return lin_alloc_aligned(la, size, 8);
}
