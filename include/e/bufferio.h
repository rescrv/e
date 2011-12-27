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

#ifndef e_bufferio_h_
#define e_bufferio_h_

// po6
#include <po6/io/fd.h>
#include <po6/net/socket.h>

// e
#include <e/buffer.h>

namespace e
{
namespace bufferio
{

ssize_t
read(po6::io::fd* fd, buffer* buf, size_t len);

ssize_t
xread(po6::io::fd* fd, buffer* buf, size_t len);

ssize_t
recv(po6::net::socket* fd, buffer* buf, size_t len, int flags);

ssize_t
xrecv(po6::net::socket* fd, buffer* buf, size_t len, int flags);

ssize_t
read(po6::io::fd* fd, buffer* buf, size_t len)
{
    len = std::min(len, static_cast<size_t>(buf->capacity() - buf->size()));
    ssize_t ret = fd->read(buf->data() + buf->size(), len);

    if (ret > 0)
    {
        buf->resize(buf->size() + ret);
    }

    return ret;
}

ssize_t
xread(po6::io::fd* fd, buffer* buf, size_t len)
{
    len = std::min(len, static_cast<size_t>(buf->capacity() - buf->size()));
    ssize_t ret = fd->xread(buf->data() + buf->size(), len);

    if (ret > 0)
    {
        buf->resize(buf->size() + ret);
    }

    return ret;
}

ssize_t
recv(po6::net::socket* fd, buffer* buf, size_t len, int flags)
{
    len = std::min(len, static_cast<size_t>(buf->capacity() - buf->size()));
    ssize_t ret = fd->recv(buf->data() + buf->size(), len, flags);

    if (ret > 0)
    {
        buf->resize(buf->size() + ret);
    }

    return ret;
}

ssize_t
xrecv(po6::net::socket* fd, buffer* buf, size_t len, int flags)
{
    len = std::min(len, static_cast<size_t>(buf->capacity() - buf->size()));
    ssize_t ret = fd->xrecv(buf->data() + buf->size(), len, flags);

    if (ret > 0)
    {
        buf->resize(buf->size() + ret);
    }

    return ret;
}

} // namespace bufferio
} // namespace e

#endif // e_bufferio_h_
