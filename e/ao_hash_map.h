// Copyright (c) 2014, Robert Escriva
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

#ifndef e_ao_hash_map_h_
#define e_ao_hash_map_h_

// C
#include <stdint.h>

// e
#include <e/compat.h>
#include <e/lookup3.h>

namespace e
{

// This is intended to be a hash map that maps a fixed, known set of keys to a
// set of (possibly mutable) values.  For example, it could be used to map a set
// of virtual servers to their physical server IDs, or to map a set of IDs to
// sequential counters.
//
// The advisable pattern for using this map is to load it up with the set of
// known keys using "put", and then synchronize with all threads.  Threads can
// then call "get" and "mod" without synchronizing those calls (although
// changing the pointer returned by "mod" should be done with synchronization).
//
// This hash table assumes you used a good hash function that will never map
// more than 4 keys to the same hash.  The table maps each key to a bucket of
// size 4, and will resize on overflow.  So it's quite critical to have a good
// hash function.  Don't despair if you accidently choose a bad function!  The
// table has an array that it will look to for handling these cases, but that's
// quite slow.  Still better than scanning the entire table, and it's unlikely
// to happen except for the densest of cases.

template <typename K, typename V, uint64_t (*H)(K), const K EMPTY>
class ao_hash_map
{

    public:
        ao_hash_map();
        ~ao_hash_map() throw ();

    public:
        bool put(K k, V v);
        bool get(K k, V* v);
        bool mod(K k, V** v);
        void reset();

    private:
        const static uint64_t BUCKET_SIZE = 4;
        struct node
        {
            node() : key(EMPTY), val(EMPTY) {}
            node(const node& other) : key(other.key), val(other.val) {}
            node& operator = (const node& rhs)
            { key = rhs.key; val = rhs.val; return *this; }
            K key;
            V val;
        };
        struct bucket
        {
            bucket()
            {
                for (size_t i = 0; i < BUCKET_SIZE; ++i)
                {
                    nodes[i] = node();
                }
            }
            node nodes[BUCKET_SIZE];
        };

    private:
        double load_factor();
        uint64_t get_index1(K k, uint64_t table_size);
        uint64_t get_index2(K k, uint64_t table_size);
        typedef uint64_t (ao_hash_map::*index_func)(K k, uint64_t table_size);
        bucket* get_bucket(bucket* table, uint64_t table_size, K k, index_func f);
        bool put(bucket* b, K k, V v);
        bool mod(bucket* b, K k, V** v);
        void cuckoo(bucket* b, K* k, V* v);
        void resize_table();
        void resize_table(bucket** table,
                          uint64_t old_table_size,
                          uint64_t new_table_size,
                          index_func f);
        void make_room_at_array_head();

    private:
        uint64_t m_table_size;
        bucket* m_table1;
        bucket* m_table2;
        uint64_t m_array_size;
        node* m_array;
        uint64_t m_elements;

