// Copyright (c) 2011-2015, Robert Escriva
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

#ifndef e_serialization_h_
#define e_serialization_h_

// C
#include <stdint.h>

// STL
#include <memory>
#include <vector>

// po6
#include <po6/net/hostname.h>
#include <po6/net/location.h>

// e
#include <e/compat.h>
#include <e/slice.h>
#include <e/varint.h>

#define E_SERIALIZATION_TRIPLET(X) \
    e::packer operator << (e::packer pa, const X& x); \
    e::unpacker operator >> (e::unpacker up, X& x)

namespace e
{
class buffer;

inline uint64_t pack_size(int8_t) { return 1; }
inline uint64_t pack_size(int16_t) { return 2; }
inline uint64_t pack_size(int32_t) { return 4; }
inline uint64_t pack_size(int64_t) { return 8; }

inline uint64_t pack_size(uint8_t) { return 1; }
inline uint64_t pack_size(uint16_t) { return 2; }
inline uint64_t pack_size(uint32_t) { return 4; }
inline uint64_t pack_size(uint64_t) { return 8; }

inline uint64_t pack_size(double) { return 8; }

class packer
{
    public:
        packer(std::string* str);
        packer(std::string* str, size_t off);
        packer(e::buffer* buf, size_t off);
        packer(const packer& other);
        ~packer() throw ();

    public:
        void append(const uint8_t* ptr, size_t ptr_sz, packer* pa);

    public:
        template <typename T> packer operator << (const std::vector<T>& rhs);
        template <typename A, typename B> packer operator << (const std::pair<A, B>& rhs);
        struct bytes_manager
        {
            bytes_manager();
            virtual ~bytes_manager() throw ();

            virtual void write(size_t off, const uint8_t* ptr, size_t ptr_sz) = 0;

            private:
                bytes_manager(const bytes_manager&);
                bytes_manager& operator = (const bytes_manager&);
        };

    private:
        packer(e::compat::shared_ptr<bytes_manager> mgr, size_t off);

    private:
        e::compat::shared_ptr<bytes_manager> m_mgr;
        size_t m_off;
};

class unpacker
{
    public:
        static unpacker error_out();

    public:
        unpacker();
        unpacker(const uint8_t* data, size_t sz);
        unpacker(const char* data, size_t sz);
        unpacker(const std::string& s);
        unpacker(const e::slice& s);
        unpacker(const unpacker& other);
        ~unpacker() throw ();

    public:
        bool error() const { return m_error; }
        size_t remain() const { return m_end - m_ptr; }
        e::slice remainder() const { return e::slice(m_ptr, m_end - m_ptr); }
        const uint8_t* start() const { return m_ptr; }
        const uint8_t* limit() const { return m_end; }
        unpacker advance(size_t sz) const;

    public:
        unpacker& operator = (const unpacker& rhs);
        template <typename T> unpacker operator >> (std::vector<T>& rhs);
        template <typename A, typename B> unpacker operator >> (std::pair<A, B>& rhs);

    private:
        const uint8_t* m_ptr;
        const uint8_t* m_end;
        bool m_error;
};

E_SERIALIZATION_TRIPLET(int8_t);
E_SERIALIZATION_TRIPLET(int16_t);
E_SERIALIZATION_TRIPLET(int32_t);
E_SERIALIZATION_TRIPLET(int64_t);
E_SERIALIZATION_TRIPLET(uint8_t);
E_SERIALIZATION_TRIPLET(uint16_t);
E_SERIALIZATION_TRIPLET(uint32_t);
E_SERIALIZATION_TRIPLET(uint64_t);
E_SERIALIZATION_TRIPLET(double);

size_t pack_size(const e::slice& s);
E_SERIALIZATION_TRIPLET(e::slice);

size_t pack_size(const po6::net::ipaddr& i);
E_SERIALIZATION_TRIPLET(po6::net::ipaddr);
size_t pack_size(const po6::net::location& l);
E_SERIALIZATION_TRIPLET(po6::net::location);
size_t pack_size(const po6::net::hostname& h);
E_SERIALIZATION_TRIPLET(po6::net::hostname);

class pack_memmove
{
    public:
        pack_memmove(const void* data, size_t size);
        pack_memmove(const pack_memmove& other);
        ~pack_memmove() throw ();

    public:
        const uint8_t* data() const { return m_data; }
        size_t size() const { return m_size; }

    public:
        pack_memmove& operator = (const pack_memmove& rhs);

    private:
        const uint8_t* m_data;
        size_t m_size;
};

e::packer
operator << (e::packer pa, const pack_memmove& x);

class unpack_memmove
{
    public:
        unpack_memmove(void* data, size_t size);
        unpack_memmove(const unpack_memmove& other);
        ~unpack_memmove() throw ();

    public:
        uint8_t* data() const { return m_data; }
        size_t size() const { return m_size; }

    public:
        unpack_memmove& operator = (const unpack_memmove& rhs);

    private:
        uint8_t* m_data;
        size_t m_size;
};

e::unpacker
operator >> (e::unpacker up, const unpack_memmove& x);

class pack_varint
{
    public:
        pack_varint(uint64_t x);
        ~pack_varint() throw ();

    public:
        uint64_t x;
};

e::packer
operator << (e::packer pa, const pack_varint& x);

class unpack_varint
{
    public:
        unpack_varint(uint64_t& x);
        ~unpack_varint() throw ();

    public:
        uint64_t& x;
};

e::unpacker
operator >> (e::unpacker up, const unpack_varint& x);

} // namespace e

// vector<T>
template <typename T>
size_t
pack_size(const typename std::vector<T>& v)
{
    size_t sz = e::varint_length(v.size());

    for (size_t i = 0; i < v.size(); ++i)
    {
        sz += pack_size(v[i]);
    }

    return sz;
}

template <typename T>
e::packer
e :: packer :: operator << (const std::vector<T>& rhs)
{
    e::packer pa(*this);
    const uint64_t sz = rhs.size();
    pa = pa << pack_varint(sz);

    for (uint32_t i = 0; i < sz; ++i)
    {
        pa = pa << rhs[i];
    }

    return pa;
}

template <typename T>
e::unpacker
e :: unpacker :: operator >> (std::vector<T>& rhs)
{
    e::unpacker up(*this);
    uint64_t sz = 0;
    up = up >> unpack_varint(sz);
    rhs.clear();

    for (uint64_t i = 0; i < sz; ++i)
    {
        rhs.push_back(T());
        up = up >> rhs.back();
    }

    return up;
}

// pair<A, B>
template <typename A, typename B>
size_t
pack_size(const std::pair<A, B>& p)
{
    return pack_size(p.first) + pack_size(p.second);
}

template <typename A, typename B>
e::packer
e :: packer :: operator << (const std::pair<A, B>& p)
{
    return *this << p.first << p.second;
}

template <typename A, typename B>
e::unpacker
e :: unpacker :: operator >> (std::pair<A, B>& p)
{
    return *this >> p.first >> p.second;
}

#undef E_SERIALIZATION_TRIPLET

#endif // e_serialization_h_
