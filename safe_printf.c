#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>

static void write_char(int c)
{
  char x = c;
  write(1, &x, 1);
}

enum {
  FLAG_ALTERNATE    = 1 <<  0,
  FLAG_ZERO_PAD     = 1 <<  1,
  FLAG_LEFT_ALIGN   = 1 <<  2,
  FLAG_SPACE_SIGN   = 1 <<  3,
  FLAG_PLUS_SIGN    = 1 <<  4,
  FLAG_PREC_ARG     = 1 <<  5,
  FLAG_HEX_UPPER    = 1 <<  6,
  FLAG_HAVE_WIDTH   = 1 <<  7,
  FLAG_HAVE_PREC    = 1 <<  8,
  FLAG_TYPE_INTMAX  = 1 <<  9,
  FLAG_TYPE_SIZE    = 1 << 10,
  FLAG_TYPE_PTRDIFF = 1 << 10,
};

static void print_padded(const char *buf, size_t len, unsigned width, int flags)
{
  char pad_char;
  if (flags & FLAG_HAVE_WIDTH) {
    if (flags & FLAG_ZERO_PAD)
      pad_char = '0';
    else
      pad_char = ' ';
  }

  if (flags & FLAG_ZERO_PAD && buf[0] < '0') {
    write_char(buf[0]);
    ++buf;
    --len;
    --width;
  }

  size_t i;

  if (!(flags & FLAG_LEFT_ALIGN) && flags & FLAG_HAVE_WIDTH) {
    for (i = 0; i + len < width; i++)
      write_char(pad_char);
  }

  for (i = 0; i < len; i++)
    write_char(buf[i]);

  if (flags & FLAG_LEFT_ALIGN && flags & FLAG_HAVE_WIDTH) {
    for (i = 0; i + len < width; i++)
      write_char(pad_char);
  }
}

static void print_signed_decimal(intmax_t k, int width, int flags)
{
  char sign;
  if (k < 0) {
    sign = '-';
    k = -k;
  } else {
    if (flags & FLAG_SPACE_SIGN)
      sign = ' ';
    else if (flags & FLAG_PLUS_SIGN)
      sign = '+';
    else
      sign = 0;
  }

  char buf[64];
  char *e = &buf[sizeof(buf) - 1];
  char *r = e;
  if (k) {
    while (k) {
      *r-- = (k % 10) + '0';
      k /= 10;
    }
  } else {
    *r-- = '0';
  }
  if (sign)
    *r-- = sign;

  print_padded(r + 1, e - r, width, flags);
}

static void print_unsigned_decimal(uintmax_t k, int width, int flags)
{
  char buf[64];
  char *e = &buf[sizeof(buf) - 1];
  char *r = e;
  if (k) {
    while (k) {
      *r-- = (k % 10) + '0';
      k /= 10;
    }
  } else {
    *r-- = '0';
  }

  print_padded(r + 1, e - r, width, flags);
}

static void print_hex(uintmax_t k, int width, int flags)
{
  const char *digits = "0123456789abcdef";
  char buf[64];
  char *e = &buf[sizeof(buf) - 1];
  char *r = e;

  if (k) {
    while (k) {
      *r-- = digits[k & 0xf];
      k >>= 4;
    }
  } else {
    *r-- = '0';
  }
  if (flags & FLAG_ALTERNATE) {
    *r-- = 'x';
    *r-- = '0';
  }

  print_padded(r + 1, e - r, width, flags);
}

