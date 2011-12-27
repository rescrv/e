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
#include <cassert>
#include <stdint.h>
#include <string.h>

// POSIX
#include <arpa/inet.h>
#include <endian.h>

// C++
#include <iomanip>
#include <iostream>

// STL
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

// po6
#include <po6/io/fd.h>

// e
#include <e/guard.h>
#include <e/slice.h>

namespace e
{

// Easily pack/unpack values for transfer over the network.  Integers are
// converted using the host to network and network to host conversion functions.
//
// Many unpackers can work concurrently on the same buffer.
// Only a single packer may operate on a buffer at time.
class packer;
class unpacker;

class buffer
{
    public:
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

        class sized
        {
            public:
                sized(size_t _sz, buffer* _buf)
                    : m_sz(_sz)
                    , m_buf(_buf)
                {
                }

                sized(const sized& s)
                    : m_sz(s.m_sz)
                    , m_buf(s.m_buf)
                {
                }

            public:
                size_t size() const { return m_sz; }
                buffer* buf() const { return m_buf; }

            public:
                sized& operator = (const sized& s)
                {
                    if (this != &s)
                    {
                        m_sz = s.m_sz;
                        m_buf = s.m_buf;
                    }

                    return *this;
                }

            private:
                size_t m_sz;
                buffer* m_buf;
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

        buffer(const std::string& str)
            : m_buf(str.size())
        {
            memmove(this->mget(), str.c_str(), str.size());
        }

        buffer(const void* buf, size_t sz)
            : m_buf(sz)
        {
            memmove(this->mget(), buf, sz);
        }

    public:
        std::string hex() const
        {
            std::ostringstream ostr;
            ostr << std::hex;
            const uint8_t* buf = cget();

            for (size_t i = 0; i < size(); ++i)
            {
                unsigned int num = buf[i];
                ostr << std::setw(2) << std::setfill('0') << num;
            }

            return ostr.str();
        }

        const void* get() const
        {
            return &m_buf.front();
        }

        std::string str() const
        {
            return std::string(reinterpret_cast<const char*>(&m_buf.front()), m_buf.size());
        }

        size_t size() const
        {
            return m_buf.size();
        }

        bool empty() const
        {
            return m_buf.size() == 0;
        }

        size_t index(uint8_t byte) const
        {
            const uint8_t* ptr = static_cast<const uint8_t*>(memchr(cget(), byte, size()));

            if (ptr == NULL)
            {
                return size();
            }
            else
            {
                return ptr - cget();
            }
        }

        bool contains(uint8_t byte) const
        {
            return memchr(cget(), byte, size()) != NULL;
        }

        buffer slice(size_t start, size_t end) const
        {
            return buffer(cget() + start, end - start);
        }

    public:
        void* get()
        {
            return &m_buf.front();
        }

        void swap(buffer& other) throw ()
        {
            m_buf.swap(other.m_buf);
        }

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

        inline packer pack();
        inline unpacker unpack() const;
        inline e::slice as_slice() const { return e::slice(reinterpret_cast<const char*>(cget()), size()); }

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

        bool operator > (const buffer& rhs) const
        {
            const buffer& lhs(*this);

            if (lhs.m_buf.size() > rhs.m_buf.size())
            {
                return true;
            }
            else if (lhs.m_buf.size() == rhs.m_buf.size())
            {
                return memcmp(lhs.get(), rhs.get(), lhs.size()) > 0;
            }

            return false;
        }

        bool operator != (const buffer& rhs) const { return !(*this == rhs); }

        buffer& operator += (const buffer& rhs)
        {
            buffer& lhs(*this);
            size_t sz = lhs.m_buf.size();
            lhs.m_buf.resize(sz + rhs.m_buf.size());
            std::copy(rhs.m_buf.begin(), rhs.m_buf.end(), lhs.m_buf.begin() + sz);
            return lhs;
        }

    private:
        friend class packer;
        friend class unpacker;
        friend size_t read(po6::io::fd*, buffer*, size_t);
        friend size_t xread(po6::io::fd*, buffer*, size_t);

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

        packer& operator << (const buffer::padding& p)
        {
            m_buf->m_buf.resize(m_buf->m_buf.size() + p.size());
            return *this;
        }

        packer& operator << (const buffer& rhs)
        {
            uint32_t sz = rhs.size();
            *this << sz;
            *m_buf += rhs;
            return *this;
        }

        packer& operator << (const slice& rhs)
        {
            uint32_t sz = rhs.size();
            *this << sz;
            size_t oldsz = m_buf->m_buf.size();
            m_buf->m_buf.resize(oldsz + rhs.size());
            memmove(m_buf->mget(), rhs.data(), sz);
            return *this;
        }

