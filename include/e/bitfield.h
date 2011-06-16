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

#ifndef e_bitfield_h_
#define e_bitfield_h_

// C
#include <stdint.h>

// STL
#include <vector>

namespace e
{

class bitfield
{
    public:
        bitfield(size_t n);
        bitfield(const bitfield& other);
        ~bitfield() throw ();

    public:
        size_t bits() const throw () { return m_num_bits; }
        size_t bytes() const throw () { return (m_num_bits / 8) + (m_num_bits % 8 ? 1 : 0); }
        void set(size_t n);
        void unset(size_t n);
        bool get(size_t n) const;

    public:
        bitfield& operator = (const bitfield& other);

    private:
        size_t m_num_bits;
        std::vector<uint8_t> m_bits;
};

inline
bitfield :: bitfield(size_t n)
    : m_num_bits(n)
    , m_bits(bytes())
{
}

inline
bitfield :: bitfield(const bitfield& other)
    : m_num_bits(other.m_num_bits)
    , m_bits(other.m_bits)
{
}

inline
bitfield :: ~bitfield() throw ()
{
}

inline void
bitfield :: set(size_t n)
{
    assert(n < m_num_bits);
    m_bits[n / 8] |= (1 << (n % 8));
}

inline void
bitfield :: unset(size_t n)
{
    assert(n < m_num_bits);
    m_bits[n / 8] &= ~(1 << (n % 8));
}

inline bool
bitfield :: get(size_t n) const
{
    assert(n < m_num_bits);
    return m_bits[n / 8] & (1 << (n % 8));
}

} // namespace e

#endif // e_bitfield_h_
