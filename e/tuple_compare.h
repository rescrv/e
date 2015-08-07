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

#ifndef e_tuple_compare_h_
#define e_tuple_compare_h_

namespace e
{

template <typename T1>
int
tuple_compare(const T1& lhs1, const T1& rhs1)
{
    if (lhs1 < rhs1)
    {
        return -1;
    }
    else if (lhs1 > rhs1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

template <typename T1, typename T2>
int
tuple_compare(const T1& lhs1, const T2& lhs2,
              const T1& rhs1, const T2& rhs2)
{
    int prefix = tuple_compare(lhs1, rhs1);

    if (prefix == 0)
    {
        return tuple_compare(lhs2, rhs2);
    }
    else
    {
        return prefix;
    }
}

template <typename T1, typename T2, typename T3>
int
tuple_compare(const T1& lhs1, const T2& lhs2, const T3& lhs3,
              const T1& rhs1, const T2& rhs2, const T3& rhs3)
{
    int prefix = tuple_compare(lhs1, lhs2, rhs1, rhs2);

    if (prefix == 0)
    {
        return tuple_compare(lhs3, rhs3);
    }
    else
    {
        return prefix;
    }
}

template <typename T1, typename T2, typename T3, typename T4>
int
tuple_compare(const T1& lhs1, const T2& lhs2, const T3& lhs3, const T4& lhs4,
              const T1& rhs1, const T2& rhs2, const T3& rhs3, const T4& rhs4)
{
    int prefix = tuple_compare(lhs1, lhs2, lhs3, rhs1, rhs2, rhs3);

    if (prefix == 0)
    {
        return tuple_compare(lhs4, rhs4);
    }
    else
    {
        return prefix;
    }
}

template <typename T1, typename T2, typename T3, typename T4, typename T5>
int
tuple_compare(const T1& lhs1, const T2& lhs2, const T3& lhs3, const T4& lhs4, const T5& lhs5,
              const T1& rhs1, const T2& rhs2, const T3& rhs3, const T4& rhs4, const T5& rhs5)
{
    int prefix = tuple_compare(lhs1, lhs2, lhs3, lhs4, rhs1, rhs2, rhs3, rhs4);

    if (prefix == 0)
    {
        return tuple_compare(lhs5, rhs5);
    }
    else
    {
        return prefix;
    }
}

template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
int
tuple_compare(const T1& lhs1, const T2& lhs2, const T3& lhs3, const T4& lhs4, const T5& lhs5, const T6& lhs6,
              const T1& rhs1, const T2& rhs2, const T3& rhs3, const T4& rhs4, const T5& rhs5, const T6& rhs6)
{
    int prefix = tuple_compare(lhs1, lhs2, lhs3, lhs4, lhs5,
                               rhs1, rhs2, rhs3, rhs4, rhs5);

    if (prefix == 0)
    {
        return tuple_compare(lhs6, rhs6);
    }
    else
    {
        return prefix;
    }
}

} // namespace e

#endif // e_tuple_compare_h_
