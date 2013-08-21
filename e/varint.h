// This code is derived from code distributed as part of Google LevelDB.
// The original is available in leveldb as util/coding.h.
// This file was retrieved Jul 15, 2013 by Robert Escriva.

// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.
//
// Endian-neutral encoding:
// * Fixed-length numbers are encoded with least-significant byte first
// * In addition we support variable length "varint" encoding
// * Strings are encoded prefixed by their length in varint format

#ifndef e_varint_h_
#define e_varint_h_

#include <stdint.h>

namespace e
{

// These either store a value in *v and return a pointer just past the parsed
// value, or return NULL on error.  These routines only look at bytes in the
// range [p..limit-1]
const char*
varint32_decode(const char* p,const char* limit, uint32_t* v);
const char*
varint64_decode(const char* p,const char* limit, uint64_t* v);

// Write directly into a character buffer and return a pointer just past the
// last byte written.
// REQUIRES: dst has enough space for the value being written
char*
varint32_encode(char* dst, uint32_t value);
char*
varint64_encode(char* dst, uint64_t value);

// Returns the length of the varint32 or varint64 encoding of "v"
inline int
varint_length(uint64_t v)
{
    int len = 1;

    while (v >= 128)
    {
        v >>= 7;
        len++;
    }

    return len;
}

}  // namespace e

#endif // e_varint_h_
