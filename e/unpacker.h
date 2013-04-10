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

#ifndef e_unpacker_h_
#define e_unpacker_h_

// C
#include <cstdlib>
#include <stdint.h>

// e
#include <e/slice.h>

namespace e
{

class unpacker
{
    public:
        unpacker();
        unpacker(const char* ptr, size_t ptr_sz);
        unpacker(const uint8_t* ptr, size_t ptr_sz);
        unpacker(const unpacker& up);

    public:
        unpacker advance(size_t by) const;
        unpacker as_error() const;
        slice as_slice() const;
        bool error() const { return m_error; }
        size_t remain() const { return m_ptr_sz; }
        size_t empty() const { return m_ptr_sz == 0; }

    public:
        unpacker operator >> (int8_t& rhs);
        unpacker operator >> (int16_t& rhs);
        unpacker operator >> (int32_t& rhs);
        unpacker operator >> (int64_t& rhs);
        unpacker operator >> (uint8_t& rhs);
        unpacker operator >> (uint16_t& rhs);
        unpacker operator >> (uint32_t& rhs);
        unpacker operator >> (uint64_t& rhs);
        unpacker operator >> (double& rhs);
        unpacker operator >> (slice& rhs);
        template <typename T> unpacker operator >> (std::vector<T>& rhs);
        template <typename A, typename B> unpacker operator >> (std::pair<A, B>& rhs);

    private:
        const uint8_t* m_ptr;
        size_t m_ptr_sz;
        bool m_error;
};

template <typename T>
inline e::unpacker
e :: unpacker :: operator >> (std::vector<T>& rhs)
{
    uint32_t sz;
    e::unpacker ret = *this >> sz;
    rhs.clear();

    for (uint32_t i = 0; !ret.error() && i < sz; ++i)
    {
        rhs.push_back(T());
        ret = ret >> rhs.back();
    }

    return ret;
}

template <typename A, typename B>
inline e::unpacker
e :: unpacker :: operator >> (std::pair<A, B>& rhs)
{
    return *this >> rhs.first >> rhs.second;
}

} // namespace e

#endif // e_unpacker_h_
