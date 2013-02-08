#include <memory>
#include <stdio.h>

namespace e
{
/* XXX: Partially adapted from code which contianed this
 *      copyright:
 * Byte-wise substring search, using the Two-Way algorithm.
 * Copyright (C) 2008, 2010 Eric Blake
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */


 
void *memmem(const void *haystack, size_t haystack_len,
                const void *needle, size_t needle_len)
{
        const char *begin = (const char*)haystack;
        const char *last_possible = begin + haystack_len - needle_len;
        const char *tail = (const char*)needle;
        char point;

        /*
         * The first occurrence of the empty string is deemed to occur at
         * the beginning of the string.
         */
        if (needle_len == 0)
                return (void *)begin;

        /*
         * Sanity check, otherwise the loop might search through the whole
         * memory.
         */
        if (haystack_len < needle_len)
                return NULL;

        point = *tail++;
        for (; begin <= last_possible; begin++) {
                if (*begin == point && !memcmp(begin + 1, tail, needle_len - 1))
                        return (void *)begin;
        }

        return NULL;
}
}
