// Copyright (c) 2012-2015, Robert Escriva
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

// e
#include "e/buffer.h"
#include "e/endian.h"
#include "e/serialization.h"
#include "e/varint.h"

using e::packer;
using e::unpacker;
using e::pack_memmove;
using e::unpack_memmove;
using e::pack_varint;
using e::unpack_varint;

namespace
{

struct string_bytes_manager : public e::packer::bytes_manager
{
    string_bytes_manager(std::string* str) : m_str(str) {}
    virtual ~string_bytes_manager() throw () {}

    virtual void write(size_t off, const uint8_t* _ptr, size_t ptr_sz)
    {
        const char* ptr = reinterpret_cast<const char*>(_ptr);

        if (m_str->size() < off)
        {
            m_str->resize(off, '\0');
        }

        assert(m_str->size() >= off);

        if (m_str->size() == off)
        {
            m_str->append(ptr, ptr_sz);
        }
        else
        {
            const size_t end = std::min(off + ptr_sz, m_str->size());
            m_str->replace(off, end, ptr, ptr_sz);
        }
    }

    private:
        std::string* m_str;

    private:
        string_bytes_manager(const string_bytes_manager&);
        string_bytes_manager& operator = (const string_bytes_manager&);
};

struct buffer_bytes_manager : public e::packer::bytes_manager
{
    buffer_bytes_manager(e::buffer* buf) : m_buf(buf) {}
    virtual ~buffer_bytes_manager() throw () {}

    virtual void write(size_t off, const uint8_t* ptr, size_t ptr_sz)
    {
        const size_t new_size = off + ptr_sz;

        if (new_size > m_buf->capacity())
        {
            abort();
        }

        memmove(m_buf->data() + off, ptr, ptr_sz);

        if (m_buf->size() < new_size)
        {
            m_buf->resize(new_size);
        }
    }

    private:
        e::buffer* m_buf;

    private:
        buffer_bytes_manager(const buffer_bytes_manager&);
        buffer_bytes_manager& operator = (const buffer_bytes_manager&);
};

} // namespace

packer :: bytes_manager :: bytes_manager()
{
}

packer :: bytes_manager :: ~bytes_manager() throw ()
{
}

packer :: packer(std::string* str)
    : m_mgr(new string_bytes_manager(str))
    , m_off(0)
{
}

packer :: packer(std::string* str, size_t off)
    : m_mgr(new string_bytes_manager(str))
    , m_off(off)
{
}

packer :: packer(e::buffer* buf, size_t off)
    : m_mgr(new buffer_bytes_manager(buf))
    , m_off(off)
{
}

packer :: packer(const packer& other)
    : m_mgr(other.m_mgr)
    , m_off(other.m_off)
{
}

packer :: ~packer() throw ()
{
}

void
packer :: append(const uint8_t* ptr, size_t ptr_sz, packer* pa)
{
    if (SIZE_MAX - m_off < ptr_sz)
    {
        abort();
    }

    m_mgr->write(m_off, ptr, ptr_sz);
    *pa = packer(m_mgr, m_off + ptr_sz);
}

packer :: packer(e::compat::shared_ptr<bytes_manager> mgr, size_t off)
    : m_mgr(mgr)
    , m_off(off)
{
}

unpacker
unpacker :: error_out()
{
    unpacker up;
    up.m_error = true;
    return up;
}

unpacker :: unpacker()
    : m_ptr(NULL)
    , m_end(0)
    , m_error(false)
{
}

unpacker :: unpacker(const uint8_t* data, size_t sz)
    : m_ptr(data)
    , m_end(m_ptr + sz)
    , m_error(false)
{
}

unpacker :: unpacker(const char* data, size_t sz)
    : m_ptr(reinterpret_cast<const uint8_t*>(data))
    , m_end(m_ptr + sz)
    , m_error(false)
{
}