void safe_vprintf(const char *fmt, va_list v)
{
  const char *p, *q;

  for (p = fmt; *p; p++) {
    if (*p != '%') {
      write_char(*p);
      continue;
    }

    unsigned flags = 0;
    unsigned width = 0, prec = 0;
    unsigned lh_mod = 0;

    for (q = p+1; *q; q++) {
      if (*q == '#')
        flags |= FLAG_ALTERNATE;
      else if (*q == '0')
        flags |= FLAG_ZERO_PAD;
      else if (*q == '-')
        flags |= FLAG_LEFT_ALIGN;
      else if (*q == ' ')
        flags |= FLAG_SPACE_SIGN;
      else if (*q == '+')
        flags |= FLAG_PLUS_SIGN;
      else
        break;
    }

    if (flags & FLAG_PLUS_SIGN && flags & FLAG_SPACE_SIGN)
      flags &= ~FLAG_SPACE_SIGN;
    if (flags & FLAG_ZERO_PAD && flags & FLAG_LEFT_ALIGN)
      flags &= ~FLAG_ZERO_PAD;

    if (*q > '0' && *q <= '9') {
      flags |= FLAG_HAVE_WIDTH;
      for (; *q >= '0' && *q <= '9'; q++)
        width = (width * 10) + (*q - '0');
    }

    if (*q == '.') {
      ++q;
      flags |= FLAG_HAVE_PREC;
      if (*q >= '0' && *q <= '9') {
        for (; *q >= '0' && *q <= '9'; q++)
          prec = (prec * 10) + (*q - '0');
      } else if (*q == '*') {
        flags |= FLAG_PREC_ARG;
        ++q;
      }
    }

    if (*q == 'j') {
      ++q;
      flags |= FLAG_TYPE_INTMAX;
    }
    if (*q == 'z') {
      ++q;
      flags |= FLAG_TYPE_SIZE;
    }
    if (*q == 't') {
      ++q;
      flags |= FLAG_TYPE_PTRDIFF;
    }
    for (; *q == 'h'; q++)
      --lh_mod;
    for (; *q == 'l'; q++)
      ++lh_mod;

    switch(*q) {
    case 'i':
    case 'd':
      {
        intmax_t k;
        if (flags & FLAG_TYPE_INTMAX)
          k = va_arg(v, intmax_t);
        else if (flags & FLAG_TYPE_SIZE)
          k = va_arg(v, ssize_t);
        else if (flags & FLAG_TYPE_PTRDIFF)
          k = va_arg(v, ptrdiff_t);
        else if (lh_mod >= 2)
          k = va_arg(v, long long);
        else if (lh_mod == 1)
          k = va_arg(v, long);
        else
          k = va_arg(v, int);

        print_signed_decimal(k, width, flags);
      }
      break;

    case 'X':
      flags |= FLAG_HEX_UPPER;
    case 'x':
    case 'u':
      {
        uintmax_t k;
        if (flags & FLAG_TYPE_INTMAX)
          k = va_arg(v, uintmax_t);
        else if (flags & FLAG_TYPE_SIZE)
          k = va_arg(v, size_t);
        else if (flags & FLAG_TYPE_PTRDIFF)
          k = va_arg(v, ptrdiff_t);
        else if (lh_mod >= 2)
          k = va_arg(v, unsigned long long);
        else if (lh_mod == 1)
          k = va_arg(v, unsigned long);
        else
          k = va_arg(v, unsigned int);

        if (*q == 'u')
          print_unsigned_decimal(k, width, flags);
        else
          print_hex(k, width, flags);
      }
      break;

    case 'p':
      {
        uintptr_t k;
        k = (uintptr_t)va_arg(v, void*);
        print_hex(k, width, flags | FLAG_ALTERNATE);
      }
      break;

    case 'c':
      {
        int k;
        k = va_arg(v, int);
        write_char(k);
      }
      break;

    case 's':
      {
        const char *s;

        if (flags & FLAG_HAVE_PREC && flags & FLAG_PREC_ARG)
          prec = va_arg(v, int);

        s = va_arg(v, const char*);

        size_t len;
        for (len = 0; s[len]; len++);

        if (flags & FLAG_HAVE_PREC && len > prec)
          len = prec;

        if (flags & FLAG_HAVE_WIDTH) {
          width -= len;
          for (; width > 0; width--)
            write_char(' ');
        }

        unsigned i;
        for (i = 0; i < len; i++)
          write_char(s[i]);
      }
      break;

    case '%':
      write_char('%');
      break;

    default:
      for (; p <= q; p++)
        write_char(*p);
    }
    p = q;
  }
}

void safe_printf(const char *fmt, ...)
{
  va_list v;
  va_start(v, fmt);
  safe_vprintf(fmt, v);
  va_end(v);
}
