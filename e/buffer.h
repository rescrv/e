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

#ifndef e_buffer_h_
#define e_buffer_h_

// C
#include <stdlib.h>

// STL
#include <string>

// e
#include <e/serialization.h>
#include <e/slice.h>

namespace e
{

class buffer
{
    public:
        static buffer* create(size_t sz) { return new (sz) buffer(sz); }
        static buffer* create(const char* buf, size_t sz) { return new (sz) buffer(buf, sz); }

    public:
        void operator delete (void* mem);
        ~buffer() throw ();

    public:
        size_t capacity() const { return m_cap; }
        size_t size() const { return m_size; }
        const uint8_t* data() const { return m_data; }
        const char* cdata() const { return reinterpret_cast<const char*>(m_data); }
        const uint8_t* end() const { return data() + m_size; }
        const char* cend() const { return cdata() + m_size; }
        uint8_t* data() { return m_data; }
        char* cdata() { return reinterpret_cast<char*>(m_data); }
        uint8_t* end() { return data() + m_size; }
        char* cend() { return cdata() + m_size; }
        bool cmp(const char* buf, size_t sz) const;
        e::slice as_slice() const { return e::slice(m_data, m_size); }
        std::string hex() const { return as_slice().hex(); }
        e::buffer* copy() const;

    public:
        void resize(size_t size);

    public:
        e::packer pack();
        e::packer pack_at(size_t off);
        e::unpacker unpack();
        e::unpacker unpack_from(size_t off);

    private:
        void* operator new (size_t sz, size_t num);
        buffer(size_t sz);
        buffer(const char* buf, size_t sz);

    private:
        size_t m_cap;
        size_t m_size;
        uint8_t m_data[1];
};

} // namespace e

#endif // e_buffer_h_
