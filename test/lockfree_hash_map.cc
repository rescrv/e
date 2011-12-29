// Copyright (c) 2011, Robert Escriva
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of this project nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// C includes
#include <cstdlib>
#include <stdint.h>

// C++
#include <iostream>

// STL
#include <tr1/functional>
#include <tr1/memory>
#include <vector>

// po6 includes
#include <po6/threads/thread.h>

// e includes
#include <e/convert.h>
#include <e/lockfree_hash_map.h>

uint64_t
id(const uint64_t& x)
{
    return x;
}

static uint64_t ops;
static uint64_t workunit;
static uint64_t done;
static uint64_t insert_modulus;
static uint64_t scan_modulus;
static uint16_t table_size;
static volatile bool stop_scan = false;

int
usage();

void
insert_thread(e::lockfree_hash_map<uint64_t, uint64_t, id>* hash_map);

void
scan_thread(e::lockfree_hash_map<uint64_t, uint64_t, id>* hash_map);

int
main(int argc, char* argv[])
{
    if (argc != 7)
    {
        return usage();
    }

    uint16_t threads;

    try
    {
        threads = e::convert::to_uint16_t(argv[1]);
        ops = e::convert::to_uint64_t(argv[2]);
        workunit = e::convert::to_uint64_t(argv[3]);
        insert_modulus = e::convert::to_uint64_t(argv[4]);
        scan_modulus = e::convert::to_uint64_t(argv[5]);
        table_size = e::convert::to_uint16_t(argv[6]);
        done = 0;
    }
    catch (std::domain_error& e)
    {
        usage();
        std::cerr << "All parameters must be numeric in nature.";
        return EXIT_FAILURE;
    }
    catch (std::out_of_range& e)
    {
        usage();
        std::cerr << "All parameters must be suitably small.";
        return EXIT_FAILURE;
    }

    e::lockfree_hash_map<uint64_t, uint64_t, id> hash_map(table_size);

    std::cout << "benchmark: " << threads << " threads will perform "
              << ops << " insert/remove operations on a hash map of size "
              << table_size << " with keys created modulo " << insert_modulus
              << " and removed modulo " << scan_modulus << "."
              << std::endl;

    std::vector<std::tr1::shared_ptr<po6::threads::thread> > workers;

    for (uint16_t i = 0; i < threads; ++i)
    {
        std::tr1::shared_ptr<po6::threads::thread> t;
        t.reset(new po6::threads::thread(std::tr1::bind(insert_thread, &hash_map)));
        workers.push_back(t);
        t->start();
    }

    po6::threads::thread scanner(std::tr1::bind(scan_thread, &hash_map));
    scanner.start();

    for (uint16_t i = 0; i < workers.size(); ++i)
    {
        workers[i]->join();
    }

    stop_scan = true;
    scanner.join();
    return EXIT_SUCCESS;
}

int
usage()
{
    std::cerr << "Usage: benchmark "
              << "<threads> "
              << "<ops> "
              << "<workunit> "
              << "<insert_modulus> "
              << "<scan_modulus> "
              << "<table_size>"
              << std::endl;
    return EXIT_FAILURE;
}

void
insert_thread(e::lockfree_hash_map<uint64_t, uint64_t, id>* hash_map)
{
    uint64_t failed = 0;
    uint64_t succeeded = 0;
    uint64_t lower = __sync_fetch_and_add(&done, workunit);
    uint64_t upper = lower + workunit;

    while (lower < ops)
    {
        upper = std::min(upper, ops);

        for (; lower < upper; ++lower)
        {
            uint64_t key = lower % insert_modulus;

            while (!hash_map->insert(key, pthread_self()))
                ++failed;
            ++succeeded;
        }

        lower = __sync_fetch_and_add(&done, workunit);
        upper = lower + workunit;
    }

    std::cout << "Thread " << pthread_self() << " performed "
              << failed << "/" << succeeded << " inserts." << std::endl;
}

void
scan_thread(e::lockfree_hash_map<uint64_t, uint64_t, id>* hash_map)
{
    while (!stop_scan)
    {
        for (uint64_t i = 0; i < scan_modulus; ++i)
        {
            for (e::lockfree_hash_map<uint64_t, uint64_t, id>::iterator p = hash_map->begin();
                    p != hash_map->end(); p.next())
            {
                if (p.key() % scan_modulus == i)
                {
                    hash_map->remove(p.key());
                }
            }
        }
    }

    for (e::lockfree_hash_map<uint64_t, uint64_t, id>::iterator p = hash_map->begin();
            p != hash_map->end(); p.next())
    {
        hash_map->remove(p.key());
    }

    for (e::lockfree_hash_map<uint64_t, uint64_t, id>::iterator p = hash_map->begin();
            p != hash_map->end(); p.next())
    {
        hash_map->remove(p.key());
    }

    assert(hash_map->begin() == hash_map->end());
}
