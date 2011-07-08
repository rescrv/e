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

#ifndef e_bit_stealing_h_
#define e_bit_stealing_h_

#ifndef __x86_64__
#error Bit stealing requires x86 64.
#endif

// C
#include <cassert>
#include <stdint.h>

// On current 64-bit x86 chips, only the 48 lower-order bits are used for
// addressing, yet a pointer has 64 bits.  Those extra 16 bits sound like a good
// opportunity for extra storage which may be CASed atomically with the
// pointer.  Intel may not provide DCAS, but this sounds like the next best
// thing.

namespace e
{
namespace bit_stealing
{

template <typename T>
T*
strip(T* t)
{
    assert(sizeof(T*) == sizeof(uint64_t));
    uint64_t ret = reinterpret_cast<uint64_t>(t);
    return reinterpret_cast<T*>(0x0000ffffffffffff & ret);
}

// t must be properly aligned for 64-bit types.  This implies that the returned
// reference will be properly aligned for 16-bit types.
template <typename T>
uint16_t&
get(T*& t)
{
    assert(sizeof(T*) == sizeof(uint64_t));
    uint16_t* ret = reinterpret_cast<uint16_t*>(&t);
    return ret[3];
}

template <typename T>
void
set(T*& t, size_t i)
{
    assert(sizeof(T*) == sizeof(uint64_t));
    assert(i < 16);
    get(t) |= (1 << i);
}

template <typename T>
void
unset(T*& t, size_t i)
{
    assert(sizeof(T*) == sizeof(uint64_t));
    assert(i < 16);
    get(t) &= ~(1 << i);
}

template <typename T>
bool
get(T*& t, size_t i)
{
    assert(sizeof(T*) == sizeof(uint64_t));
    assert(i < 16);
    return (get(t) & (1 << i)) > 0;
}

} // namespace bit_stealing
} // namespace e

#endif // e_bit_stealing_h_