unpacker :: unpacker(const std::string& s)
    : m_ptr(reinterpret_cast<const uint8_t*>(s.data()))
    , m_end(m_ptr + s.size())
    , m_error(false)
{
}

unpacker :: unpacker(const e::slice& s)
    : m_ptr(s.data())
    , m_end(m_ptr + s.size())
    , m_error(false)
{
}

unpacker :: unpacker(const unpacker& other)
    : m_ptr(other.m_ptr)
    , m_end(other.m_end)
    , m_error(other.m_error)
{
}

unpacker :: ~unpacker() throw ()
{
}

unpacker
unpacker :: advance(size_t sz) const
{
    const uint8_t* ptr = m_ptr + sz;

    if (ptr > m_end)
    {
        return error_out();
    }

    return unpacker(ptr, m_end - ptr);
}

unpacker&
unpacker :: operator = (const unpacker& rhs)
{
    // no self assign check needed
    m_ptr = rhs.m_ptr;
    m_end = rhs.m_end;
    m_error = rhs.m_error;
    return *this;
}

#define PACKER(TYPE, PACKF) \
    packer \
    e :: operator << (packer pa, const TYPE& rhs) \
    { \
        uint8_t buf[16]; \
        uint8_t* ptr = PACKF(rhs, buf); \
        pa.append(buf, ptr - buf, &pa); \
        return pa; \
    }

PACKER(int8_t, e::pack8be)
PACKER(int16_t, e::pack16be)
PACKER(int32_t, e::pack32be)
PACKER(int64_t, e::pack64be)
PACKER(uint8_t, e::pack8be)
PACKER(uint16_t, e::pack16be)
PACKER(uint32_t, e::pack32be)
PACKER(uint64_t, e::pack64be)
PACKER(double, e::packdoublebe)

#define UNPACKER(TYPE, UNPACKF) \
    unpacker \
    e :: operator >> (unpacker up, TYPE& rhs) \
    { \
        const uint8_t* start = up.start(); \
        const uint8_t* limit = up.limit(); \
        const size_t sz = pack_size(rhs); \
        if (!up.error() && start + sz <= limit) \
        { \
            UNPACKF(start, &rhs); \
            return unpacker(start + sz, limit - start - sz); \
        } \
        else \
        { \
            return unpacker::error_out(); \
        } \
    }

UNPACKER(int8_t, e::unpack8be)
UNPACKER(int16_t, e::unpack16be)
UNPACKER(int32_t, e::unpack32be)
UNPACKER(int64_t, e::unpack64be)
UNPACKER(uint8_t, e::unpack8be)
UNPACKER(uint16_t, e::unpack16be)
UNPACKER(uint32_t, e::unpack32be)
UNPACKER(uint64_t, e::unpack64be)
UNPACKER(double, e::unpackdoublebe)

size_t
e :: pack_size(const e::slice& s)
{
    return varint_length(s.size()) + s.size();
}

packer
e :: operator << (packer pa, const e::slice& rhs)
{
    return pa << pack_varint(rhs.size()) << pack_memmove(rhs.data(), rhs.size());
}

unpacker
e :: operator >> (unpacker up, e::slice& rhs)
{
    uint64_t size;
    up = up >> unpack_varint(size);

    if (up.error())
    {
        return up;
    }

    rhs = e::slice(up.start(), size);
    return up.advance(size);
}

size_t
e :: pack_size(const po6::net::ipaddr& ip)
{
    if (ip.family() == AF_INET)
    {
        return 5;
    }
    else if (ip.family() == AF_INET6)
    {
        return 17;
    }
    else
    {
        return 1;
    }
}

packer
e :: operator << (packer pa, const po6::net::ipaddr& rhs)
{
    assert(rhs.family() == AF_INET || rhs.family() == AF_INET6 || rhs.family() == AF_UNSPEC);

    if (rhs.family() == AF_INET)
    {
        return pa << uint8_t(4) << pack_memmove(&rhs.v4addr(), 4);
    }
    else if (rhs.family() == AF_INET6)
    {
        return pa << uint8_t(6) << pack_memmove(&rhs.v6addr(), 16);
    }
    else
    {
        return pa << uint8_t(0);
    }
}

