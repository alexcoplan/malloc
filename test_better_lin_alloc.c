#include "better_lin_alloc.h"
#include "test.h"

#include <string.h>

static void test_empty(void)
{
  uint8_t ffs[16];
  uint8_t buf[16];

  memset(buf, 0xff, sizeof(buf));
  memset(ffs, 0xff, sizeof(ffs));

  better_lin_alloc_t la;
  better_lin_alloc_init(&la, buf, 0);

  ASSERT_PTR_EQ(better_lin_alloc(&la, 1, 0), NULL);
  ASSERT_EQ(memcmp(ffs, buf, sizeof(buf)), 0);
}

static void test_one(void)
{
  uint8_t buf[16];

  better_lin_alloc_t bla;
  better_lin_alloc_init(&bla, buf, sizeof(buf));

  uint8_t *ptr = better_lin_alloc(&bla, 1, 0);
  ASSERT_PTR_NEQ(ptr, NULL);

  *ptr = 0xaa;

  ASSERT_PTR_EQ(better_lin_alloc(&bla, 1, 0), NULL);
}

static void test_two(void)
{
  uint8_t buf[32];

  better_lin_alloc_t la;
  better_lin_alloc_init(&la, buf, sizeof(buf));

  // We can allocate three blocks of 2 if we don't care about alignment.
  uint8_t *ptrs[3];
  for (unsigned i = 0; i < ARRAY_LENGTH(ptrs); i++) {
    ptrs[i] = better_lin_alloc(&la, 2, 0);
    ASSERT_PTR_NEQ(ptrs[i], NULL);

    ptrs[i][0] = 2*i;
    ptrs[i][1] = 2*i+1;
  }

  ASSERT_PTR_EQ(better_lin_alloc(&la, 2, 0), NULL);

  for (unsigned i = 0; i < ARRAY_LENGTH(ptrs); i++) {
    ASSERT_EQ(ptrs[i][0], 2*i);
    ASSERT_EQ(ptrs[i][1], 2*i+1);
  }

  // Now let's try again, but this time caring about alignment.
  better_lin_alloc_init(&la, buf, sizeof(buf));

  for (unsigned i = 0; i < 2; i++) {
    ptrs[i] = better_lin_alloc(&la, 2, 8);
    ASSERT_PTR_NEQ(ptrs[i], NULL);

    ptrs[i][0] = 2*i;
    ptrs[i][1] = 2*i+1;
  }
  ASSERT_PTR_EQ(better_lin_alloc(&la, 2, 8), NULL);
  ASSERT_PTR_EQ(better_lin_alloc(&la, 1, 0), NULL);

  for (unsigned i = 0; i < 2; i++) {
    ASSERT_EQ(ptrs[i][0], 2*i);
    ASSERT_EQ(ptrs[i][1], 2*i+1);
  }
}

RUN_TESTS(
    test_empty,
    test_one,
    test_two,
)
