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

// C
#include <stddef.h>

// STL
#include <memory>

// e
#include "e/buffer.h"

using e::buffer;

void*
buffer :: operator new (size_t, size_t num)
{
    return new char[offsetof(buffer, m_data) + num];
}

void
buffer :: operator delete (void* mem)
{
    delete[] static_cast<char*>(mem);
}

buffer :: buffer(size_t sz)
    : m_cap(sz)
    , m_size(0)
    , m_data()
{
}

buffer :: buffer(const char* buf, size_t sz)
    : m_cap(sz)
    , m_size(sz)
    , m_data()
{
    memmove(m_data, buf, sz);
}

buffer :: ~buffer() throw ()
{
}

bool
buffer :: cmp(const char* buf, size_t sz) const
{
    if (m_size == sz)
    {
        return memcmp(m_data, buf, sz) == 0;
    }

    return false;
}

buffer*
buffer :: copy() const
{
    std::auto_ptr<buffer> ret(create(m_cap));
    ret->m_cap = m_cap;
    ret->m_size = m_size;
    memmove(ret->m_data, m_data, m_cap);
    return ret.release();
}

void
buffer :: resize(size_t sz)
{
    assert(sz <= m_cap);
    m_size = sz;
}

e::packer
buffer :: pack()
{
    return pack_at(0);
}

e::packer
buffer :: pack_at(size_t off)
{
    return packer(this, off);
}

e::unpacker
buffer :: unpack()
{
    return unpack_from(0);
}

e::unpacker
buffer :: unpack_from(size_t off)
{
    if (off > m_size)
    {
        return e::unpacker::error_out();
    }

    return e::unpacker(m_data + off, m_size - off);
}
