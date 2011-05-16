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

#ifndef e_buffer_h_
#define e_buffer_h_

// C
#include <stdint.h>
#include <string.h>

// POSIX
#include <arpa/inet.h>
#include <endian.h>

// C++
#include <exception>

// STL
#include <algorithm>
#include <vector>

namespace e
{

// Easily pack/unpack values for transfer over the network.  Integers are
// converted using the host to network and network to host conversion functions.
//
// Many unpackers can work concurrently on the same buffer.
// Only a single packer may operate on a buffer at time.

class buffer
{
    public:
        class exception : public std::exception
        {
        };

        class padding
        {
            public:
                padding(size_t sz)
                    : m_sz(sz)
                {
                }

            public:
                size_t size() const
                {
                    return m_sz;
                }

            private:
                size_t m_sz;
        };

        class packer
        {
            public:
                packer(buffer* buf)
                    : m_buf(buf)
                {
                }

            public:
                packer& operator << (const uint64_t& rhs)
                {
                    size_t size = m_buf->m_buf.size();
                    m_buf->m_buf.resize(size + sizeof(uint64_t));
                    uint64_t a = htobe64(rhs);
                    memmove(m_buf->mget() + size, &a, sizeof(uint64_t));
                    return *this;
                }

                packer& operator << (const uint32_t& rhs)
                {
                    size_t size = m_buf->m_buf.size();
                    m_buf->m_buf.resize(size + sizeof(uint32_t));
                    uint32_t a = htonl(rhs);
                    memmove(m_buf->mget() + size, &a, sizeof(uint32_t));
                    return *this;
                }

                packer& operator << (const uint16_t& rhs)
                {
                    size_t size = m_buf->m_buf.size();
                    m_buf->m_buf.resize(size + sizeof(uint16_t));
                    uint16_t a = htons(rhs);
                    memmove(m_buf->mget() + size, &a, sizeof(uint16_t));
                    return *this;
                }

                packer& operator << (const uint8_t& rhs)
                {
                    m_buf->m_buf.push_back(rhs);
                    return *this;
                }

                packer& operator << (const padding& p)
                {
                    m_buf->m_buf.resize(m_buf->m_buf.size() + p.size());
                    return *this;
                }

                packer& operator << (const buffer& rhs)
                {
                    *m_buf += rhs;
                    return *this;
                }

            private:
                packer(const packer&);

            private:
                packer& operator = (const packer&);

            private:
                buffer* m_buf;
        };

        class unpacker
        {
            public:
                unpacker(const buffer& buf)
                    : m_buf(buf)
                    , m_off(0)
                {
                }

            public:
                unpacker& operator >> (uint64_t& rhs)
                {
                    if (m_off + sizeof(uint64_t) > m_buf.m_buf.size())
                    {
                        throw exception();
                    }

                    memmove(&rhs, m_buf.cget() + m_off, sizeof(uint64_t));
                    m_off += sizeof(uint64_t);
                    rhs = be64toh(rhs);
                    return *this;
                }

                unpacker& operator >> (uint32_t& rhs)
                {
                    if (m_off + sizeof(uint32_t) > m_buf.m_buf.size())
                    {
                        throw exception();
                    }

                    memmove(&rhs, m_buf.cget() + m_off, sizeof(uint32_t));
                    m_off += sizeof(uint32_t);
                    rhs = ntohl(rhs);
                    return *this;
                }

                unpacker& operator >> (uint16_t& rhs)
                {
                    if (m_off + sizeof(uint16_t) > m_buf.m_buf.size())
                    {
                        throw exception();
                    }

                    memmove(&rhs, m_buf.cget() + m_off, sizeof(uint16_t));
                    m_off += sizeof(uint16_t);
                    rhs = ntohs(rhs);
                    return *this;
                }

                unpacker& operator >> (uint8_t& rhs)
                {
                    if (m_off + sizeof(uint8_t) > m_buf.m_buf.size())
                    {
                        throw exception();
                    }

                    memmove(&rhs, m_buf.cget() + m_off, sizeof(uint8_t));
                    m_off += sizeof(uint8_t);
                    return *this;
                }

                unpacker& operator >> (padding p)
                {
                    if (m_off + p.size() > m_buf.m_buf.size())
                    {
                        throw exception();
                    }

                    m_off += p.size();
                    return *this;
                }

                unpacker& operator >> (buffer& b)
                {
                    b.clear();
                    b.m_buf.resize(m_buf.size() - m_off);
                    memmove(b.mget(), m_buf.cget() + m_off, m_buf.size() - m_off);
                    m_off = m_buf.size();
                    return *this;
                }

            private:
                unpacker(const unpacker&);

            private:
                unpacker& operator = (const unpacker&);

            private:
                const buffer& m_buf;
                size_t m_off;
        };

    public:
        buffer()
            : m_buf()
        {
        }

        buffer(size_t sz)
            : m_buf()
        {
            m_buf.reserve(sz);
        }

        buffer(const char* buf, size_t sz)
            : m_buf(sz)
        {
            memmove(this->mget(), buf, sz);
        }

        buffer(const void* buf, size_t sz)
            : m_buf(sz)
        {
            memmove(this->mget(), buf, sz);
        }

    public:
        const void* get() const
        {
            return &m_buf.front();
        }

        size_t size() const
        {
            return m_buf.size();
        }

        bool empty() const
        {
            return m_buf.size() == 0;
        }

    public:
        void clear()
        {
            m_buf.clear();
        }

        size_t trim_prefix(size_t sz)
        {
            sz = std::min(sz, m_buf.size());
            size_t remain = m_buf.size() - sz;
            memmove(this->mget(), this->mget() + sz, remain);
            m_buf.resize(remain);
            return sz;
        }

    public:
        bool operator < (const buffer& rhs) const
        {
            const buffer& lhs(*this);

            if (lhs.m_buf.size() < rhs.m_buf.size())
            {
                return true;
            }
            else if (lhs.m_buf.size() == rhs.m_buf.size())
            {
                return memcmp(lhs.get(), rhs.get(), lhs.size()) < 0;
            }

            return false;
        }

        bool operator == (const buffer& rhs) const
        {
            const buffer& lhs(*this);

            if (lhs.m_buf.size() == rhs.m_buf.size())
            {
                return memcmp(lhs.get(), rhs.get(), lhs.size()) == 0;
            }

            return false;
        }

        buffer& operator += (const buffer& rhs)
        {
            buffer& lhs(*this);
            size_t sz = lhs.m_buf.size();
            lhs.m_buf.resize(sz + rhs.m_buf.size());
            std::copy(rhs.m_buf.begin(), rhs.m_buf.end(), lhs.m_buf.begin() + sz);
            return lhs;
        }

    private:
        // Get a mutable reference to the buffer.
        uint8_t* mget()
        {
            return &m_buf.front();
        }

        // Get a constant reference to the buffer.
        const uint8_t* cget() const
        {
            return &m_buf.front();
        }

    private:
        std::vector<uint8_t> m_buf;
};

} // namespace e

#endif // e_buffer_h_
