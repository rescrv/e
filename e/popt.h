// Copyright (c) 2013, Robert Escriva
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

#ifndef e_popt_h_
#define e_popt_h_

// C
#include <cassert>

// Popt
#include <popt.h>

// C++
#include <iostream>

// STL
#include <string>
#include <utility>
#include <vector>

// po6
#include <po6/pathname.h>

namespace e
{
// Forward Declarations
class argument;
class argparser;

class argparser
{
    public:
        argparser();
        argparser(const argparser&);
        ~argparser() throw ();

    public:
        void help();
        void usage();
        bool parse(int argc, const char* argv[]);
        const char** args();
        size_t args_sz();

    public:
        void option_string(const char*);
        void autohelp();
        argument& arg();
        void add(const char* name, const argparser&);

    public:
        argparser& operator = (const argparser&);

    private:
        size_t size();
        size_t layout(size_t start);
        void handle(size_t which);

    private:
        bool m_autohelp;
        const char* m_optstr;
        poptContext m_ctx;
        const char** m_args;
        size_t m_args_sz;
        std::vector<poptOption> m_popts;
        std::vector<argument> m_arguments;
        std::vector<std::pair<std::string, argparser> > m_subparsers;
};

class argument
{
    public:
        argument();
        argument(const argument& other);
        ~argument() throw ();

    public:
        poptOption option() { return m_opt; }

    public:
        argument& name(char sn, const char* ln);
        argument& long_name(const char* n);
        argument& short_name(char n);
        argument& description(const char* desc);
        argument& metavar(const char* meta);
        argument& as_string(const char** v);
        argument& as_long(long* v);
        argument& as_double(double* v);
        argument& set_true(bool* b);
        argument& set_false(bool* b);
        argument& hidden();

    public:
        argument& operator = (const argument& rhs);

    private:
        friend class argparser;

