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

#ifndef e_buffer_vector_h_
#define e_buffer_vector_h_

// e
#include <e/buffer.h>

namespace e
{

template <typename T>
e::buffer::packer
operator << (e::buffer::packer lhs, const std::vector<T>& rhs)
{
    uint32_t cols = rhs.size();
    e::buffer::packer ret = lhs << cols;

    for (uint32_t i = 0; i < cols; ++i)
    {
        ret = ret << rhs[i];
    }

    if (ret.overflow())
    {
        return e::buffer::packer(lhs, true);
    }

    return ret;
}

template <typename T>
e::buffer::unpacker
operator >> (e::buffer::unpacker lhs, std::vector<T>& rhs)
{
    // XXX This introduces the potential for DOS attacks
    uint32_t cols = 0;
    e::buffer::unpacker ret = lhs >> cols;
    rhs.resize(cols);

    for (uint32_t i = 0; i < cols; ++i)
    {
        ret = ret >> rhs[i];
    }

    if (ret.overflow())
    {
        return e::buffer::unpacker(lhs, true);
    }

    return ret;
}

} // namespace e

#endif // e_buffer_vector_h_
