#include "basic_lin_alloc.h"
#include "safe_printf.h"

#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// macOS
#include <malloc/malloc.h>

static pthread_mutex_t malloc_lock = PTHREAD_MUTEX_INITIALIZER;
static uint8_t mem[16 * 1024 * 1024];

static bool done_lin_init = false;
static basic_lin_alloc_t la;

static void *alloc_internal_align(size_t size, size_t align)
{
  pthread_mutex_lock(&malloc_lock);

  if (!done_lin_init) {
    basic_lin_alloc_init(&la, mem, sizeof(mem));
    done_lin_init = true;
  }

  void *out = basic_lin_alloc_aligned(&la, size, align);

  pthread_mutex_unlock(&malloc_lock);
  return out;
}

static void *alloc_internal(size_t size)
{
  return alloc_internal_align(size, 8);
}

void *malloc(size_t size)
{
  void *out = alloc_internal(size);
  safe_printf("malloc(%zu) -> %p\n", size, out);
  return out;
}

void *valloc(size_t size)
{
  void *out = alloc_internal_align(size, 4 * 1024);
  safe_printf("valloc(%zu) -> %p\n", size, out);
  return out;
}

void free(void *ptr)
{
  safe_printf("free(%p)\n", ptr);
  // Nothing to do!
}

void *calloc_internal(size_t nmemb, size_t size)
{
  const size_t total = nmemb * size;
  void *ptr = alloc_internal(total);
  memset(ptr, 0, total);
  return ptr;
}

void *calloc(size_t nmemb, size_t size)
{
  void *ptr = calloc_internal(nmemb, size);
  safe_printf("calloc(%zu, %zu) -> %p\n", nmemb, size, ptr);
  return ptr;
}

int posix_memalign(void **out, size_t align, size_t size)
{
  void *ptr = alloc_internal(size);
  safe_printf("posix_memalign(align=%zu, size=%zu) -> %p\n",
      align, size, ptr);

  if (!ptr)
    return 12;

  *out = ptr;
  return 0;
}

// TODO: implement proper realloc with a new fancier almost_basic_lin_alloc.
void *realloc(void *ptr, size_t size)
{
  void *out = NULL;
  if (!ptr) {
    out = alloc_internal(size);
  }

  safe_printf("realloc(%p, %zu) -> %p\n", ptr, size, out);
  return out;
}

void *reallocf(void *ptr, size_t size)
{
  safe_printf("reallocf(%p, %zu)\n", ptr, size);
  return NULL;
}

void *malloc_zone_calloc(malloc_zone_t *zone, size_t nmemb, size_t size)
{
  void *ptr = calloc_internal(nmemb, size);
  safe_printf("malloc_zone_calloc(%p, %zu, %zu) -> %p\n",
      zone, nmemb, size, ptr);
  return ptr;
}

void *malloc_zone_malloc(malloc_zone_t *zone, size_t size)
{
  void *ptr = alloc_internal(size);
  safe_printf("malloc_zone_malloc(%p, %zu) -> %p\n",
      zone, size, ptr);
  return ptr;
}
