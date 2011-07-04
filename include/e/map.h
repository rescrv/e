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

#ifndef e_map_h_
#define e_map_h_

// STL
#include <map>

// po6
#include <po6/threads/mutex.h>

namespace e
{

// A fast associative datastructure which provides linearizability and allows
// many threads to invoke the routines simultaneously.

template <typename K, typename V>
class map
{
    public:
        map();

    public:
        bool contains(const K& k);

    public:
        bool lookup(const K& k, V* v);
        bool insert(const K& k, const V& v);
        bool remove(const K& k);

    private:
        po6::threads::mutex m_lock;
        std::map<K, V> m_map;
};

template <typename K, typename V>
map<K, V> :: map()
    : m_lock()
    , m_map()
{
}

template <typename K, typename V>
bool
map<K, V> :: contains(const K& k)
{
    po6::threads::mutex::hold hold(&m_lock);
    return m_map.find(k) != m_map.end();
}

template <typename K, typename V>
bool
map<K, V> :: lookup(const K& k, V* v)
{
    po6::threads::mutex::hold hold(&m_lock);
    typename std::map<K, V>::iterator iter = m_map.find(k);

    if (iter != m_map.end())
    {
        *v = iter->second;
    }

    return iter == m_map.end();
}

template <typename K, typename V>
bool
map<K, V> :: insert(const K& k, const V& v)
{
    po6::threads::mutex::hold hold(&m_lock);
    std::pair<typename std::map<K, V>::iterator, bool> result(m_map.insert(std::make_pair(k, v)));
    return result.second;
}

template <typename K, typename V>
bool
map<K, V> :: remove(const K& k)
{
    po6::threads::mutex::hold hold(&m_lock);
    return m_map.erase(k) > 0;
}

} // namespace e

#endif // e_map_h_
