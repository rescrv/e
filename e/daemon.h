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

#ifndef e_daemon_h_
#define e_daemon_h_

// C
#include <assert.h>
#include <stdio.h>

// POSIX
#include <signal.h>
#include <sys/stat.h>

// STL
#include <string>

// po6
#include <po6/io/fd.h>

namespace e
{

inline bool
create_pidfile(const char* path)
{
    char buf[21];
    ssize_t buf_sz = sprintf(buf, "%d\n", getpid());
    assert(buf_sz < static_cast<ssize_t>(sizeof(buf)));
    po6::io::fd pid(open(path, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR));

    if (pid.get() < 0 || pid.xwrite(buf, buf_sz) != buf_sz)
    {
        PLOG(ERROR) << "could not create pidfile " << path;
        return false;
    }
}

inline bool
create_pidfile(const std::string& path)
{
    return create_pidfile(path.c_str());
}

inline bool
install_signal_handler(int signum, void (*f)(int))
{
    struct sigaction handle;
    handle.sa_handler = f;
    sigfillset(&handle.sa_mask);
    handle.sa_flags = SA_RESTART;
    return sigaction(signum, &handle, NULL) >= 0;
}

inline bool
block_all_signals()
{
    sigset_t ss;
    return sigfillset(&ss) >= 0 &&
           pthread_sigmask(SIG_SETMASK, &ss, NULL) >= 0;
}

inline bool
generate_token(uint64_t* token)
{
    po6::io::fd sysrand(open("/dev/urandom", O_RDONLY));

    if (sysrand.get() < 0)
    {
        return false;
    }

    if (sysrand.read(token, sizeof(*token)) != sizeof(*token))
    {
        return false;
    }

    return true;
}

} // namespace e

#endif // e_daemon_h_
