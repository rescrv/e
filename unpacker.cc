// Copyright (c) 2011, Robert Escriva
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of this project nor the names of its contributors may
//       be used to endorse or promote products derived from this software
//       without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#define __STDC_LIMIT_MACROS

// e
#include "e/endian.h"
#include "e/unpacker.h"

e :: unpacker :: unpacker()
    : m_ptr(NULL)
    , m_ptr_sz(0)
    , m_error(true)
{
}

e :: unpacker :: unpacker(const char* ptr, size_t ptr_sz)
    : m_ptr(reinterpret_cast<const uint8_t*>(ptr))
    , m_ptr_sz(ptr_sz)
    , m_error(false)
{
}

e :: unpacker :: unpacker(const uint8_t* ptr, size_t ptr_sz)
    : m_ptr(ptr)
    , m_ptr_sz(ptr_sz)
    , m_error(false)
{
}

e :: unpacker :: unpacker(const unpacker& other)
    : m_ptr(other.m_ptr)
    , m_ptr_sz(other.m_ptr_sz)
    , m_error(other.m_error)
{
}

e::unpacker
e :: unpacker :: advance(size_t by) const
{
    if (!m_error && by <= m_ptr_sz)
    {
        return unpacker(m_ptr + by, m_ptr_sz - by);
    }
    else
    {
        return as_error();
    }
}

e::unpacker
e :: unpacker :: as_error() const
{
    unpacker ret(*this);
    ret.m_error = true;
    return ret;
}

e::slice
e :: unpacker :: as_slice() const
{
    return e::slice(m_ptr, m_ptr_sz);
}

#define UNPACKER(TYPE, UNPACKF) \
    e::unpacker \
    e :: unpacker :: operator >> (TYPE& rhs) \
    { \
        if (!m_error && sizeof(TYPE) <= m_ptr_sz) \
        { \
            UNPACKF(m_ptr, &rhs); \
            return unpacker(m_ptr + sizeof(TYPE), m_ptr_sz - sizeof(TYPE)); \
        } \
        else \
        { \
            return as_error(); \
        } \
    } \

UNPACKER(int8_t, e::unpack8be)
UNPACKER(int16_t, e::unpack16be)
UNPACKER(int32_t, e::unpack32be)
UNPACKER(int64_t, e::unpack64be)
UNPACKER(uint8_t, e::unpack8be)
UNPACKER(uint16_t, e::unpack16be)
UNPACKER(uint32_t, e::unpack32be)
UNPACKER(uint64_t, e::unpack64be)
UNPACKER(double, e::unpackdoublebe)

e::unpacker
e :: unpacker :: operator >> (slice& rhs)
{
    uint32_t sz;
    e::unpacker tmp = *this >> sz;

    if (!tmp.m_error && sz <= tmp.m_ptr_sz)
    {
        rhs = slice(tmp.m_ptr, sz);
        return unpacker(tmp.m_ptr + sz, tmp.m_ptr_sz - sz);
    }
    else
    {
        return tmp.as_error();
    }
}
