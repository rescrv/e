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

// C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// POSIX
#include <sys/stat.h>

// STL
#include <iostream>
#include <sstream>
#include <vector>

// po6
#include <po6/io/fd.h>

// e
#include "e/identity.h"

static bool
atomic_read(const char* path, std::string* contents)
{
    contents->clear();
    po6::io::fd fd(open(path, O_RDONLY));

    if (fd.get() < 0)
    {
        return false;
    }

    char buf[512];
    ssize_t amt;

    while ((amt = fd.xread(buf, 512)) > 0)
    {
        contents->append(buf, amt);
    }

    return amt == 0;
}

static bool
atomic_write(const char* path, const std::string& contents)
{
    std::string tmp(path);
    tmp += ".tmp";
    po6::io::fd fd(open(tmp.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR));
    return fd.get() >= 0 &&
           fd.xwrite(contents.data(), contents.size()) == ssize_t(contents.size()) &&
           fsync(fd.get()) >= 0 &&
           rename(tmp.c_str(), path) >= 0;
}

bool
e :: load_identity(const char* path, bool* saved, uint64_t* id,
                   bool set_bind_to, po6::net::location* bind_to,
                   bool set_rendezvous, std::string* rendezvous)
{
    std::string ident;

    if (!atomic_read(path, &ident))
    {
        if (errno != ENOENT)
        {
            return false;
        }

        *saved = false;
        return true;
    }

    *saved = true;
    std::vector<char> buf(ident.c_str(), ident.c_str() + ident.size() + 1);
    const char* ptr = &buf[0];
    const char* const end = ptr + buf.size();
    const char* eol = NULL;
    char* tmp = NULL;

    if (strncmp(ptr, "id=", 3) != 0)
    {
        return false;
    }

    ptr += 3;
    errno = 0;
    *id = strtoull(ptr, &tmp, 10);

    if (errno != 0 || *tmp != '\n')
    {
        return false;
    }

    *tmp = '\0';
    ptr = tmp + 1;

    if (strncmp(ptr, "bind_to=", 8) != 0)
    {
        return false;
    }

    ptr += 8;
    eol = strchr(ptr, '\n');

    if (!eol)
    {
        return false;
    }

    tmp = &buf[0] + (eol - &buf[0]);
    *tmp = '\0';
    const char* colon = strrchr(ptr, ':');

    if (!colon)
    {
        return false;
    }

    errno = 0;
    unsigned long port = strtoul(colon + 1, &tmp, 10);

    if (errno != 0 || *tmp != '\0')
    {
        return false;
    }

    std::string host;

    if (*ptr == '[' && colon > ptr && *(colon - 1) == ']')
    {
        host.assign(ptr + 1, colon - 1);
    }
    else
    {
        host.assign(ptr, colon);
    }

    if (!set_bind_to)
    {
        if (!bind_to->set(host.c_str(), port))
        {
            return false;
        }
    }

    ptr = eol + 1;

    if (ptr < end && !set_rendezvous)
    {
        eol = strchr(ptr, '\n');
        eol = eol ? eol : end;
        *rendezvous = std::string(ptr, eol);
    }

    return true;
}

bool
e :: save_identity(const char* path, uint64_t id,
                   const po6::net::location& bind_to,
                   const std::string& rendezvous)
{
    std::ostringstream ostr;
    ostr << "id=" << id << "\n"
         << "bind_to=" << bind_to << "\n"
         << rendezvous << "\n";
    return atomic_write(path, ostr.str());
}
