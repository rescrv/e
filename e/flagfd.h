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

#ifndef e_flagfd_h_
#define e_flagfd_h_

// POSIX
#include <limits.h>
#include <unistd.h>

// po6
#include <po6/io/fd.h>

namespace e
{

class flagfd
{
    public:
        flagfd()
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
        ~flagfd() throw () {}

    public:
        bool valid() const { return m_read.get() >= 0; }
        int error() const { return m_error; }
        int poll_fd() { return m_read.get(); }
        bool isset() { return m_flagged; }
        void set()
        {
            if (!m_flagged)
            {
                char c;
                m_write.xwrite(&c, 1);
            }

            m_flagged = true;
        }
        void clear()
        {
            if (m_flagged)
            {
                char buf[32];

                while (m_read.read(buf, 32) == 32)
                    ;
            }

            m_flagged = false;
        }

    private:
        po6::io::fd m_read;
        po6::io::fd m_write;
        bool m_flagged;
        int m_error;

    private:
        flagfd(const flagfd&);
        flagfd& operator = (const flagfd&);
};

} // namespace e

#endif // e_flagfd_h_
