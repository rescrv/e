// Copyright (c) 2012, Robert Escriva
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

#ifdef _MSC_VER
#include "memmem.h"
#endif

// C
#include <cstddef>
#include <cstring>

// STL
#include <algorithm>
#include <iomanip>
#include <memory>
#include <sstream>

#include <iostream>

// e
#include "e/assert.h"
#include "e/buffer.h"
#include "e/endian.h"

bool
e :: buffer :: cmp(const char* buf, uint32_t sz) const throw ()
{
    if (m_size == sz)
    {
        return memcmp(m_data, buf, sz) == 0;
    }

    return false;
}

e::buffer*
e :: buffer :: copy() const
{
    std::auto_ptr<e::buffer> ret(create(m_cap));
    ret->m_cap = m_cap;
    ret->m_size = m_size;
    memmove(ret->m_data, m_data, m_cap);
    return ret.release();
}

uint32_t
e :: buffer :: index(const uint8_t* mem, size_t sz) const throw ()
{
    const uint8_t* loc = static_cast<const uint8_t*>(memmem(m_data, m_size, mem, sz));

    if (loc == NULL)
    {
        return m_cap;
    }
    else if (loc <= m_data)
    {
        return 0;
    }
    else
    {
        return loc - m_data;
    }
}

uint32_t
e :: buffer :: index(uint8_t byte) const throw ()
{
    const uint8_t* loc;
    loc = static_cast<const uint8_t*>(memchr(m_data, byte, m_size));
    return loc ? loc - m_data : m_cap;
}

e::buffer::packer
e :: buffer :: pack()
{
    return packer(this, 0);
}

e::buffer::packer
e :: buffer :: pack_at(uint32_t off)
{
    return packer(this, off);
}

void
e :: buffer :: resize(uint32_t sz) throw ()
{
    EASSERT(sz <= m_cap);
    m_size = sz;
}

void
e :: buffer :: shift(uint32_t off) throw ()
{
    if (off < m_size)
    {
        memmove(m_data, m_data + off, m_size - off);
        m_size -= off;
    }
    else
    {
        m_size = 0;
    }
}

e::unpacker
e :: buffer :: unpack_from(uint32_t off)
{
    unpacker up(m_data + off, m_size - off);

    if (off <= m_size)
    {
        return up;
    }
    else
    {
        return up.as_error();
    }
}

void*
e :: buffer :: operator new (size_t, uint32_t num)
{
    return new char[offsetof(buffer, m_data) + num];
}

void
e :: buffer :: operator delete (void* mem)
{
    delete[] static_cast<char*>(mem);
}

e :: buffer :: buffer(uint32_t sz)
    : m_cap(sz)
    , m_size(0)
    , m_data()
{
}

e :: buffer :: buffer(const char* buf, uint32_t sz)
    : m_cap(sz)
    , m_size(sz)
    , m_data()
{
    memmove(m_data, buf, sz);
}

e :: buffer :: packer :: packer(buffer* buf, uint32_t off)
    : m_buf(buf)
    , m_off(off)
{
    EASSERT(off <= m_buf->m_cap);
}

e :: buffer :: packer :: packer(const packer& p)
    : m_buf(p.m_buf)
    , m_off(p.m_off)
{
}

e::buffer::packer
e :: buffer :: packer :: copy(const slice& from)
{
    uint64_t newsize = m_off + from.size();
    EASSERT(newsize <= m_buf->m_cap);
    memmove(m_buf->m_data + m_off, from.data(), from.size());
    m_buf->m_size = std::max(m_buf->m_size, static_cast<uint32_t>(newsize));
    return packer(m_buf, newsize);
}

e::buffer::packer
e :: buffer :: packer :: operator << (int8_t rhs)
{
    uint8_t nrhs = static_cast<uint8_t>(rhs);
    return *this << nrhs;
}

e::buffer::packer
e :: buffer :: packer :: operator << (int16_t rhs)
{
    uint16_t nrhs = static_cast<uint16_t>(rhs);
    return *this << nrhs;
}

e::buffer::packer
e :: buffer :: packer :: operator << (int32_t rhs)
{
    uint32_t nrhs = static_cast<uint32_t>(rhs);
    return *this << nrhs;
}

e::buffer::packer
e :: buffer :: packer :: operator << (int64_t rhs)
{
    uint64_t nrhs = static_cast<uint64_t>(rhs);
    return *this << nrhs;
}

e::buffer::packer
e :: buffer :: packer :: operator << (uint8_t rhs)
{
    uint64_t newsize = m_off + sizeof(uint8_t);
    EASSERT(newsize <= m_buf->m_cap);
    e::pack8be(rhs, m_buf->m_data + m_off);
    m_buf->m_size = std::max(m_buf->m_size, static_cast<uint32_t>(newsize));
    return packer(m_buf, newsize);
}

e::buffer::packer
e :: buffer :: packer :: operator << (uint16_t rhs)
{
    uint64_t newsize = m_off + sizeof(uint16_t);
    EASSERT(newsize <= m_buf->m_cap);
    e::pack16be(rhs, m_buf->m_data + m_off);
    m_buf->m_size = std::max(m_buf->m_size, static_cast<uint32_t>(newsize));
    return packer(m_buf, newsize);
}

e::buffer::packer
e :: buffer :: packer :: operator << (uint32_t rhs)
{
    uint64_t newsize = m_off + sizeof(uint32_t);
    EASSERT(newsize <= m_buf->m_cap);
    e::pack32be(rhs, m_buf->m_data + m_off);
    m_buf->m_size = std::max(m_buf->m_size, static_cast<uint32_t>(newsize));
    return packer(m_buf, newsize);
}

e::buffer::packer
e :: buffer :: packer :: operator << (uint64_t rhs)
{
    uint64_t newsize = m_off + sizeof(uint64_t);
    EASSERT(newsize <= m_buf->m_cap);
    e::pack64be(rhs, m_buf->m_data + m_off);
    m_buf->m_size = std::max(m_buf->m_size, static_cast<uint32_t>(newsize));
    return packer(m_buf, newsize);
}

e::buffer::packer
e :: buffer :: packer :: operator << (const slice& rhs)
{
    uint64_t newsize = m_off + sizeof(uint32_t) + rhs.size();
    EASSERT(newsize <= m_buf->m_cap);
    EASSERT(rhs.size() <= UINT32_MAX);
    uint32_t sz = rhs.size();
    e::pack32be(sz, m_buf->m_data + m_off);
    memmove(m_buf->m_data + m_off + sizeof(uint32_t), rhs.data(), sz);
    m_buf->m_size = std::max(m_buf->m_size, static_cast<uint32_t>(newsize));
    return packer(m_buf, newsize);
}