    private:
        poptOption m_opt;
        char m_shortn;
        std::string m_longn;
        std::string m_desc;
        std::string m_meta;
        bool* m_true;
        bool* m_false;
};

inline
argparser :: argparser()
    : m_autohelp(false)
    , m_optstr(NULL)
    , m_ctx(NULL)
    , m_args(NULL)
    , m_args_sz(0)
    , m_popts()
    , m_arguments()
    , m_subparsers()
{
}

inline
argparser :: argparser(const argparser& other)
    : m_autohelp(other.m_autohelp)
    , m_optstr(other.m_optstr)
    , m_ctx(other.m_ctx)
    , m_args(other.m_args)
    , m_args_sz(other.m_args_sz)
    , m_popts(other.m_popts)
    , m_arguments(other.m_arguments)
    , m_subparsers(other.m_subparsers)
{
}

inline
argparser :: ~argparser() throw ()
{
    if (m_ctx)
    {
        poptFreeContext(m_ctx);
    }
}

inline void
argparser :: help()
{
    poptPrintHelp(m_ctx, stdout, 0);
}

inline void
argparser :: usage()
{
    poptPrintUsage(m_ctx, stdout, 0);
}

inline bool
argparser :: parse(int argc, const char* argv[])
{
    assert(argc > 0);
    po6::pathname pretty(argv[0]);
    pretty = pretty.basename();
    argv[0] = pretty.get();

    if (strncmp(argv[0], "lt-", 3) == 0)
    {
        argv[0] += 3;
    }

    assert(m_ctx == NULL);
    int rc;
    layout(1);
    m_ctx = poptGetContext(NULL, argc, argv, &m_popts.front(),
                           POPT_CONTEXT_POSIXMEHARDER);

    if (m_optstr)
    {
        poptSetOtherOptionHelp(m_ctx, m_optstr);
    }

    while ((rc = poptGetNextOpt(m_ctx)) != -1)
    {
        if (rc <= 0)
        {
            switch (rc)
            {
                case POPT_ERROR_NOARG:
                case POPT_ERROR_BADOPT:
                case POPT_ERROR_BADNUMBER:
                case POPT_ERROR_OVERFLOW:
                    std::cerr << poptStrerror(rc)
                              << " "
                              << poptBadOption(m_ctx, 0)
                              << std::endl;
                    return false;
                case POPT_ERROR_OPTSTOODEEP:
                case POPT_ERROR_BADQUOTE:
                case POPT_ERROR_ERRNO:
                default:
                    std::cerr << "logic error in argument parsing"
                              << std::endl;
                    return false;
            }

            continue;
        }
        else
        {
            handle(rc - 1);
        }
    }

    m_args = poptGetArgs(m_ctx);

    while (m_args && m_args[m_args_sz])
    {
        ++m_args_sz;
    }

    return true;
}

inline const char**
argparser :: args()
{
    return m_args;
}

inline size_t
argparser :: args_sz()
{
    return m_args_sz;
}

inline void
argparser :: option_string(const char* optstr)
{
    m_optstr = optstr;
}

inline void
argparser :: autohelp()
{
    m_autohelp = true;
}

argument&
argparser :: arg()
{
    m_arguments.push_back(argument());
    return m_arguments.back();
}

void
argparser :: add(const char* name, const argparser& ap)
{
    m_subparsers.push_back(std::make_pair(name, ap));
}

inline
argparser&
argparser :: operator = (const argparser& rhs)
{
    if (this != &rhs)
    {
        m_autohelp = rhs.m_autohelp;
        m_optstr = rhs.m_optstr;
        m_ctx = rhs.m_ctx;
        m_args = rhs.m_args;
        m_args_sz = rhs.m_args_sz;
        m_popts = rhs.m_popts;
        m_arguments = rhs.m_arguments;
        m_subparsers = rhs.m_subparsers;
    }

    return *this;
}

inline size_t
argparser :: size()
{
    size_t sz = m_arguments.size();

    for (size_t i = 0; i < m_subparsers.size(); ++i)
    {
        sz += m_subparsers[i].second.size();
    }

    return sz;
}

inline size_t
argparser :: layout(size_t idx)
{
    m_popts.clear();
    struct poptOption ae[] = {POPT_AUTOHELP POPT_TABLEEND};

    if (m_autohelp)
    {
        m_popts.push_back(ae[0]);
    }

    for (size_t i = 0; i < m_arguments.size(); ++i)
    {
        m_popts.push_back(m_arguments[i].option());
        m_popts.back().val = idx;
        ++idx;
    }

    for (size_t i = 0; i < m_subparsers.size(); ++i)
    {
        idx = m_subparsers[i].second.layout(idx);
        poptOption po = {NULL, 0, POPT_ARG_INCLUDE_TABLE,
                         &m_subparsers[i].second.m_popts.front(), 0,
                         m_subparsers[i].first.c_str(), NULL};
        m_popts.push_back(po);
    }

    m_popts.push_back(ae[1]);
    return idx;
}

inline void
argparser :: handle(size_t idx)
{
    if (idx < m_arguments.size())
    {
        if (m_arguments[idx].m_true)
        {
            *m_arguments[idx].m_true = true;
        }

        if (m_arguments[idx].m_false)
        {
            *m_arguments[idx].m_false = false;
        }
    }

    idx -= m_arguments.size();

    for (size_t i = 0; i < m_subparsers.size(); ++i)
    {
        size_t sz = m_subparsers[i].second.size();

        if (idx < sz)
        {
            m_subparsers[i].second.handle(idx);
            return;
        }

        idx -= sz;
    }
}

inline
argument :: argument()
    : m_opt()
    , m_shortn(0)
    , m_longn()
    , m_desc()
    , m_meta()
    , m_true(NULL)
    , m_false(NULL)
{
    m_opt.argInfo = POPT_ARG_NONE;
}

inline
argument :: argument(const argument& other)
    : m_opt(other.m_opt)
    , m_shortn(other.m_shortn)
    , m_longn(other.m_longn)
    , m_desc(other.m_desc)
    , m_meta(other.m_meta)
    , m_true(other.m_true)
    , m_false(other.m_false)
{
}

inline
argument :: ~argument() throw ()
{
}

argument&
argument :: name(char sn, const char* ln)
{
    return short_name(sn).long_name(ln);
}

argument&
argument :: long_name(const char* n)
{
    m_longn = n;
    m_opt.longName = m_longn.c_str();
    return *this;
}

argument&
argument :: short_name(char n)
{
    m_opt.shortName = n;
    return *this;
}

argument&
argument :: description(const char* desc)
{
    m_desc = desc;
    m_opt.descrip = m_desc.c_str();
    return *this;
}

argument&
argument :: metavar(const char* meta)
{
    m_meta = meta;
    m_opt.argDescrip = m_meta.c_str();
    return *this;
}

argument&
argument :: as_string(const char** v)
{
    m_opt.argInfo = POPT_ARG_STRING;
    m_opt.arg = v;
    return *this;
}

argument&
argument :: as_long(long* v)
{
    m_opt.argInfo = POPT_ARG_LONG;
    m_opt.arg = v;
    return *this;
}

argument&
argument :: as_double(double* v)
{
    m_opt.argInfo = POPT_ARG_DOUBLE;
    m_opt.arg = v;
    return *this;
}

argument&
argument :: set_true(bool* b)
{
    m_true = b;
    return *this;
}

argument&
argument :: set_false(bool* b)
{
    m_false = b;
    return *this;
}

argument&
argument :: hidden()
{
    m_opt.argInfo |= POPT_ARGFLAG_DOC_HIDDEN;
    return *this;
}

argument&
argument :: operator = (const argument& rhs)
{
    if (this != &rhs)
    {
        m_opt = rhs.m_opt;
        m_shortn = rhs.m_shortn;
        m_longn = rhs.m_longn;
        m_desc = rhs.m_desc;
        m_meta = rhs.m_meta;
        m_true = rhs.m_true;
        m_false = rhs.m_false;
    }

    return *this;
}

} // namespace e

#endif // e_popt_h_
