// Copyright (c) 2012,2015, Robert Escriva
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

#ifndef e_endian_h_
#define e_endian_h_

// C
#include <stdint.h>

namespace e
{

uint8_t* pack8be(uint8_t number, uint8_t* buffer);
uint8_t* pack8le(uint8_t number, uint8_t* buffer);
uint8_t* pack16be(uint16_t number, uint8_t* buffer);
uint8_t* pack16le(uint16_t number, uint8_t* buffer);
uint8_t* pack32be(uint32_t number, uint8_t* buffer);
uint8_t* pack32le(uint32_t number, uint8_t* buffer);
uint8_t* pack64be(uint64_t number, uint8_t* buffer);
uint8_t* pack64le(uint64_t number, uint8_t* buffer);
uint8_t* packfloatbe(float number, uint8_t* buffer);
uint8_t* packfloatle(float number, uint8_t* buffer);
uint8_t* packdoublebe(double number, uint8_t* buffer);
uint8_t* packdoublele(double number, uint8_t* buffer);

const uint8_t* unpack8be(const uint8_t* buffer, uint8_t* number);
const uint8_t* unpack8le(const uint8_t* buffer, uint8_t* number);
const uint8_t* unpack16be(const uint8_t* buffer, uint16_t* number);
const uint8_t* unpack16le(const uint8_t* buffer, uint16_t* number);
const uint8_t* unpack32be(const uint8_t* buffer, uint32_t* number);
const uint8_t* unpack32le(const uint8_t* buffer, uint32_t* number);
const uint8_t* unpack64be(const uint8_t* buffer, uint64_t* number);
const uint8_t* unpack64le(const uint8_t* buffer, uint64_t* number);
const uint8_t* unpackfloatbe(const uint8_t* buffer, float* number);
const uint8_t* unpackfloatle(const uint8_t* buffer, float* number);
const uint8_t* unpackdoublebe(const uint8_t* buffer, double* number);
const uint8_t* unpackdoublele(const uint8_t* buffer, double* number);

#define SIGNED_WRAPPER(SZ, END) \
    inline const uint8_t* \
    unpack ## SZ ## END(const uint8_t* buffer, int ## SZ ## _t* number) \
    { \
        uint ## SZ ## _t tmp; \
        const uint8_t* ret = unpack ## SZ ## END(buffer, &tmp); \
        *number = static_cast<int ## SZ ## _t>(tmp); \
        return ret; \
    }

SIGNED_WRAPPER(8, be)
SIGNED_WRAPPER(8, le)
SIGNED_WRAPPER(16, be)
SIGNED_WRAPPER(16, le)
SIGNED_WRAPPER(32, be)
SIGNED_WRAPPER(32, le)
SIGNED_WRAPPER(64, be)
SIGNED_WRAPPER(64, le)

#undef SIGNED_WRAPPER

#define PACKCHAR_INT_WRAPPER(SZ, END) \
    inline const char* \
    unpack ## SZ ## END(const char* buffer, uint ## SZ ## _t* number) \
    { \
        return reinterpret_cast<const char*>(unpack ## SZ ## END(reinterpret_cast<const uint8_t*>(buffer), number)); \
    } \
    inline const char* \
    unpack ## SZ ## END(const char* buffer, int ## SZ ## _t* number) \
    { \
        return reinterpret_cast<const char*>(unpack ## SZ ## END(reinterpret_cast<const uint8_t*>(buffer), number)); \
    } \
    inline char* \
    pack ## SZ ## END(int ## SZ ## _t number, char* buffer) \
    { \
        return reinterpret_cast<char*>(pack ## SZ ## END(number, reinterpret_cast<uint8_t*>(buffer))); \
    }

PACKCHAR_INT_WRAPPER(8, be)
PACKCHAR_INT_WRAPPER(8, le)
PACKCHAR_INT_WRAPPER(16, be)
PACKCHAR_INT_WRAPPER(16, le)
PACKCHAR_INT_WRAPPER(32, be)
PACKCHAR_INT_WRAPPER(32, le)
PACKCHAR_INT_WRAPPER(64, be)
PACKCHAR_INT_WRAPPER(64, le)

#undef PACKCHAR_INT_WRAPPER

#define PACKCHAR_FLOAT_WRAPPER(TYPE, END) \
    inline const char* \
    unpack ## TYPE ## END(const char* buffer, TYPE* number) \
    { \
        return reinterpret_cast<const char*>(unpack ## TYPE ## END(reinterpret_cast<const uint8_t*>(buffer), number)); \
    } \
    inline char* \
    pack ## TYPE ## END(TYPE number, char* buffer) \
    { \
        return reinterpret_cast<char*>(pack ## TYPE ## END(number, reinterpret_cast<uint8_t*>(buffer))); \
    }

PACKCHAR_FLOAT_WRAPPER(float, be)
PACKCHAR_FLOAT_WRAPPER(float, le)
PACKCHAR_FLOAT_WRAPPER(double, be)
PACKCHAR_FLOAT_WRAPPER(double, le)

#undef PACKCHAR_FLOAT_WRAPPER

} // namespace e

#endif // e_endian_h_