unpacker
e :: operator >> (unpacker up, po6::net::ipaddr& rhs)
{
    uint8_t type;
    up = up >> type;

    if (up.error())
    {
        return up;
    }

    if (type == 4)
    {
        in_addr v4;
        up = up >> unpack_memmove(&v4, 4);
        rhs = po6::net::ipaddr(v4);
        return up;
    }
    else if (type == 6)
    {
        in6_addr v6;
        up = up >> unpack_memmove(&v6, 4);
        rhs = po6::net::ipaddr(v6);
        return up;
    }
    else if (type == 0)
    {
        rhs = po6::net::ipaddr();
        return up;
    }
    else
    {
        return unpacker::error_out();
    }
}

size_t
e :: pack_size(const po6::net::location& rhs)
{
    return pack_size(rhs.address) + sizeof(uint16_t);
}

packer
e :: operator << (packer lhs, const po6::net::location& rhs)
{
    return lhs << rhs.address << rhs.port;
}

unpacker
e :: operator >> (unpacker lhs, po6::net::location& rhs)
{
    return lhs >> rhs.address >> rhs.port;
}

packer
e :: operator << (e::packer lhs, const po6::net::hostname& rhs)
{
    return lhs << e::slice(rhs.address.data(), rhs.address.size()) << rhs.port;
}

unpacker
e :: operator >> (e::unpacker lhs, po6::net::hostname& rhs)
{
    e::slice address;
    lhs = lhs >> address >> rhs.port;
    rhs.address = std::string(address.cdata(), address.size());
    return lhs;
}

size_t
e :: pack_size(const po6::net::hostname& rhs)
{
    return pack_size(e::slice(rhs.address)) + sizeof(uint16_t);
}

pack_memmove :: pack_memmove(const void* d, size_t s)
    : m_data(static_cast<const uint8_t*>(d))
    , m_size(s)
{
}

pack_memmove :: pack_memmove(const pack_memmove& other)
    : m_data(other.m_data)
    , m_size(other.m_size)
{
}

pack_memmove :: ~pack_memmove() throw ()
{
}

packer
e :: operator << (packer pa, const pack_memmove& x)
{
    pa.append(x.data(), x.size(), &pa);
    return pa;
}

unpack_memmove :: unpack_memmove(void* d, size_t s)
    : m_data(static_cast<uint8_t*>(d))
    , m_size(s)
{
}

unpack_memmove :: unpack_memmove(const unpack_memmove& other)
    : m_data(other.m_data)
    , m_size(other.m_size)
{
}

unpack_memmove :: ~unpack_memmove() throw ()
{
}

e::unpacker
e :: operator >> (e::unpacker up, const unpack_memmove& x)
{
    if (up.remain() < x.size())
    {
        return unpacker::error_out();
    }

    memmove(x.data(), up.start(), x.size());
    return up.advance(x.size());
}

pack_varint :: pack_varint(uint64_t _x)
    : x(_x)
{
}

pack_varint :: ~pack_varint() throw ()
{
}

e::packer
e :: operator << (e::packer pa, const pack_varint& x)
{
    char buf[VARINT_64_MAX_SIZE];
    char* ptr = varint64_encode(buf, x.x);
    return pa << pack_memmove(buf, ptr - buf);
}

unpack_varint :: unpack_varint(uint64_t& _x)
    : x(_x)
{
}

unpack_varint :: ~unpack_varint() throw ()
{
}

e::unpacker
e :: operator >> (e::unpacker up, const unpack_varint& x)
{
    const uint8_t* ptr = varint64_decode(up.start(), up.limit(), &x.x);

    if (ptr == NULL)
    {
        return unpacker::error_out();
    }

    return up.advance(ptr - up.start());
}
