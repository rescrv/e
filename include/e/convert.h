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

#ifndef e_convert_h_
#define e_convert_h_

// C
#include <stdint.h>
#include <stdlib.h>

// STL
#include <stdexcept>
#include <string>

// POSIX
#include <errno.h>

namespace e
{
namespace convert
{

// These have the same gotchas inherent to stro*.  I plan to someday extend them
// to safely convert integers, but until then they just ease usage of strto*
// without fixing the problem.

inline uint64_t
to_uint64_t(const std::string& s, int base = 0)
{
    int olderrno = errno;
    int newerrno;
    char* endptr;
    unsigned long long int ret;

    errno = 0;
    ret = strtoull(s.c_str(), &endptr, base);
    newerrno = errno;
    errno = olderrno;

    if (*endptr != '\0' || newerrno == EINVAL)
    {
        throw std::domain_error("The number is not valid for the given base.");
    }
    if (newerrno == ERANGE || (ret & 0xffffffffffffffff) != ret)
    {
        throw std::out_of_range("The number does not fit in a uint64_t");
    }

    return static_cast<uint64_t>(ret);
}

inline uint32_t
to_uint32_t(const std::string& s, int base = 0)
{
    int olderrno = errno;
    int newerrno;
    char* endptr;
    unsigned long int ret;

    errno = 0;
    ret = strtoul(s.c_str(), &endptr, base);
    newerrno = errno;
    errno = olderrno;

    if (*endptr != '\0' || newerrno == EINVAL)
    {
        throw std::domain_error("The number is not valid for the given base.");
    }
    if (newerrno == ERANGE || (ret & 0xffffffff) != ret)
    {
        throw std::out_of_range("The number does not fit in a uint32_t");
    }

    return static_cast<uint32_t>(ret);
}

inline uint16_t
to_uint16_t(const std::string& s, int base = 0)
{
    int olderrno = errno;
    int newerrno;
    char* endptr;
    unsigned long int ret;

    errno = 0;
    ret = strtoul(s.c_str(), &endptr, base);
    newerrno = errno;
    errno = olderrno;

    if (*endptr != '\0' || newerrno == EINVAL)
    {
        throw std::domain_error("The number is not valid for the given base.");
    }
    if (newerrno == ERANGE || (ret & 0xffff) != ret)
    {
        throw std::out_of_range("The number does not fit in a uint16_t");
    }

    return static_cast<uint16_t>(ret);
}

inline uint8_t
to_uint8_t(const std::string& s, int base = 0)
{
    int olderrno = errno;
    int newerrno;
    char* endptr;
    unsigned long int ret;

    errno = 0;
    ret = strtoul(s.c_str(), &endptr, base);
    newerrno = errno;
    errno = olderrno;

    if (*endptr != '\0' || newerrno == EINVAL)
    {
        throw std::domain_error("The number is not valid for the given base.");
    }
    if (newerrno == ERANGE || (ret & 0xff) != ret)
    {
        throw std::out_of_range("The number does not fit in a uint8_t");
    }

    return static_cast<uint8_t>(ret);
}

} // namespace convert
} // namespace e

#endif // e_convert_h_
