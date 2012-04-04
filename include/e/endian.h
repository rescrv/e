// Copyright (c) 2012, Robert Escriva
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

// Pack values into the buffer
inline uint8_t*
pack8be(uint8_t number, uint8_t* buffer)
{
    buffer[0] = number;
    return buffer + sizeof(uint8_t);
}

inline uint8_t*
pack8le(uint8_t number, uint8_t* buffer)
{
    buffer[0] = number;
    return buffer + sizeof(uint8_t);
}

inline uint8_t*
pack16be(uint16_t number, uint8_t* buffer)
{
    buffer[0] = number >> 8;
    buffer[1] = number & 0xff;
    return buffer + sizeof(uint16_t);
}

inline uint8_t*
pack16le(uint16_t number, uint8_t* buffer)
{
    buffer[0] = number & 0xff;
    buffer[1] = number >> 8;
    return buffer + sizeof(uint16_t);
}

inline uint8_t*
pack32be(uint32_t number, uint8_t* buffer)
{
    buffer[0] = number >> 24;
    buffer[1] = (number >> 16) & 0xff;
    buffer[2] = (number >> 8) & 0xff;
    buffer[3] = number & 0xff;
    return buffer + sizeof(uint32_t);
}

inline uint8_t*
pack32le(uint32_t number, uint8_t* buffer)
{
    buffer[0] = number & 0xff;
    buffer[1] = (number >> 8) & 0xff;
    buffer[2] = (number >> 16) & 0xff;
    buffer[3] = number >> 24;
    return buffer + sizeof(uint32_t);
}

inline uint8_t*
pack64be(uint64_t number, uint8_t* buffer)
{
    buffer[0] = number >> 56;
    buffer[1] = (number >> 48) & 0xff;
    buffer[2] = (number >> 40) & 0xff;
    buffer[3] = (number >> 32) & 0xff;
    buffer[4] = (number >> 24) & 0xff;
    buffer[5] = (number >> 16) & 0xff;
    buffer[6] = (number >> 8) & 0xff;
    buffer[7] = number & 0xff;
    return buffer + sizeof(uint64_t);
}

inline uint8_t*
pack64le(uint64_t number, uint8_t* buffer)
{
    buffer[0] = number & 0xff;
    buffer[1] = (number >> 8) & 0xff;
    buffer[2] = (number >> 16) & 0xff;
    buffer[3] = (number >> 24) & 0xff;
    buffer[4] = (number >> 32) & 0xff;
    buffer[5] = (number >> 40) & 0xff;
    buffer[6] = (number >> 48) & 0xff;
    buffer[7] = number >> 56;
    return buffer + sizeof(uint64_t);
}

// Unpack values from the buffer
inline const uint8_t*
unpack8be(const uint8_t* buffer, uint8_t* number)
{
    *number = buffer[0];
    return buffer + sizeof(uint8_t);
}

inline const uint8_t*
unpack8le(const uint8_t* buffer, uint8_t* number)
{
    *number = buffer[0];
    return buffer + sizeof(uint8_t);
}

inline const uint8_t*
unpack16be(const uint8_t* buffer, uint16_t* number)
{
    *number = static_cast<uint16_t>(buffer[0]) << 8
            | static_cast<uint16_t>(buffer[1]);
    return buffer + sizeof(uint16_t);
}

inline const uint8_t*
unpack16le(const uint8_t* buffer, uint16_t* number)
{
    *number = static_cast<uint16_t>(buffer[0])
            | static_cast<uint16_t>(buffer[1]) << 8;
    return buffer + sizeof(uint16_t);
}

inline const uint8_t*
unpack32be(const uint8_t* buffer, uint32_t* number)
{
    *number = static_cast<uint32_t>(buffer[0]) << 24
            | static_cast<uint32_t>(buffer[1]) << 16
            | static_cast<uint32_t>(buffer[2]) << 8
            | static_cast<uint32_t>(buffer[3]);
    return buffer + sizeof(uint32_t);
}

inline const uint8_t*
unpack32le(const uint8_t* buffer, uint32_t* number)
{
    *number = static_cast<uint32_t>(buffer[0])
            | static_cast<uint32_t>(buffer[1]) << 8
            | static_cast<uint32_t>(buffer[2]) << 16
            | static_cast<uint32_t>(buffer[3]) << 24;
    return buffer + sizeof(uint32_t);
}

inline const uint8_t*
unpack64be(const uint8_t* buffer, uint64_t* number)
{
    *number = static_cast<uint64_t>(buffer[0]) << 56
            | static_cast<uint64_t>(buffer[1]) << 48
            | static_cast<uint64_t>(buffer[2]) << 40
            | static_cast<uint64_t>(buffer[3]) << 32
            | static_cast<uint64_t>(buffer[4]) << 24
            | static_cast<uint64_t>(buffer[5]) << 16
            | static_cast<uint64_t>(buffer[6]) << 8
            | static_cast<uint64_t>(buffer[7]);
    return buffer + sizeof(uint64_t);
}

inline const uint8_t*
unpack64le(const uint8_t* buffer, uint64_t* number)
{
    *number = static_cast<uint64_t>(buffer[0])
            | static_cast<uint64_t>(buffer[1]) << 8
            | static_cast<uint64_t>(buffer[2]) << 16
            | static_cast<uint64_t>(buffer[3]) << 24
            | static_cast<uint64_t>(buffer[4]) << 32
            | static_cast<uint64_t>(buffer[5]) << 40
            | static_cast<uint64_t>(buffer[6]) << 48
            | static_cast<uint64_t>(buffer[7]) << 56;
    return buffer + sizeof(uint64_t);
}

} // namespace e

#endif // e_endian_h_
