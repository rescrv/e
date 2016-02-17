// Copyright (c) 2015, Robert Escriva
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

// POSIX
#include <limits.h>
#include <unistd.h>

// e
#include "e/flagfd.h"

using e::flagfd;

flagfd :: flagfd()
    : m_read()
    , m_write()
    , m_flagged(false)
    , m_error(0)
{
    int fds[2];

    if (pipe(fds) < 0)
    {
        m_error = errno;
        return;
    }

    m_read = fds[0];
    m_write = fds[1];
}

flagfd :: ~flagfd() throw ()
{
}

bool
flagfd :: valid() const
{
    return m_read.get() >= 0;
}

int
flagfd :: error() const
{
    return m_error;
}

int
flagfd :: poll_fd()
{
    return m_read.get();
}

bool
flagfd :: isset()
{
    return m_flagged;
}

void
flagfd :: set()
{
    if (!m_flagged)
    {
        char c = 'A';
        PO6_EXPLICITLY_IGNORE(m_write.xwrite(&c, 1));
    }

    m_flagged = true;
}

void
flagfd :: clear()
{
    if (m_flagged)
    {
        char buf[32];

        while (m_read.read(buf, 32) == 32)
            ;
    }

    m_flagged = false;
}