    private:
        ao_hash_map(const ao_hash_map&);
        ao_hash_map& operator = (const ao_hash_map&);
};

template <typename K, typename V, uint64_t (*H)(K), const K E>
ao_hash_map<K, V, H, E> :: ao_hash_map()
    : m_table_size(0)
    , m_table1(NULL)
    , m_table2(NULL)
    , m_array_size(0)
    , m_array(NULL)
    , m_elements(0)
{
}

template <typename K, typename V, uint64_t (*H)(K), const K E>
ao_hash_map<K, V, H, E> :: ~ao_hash_map() throw ()
{
    reset();
}

template <typename K, typename V, uint64_t (*H)(K), const K EMPTY>
bool
ao_hash_map<K, V, H, EMPTY> :: put(K k, V v)
{
    for (int attempt = 0; attempt < 128; ++attempt)
    {
        if (m_table_size == 0 ||
            load_factor() > .9 ||
            (load_factor() > .75 && attempt >= 64))
        {
            resize_table();
        }

        bucket* b1 = get_bucket(m_table1, m_table_size, k, &ao_hash_map::get_index1);
        bucket* b2 = get_bucket(m_table2, m_table_size, k, &ao_hash_map::get_index2);
        assert(b1 && b2);

        if (put(b1, k, v))
        {
            return true;
        }
        if (put(b2, k, v))
        {
            return true;
        }

        bucket* tables[2] = { b1, b2 };
        cuckoo(tables[attempt & 1], &k, &v);
        assert(k != EMPTY);
    }

    for (size_t i = 0; i < m_array_size; ++i)
    {
        if (m_array[i].key == k)
        {
            m_array[i].val = v;
            return true;
        }
    }

    make_room_at_array_head();
    assert(m_array_size > 0 && m_array[0].key == EMPTY);
    m_array[0].key = k;
    m_array[0].val = v;
    ++m_elements;
    return true;
}

template <typename K, typename V, uint64_t (*H)(K), const K E>
bool
ao_hash_map<K, V, H, E> :: get(K k, V* v)
{
    V* ptr;
    bool ret = mod(k, &ptr);
    *v = *ptr;
    return ret;
}

template <typename K, typename V, uint64_t (*H)(K), const K EMPTY>
bool
ao_hash_map<K, V, H, EMPTY> :: mod(K k, V** v)
{
    bucket* b1 = get_bucket(m_table1, m_table_size, k, &ao_hash_map::get_index1);
    bucket* b2 = get_bucket(m_table2, m_table_size, k, &ao_hash_map::get_index2);

    if (mod(b1, k, v) || mod(b2, k, v))
    {
        return true;
    }

    for (uint64_t i = 0; i < m_array_size; ++i)
    {
        if (m_array[i].key == k)
        {
            *v = &m_array[i].val;
            return true;
        }
    }

    return false;
}

template <typename K, typename V, uint64_t (*H)(K), const K E>
void
ao_hash_map<K, V, H, E> :: reset()
{
    if (m_table1)
    {
        delete[] m_table1;
    }

    if (m_table2)
    {
        delete[] m_table2;
    }

    if (m_array)
    {
        delete[] m_array;
    }

    m_table_size = 0;
    m_array_size = 0;
}

#if 0
template <typename K, typename V, uint64_t (*H)(K), const K EMPTY>
void
ao_hash_map<K, V, H, EMPTY> :: dump()
{
    std::cerr << "begin dump" << "==============================" << std::endl;
    std::cout << "table_size=" << m_table_size << std::endl;
    std::cout << "array_size=" << m_array_size << std::endl;

    for (size_t i = 0; i < m_table_size; ++i)
    {
        for (size_t j = 0; j < BUCKET_SIZE; ++j)
        {
            node* n = &m_table1[i].nodes[j];

            if (n->key == EMPTY && n->val == EMPTY)
            {
                continue;
            }

            std::cout << "table1[" << i << "].nodes[" << j << "].key=" << n->key << std::endl;
            std::cout << "table1[" << i << "].nodes[" << j << "].val=" << n->val << std::endl;
        }
    }

    for (size_t i = 0; i < m_table_size; ++i)
    {
        for (size_t j = 0; j < BUCKET_SIZE; ++j)
        {
            node* n = &m_table2[i].nodes[j];

            if (n->key == EMPTY && n->val == EMPTY)
            {
                continue;
            }

            std::cout << "table2[" << i << "].nodes[" << j << "].key=" << n->key << std::endl;
            std::cout << "table2[" << i << "].nodes[" << j << "].val=" << n->val << std::endl;
        }
    }

    for (size_t i = 0; i < m_array_size; ++i)
    {
        node* n = m_array + i;
        std::cout << "array[" << i << "].key=" << n->key << std::endl;
        std::cout << "array[" << i << "].val=" << n->val << std::endl;
    }

    std::cerr << "end dump" << "================================" << std::endl;
}
#endif

template <typename K, typename V, uint64_t (*H)(K), const K EMPTY>
double
ao_hash_map<K, V, H, EMPTY> :: load_factor()
{
    if (m_table_size == 0)
    {
        return 1.0;
    }

    double total = m_table_size * 2 + m_array_size;
    return double(m_elements) / total;
}

template <typename K, typename V, uint64_t (*H)(K), const K E>
uint64_t
ao_hash_map<K, V, H, E> :: get_index1(K k, uint64_t table_size)
{
    uint64_t idx = e::lookup3_64(H(k)) & (table_size - 1);
    assert(idx < table_size);
    return idx;
}

template <typename K, typename V, uint64_t (*H)(K), const K E>
uint64_t
ao_hash_map<K, V, H, E> :: get_index2(K k, uint64_t table_size)
{
    e::compat::hash<uint64_t> H2;
    uint64_t idx = e::lookup3_64(H2(H(k))) & (table_size - 1);
    assert(idx < table_size);
    return idx;
}

template <typename K, typename V, uint64_t (*H)(K), const K E>
typename ao_hash_map<K, V, H, E>::bucket*
ao_hash_map<K, V, H, E> :: get_bucket(bucket* table, uint64_t table_size, K k, index_func f)
{
    if (table_size == 0)
    {
        return NULL;
    }

    return table + (this->*f)(k, table_size);
}

template <typename K, typename V, uint64_t (*H)(K), const K EMPTY>
bool
ao_hash_map<K, V, H, EMPTY> :: put(bucket* b, K k, V v)
{
    for (uint64_t i = 0; i < BUCKET_SIZE; ++i)
    {
        if (b->nodes[i].key == k)
        {
            b->nodes[i].val = v;
            return true;
        }
        else if (b->nodes[i].key == EMPTY)
        {
            b->nodes[i].key = k;
            b->nodes[i].val = v;
            ++m_elements;
            return true;
        }
    }

    return false;
}

template <typename K, typename V, uint64_t (*H)(K), const K EMPTY>
bool
ao_hash_map<K, V, H, EMPTY> :: mod(bucket* b, K k, V** v)
{
    for (uint64_t i = 0; i < BUCKET_SIZE; ++i)
    {
        if (b->nodes[i].key == k)
        {
            *v = &b->nodes[i].val;
            return true;
        }
    }

    return false;
}

template <typename K, typename V, uint64_t (*H)(K), const K EMPTY>
void
ao_hash_map<K, V, H, EMPTY> :: cuckoo(bucket* b, K* k, V* v)
{
    K popped_k = b->nodes[BUCKET_SIZE - 1].key;
    V popped_v = b->nodes[BUCKET_SIZE - 1].val;

    for (unsigned i = BUCKET_SIZE - 1; i > 0; --i)
    {
        assert(b->nodes[i].key != EMPTY);
        b->nodes[i] = b->nodes[i - 1];
    }

    b->nodes[0].key = *k;
    b->nodes[0].val = *v;
    *k = popped_k;
    *v = popped_v;
}

template <typename K, typename V, uint64_t (*H)(K), const K EMPTY>
void
ao_hash_map<K, V, H, EMPTY> :: resize_table()
{
    const uint64_t new_table_size = m_table_size > 0 ? m_table_size * 2 : 8;
    resize_table(&m_table1, m_table_size, new_table_size, &ao_hash_map::get_index1);
    resize_table(&m_table2, m_table_size, new_table_size, &ao_hash_map::get_index2);
    m_table_size = new_table_size;
}

template <typename K, typename V, uint64_t (*H)(K), const K EMPTY>
void
ao_hash_map<K, V, H, EMPTY> :: resize_table(bucket** table,
                                            size_t old_table_size,
                                            size_t new_table_size,
                                            index_func f)
{
    bucket* old_table = *table;
    bucket* new_table = new bucket[new_table_size];

    for (uint64_t bidx = 0; bidx < old_table_size; ++bidx)
    {
        for (uint64_t nidx = 0; nidx < BUCKET_SIZE; ++nidx)
        {
            node* n = &old_table[bidx].nodes[nidx];

            if (n->key == EMPTY)
            {
                break;
            }

            uint64_t new_bidx = (this->*f)(n->key, new_table_size);
            assert(new_bidx < new_table_size);
            bucket* b = new_table + new_bidx;
            bool x = put(b, n->key, n->val);
            assert(x);
            --m_elements;
        }
    }

    if (old_table)
    {
        delete[] old_table;
    }

    *table = new_table;
}

template <typename K, typename V, uint64_t (*H)(K), const K EMPTY>
void
ao_hash_map<K, V, H, EMPTY> :: make_room_at_array_head()
{
    node* new_array = new node[m_array_size + 1];

    for (size_t i = 0; i < m_array_size; ++i)
    {
        new_array[i + 1] = m_array[i];
    }

    new_array[0].key = EMPTY;
    new_array[0].val = EMPTY;

    if (m_array)
    {
        delete[] m_array;
    }

    ++m_array_size;
    m_array = new_array;
}

} // namespace e

#endif // e_ao_hash_map_h_
