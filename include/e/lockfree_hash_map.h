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

#ifndef e_lockfree_hash_map_h_
#define e_lockfree_hash_map_h_

// STL
#include <vector>

// e
#include <e/bit_stealing.h>
#include <e/hazard_ptrs.h>

namespace e
{

template <typename K, typename V, uint64_t (*H)(const K&)>
class lockfree_hash_map
{
    public:
        lockfree_hash_map(uint16_t magnitude = 5);
        ~lockfree_hash_map() throw ();

    public:
        bool contains(const K& k);
        bool lookup(const K& k, V* v);
        bool insert(const K& k, const V& v);
        bool remove(const K& k);

    private:
        enum bits
        {
            VALID   = 0,
            DELETED = 8
        };

        class node;
        typedef std::auto_ptr<typename hazard_ptrs<node, 3>::hazard_ptr> hazard_ptr;

    private:
        lockfree_hash_map(const lockfree_hash_map&);

    private:
        bool is_clean(node* ptr) const
        {
            return e::bit_stealing::get(ptr, VALID) &&
                   !e::bit_stealing::get(ptr, DELETED);
        }

    private:
        bool cas(node** loc, node* A, node* B)
        {
            return __sync_bool_compare_and_swap(loc, A, B);
        }

        bool find(const hazard_ptr& hptr, uint64_t hash, const K& key,
                  node*** prev, node** cur);

    private:
        lockfree_hash_map& operator = (const lockfree_hash_map&);

    private:
        hazard_ptrs<node, 3> m_hazards;
        std::vector<node*> m_table;
        uint64_t m_size;
};

template <typename K, typename V, uint64_t (*H)(const K&)>
lockfree_hash_map<K, V, H> :: lockfree_hash_map(uint16_t magnitude)
    : m_hazards()
    , m_table()
    , m_size(0)
{
    node* valid_empty = NULL;
    e::bit_stealing::set(valid_empty, VALID);
    m_table.resize((1 << magnitude), valid_empty);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
lockfree_hash_map<K, V, H> :: ~lockfree_hash_map() throw ()
{
    for (size_t i = 0; i < m_table.size(); ++i)
    {
        node* n = e::bit_stealing::strip(m_table[i]);

        while (n)
        {
            node* tmp = n;
            n = e::bit_stealing::strip(n->next);
            delete tmp;
        }
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
inline bool
lockfree_hash_map<K, V, H> :: contains(const K& k)
{
    return lookup(k, NULL);
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
lockfree_hash_map<K, V, H> :: lookup(const K& k, V* v)
{
    hazard_ptr hptr = m_hazards.get();
    const uint64_t hash = H(k);

    while (true)
    {
        node** prev;
        node* cur;

        if (find(hptr, hash, k, &prev, &cur))
        {
            assert(is_clean(cur));

            if (v)
            {
                *v = e::bit_stealing::strip(cur)->value;
            }

            return true;
        }
        else
        {
            return false;
        }
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
lockfree_hash_map<K, V, H> :: insert(const K& k, const V& v)
{
    hazard_ptr hptr = m_hazards.get();
    const uint64_t hash = H(k);
    __sync_add_and_fetch(&m_size, 1);

    while (true)
    {
        node** prev;
        node* cur;

        if (find(hptr, hash, k, &prev, &cur))
        {
            return false;
        }

        assert(is_clean(cur));
        std::auto_ptr<node> nn(new node(hash, k, v, cur));
        node* inserted = nn.get();
        e::bit_stealing::set(inserted, VALID);

        if (cas(prev, cur, inserted))
        {
            nn.release();
            return true;
        }
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
lockfree_hash_map<K, V, H> :: remove(const K& k)
{
    hazard_ptr hptr = m_hazards.get();
    const uint64_t hash = H(k);
    __sync_fetch_and_sub(&m_size, 1);

    while (true)
    {
        node** prev;
        node* cur;

        if (!find(hptr, hash, k, &prev, &cur))
        {
            return false;
        }

        assert(is_clean(cur));
        node* next_old = e::bit_stealing::strip(cur)->next;
        node* next_new = next_old;
        assert(e::bit_stealing::get(next_old, VALID));

        if (e::bit_stealing::get(next_old, DELETED))
        {
            continue;
        }

        // Mark it as deleted.
        e::bit_stealing::set(next_new, DELETED);

        if (!cas(&e::bit_stealing::strip(cur)->next, next_old, next_new))
        {
            continue;
        }

        e::bit_stealing::unset(next_new, DELETED);
        e::bit_stealing::unset(cur, DELETED);
        assert(is_clean(cur));

        if (cas(prev, cur, next_new))
        {
            hptr->retire(e::bit_stealing::strip(cur));
        }
        else
        {
            find(hptr, hash, k, &prev, &cur);
        }

        return true;
    }
}

template <typename K, typename V, uint64_t (*H)(const K&)>
class lockfree_hash_map<K, V, H> :: node
{
    public:
        node(uint64_t h, const K& k, const V& v, node* n)
            : hash(h)
            , next(n)
            , key(k)
            , value(v)
        {
        }

    public:
        uint64_t hash;
        node* next;
        K key;
        V value;
};

template <typename K, typename V, uint64_t (*H)(const K&)>
bool
lockfree_hash_map<K, V, H> :: find(const hazard_ptr& hptr, uint64_t hash,
                                   const K& key, node*** prev, node** cur)
{
    const uint64_t mask = m_table.size() - 1;
    const uint64_t offset = hash & mask;

    while (true)
    {
        *prev = &m_table[offset];
        *cur = **prev;
        assert(e::bit_stealing::get(*cur, VALID));
        hptr->set(1, e::bit_stealing::strip(*cur));

        if (**prev != *cur || e::bit_stealing::get(*cur, DELETED))
        {
            continue;
        }

        while (true)
        {
            assert(e::bit_stealing::get(*cur, VALID));
            node* cur_stripped = e::bit_stealing::strip(*cur);

            if (cur_stripped == NULL)
            {
                return false;
            }

            node* next = cur_stripped->next;
            bool cmark = e::bit_stealing::get(next, DELETED);
            hptr->set(0, e::bit_stealing::strip(next));

            if (cur_stripped->next != next)
            {
                break;
            }

            uint64_t chash = cur_stripped->hash;
            K& ckey = cur_stripped->key;

            if (**prev != *cur || e::bit_stealing::get(*cur, DELETED))
            {
                break;
            }

            if (!cmark)
            {
                if ((hash == chash && key >= ckey) || hash > chash)
                {
                    return ckey == key;
                }

                *prev = &cur_stripped->next;
                hptr->set(2, cur_stripped);
            }
            else
            {
                node* A = *cur;
                node* B = next;
                e::bit_stealing::unset(A, DELETED);
                e::bit_stealing::unset(B, DELETED);

                if (cas(*prev, A, B))
                {
                    hptr->retire(cur_stripped);
                }
                else
                {
                    break;
                }
            }

            *cur = next;
            hptr->set(1, e::bit_stealing::strip(*cur));
        }
    }
}

} // namespace e

#endif // e_lockfree_hash_map_h_
