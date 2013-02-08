// Copyright (c) 2011-2012, Robert Escriva
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
#include <vector>

// e
#include <e/slice.h>
#include <e/unpacker.h>

namespace e
{

class buffer
{
    public:
        class packer;

    public:
        static buffer* create(uint32_t sz) { return new (sz) buffer(sz); }
        static buffer* create(const char* buf, uint32_t sz) { return new (sz) buffer(buf, sz); }

    public:
        slice as_slice() const { return slice(m_data, m_size); }
        uint32_t capacity() const throw () { return m_cap; }
        bool cmp(const char* buf, uint32_t sz) const throw ();
        e::buffer* copy() const;
        const uint8_t* data() const throw () { return m_data; }
        const uint8_t* end() const throw () { return m_data + m_size; }
        bool empty() const throw () { return m_size == 0; }
        std::string hex() const { return as_slice().hex(); }
        uint32_t index(const uint8_t* mem, size_t sz) const throw ();
        uint32_t index(const char* mem, size_t sz) const throw ()
        { return index(reinterpret_cast<const uint8_t*>(mem), sz); }
        uint32_t index(uint8_t byte) const throw ();
        uint32_t remain() const throw () { assert(m_cap >= m_size); return m_cap - m_size; }
        uint32_t size() const throw () { return m_size; }

    public:
        void clear() throw () { m_size = 0; }
        uint8_t* data() throw () { return m_data; }
        uint8_t* end() throw () { return m_data + m_size; }
        void extend(uint32_t by) throw () { assert(m_cap >= m_size + by); m_size += by; }
        packer pack();
        packer pack_at(uint32_t off);
        void resize(uint32_t size) throw ();
        void shift(uint32_t off) throw ();
        unpacker unpack_from(uint32_t off);

    public:
        void* operator new (size_t sz, uint32_t num);
        void operator delete (void* mem);
        template <typename T> packer operator << (const T& t);
        template <typename T> unpacker operator >> (T& t);

    private:
#ifdef _MSC_VER
        buffer(uint32_t sz);
        buffer(const char* buf, uint32_t sz);
#else
        __attribute__ ((visibility ("default"))) buffer(uint32_t sz);
        __attribute__ ((visibility ("default"))) buffer(const char* buf, uint32_t sz);
#endif

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
        uint32_t remain() const { return m_buf->m_cap - m_off; }
        // Unlike the operator on slices, this does not pack the size.  It
        // simply copies the contents of "from" and advances the internal
        // offset, performing error checking along the way.
        packer copy(const slice& from);

    public:
        packer operator << (int8_t rhs);
        packer operator << (int16_t rhs);
        packer operator << (int32_t rhs);
        packer operator << (int64_t rhs);
        packer operator << (uint8_t rhs);
        packer operator << (uint16_t rhs);
        packer operator << (uint32_t rhs);
        packer operator << (uint64_t rhs);
        packer operator << (const slice& rhs);
        template <typename T> packer operator << (const std::vector<T>& rhs);
        template <typename A, typename B> packer operator << (const std::pair<A, B>& rhs);

    private:
        buffer* m_buf;
        uint32_t m_off;
};

template <typename T>
inline e::buffer::packer
e :: buffer :: operator << (const T& t)
{
    return packer(this, 0) << t;
}

template <typename T>
inline e::unpacker
e :: buffer :: operator >> (T& t)
{
    return unpacker(m_data, m_size) >> t;
}

template <typename T>
inline e::buffer::packer
e :: buffer :: packer :: operator << (const std::vector<T>& rhs)
{
    uint32_t sz = rhs.size();
    e::buffer::packer ret = *this << sz;

    for (uint32_t i = 0; i < sz; ++i)
    {
        ret = ret << rhs[i];
    }

    return ret;
}

template <typename A, typename B>
inline e::buffer::packer
e :: buffer :: packer :: operator << (const std::pair<A, B>& rhs)
{
    return *this << rhs.first << rhs.second;
}

} // namespace e

#endif // e_buffer_h_
