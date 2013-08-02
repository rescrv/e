// Copyright (c) 2013, Robert Escriva
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

#ifndef e_strescape_h_
#define e_strescape_h_

// C
#include <cstdio>

// STL
#include <string>
#include <vector>

namespace e
{

std::string
strescape(const std::string& input)
{
    const char* data = input.c_str();
    size_t data_sz = input.size();
    std::vector<char> tmp(data_sz * 4 + 1);
    char* ptr = &tmp.front();

    for (size_t i = 0; i < data_sz; ++i)
    {
        if (isalnum(data[i]) ||
            (ispunct(data[i]) && data[i] != '\'') ||
            data[i] == ' ')
        {
            *ptr = data[i];
            ++ptr;
        }
        else if (data[i] == '\n')
        {
            *ptr = '\\';
            ++ptr;
            *ptr = 'n';
            ++ptr;
        }
        else if (data[i] == '\r')
        {
            *ptr = '\\';
            ++ptr;
            *ptr = 'r';
            ++ptr;
        }
        else if (data[i] == '\t')
        {
            *ptr = '\\';
            ++ptr;
            *ptr = 't';
            ++ptr;
        }
        else if (data[i] == '\'')
        {
            *ptr = '\\';
            ++ptr;
            *ptr = '\'';
            ++ptr;
        }
        else
        {
            *ptr = '\\';
            ++ptr;
            *ptr = 'x';
            ++ptr;
            sprintf(ptr, "%02x", data[i] & 0xff);
            ptr += 2;
        }
    }

    *ptr = '\0';
    return std::string(&tmp.front(), ptr);
}

} // namespace e

#endif // e_strescape_h_
