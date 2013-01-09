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

#ifndef e_slice_h_
#define e_slice_h_

// C
#include <cassert>
#include <cstring>
#include <stdint.h>

// STL
#include <string>
#include <vector>

namespace e
{

// Refer to another piece of memory.  The memory is assumed to be managed
// somewhere else, and must outlive the use of all slices using it.
class slice
{
    public:
        slice();
        slice(const uint8_t* data, size_t sz);
        slice(const std::string& str);
        slice(const std::vector<uint8_t>& buf);
        template <typename T> slice(const T* data, size_t sz);
        slice(const slice& other);
        ~slice() throw ();

    public:
        int compare(const slice& rhs) const;
        const uint8_t* data() const { return m_data; }
        bool empty() const { return m_sz == 0; }
        std::string hex() const;
        size_t size() const { return m_sz; }

    public:
        void advance(size_t sz);
        void reset();
        void reset(const uint8_t* data, size_t sz);

    public:
        slice& operator = (const slice& rhs);
        bool operator < (const slice& rhs) const { return compare(rhs) < 0; }
        bool operator <= (const slice& rhs) const { return compare(rhs) <= 0; }
        bool operator == (const slice& rhs) const { return compare(rhs) == 0; }
        bool operator != (const slice& rhs) const { return compare(rhs) != 0; }
        bool operator >= (const slice& rhs) const { return compare(rhs) >= 0; }
        bool operator > (const slice& rhs) const { return compare(rhs) > 0; }

    private:
        const uint8_t* m_data;
        size_t m_sz;
};

inline
slice :: slice()
    : m_data(NULL)
    , m_sz(0)
{
}

inline
slice :: slice(const uint8_t* d, size_t sz)
    : m_data(d)
    , m_sz(sz)
{
}

inline
slice :: slice(const std::string& str)
    : m_data(reinterpret_cast<const uint8_t*>(str.data()))
    , m_sz(str.size())
{
}

inline
slice :: slice(const std::vector<uint8_t>& buf)
    : m_data(&buf.front())
    , m_sz(buf.size())
{
}

// sz is the number of bytes, not the number of pointed-to elements.
template <typename T>
inline
slice :: slice(const T* d, size_t sz)
    : m_data(reinterpret_cast<const uint8_t*>(d))
    , m_sz(sz)
{
}

inline
slice :: slice(const slice& other)
    : m_data(other.m_data)
    , m_sz(other.m_sz)
{
}

inline
slice :: ~slice() throw ()
{
}

inline int
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

inline void
slice :: advance(size_t sz)
{
    assert(sz <= m_sz);
    m_data += sz;
    m_sz -= sz;
}

inline void
slice :: reset()
{
    m_data = NULL;
    m_sz = 0;
}

inline void
slice :: reset(const uint8_t* d, size_t sz)
{
    m_data = d;
    m_sz = sz;
}

inline slice&
slice :: operator = (const slice& rhs)
{
    // We do not need to check for self-assignment.
    m_data = rhs.m_data;
    m_sz = rhs.m_sz;
    return *this;
}

inline bool
operator < (const slice& lhs, const std::string& rhs)
{
    return lhs < e::slice(rhs.data(), rhs.size());
}

inline bool
operator <= (const slice& lhs, const std::string& rhs)
{
    return lhs <= e::slice(rhs.data(), rhs.size());
}

inline bool
operator == (const slice& lhs, const std::string& rhs)
{
    return lhs == e::slice(rhs.data(), rhs.size());
}

inline bool
operator != (const slice& lhs, const std::string& rhs)
{
    return lhs != e::slice(rhs.data(), rhs.size());
}

inline bool
operator >= (const slice& lhs, const std::string& rhs)
{
    return lhs >= e::slice(rhs.data(), rhs.size());
}

inline bool
operator > (const slice& lhs, const std::string& rhs)
{
    return lhs < e::slice(rhs.data(), rhs.size());
}

inline bool
operator < (const std::string& lhs, const slice& rhs)
{
    return e::slice(lhs.data(), lhs.size()) < rhs;
}

inline bool
operator <= (const std::string& lhs, const slice& rhs)
{
    return e::slice(lhs.data(), lhs.size()) <= rhs;
}

inline bool
operator == (const std::string& lhs, const slice& rhs)
{
    return e::slice(lhs.data(), lhs.size()) == rhs;
}

inline bool
operator != (const std::string& lhs, const slice& rhs)
{
    return e::slice(lhs.data(), lhs.size()) != rhs;
}

inline bool
operator >= (const std::string& lhs, const slice& rhs)
{
    return e::slice(lhs.data(), lhs.size()) >= rhs;
}

inline bool
operator > (const std::string& lhs, const slice& rhs)
{
    return e::slice(lhs.data(), lhs.size()) > rhs;
}

} // namespace e

#endif // e_slice_h_
