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

#ifndef e_safe_math_h_
#define e_safe_math_h_

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

// C
#include <stdint.h>

namespace e
{

inline bool
safe_add(int64_t lhs, int64_t rhs, int64_t* result)
{
#ifdef _MSC_VER
    return false; /* not required for client */
#else
    uint64_t OF = 0;
    int64_t sum = lhs;
    __asm__ __volatile__("addq %2, %0\n\t"
                         "mov $0, %%rax\n\t"
                         "seto %%al\n\t"
                         "movq %%rax, %1\n\t"
                         : "+r" (sum), "=r" (OF)
                         : "r" (rhs)
                         : "rax");
    *result = sum;
    return OF == 0;
#endif
}

inline bool
safe_sub(int64_t lhs, int64_t rhs, int64_t* result)
{
#ifdef _MSC_VER
    return false;
#else
    uint64_t OF = 0;
    int64_t difference = lhs;
    __asm__ __volatile__("subq %2, %0\n\t"
                         "mov $0, %%rax\n\t"
                         "seto %%al\n\t"
                         "movq %%rax, %1\n\t"
                         : "+r" (difference), "=r" (OF)
                         : "r" (rhs)
                         : "rax");
    *result = difference;
    return OF == 0;
#endif
}

inline bool
safe_mul(int64_t lhs, int64_t rhs, int64_t* result)
{
#ifdef _MSC_VER
    return false;
#else
    uint64_t OF = 0;
    int64_t product = lhs;
    __asm__ __volatile__("imulq %2\n\t"
                         "mov $0, %%rdx\n\t"
                         "seto %%dl\n\t"
                         : "+a" (product), "=d" (OF)
                         : "r" (rhs)
                         : );
    *result = product;
    return OF == 0;
#endif
}

inline bool
safe_div(int64_t lhs, int64_t rhs, int64_t* result)
{
    if (rhs == 0)
    {
        return false;
    }

    if (rhs == -1 && (lhs == INT64_MIN))
    {
        return false;
    }

    int64_t div = lhs / rhs;
    int64_t mod = static_cast<int64_t>(lhs - static_cast<uint64_t>(div) * rhs);

    if (mod && ((rhs ^ mod) < 0))
    {
        --div;
        // mod += rhs;
    }

    *result = div;
    return true;
}

inline bool
safe_mod(int64_t lhs, int64_t rhs, int64_t* result)
{
    if (rhs == 0)
    {
        return false;
    }

    if (rhs == -1 && (lhs == INT64_MIN))
    {
        return false;
    }

    int64_t div = lhs / rhs;
    int64_t mod = static_cast<int64_t>(lhs - static_cast<uint64_t>(div) * rhs);

    if (mod && ((rhs ^ mod) < 0))
    {
        --div;
        mod += rhs;
    }

    *result = mod;
    return true;
}

} // namespace e

#endif // e_safe_math_h_
