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
#include <tr1/memory>
#include <vector>

// po6 includes
#include <po6/threads/barrier.h>
#include <po6/threads/thread.h>

// e includes
#include <e/convert.h>
#include <e/locking_iterable_fifo.h>

static bool done = false;
static uint64_t ops = 0;
static e::locking_iterable_fifo<uint64_t> fifo;

void
usage();

void
worker_thread(po6::threads::barrier* bar);

int
main(int argc, char* argv[])
{
    if (argc != 3)
    {
        usage();
        return EXIT_FAILURE;
    }

    uint16_t threads;

    try
    {
        threads = e::convert::to_uint16_t(argv[1]);
        ops = e::convert::to_uint64_t(argv[2]);
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

    std::cout << "benchmark: " << threads << " threads iterate while "
              << " one thread pushes "
              << ops << " elements onto the lockfree fifo."
              << std::endl;

    std::vector<std::tr1::shared_ptr<po6::threads::thread> > workers;
    po6::threads::barrier bar(threads + 1);

    for (uint16_t i = 0; i < threads; ++i)
    {
        std::tr1::shared_ptr<po6::threads::thread> t;
        t.reset(new po6::threads::thread(std::tr1::bind(worker_thread, &bar)));
        workers.push_back(t);
        t->start();
    }

    bar.wait();
    fifo.append(0);

    for (uint64_t i = 1; i < ops; ++i)
    {
        fifo.append(i);
        assert(!fifo.empty());
        assert(fifo.oldest() == i - 1);
        fifo.remove_oldest();
    }

    for (uint16_t i = 0; i < threads; ++i)
    {
        workers[i]->join();
    }

    return EXIT_SUCCESS;
}

void
usage()
{
    std::cerr << "Usage: benchmark "
              << "<threads> <ops>"
              << std::endl;
    exit(EXIT_FAILURE);
}

void
worker_thread(po6::threads::barrier* bar)
{
    e::locking_iterable_fifo<uint64_t>::iterator it = fifo.iterate();
    bar->wait();

    for (uint64_t i = 0; i < ops; ++i)
    {
        while (!it.valid())
            ;

        assert(*it == i);
        it.next();
    }
}
