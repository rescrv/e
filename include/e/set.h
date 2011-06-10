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

#ifndef e_set_h_
#define e_set_h_

// STL
#include <set>

// po6
#include <po6/threads/mutex.h>

namespace e
{

// A fast associative datastructure which provides linearizability and allows
// many threads to invoke the routines simultaneously.

template <typename K>
class set
{
    public:
        set();

    public:
        bool contains(const K& k);

    public:
        bool insert(const K& k);
        bool remove(const K& k);

    private:
        po6::threads::mutex m_lock;
        std::set<K> m_set;
};

template <typename K>
set<K> :: set()
    : m_lock()
    , m_set()
{
}

template <typename K>
bool
set<K> :: contains(const K& k)
{
    po6::threads::mutex::hold hold(&m_lock);
    return m_set.find(k) != m_set.end();
}

template <typename K>
bool
set<K> :: insert(const K& k)
{
    po6::threads::mutex::hold hold(&m_lock);
    std::pair<typename std::set<K>::iterator, bool> result(m_set.insert(k));
    return result.second;
}

template <typename K>
bool
set<K> :: remove(const K& k)
{
    po6::threads::mutex::hold hold(&m_lock);
    return m_set.erase(k) > 0;
}

} // namespace e

#endif // e_set_h_
