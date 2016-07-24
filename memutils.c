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
 * in Seth Carter's Slice engine. <http://glassblade-games.com/slice-engine>
 *
 * To the maximum extent permissible by law, this file is hereby released
 * into the public domain.
 */

#include <stdlib.h>
#include "memutils.h"

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

/* Uncomment the following line if you can guarantee that you will ALWAYS
 * supply a non-null error callback to safe_malloc and safe_realloc. */

//#define CALLBACK_ALWAYS_SUPPLIED
#define DEFAULT_CALLBACK 0

#ifdef CALLBACK_ALWAYS_SUPPLIED
#define CALLBACK_NONNULL true
#else
#define CALLBACK_NONNULL err_callback
#include <stdio.h>
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
  while (normal(len >= sizeof(unsigned long)))
  {
    *(unsigned long*)(buf) = 0;
    buf += sizeof(unsigned long);
    len -= sizeof(unsigned long);
  }
  if (len >= sizeof(unsigned int))
  {
    *(unsigned int *)(buf) = 0;
    buf += sizeof(unsigned int);
    len -= sizeof(unsigned int);
  }
  if (len >= sizeof(unsigned short))
  {
    *(unsigned short *)(buf) = 0;
    buf += sizeof(unsigned short);
    len -= sizeof(unsigned short);
  }
  if (len)
    *buf = 0;
}

void copy_memory (char* buf, const char* src, size_t len)
{
  while (normal(len >= sizeof(unsigned long)))
  {
    *(unsigned long*)(buf) = *(unsigned long*)(src);
    buf += sizeof(unsigned long);
    len -= sizeof(unsigned long);
    src += sizeof(unsigned long);
  }
  if (len >= sizeof(unsigned int))
  {
    *(unsigned int *)(buf) = *(unsigned int*)(src);
    buf += sizeof(unsigned int);
    len -= sizeof(unsigned int);
    src += sizeof(unsigned int);
  }
  if (len >= sizeof(unsigned short))
  {
    *(unsigned short *)(buf) = *(unsigned short*)(src);
    buf += sizeof(unsigned short);
    len -= sizeof(unsigned short);
    src += sizeof(unsigned short);
  }
  if (len)
    *buf = *src;
}


#define MEMERR_EXIT 0xDEADBEEF

void* safe_malloc (size_t len, void (*err_callback) (char*))
{
  void* ptr = malloc(len);
  if (normal((long)(ptr)))
    return ptr;
  else
  {
    if (CALLBACK_NONNULL)
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
    if (CALLBACK_NONNULL)
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
    if (!*items) *items = safe_malloc(CHUNK_SIZE * PSIZE, NULL);
    else if (rare(!(*count % CHUNK_SIZE)))
      *items = safe_realloc(*items, PSIZE * (*count + CHUNK_SIZE), 0);
    (*items)[*count] = item;
    *count += 1;
  }
}

void remove_item_from_list (void*** items, size_t* count, void* item)
{
  register int i;
  if (normal(items && count))
  {
    for (i = 0; i < *count; i++)
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
#define haszero32(v) (((v) - 0x01010101) & ~(v) & 0x80808080)
#define hasval64(x,n) (haszero64((x) ^ (0x0101010101010101 * (n))))

size_t mu_strlen (const char* str)
{
  if (!str) return 0;
  unsigned long* ip = (unsigned long*)(str);
  if (sizeof(unsigned long) > 4)
  {
    while (normal(!haszero64(*ip))) ip++;
  }
  else
  {
    while (normal(!haszero32(*ip))) ip++;
  }
  char* p = (char*)(ip);
  while (normal(*p)) p++;
  return (size_t)(p - str);
}

char* mu_strcpy (char* dest, const char* src)
{
  if (src && dest) copy_memory(dest, src, mu_strlen(src));
  return dest;
}

char* mu_strdup (const char* src)
{
  if (!src) return NULL;
  register size_t len = mu_strlen(src) + 1;
  char* dest = (char*) safe_malloc(len, DEFAULT_CALLBACK);
  copy_memory(dest, src, len);
  return dest;
}

char* astrcat (const char* str1, const char* str2)
{
  register size_t len1 = mu_strlen(str1);
  register size_t len2 = mu_strlen(str2);
  char* dest = (char*) safe_malloc(len1 + len2, DEFAULT_CALLBACK);
  if (str1) copy_memory(dest, str1, len1);
  if (str2) copy_memory(dest + len1, str2, len2);
  return dest;
}

size_t strsplit (char*** parts, char* str, char split)
{
  if (!str) return 0;
  size_t partcount = 0;
  add_item_to_list((void***)(parts), &partcount, str);
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

char* strjoin (char** strings, size_t count, char* joiner)
{
  char *out1, *out2, *temp;
  temp = astrcat(*strings, joiner);
  out1 = astrcat(temp, *(++strings));
  free(temp);
  while (--count > 1)
  {
    temp = astrcat(out1, joiner);
    out2 = astrcat(temp, *(++strings));
    free(temp);
    free(out1);
    out1 = out2;
  }
  return out1;
}

unsigned char streq (const char* str1, const char* str2)
{
  unsigned long* ip1 = (unsigned long*)(str1);
  unsigned long* ip2 = (unsigned long*)(str2);
  if (sizeof(unsigned long) > 4)
  {
    while (normal(!(haszero64(*ip1) || haszero64(*ip2))))
    {
      if (*ip1 != *ip2) return 0;
      ip1++; ip2++;
    }
  }
  else
  {
    while (normal(!(haszero32(*ip1) || haszero32(*ip2))))
    {
      if (*ip1 != *ip2) return 0;
      ip1++; ip2++;
    }
  }
  char* p1 = (char*)(ip1);
  char* p2 = (char*)(ip2);
  while (normal(*p1 && *p2))
  {
    if (*p1 != *p2) return 0;
    p1++; p2++;
  }
  return !(*p1 || *p2);
}
