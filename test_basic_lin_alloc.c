#include "basic_lin_alloc.h"
#include "test.h"

#include <string.h>

static void test_empty(void)
{
  uint8_t ffs[16];
  uint8_t buf[16];

  memset(buf, 0xff, sizeof(buf));
  memset(ffs, 0xff, sizeof(ffs));

  basic_lin_alloc_t la;
  basic_lin_alloc_init(&la, buf, 0);

  ASSERT_PTR_EQ(basic_lin_alloc(&la, 1), NULL);
  ASSERT_EQ(memcmp(ffs, buf, sizeof(buf)), 0);
}

static void test_one(void)
{
  uint8_t buf[16];

  basic_lin_alloc_t la;
  basic_lin_alloc_init(&la, buf, sizeof(buf));

  uint8_t *one = basic_lin_alloc(&la, 1);
  ASSERT_PTR_NEQ(one, NULL);

  *one = 0xab;

  // Now don't have space for metadata.
  ASSERT_PTR_EQ(basic_lin_alloc(&la, 1), NULL);

  ASSERT_EQ(*one, 0xab);
}

static void test_two(void)
{
  uint8_t buf[32];

  basic_lin_alloc_t la;
  basic_lin_alloc_init(&la, buf, sizeof(buf));

  // We can allocate three blocks of 2 if we don't care about alignment.
  uint8_t *ptrs[3];
  for (unsigned i = 0; i < ARRAY_LENGTH(ptrs); i++) {
    ptrs[i] = basic_lin_alloc_aligned(&la, 2, 0);
    ASSERT_PTR_NEQ(ptrs[i], NULL);

    ptrs[i][0] = 2*i;
    ptrs[i][1] = 2*i+1;
  }

  ASSERT_PTR_EQ(basic_lin_alloc_aligned(&la, 2, 0), NULL);

  for (unsigned i = 0; i < ARRAY_LENGTH(ptrs); i++) {
    ASSERT_EQ(ptrs[i][0], 2*i);
    ASSERT_EQ(ptrs[i][1], 2*i+1);
  }

  // Now let's try again, but this time caring about alignment.
  basic_lin_alloc_init(&la, buf, sizeof(buf));

  for (unsigned i = 0; i < 2; i++) {
    ptrs[i] = basic_lin_alloc_aligned(&la, 2, 8);
    ASSERT_PTR_NEQ(ptrs[i], NULL);

    ptrs[i][0] = 2*i;
    ptrs[i][1] = 2*i+1;
  }
  ASSERT_PTR_EQ(basic_lin_alloc_aligned(&la, 2, 8), NULL);
  ASSERT_PTR_EQ(basic_lin_alloc_aligned(&la, 1, 0), NULL);

  for (unsigned i = 0; i < 2; i++) {
    ASSERT_EQ(ptrs[i][0], 2*i);
    ASSERT_EQ(ptrs[i][1], 2*i+1);
  }
}

RUN_TESTS(
    test_empty,
    test_one,
    test_two
)
