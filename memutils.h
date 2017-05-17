/*
 * memutils - a tiny library of memory and string utilities - HEADER
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

#ifndef MEMUTILS_H
#define MEMUTILS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Sets len bytes in buf to zero */
void zero_memory (char* buf, size_t len);

/* Copies len bytes from src to dest. */
void copy_memory (char* dest, const char* src, size_t len);

/* Wraps malloc and realloc to ensure that they do not return null, and
 * calls a callback if allocation fails. */
void* safe_malloc (size_t len, void (*err_callback) (char*));
void* safe_realloc (void* old, size_t len, void (*err_callback) (char*));

/*
 * Adds and removes items from unordered lists of pointers, reallocating
 * if necessary.
 */
void add_item_to_list (void*** items, size_t* count, void* item);
void remove_item_from_list (void*** items, size_t* count, void* item);

/* Returns the length of a string, not including the null byte.
 * Functions identically to POSIX strlen, but potentially faster. */
size_t mu_strlen (const char* str);

/* Allocates a buffer and copies src into it. */
char* mu_strdup (const char* src);

/* Concatenates str1 and str2 into a newly allocated buffer. */
char* astrcat (const char* str1, const char* str2);

/* Splits a string into parts every time the character split occurs.
 * Places a pointer to the start of the list in the value pointed to
 * by parts, and returns the number of parts. */
size_t strsplit (char*** parts, char* str, char split);

char* strjoin (char** strings, size_t count, char* joiner);
unsigned char streq (const char* str1, const char* str2);

#ifdef __cplusplus
}
#endif
/*
#define memcpy copy_memory
#define strlen mu_strlen
#define strdup mu_strdup
#define strcpy mu_strcpy
*/
#endif
