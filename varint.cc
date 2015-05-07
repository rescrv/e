// This code is derived from code distributed as part of Google LevelDB.
// The original is available in leveldb as util/coding.cc.
// This file was retrieved Jul 15, 2013 by Robert Escriva.

// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

// C
#include <stdlib.h>

// e
#include "e/varint.h"

namespace
{

const char*
decode_32_fallback(const char* p,
                   const char* limit,
                   uint32_t* value)

{
    uint32_t result = 0;

    for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7)
    {
        uint32_t byte = *(reinterpret_cast<const unsigned char*>(p));
        p++;

        if (byte & 128)
        {
            // More bytes are present
            result |= ((byte & 127) << shift);
        }
        else
        {
            result |= (byte << shift);
            *value = result;
            return reinterpret_cast<const char*>(p);
        }
    }

    return NULL;
}

} // namespace

const char*
e :: varint32_decode(const char* p,
                     const char* limit,
                     uint32_t* value)

{
    if (p < limit)
    {
        uint32_t result = *(reinterpret_cast<const unsigned char*>(p));

        if ((result & 128) == 0)
        {
            *value = result;
            return p + 1;
        }
    }

    return decode_32_fallback(p, limit, value);
}

const char*
e :: varint64_decode(const char* p, const char* limit, uint64_t* value)
{
    uint64_t result = 0;

    for (uint32_t shift = 0; shift <= 63 && p < limit; shift += 7)
    {
        uint64_t byte = *(reinterpret_cast<const unsigned char*>(p));
        p++;

        if (byte & 128)
        {
            // More bytes are present
            result |= ((byte & 127) << shift);
        }
        else
        {
            result |= (byte << shift);
            *value = result;
            return reinterpret_cast<const char*>(p);
        }
    }

    return NULL;
}

char*
e :: varint32_encode(char* dst, uint32_t v)
{
    // Operate on characters as unsigneds
    unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);
    static const int B = 128;

    if (v < (1<<7))
    {
        *(ptr++) = v;
    }
    else if (v < (1<<14))
    {
        *(ptr++) = v | B;
        *(ptr++) = v>>7;
    }
    else if (v < (1<<21))
    {
        *(ptr++) = v | B;
        *(ptr++) = (v>>7) | B;
        *(ptr++) = v>>14;
    }
    else if (v < (1<<28))
    {
        *(ptr++) = v | B;
        *(ptr++) = (v>>7) | B;
        *(ptr++) = (v>>14) | B;
        *(ptr++) = v>>21;
    }
    else
    {
        *(ptr++) = v | B;
        *(ptr++) = (v>>7) | B;
        *(ptr++) = (v>>14) | B;
        *(ptr++) = (v>>21) | B;
        *(ptr++) = v>>28;
    }

    return reinterpret_cast<char*>(ptr);
}

char*
e :: varint64_encode(char* dst, uint64_t v)
{
    static const unsigned B = 128;
    unsigned char* ptr = reinterpret_cast<unsigned char*>(dst);

    while (v >= B)
    {
        *(ptr++) = (v & (B-1)) | B;
        v >>= 7;
    }

    *(ptr++) = static_cast<unsigned char>(v);
    return reinterpret_cast<char*>(ptr);
}
