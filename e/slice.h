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

#ifndef e_slice_h_
#define e_slice_h_

// C
#include <assert.h>
#include <string.h>
#include <stdint.h>

// STL
#include <string>
#include <vector>

namespace e
{

// Refer to another piece of memory.  The memory is assumed to be managed
// somewhere else, and must outlive the use of all slices using it.
//
// Comparisons are NOT lexicographic, just guaranteed to follow be consistent.
class slice
{
    public:
        slice();
        slice(const char* data);
        slice(const char* data, size_t sz);
        slice(const uint8_t* data, size_t sz);
        slice(const std::string& str);
        slice(const std::vector<uint8_t>& buf);
        slice(const slice& other);
        ~slice() throw ();

    public:
        int compare(const slice& rhs) const;
        const uint8_t* data() const { return m_data; }
        const char* cdata() const { return reinterpret_cast<const char*>(m_data); }
        bool empty() const { return m_sz == 0; }
        std::string hex() const;
        size_t size() const { return m_sz; }
        bool starts_with(const e::slice& other) const;
        std::string str() const { return std::string(cdata(), size()); }

    public:
        void advance(size_t sz);
        void reset();
        void reset(const uint8_t* data, size_t sz);

    public:
        slice& operator = (const slice& rhs);
        bool operator < (const slice& rhs) const { return compare(rhs) < 0; }
        bool operator <= (const slice& rhs) const { return compare(rhs) <= 0; }
        bool operator == (const slice& rhs) const { return compare(rhs) == 0; }
        bool operator != (const slice& rhs) const { return compare(rhs) != 0; }
        bool operator >= (const slice& rhs) const { return compare(rhs) >= 0; }
        bool operator > (const slice& rhs) const { return compare(rhs) > 0; }

    private:
        const uint8_t* m_data;
        size_t m_sz;
};

} // namespace e

#endif // e_slice_h_
