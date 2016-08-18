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

// C
#include <stdlib.h>

// STL
#include <iomanip>
#include <sstream>

// e
#include "e/slice.h"
#include "e/base64.h"

using e::slice;

slice :: slice()
    : m_data(NULL)
    , m_sz(0)
{
}

slice :: slice(const char* data)
    : m_data(reinterpret_cast<const uint8_t*>(data))
    , m_sz(strlen(data))
{
}

slice :: slice(const char* d, size_t sz)
    : m_data(reinterpret_cast<const uint8_t*>(d))
    , m_sz(sz)
{
}

slice :: slice(const uint8_t* d, size_t sz)
    : m_data(d)
    , m_sz(sz)
{
}

slice :: slice(const std::string& s)
    : m_data(reinterpret_cast<const uint8_t*>(s.data()))
    , m_sz(s.size())
{
}

slice :: slice(const std::vector<uint8_t>& buf)
    : m_data(&buf.front())
    , m_sz(buf.size())
{
}

slice :: slice(const slice& other)
    : m_data(other.m_data)
    , m_sz(other.m_sz)
{
}

slice :: ~slice() throw ()
{
}

int
slice :: compare(const slice& rhs) const
{
    if (m_sz < rhs.m_sz)
    {
        return -1;
    }
    else if (m_sz > rhs.m_sz)
    {
        return 1;
    }
    else
    {
        return memcmp(m_data, rhs.m_data, m_sz);
    }
}

std::string
slice :: hex() const
{
    std::ostringstream ostr;
    ostr << std::hex;

    for (uint32_t i = 0; i < m_sz; ++i)
    {
        unsigned int num = m_data[i];
        ostr << std::setw(2) << std::setfill('0') << num;
    }

    return ostr.str();
}

std::string
slice :: b64() const
{
    std::vector<char> buf(m_sz * 2);
    ssize_t sz = b64_ntop(m_data, m_sz, &buf[0], buf.size());
    assert(sz >= 0 && sz <= buf.size());
    return std::string(&buf[0], sz);
}

bool
slice :: starts_with(const e::slice& prefix) const
{
    return size() >= prefix.size() &&
           memcmp(data(), prefix.data(), prefix.size()) == 0;
}

void
slice :: advance(size_t sz)
{
    assert(sz <= m_sz);
    m_data += sz;
    m_sz -= sz;
}

void
slice :: reset()
{
    m_data = NULL;
    m_sz = 0;
}

void
slice :: reset(const uint8_t* d, size_t sz)
{
    m_data = d;
    m_sz = sz;
}

slice&
slice :: operator = (const slice& rhs)
{
    // We do not need to check for self-assignment.
    m_data = rhs.m_data;
    m_sz = rhs.m_sz;
    return *this;
}
