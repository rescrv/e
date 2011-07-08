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

#ifndef e_lockfree_hash_set_h_
#define e_lockfree_hash_set_h_

// e
#include <e/lockfree_hash_map.h>

namespace e
{

template <typename K, uint64_t (*H)(const K&)>
class lockfree_hash_set
{
    public:
        lockfree_hash_set();
        ~lockfree_hash_set() throw ();

    public:
        bool contains(const K& k);
        bool insert(const K& k);
        bool remove(const K& k);

    private:
        class node;

    private:
        lockfree_hash_set(const lockfree_hash_set&);

    private:
        lockfree_hash_set& operator = (const lockfree_hash_set&);

    private:
        lockfree_hash_map<K, int, H> m_map;
};

template <typename K, uint64_t (*H)(const K&)>
lockfree_hash_set<K, H> :: lockfree_hash_set()
    : m_map()
{
}

template <typename K, uint64_t (*H)(const K&)>
lockfree_hash_set<K, H> :: ~lockfree_hash_set() throw ()
{
}

template <typename K, uint64_t (*H)(const K&)>
inline bool
lockfree_hash_set<K, H> :: contains(const K& k)
{
    return m_map.contains(k);
}

template <typename K, uint64_t (*H)(const K&)>
inline bool
lockfree_hash_set<K, H> :: insert(const K& k)
{
    return m_map.insert(k, 1);
}

template <typename K, uint64_t (*H)(const K&)>
inline bool
lockfree_hash_set<K, H> :: remove(const K& k)
{
    return m_map.remove(k);
}

} // namespace e

#endif // e_lockfree_hash_set_h_
