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

#ifndef e_buffer_h_
#define e_buffer_h_

// C
#include <cstdlib>
#include <stdint.h>

// STL
#include <string>

// e
#include <e/slice.h>

namespace e
{

class buffer
{
    public:
        class packer;
        class padding;
        class unpacker;

    public:
        static buffer* create(uint32_t sz) { return new (sz) buffer(sz); }
        static buffer* create(const char* buf, uint32_t sz) { return new (sz) buffer(buf, sz); }

    public:
        slice as_slice() const { return slice(m_data, m_size); }
        uint32_t capacity() const throw () { return m_cap; }
        bool cmp(const char* buf, uint32_t sz) const throw ();
        const uint8_t* data() const throw () { return m_data; }
        std::string hex() const;
        uint32_t index(uint8_t b) const throw ();
        uint32_t size() const throw () { return m_size; }

    public:
        void clear() throw () { m_size = 0; }
        uint8_t* data() throw () { return m_data; }
        packer pack_at(uint32_t off);
        void resize(uint32_t size) throw ();
        void shift(uint32_t off) throw ();
        unpacker unpack_from(uint32_t off);

    public:
        void* operator new (size_t sz, uint32_t num);
        void operator delete (void* mem);
        template <typename T> packer operator << (const T& t);
        template <typename T> unpacker operator >> (T& t);
        unpacker operator >> (padding t);

    private:
        __attribute__ ((visibility ("default"))) buffer(uint32_t sz);
        __attribute__ ((visibility ("default"))) buffer(const char* buf, uint32_t sz);

    // Please see:
    // http://stackoverflow.com/questions/4559558/one-element-array-in-struct
    private:
        uint32_t m_cap;
        uint32_t m_size;
        uint8_t m_data[1];
};

class buffer::packer
{
    public:
        packer(buffer* buf, uint32_t off);
        packer(const packer& p);

    public:
        packer as_error() const;
        bool error() const { return m_error; }
        uint32_t remain() const { return m_buf->m_cap - m_off; }

    public:
        packer operator << (uint8_t rhs);
        packer operator << (uint16_t rhs);
        packer operator << (uint32_t rhs);
        packer operator << (uint64_t rhs);
        packer operator << (const slice& rhs);
        packer operator << (const buffer::padding& rhs);

    private:
        buffer* m_buf;
        uint32_t m_off;
        bool m_error;
};

class buffer::padding
{
    public:
        padding(uint32_t pad) : m_pad(pad) {}

    private:
        friend class packer;
        friend class unpacker;

    private:
        uint32_t m_pad;
};

class buffer::unpacker
{
    public:
        unpacker(const buffer* buf, uint32_t off);
        unpacker(const unpacker& up);

    public:
        unpacker as_error() const;
        slice as_slice() const;
        bool error() const { return m_error; }
        uint32_t remain() const { return m_buf->m_size - m_off; }

    public:
        unpacker operator >> (uint8_t& rhs);
        unpacker operator >> (uint16_t& rhs);
        unpacker operator >> (uint32_t& rhs);
        unpacker operator >> (uint64_t& rhs);
        unpacker operator >> (slice& rhs);
        unpacker operator >> (buffer::padding rhs);

    private:
        const buffer* m_buf;
        uint32_t m_off;
        bool m_error;
};

template <typename T>
inline e::buffer::packer
e :: buffer :: operator << (const T& t)
{
    return packer(this, 0) << t;
}

template <typename T>
inline e::buffer::unpacker
e :: buffer :: operator >> (T& t)
{
    return unpacker(this, 0) >> t;
}

// This is needed because buffer::padding is passed by value, and the above
// templates take objects by reference.
inline e::buffer::unpacker
e :: buffer :: operator >> (buffer::padding t)
{
    return unpacker(this, 0) >> t;
}

} // namespace e

#endif // e_buffer_h_