        template <typename T>
        packer& operator << (const std::vector<T>& rhs)
        {
            uint16_t elem = rhs.size();
            *this << elem;

            for (size_t i = 0; i < rhs.size(); ++i)
            {
                *this << rhs[i];
            }

            return *this;
        }

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
        size_t remain() const
        {
            if (m_buf.m_buf.size() < m_off)
            {
                return 0;
            }
            else
            {
                return m_buf.m_buf.size() - m_off;
            }
        }

        void leftovers(e::buffer* buf) const
        {
            size_t rem = remain();
            buf->m_buf.resize(rem);
            memmove(buf->mget(), m_buf.cget() + m_off, rem);
        }

        size_t offset() const
        {
            return m_off;
        }

    public:
        void rewind(size_t off)
        {
            assert(m_off >= off);
            m_off = off;
        }

    public:
        unpacker& operator >> (uint64_t& rhs)
        {
            if (m_off + sizeof(uint64_t) > m_buf.m_buf.size())
            {
                throw std::out_of_range("Nothing left to unpack.");
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
                throw std::out_of_range("Nothing left to unpack.");
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
                throw std::out_of_range("Nothing left to unpack.");
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
                throw std::out_of_range("Nothing left to unpack.");
            }

            memmove(&rhs, m_buf.cget() + m_off, sizeof(uint8_t));
            m_off += sizeof(uint8_t);
            return *this;
        }

        unpacker& operator >> (buffer::padding p)
        {
            if (m_off + p.size() > m_buf.m_buf.size())
            {
                throw std::out_of_range("Nothing left to unpack.");
            }

            m_off += p.size();
            return *this;
        }

        unpacker& operator >> (buffer::sized s)
        {
            if (m_off + s.size() > m_buf.m_buf.size())
            {
                throw std::out_of_range("Nothing left to unpack.");
            }

            s.buf()->clear();
            s.buf()->m_buf.resize(s.size());
            memmove(s.buf()->mget(), m_buf.cget() + m_off, s.size());
            m_off += s.size();
            return *this;
        }

        unpacker& operator >> (buffer& b)
        {
            b.clear();
            uint32_t sz;
            *this >> sz;

            if (m_off + sz > m_buf.m_buf.size())
            {
                m_off -= sizeof(uint32_t);
                throw std::out_of_range("Nothing left to unpack.");
            }

            b.clear();
            b.m_buf.resize(sz);
            memmove(b.mget(), m_buf.cget() + m_off, sz);
            m_off += sz;
            return *this;
        }

        unpacker& operator >> (slice& rhs)
        {
            uint32_t sz;
            *this >> sz;

            if (m_off + sz > m_buf.m_buf.size())
            {
                m_off -= sizeof(uint32_t);
                throw std::out_of_range("Nothing left to unpack.");
            }

            rhs.reset(reinterpret_cast<const char*>(m_buf.cget()) + m_off, sz);
            return *this;
        }

        template <typename T>
        unpacker& operator >> (std::vector<T>& rhs)
        {
            size_t off = this->offset();

            try
            {
                uint16_t cols;
                *this >> cols;

                for (uint16_t i = 0; i < cols; ++i)
                {
                    T tmp;
                    *this >> tmp;
                    rhs.push_back(tmp);
                }
            }
            catch (std::out_of_range& e)
            {
                rewind(off);
                throw e;
            }

            return *this;
        }

    private:
        unpacker& operator = (const unpacker&);

    private:
        const buffer& m_buf;
        size_t m_off;
};

inline packer
buffer :: pack()
{
    return packer(this);
}

inline unpacker
buffer :: unpack() const
{
    return unpacker(*this);
}

inline size_t
read(po6::io::fd* fd, buffer* buf, size_t amt)
{
    size_t oldsize = buf->m_buf.size();
    buf->m_buf.resize(oldsize + amt);
    e::guard g = makeobjguard(buf->m_buf, &std::vector<uint8_t>::resize, oldsize, 0);
    size_t ret = fd->read(&buf->m_buf.front() + oldsize, amt);
    buf->m_buf.resize(oldsize + ret);
    g.dismiss();
    return ret;
}

inline size_t
xread(po6::io::fd* fd, buffer* buf, size_t amt)
{
    size_t oldsize = buf->m_buf.size();
    buf->m_buf.resize(oldsize + amt);
    return fd->xread(&buf->m_buf.front() + oldsize, amt);
}

} // namespace e

#endif // e_buffer_h_
