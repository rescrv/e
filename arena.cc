// Copyright (c) 2014-2015, Robert Escriva
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
#include "e/arena.h"
#include "e/buffer.h"

using e::arena;

arena :: arena()
    : m_to_free()
    , m_buffers()
    , m_start()
    , m_limit()
{
}

arena :: ~arena()
{
    for (size_t i = 0; i < m_to_free.size(); ++i)
    {
        free(m_to_free[i]);
    }

    for (size_t i = 0; i < m_buffers.size(); ++i)
    {
        delete m_buffers[i];
    }
}

void
arena :: reserve(size_t sz)
{
    if (m_start + sz <= m_limit)
    {
        return;
    }

    unsigned char* tmp = NULL;
    raw_allocate(sz, &tmp);

    if (tmp)
    {
        m_start = tmp;
        m_limit = m_start + sz;
    }
}

void
arena :: allocate(size_t sz, char** ptr)
{
    return allocate(sz, reinterpret_cast<unsigned char**>(ptr));
}

void
arena :: allocate(size_t sz, unsigned char** ptr)
{
    if (m_start + sz <= m_limit)
    {
        *ptr = m_start;
        m_start += sz;
    }
    else
    {
        raw_allocate(sz, ptr);
    }
}

void
arena :: takeover(char* ptr)
{
    takeover(reinterpret_cast<unsigned char*>(ptr));
}

void
arena :: takeover(unsigned char* ptr)
{
    m_to_free.push_back(ptr);
}

void
arena :: takeover(e::buffer* buf)
{
    m_buffers.push_back(buf);
}

void
arena :: clear()
{
    for (size_t i = 0; i < m_to_free.size(); ++i)
    {
        free(m_to_free[i]);
    }

    for (size_t i = 0; i < m_buffers.size(); ++i)
    {
        delete m_buffers[i];
    }
}

void
arena :: raw_allocate(size_t sz, unsigned char** ptr)
{
    unsigned char* tmp = reinterpret_cast<unsigned char*>(malloc(sz));
    *ptr = tmp;

    if (tmp)
    {
        m_to_free.push_back(tmp);
    }
}
