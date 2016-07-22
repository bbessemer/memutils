/*
 * memutils - a tiny library of memory and string utilities - IMPLEMENTATION
 *
 * The recommended method of using memutils is simply to drop this file and its
 * header into your project. The header automatically redefines memcpy, strlen,
 * strcpy, and strdup as their memutils counterparts. It does not redefine
 * malloc or realloc. You may do this yourself if you wish.
 *
 * Written 2015-16 by Brent Bessemer.
 * Several of the functions in this file are inspired by similar counterparts
 * in Seth Carter's Slice engine.
 *
 * To the maximum extent permissible by law, this file is hereby released
 * into the public domain.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "memutils.h"

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#ifdef UNSAFE_SKIP_CHECKS
#define contingency(condition) false
#else
#define contingency(condition) __builtin_expect(condtion, false)
#endif
#define normal(condition) __builtin_expect(condition, true)
#define rare(condition) __builtin_expect(condition, false)

void zero_memory (char* buf, size_t len)
{
  while (normal(len >= 8))
  {
    *(uint64_t *)(buf) = 0;
    buf += 8;
    len -= 8;
  }
  if (len >= 4)
  {
    *(uint32_t *)(buf) = 0;
    buf += 4;
    len -= 4;
  }
  if (len >= 2)
  {
    *(short *)(buf) = 0;
    buf += 2;
    len -= 2;
  }
  if (len)
    *buf = 0;
}

void copy_memory (char* buf, const char* src, size_t len)
{
  while (len >= 8)
  {
    *(uint64_t*)(buf) = *(uint64_t*)(src);
    buf += 8;
    src += 8;
    len -= 8;
  }
  if (len >= 4)
  {
    *(uint32_t*)(buf) = *(uint32_t*)(src);
    buf += 4;
    src += 4;
    len -= 4;
  }
  if (len >= 2)
  {
    *(short*)(buf) = *(short*)(src);
    buf += 2;
    src += 2;
    len -= 2;
  }
  if (len)
    *buf = 0;
}


#define MEMERR_EXIT 0xDEADBEEF

void* safe_malloc (size_t len, void (*err_callback) (char*))
{
  void* ptr = malloc(len);
  if (normal((long)(ptr)))
    return ptr;
  else
  {
    if (err_callback)
      err_callback("FATAL: Memory allocation error!\n");
    else
      puts("FATAL: Memory allocation error!\n");
    exit(MEMERR_EXIT);
  }
}

void* safe_realloc (void* old, size_t len, void (*err_callback) (char*))
{
  void* ptr = realloc(old, len);
  if (ptr)
    return ptr;
  else
  {
    if (err_callback)
      err_callback("FATAL: Memory allocation error!\n");
    else
      puts("FATAL: Memory allocation error!\n");
    exit(MEMERR_EXIT);
  }
}

#define CHUNK_SIZE 64
#define PSIZE (size_t)(sizeof(void*))

void add_item_to_list (void*** items, size_t* count, void* item)
{
  if (normal(items && count))
  {
    if (rare(!(*count % CHUNK_SIZE)))
      *items = safe_realloc(*items, PSIZE * (*count + CHUNK_SIZE), 0);
    *items[*count] = item;
    *count += 1;
  }
}

void remove_item_from_list (void*** items, size_t* count, void* item)
{
  if (normal(items && count))
  {
    for (int i = 0; i < *count; i++)
    {
      if (*items[i] == item)
      {
        *items[i] = *items[*count - 1];
        *count -= 1;
        break;
      }
    }
    if (rare(!(*count)))
      free(*items);
    else if (rare(!(*count % CHUNK_SIZE)))
      *items = safe_realloc(*items, PSIZE * (*count + CHUNK_SIZE), 0);
  }
}

/*
 * Taken from https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord
 * In the public domain.
 */

#define haszero64(v) (((v) - 0x0101010101010101) & ~(v) & 0x8080808080808080)
#define hasval64(x,n) (haszero((x) ^ (0x0101010101010101 * (n))))

size_t my_strlen (const char* str)
{
  uint64_t* ip = (uint64_t*)(str);
  while (normal(!haszero64(*ip))) ip++;
  char* p = (char*)(ip);
  while (normal(*p)) p++;
  return (size_t)(p - str);
}

char* my_strcpy (char* dest, const char* src)
{
  copy_memory(dest, src, my_strlen(src));
  return dest;
}

char* my_strdup (const char* src)
{
  register size_t len = my_strlen(src) + 1;
  char* dest = (char*) safe_malloc(len, 0);
  copy_memory(dest, src, len);
  return dest;
}

char* astrcat (const char* str1, const char* str2)
{
  register size_t len1 = my_strlen(str1);
  register size_t len2 = my_strlen(str2);
  char* dest = (char*) safe_malloc(len1 + len2, 0);
  copy_memory(dest, str1, len1);
  copy_memory(dest + len1, str2, len2);
  return dest;
}

size_t strsplit (char*** parts, char* str, char split)
{
  size_t partcount = 0;
  for (; normal(*str); str++)
  {
    if (rare(*str == split))
    {
      *str = 0;
      add_item_to_list((void***)(parts), &partcount, ++str);
    }
  }
  return partcount;
}
