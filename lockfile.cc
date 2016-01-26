// Copyright (c) 2016, Robert Escriva
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

// C
#include <assert.h>
#include <string.h>

// POSIX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// e
#include "e/lockfile.h"
#include "file_lock_table.h"

using e::lockfile;

lockfile :: lockfile()
    : m_fd()
    , m_dev(0)
    , m_ino(0)
{
}

lockfile :: ~lockfile() throw ()
{
    if (m_fd.get())
    {
        file_lock_table* flt = file_lock_table::the_one_and_only();
        assert(flt);
        flt->release(m_dev, m_ino);
    }
}

bool
lockfile :: lock(const char* name)
{
    int fd = open(name, O_RDWR|O_CREAT, S_IRWXU);

    if (fd < 0)
    {
        return false;
    }

    if (!lock(fd))
    {
        close(fd);
        return false;
    }

    return true;
}

bool
lockfile :: lock(int fd)
{
    assert(fd >= 0);
    assert(m_fd.get() < 0);
    m_fd = fd;
    struct stat stbuf;

    if (fstat(m_fd.get(), &stbuf) < 0)
    {
        m_fd.close();
        return false;
    }

    m_dev = stbuf.st_dev;
    m_ino = stbuf.st_ino;
    file_lock_table* flt = file_lock_table::the_one_and_only();
    assert(flt);

    struct flock f;
    memset(&f, 0, sizeof(f));
    f.l_type = F_WRLCK;
    f.l_whence = SEEK_SET;
    f.l_start = 0;
    f.l_len = 0;

    if (fcntl(m_fd.get(), F_SETLK, &f) < 0 ||
        !flt->acquire(m_dev, m_ino))
    {
        m_fd.close();
        return false;
    }

    return true;
}

#if 0


    m_dir_fd = dir_fd;
    m_lock_fd = lock_fd;
    m_lock_dev = stbuf.st_dev;
    m_lock_ino = stbuf.st_ino;
    g_flt.dismiss();
    g_lock_fd.dismiss();
    g_dir_fd.dismiss();
    return true;
#endif
