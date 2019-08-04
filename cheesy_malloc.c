#include "better_lin_alloc.h"
#include "safe_printf.h"

#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#ifdef PLATFORM_DARWIN
#include <malloc/malloc.h>
#endif

static pthread_mutex_t malloc_lock = PTHREAD_MUTEX_INITIALIZER;
static uint8_t mem[1024 * 1024 * 1024];

static bool done_lin_init = false;
static better_lin_alloc_t bla;

static void ensure_init_locked(void)
{
  if (!done_lin_init) {
    better_lin_alloc_init(&bla, mem, sizeof(mem));
    done_lin_init = true;
  }
}

static void *alloc_internal_align(size_t size, size_t align)
{
  pthread_mutex_lock(&malloc_lock);

  ensure_init_locked();
  void *out = better_lin_alloc(&bla, size, align);

  pthread_mutex_unlock(&malloc_lock);
  return out;
}

static void *realloc_internal(void *orig, size_t size, size_t align)
{
  pthread_mutex_lock(&malloc_lock);

  ensure_init_locked();
  void *out = better_lin_realloc(&bla, orig, size, align);

  pthread_mutex_unlock(&malloc_lock);
  return out;
}

static void *alloc_internal(size_t size)
{
  return alloc_internal_align(size, 16);
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
  if (ptr) {
    memset(ptr, 0, total);
  }
  return ptr;
}

void *calloc(size_t nmemb, size_t size)
{
  safe_printf("calloc(%zu, %zu) -> ", nmemb, size);
  void *ptr = calloc_internal(nmemb, size);
  safe_printf("%p\n", ptr);
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

void *realloc(void *ptr, size_t size)
{
  void *out = realloc_internal(ptr, size, 16);
  safe_printf("realloc(%p, %zu) -> %p\n", ptr, size, out);
  return out;
}

void *reallocf(void *ptr, size_t size)
{
  void *out = realloc_internal(ptr, size, 16);
  safe_printf("reallocf(%p, %zu) -> %p\n", ptr, size, out);
  return out;
}

#ifdef PLATFORM_DARWIN

void *malloc_zone_malloc(malloc_zone_t *zone, size_t size)
{
  void *ptr = alloc_internal(size);
  safe_printf("malloc_zone_malloc(%p, %zu) -> %p\n",
      zone, size, ptr);
  return ptr;
}

void *malloc_zone_calloc(malloc_zone_t *zone, size_t nmemb, size_t size)
{
  void *ptr = calloc_internal(nmemb, size);
  safe_printf("malloc_zone_calloc(%p, %zu, %zu) -> %p\n",
      zone, nmemb, size, ptr);
  return ptr;
}

void malloc_zone_free(malloc_zone_t *zone, void *ptr)
{
  safe_printf("malloc_zone_free(%p, %p)\n", zone, ptr);
}

#endif
