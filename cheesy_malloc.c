#include "better_lin_alloc.h"
#include "safe_printf.h"

#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

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
    safe_printf("cheesy mem: [%p, %p)\n", mem, mem + sizeof(mem));
    better_lin_alloc_init(&bla, mem, sizeof(mem));
    done_lin_init = true;
  }
}

static void *alloc_internal_align(size_t size, size_t align)
{
  pthread_mutex_lock(&malloc_lock);

  ensure_init_locked();
  uint8_t *out = better_lin_alloc(&bla, size, align);
  assert(out >= mem);
  assert(out < mem + sizeof(mem));

  pthread_mutex_unlock(&malloc_lock);
  return out;
}

static void *realloc_internal_align(void *orig, size_t size, size_t align)
{
  pthread_mutex_lock(&malloc_lock);

  ensure_init_locked();
  void *out = better_lin_realloc(&bla, orig, size, align);

  pthread_mutex_unlock(&malloc_lock);
  return out;
}

static void *realloc_internal(void *orig, size_t size)
{
  return realloc_internal_align(orig, size, 16);
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

static void *valloc_internal(size_t size)
{
  void *out = alloc_internal_align(size, 4 * 1024);
  return out;
}

void *valloc(size_t size)
{
  void *out = valloc_internal(size);
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
  void *out = realloc_internal(ptr, size);
  safe_printf("realloc(%p, %zu) -> %p\n", ptr, size, out);
  return out;
}

void *reallocf(void *ptr, size_t size)
{
  void *out = realloc_internal(ptr, size);
  safe_printf("reallocf(%p, %zu) -> %p\n", ptr, size, out);
  return out;
}

#ifdef PLATFORM_DARWIN

static size_t alloc_size_internal(const void *orig)
{
  pthread_mutex_lock(&malloc_lock);

  size_t out = better_lin_size(&bla, orig);

  pthread_mutex_unlock(&malloc_lock);
  return out;
}

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

void *malloc_zone_valloc(malloc_zone_t *zone, size_t size)
{
  void *ptr = valloc_internal(size);
  safe_printf("malloc_zone_valloc(%p, %zu) -> %p\n",
      zone, size, ptr);
  return ptr;
}

void *malloc_zone_realloc(malloc_zone_t *zone, void *orig, size_t size)
{
  void *ptr = realloc_internal(orig, size);
  safe_printf("malloc_zone_realloc(%p, %p, %zu) -> %p\n",
      zone, orig, size, ptr);
  return ptr;
}

void *malloc_zone_memalign(malloc_zone_t *zone, size_t align, size_t size)
{
  void *ptr = alloc_internal_align(size, align);
  safe_printf("malloc_zone_memalign(%p, %zu, %zu) -> %p\n",
      zone, align, size, ptr);
  return ptr;
}

malloc_zone_t *malloc_zone_from_ptr(const void *ptr)
{
  malloc_zone_t *ret = malloc_default_zone();
  safe_printf("malloc_zone_from_ptr(%p) -> %p\n", ptr, ret);
  return ret;
}

void malloc_zone_free(malloc_zone_t *zone, void *ptr)
{
  safe_printf("malloc_zone_free(%p, %p)\n", zone, ptr);
}

size_t malloc_size(const void *ptr)
{
  size_t ret = alloc_size_internal(ptr);
  safe_printf("malloc_size(%p) -> %zu\n", ptr, ret);
  return ret;
}

void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
  uint32_t *stupid = NULL;
  *stupid = 0;
  return ptr;
}

void *reallocarrayf(void *ptr, size_t nmemb, size_t size)
{
  uint32_t *stupid = NULL;
  *stupid = 0;
  return ptr;
}

void vfree(const void *addr)
{
  uint32_t *stupid = NULL;
  *stupid = 0;
}

#endif
