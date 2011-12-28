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

// C
#include <cassert>
#include <cstring>

// STL
#include <algorithm>
#include <iomanip>
#include <sstream>

#include <iostream>

// e
#include "../include/e/buffer.h"

bool
e :: buffer :: cmp(const char* buf, uint32_t sz) const throw ()
{
    if (m_size == sz)
    {
        return memcmp(m_data, buf, sz) == 0;
    }

    return false;
}

std::string
e:: buffer :: hex() const
{
    std::ostringstream ostr;
    ostr << std::hex;

    for (uint32_t i = 0; i < m_size; ++i)
    {
        unsigned int num = m_data[i];
        ostr << std::setw(2) << std::setfill('0') << num;
    }

    return ostr.str();
}

uint32_t
e :: buffer :: index(uint8_t b) const throw ()
{
    const uint8_t* loc;
    loc = static_cast<const uint8_t*>(memchr(m_data, b, m_size));
    return loc ? loc - m_data : m_cap;
}

e::buffer::packer
e :: buffer :: pack_at(uint32_t off)
{
    return packer(this, off);
}

void
e :: buffer :: resize(uint32_t sz) throw ()
{
    assert(sz <= m_cap);
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

e::buffer::unpacker
e :: buffer :: unpack_from(uint32_t off)
{
    return unpacker(this, off);
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
    , m_error(off > m_buf->m_cap)
{
}

e :: buffer :: packer :: packer(const packer& p)
    : m_buf(p.m_buf)
    , m_off(p.m_off)
    , m_error(p.m_error)
{
}

e::buffer::packer
e :: buffer :: packer :: as_error() const
{
    packer ret(*this);
    ret.m_error = true;
    return ret;
}

e::buffer::packer
e :: buffer :: packer :: operator << (uint8_t rhs)
{
    uint64_t newsize = m_off + sizeof(uint8_t);

    if (!m_error && newsize <= m_buf->m_cap)
    {
        m_buf->m_data[m_off] = rhs;
        m_buf->m_size = std::max(m_buf->m_size, static_cast<uint32_t>(newsize));
        return packer(m_buf, newsize);
    }
    else
    {
        return as_error();
    }
}

e::buffer::packer
e :: buffer :: packer :: operator << (uint16_t rhs)
{
    uint64_t newsize = m_off + sizeof(uint16_t);

    if (!m_error && newsize <= m_buf->m_cap)
    {
        m_buf->m_data[m_off] = rhs >> 8;
        m_buf->m_data[m_off + 1] = rhs & 0xff;
        m_buf->m_size = std::max(m_buf->m_size, static_cast<uint32_t>(newsize));
        return packer(m_buf, newsize);
    }
    else
    {
        return as_error();
    }
}

e::buffer::packer
e :: buffer :: packer :: operator << (uint32_t rhs)
{
    uint64_t newsize = m_off + sizeof(uint32_t);

    if (!m_error && newsize <= m_buf->m_cap)
    {
        m_buf->m_data[m_off] = rhs >> 24;
        m_buf->m_data[m_off + 1] = (rhs >> 16) & 0xff;
        m_buf->m_data[m_off + 2] = (rhs >> 8) & 0xff;
        m_buf->m_data[m_off + 3] = rhs & 0xff;
        m_buf->m_size = std::max(m_buf->m_size, static_cast<uint32_t>(newsize));
        return packer(m_buf, newsize);
    }
    else
    {
        return as_error();
    }
}

e::buffer::packer
e :: buffer :: packer :: operator << (uint64_t rhs)
{
    uint64_t newsize = m_off + sizeof(uint64_t);

    if (!m_error && newsize <= m_buf->m_cap)
    {
        m_buf->m_data[m_off] = rhs >> 56;
        m_buf->m_data[m_off + 1] = (rhs >> 48) & 0xff;
        m_buf->m_data[m_off + 2] = (rhs >> 40) & 0xff;
        m_buf->m_data[m_off + 3] = (rhs >> 32) & 0xff;
        m_buf->m_data[m_off + 4] = (rhs >> 24) & 0xff;
        m_buf->m_data[m_off + 5] = (rhs >> 16) & 0xff;
        m_buf->m_data[m_off + 6] = (rhs >> 8) & 0xff;
        m_buf->m_data[m_off + 7] = rhs & 0xff;
        m_buf->m_size = std::max(m_buf->m_size, static_cast<uint32_t>(newsize));
        return packer(m_buf, newsize);
    }
    else
    {
        return as_error();
    }
}

e::buffer::packer
e :: buffer :: packer :: operator << (const slice& rhs)
{
    uint64_t newsize = m_off + sizeof(uint32_t) + rhs.size();

    if (!m_error && newsize <= m_buf->m_cap && rhs.size() <= UINT32_MAX)
    {
        uint32_t sz = rhs.size();
        *this << sz; // XXX perf loss
        memmove(m_buf->m_data + m_off + sizeof(uint32_t), rhs.data(), sz);
        m_buf->m_size = std::max(m_buf->m_size, static_cast<uint32_t>(newsize));
        return packer(m_buf, newsize);
    }
    else
    {
        return as_error();
    }
}

e::buffer::packer
e :: buffer :: packer :: operator << (const buffer::padding& rhs)
{
    uint64_t newsize = m_off + rhs.m_pad;

    if (!m_error && newsize <= m_buf->m_cap)
    {
        // Zero the new bytes which the padding adds to the buffer's size.
        // The padded region is not zeroed absolutely because the padding can be
        // used to skip regions (e.g., headers) so that the packer can pack
        // after the header.
        if (m_buf->m_size < newsize)
        {
            memset(m_buf->m_data + m_buf->m_size, 0, newsize - m_buf->m_size);
            m_buf->m_size = newsize;
        }

        return packer(m_buf, newsize);
    }
    else
    {
        return as_error();
    }
}

e :: buffer :: unpacker :: unpacker(const buffer* buf, uint32_t off)
    : m_buf(buf)
    , m_off(off)
    , m_error(off > m_buf->m_cap)
{
}

e :: buffer :: unpacker :: unpacker(const unpacker& p)
    : m_buf(p.m_buf)
    , m_off(p.m_off)
    , m_error(p.m_error)
{
}

e::buffer::unpacker
e :: buffer :: unpacker :: as_error() const
{
    unpacker ret(*this);
    ret.m_error = true;
    return ret;
}

e::slice
e :: buffer :: unpacker :: as_slice() const
{
    return e::slice(m_buf->m_data + m_off, m_buf->m_size - m_off);
}

e::buffer::unpacker
e :: buffer :: unpacker :: operator >> (uint8_t& rhs)
{
    uint64_t newsize = m_off + sizeof(uint8_t);

    if (!m_error && newsize <= m_buf->m_size)
    {
        rhs = m_buf->m_data[m_off];
        return unpacker(m_buf, newsize);
    }
    else
    {
        return as_error();
    }
}

e::buffer::unpacker
e :: buffer :: unpacker :: operator >> (uint16_t& rhs)
{
    uint64_t newsize = m_off + sizeof(uint16_t);

    if (!m_error && newsize <= m_buf->m_size)
    {
        rhs = static_cast<uint16_t>(m_buf->m_data[m_off]) << 8
            | static_cast<uint16_t>(m_buf->m_data[m_off + 1]);
        return unpacker(m_buf, newsize);
    }
    else
    {
        return as_error();
    }
}

e::buffer::unpacker
e :: buffer :: unpacker :: operator >> (uint32_t& rhs)
{
    uint64_t newsize = m_off + sizeof(uint32_t);

    if (!m_error && newsize <= m_buf->m_size)
    {
        rhs = static_cast<uint32_t>(m_buf->m_data[m_off]) << 24
            | static_cast<uint32_t>(m_buf->m_data[m_off + 1]) << 16
            | static_cast<uint32_t>(m_buf->m_data[m_off + 2]) << 8
            | static_cast<uint32_t>(m_buf->m_data[m_off + 3]);
        return unpacker(m_buf, newsize);
    }
    else
    {
        return as_error();
    }
}

e::buffer::unpacker
e :: buffer :: unpacker :: operator >> (uint64_t& rhs)
{
    uint64_t newsize = m_off + sizeof(uint64_t);

    if (!m_error && newsize <= m_buf->m_size)
    {
        rhs = static_cast<uint64_t>(m_buf->m_data[m_off]) << 56
            | static_cast<uint64_t>(m_buf->m_data[m_off + 1]) << 48
            | static_cast<uint64_t>(m_buf->m_data[m_off + 2]) << 40
            | static_cast<uint64_t>(m_buf->m_data[m_off + 3]) << 32
            | static_cast<uint64_t>(m_buf->m_data[m_off + 4]) << 24
            | static_cast<uint64_t>(m_buf->m_data[m_off + 5]) << 16
            | static_cast<uint64_t>(m_buf->m_data[m_off + 6]) << 8
            | static_cast<uint64_t>(m_buf->m_data[m_off + 7]);
        return unpacker(m_buf, newsize);
    }
    else
    {
        return as_error();
    }
}

e::buffer::unpacker
e :: buffer :: unpacker :: operator >> (slice& rhs)
{
    uint32_t sz;
    e::buffer::unpacker tmp = *this >> sz;

    if (!tmp.m_error && tmp.m_off + sz <= m_buf->m_size)
    {
        rhs = slice(m_buf->m_data + tmp.m_off, sz);
        return unpacker(m_buf, tmp.m_off + sz);
    }
    else
    {
        return as_error();
    }
}

e::buffer::unpacker
e :: buffer :: unpacker :: operator >> (buffer::padding rhs)
{
    uint64_t newsize = m_off + rhs.m_pad;

    if (!m_error && newsize <= m_buf->m_size)
    {
        return unpacker(m_buf, newsize);
    }
    else
    {
        return as_error();
    }
}
