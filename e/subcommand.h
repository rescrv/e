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

#ifndef e_subcommand_h_
#define e_subcommand_h_

// C
#include <cstdlib>
#include <cstring>

// POSIX
#include <errno.h>
#include <unistd.h>

// C++
#include <iostream>
#include <iomanip>

// e
#include <e/popt.h>

namespace e
{

struct subcommand
{
    subcommand(const char* n, const char* d) : name(n), description(d) {}
    const char* name;
    const char* description;
};

namespace subcommands
{

int
help(const char* cmd, e::argparser* ap, subcommand* commands, size_t commands_sz)
{
    ap->help();
    size_t max_command_sz = 0;

    for (size_t i = 0; i < commands_sz; ++i)
    {
        size_t command_sz = strlen(commands[i].name);
        max_command_sz = std::max(max_command_sz, command_sz);
    }

    size_t pad = ((max_command_sz + 3ULL) & ~3ULL) + 4;

    std::cout << std::setfill(' ') << "\nAvailable commands:\n";

    for (size_t i = 0; i < commands_sz; ++i)
    {
        std::cout << "    " << std::left << std::setw(pad)
                  << commands[i].name
                  << commands[i].description << "\n";
    }

    std::cout << "\nSee '" << cmd << " help <command>' for more information on a specific command." << std::endl;
    return EXIT_FAILURE;
}

} // subcommands

int
dispatch_to_subcommands(int arg, const char* argv[],
                        const char* cmd,
                        const char* name,
                        const char* version,
                        const char* prefix,
                        const char* env_var,
                        const char* default_path,
                        subcommand* commands,
                        size_t commands_sz)
{
    bool flag_help = false;
    bool flag_usage = false;
    bool flag_version = false;
    bool flag_completion = false;
    const char* arg_path = NULL;
    const char* orig_argv0 = argv[0];

    e::argparser help_ap;
    help_ap.option_string("[COMMAND] [ARGS]");
    help_ap.arg().name('?', "help")
                 .description("Show this help message")
                 .set_true(&flag_help);
    help_ap.arg().long_name("usage")
                 .description("Display brief usage message")
                 .set_true(&flag_usage);
    help_ap.arg().long_name("version")
                 .description("Print the version and exit")
                 .set_true(&flag_version);

    e::argparser global_ap;
    global_ap.arg().long_name("exec-path")
                   .description("Path to where the subcommands are installed")
                   .metavar("PATH")
                   .as_string(&arg_path);

    e::argparser ap;
    ap.option_string("[OPTIONS] <command> [<args>]");
    ap.add("Help options:", help_ap);
    ap.add("Global options:", global_ap);
    ap.arg().long_name("dump-completion")
                 .set_true(&flag_completion)
                 .hidden();

    if (!ap.parse(arg, argv))
    {
        return EXIT_FAILURE;
    }

    if (flag_help)
    {
        return subcommands::help(cmd, &ap, commands, commands_sz);
    }

    if (flag_usage)
    {
        ap.usage();
        return EXIT_FAILURE;
    }

    if (flag_version)
    {
        std::cout << name << " version " << version << std::endl;
        return EXIT_SUCCESS;
    }

    if (flag_completion)
    {
        std::cout << "Pretend this is a bash completion script\n"; // XXX
        return EXIT_SUCCESS;
    }

    if (ap.args_sz() == 0)
    {
        return subcommands::help(cmd, &ap, commands, commands_sz);
    }

    // add the path where the binaries should be, preferring cmd-line over
    // environment over default.
    std::string path;
    char* env_path = getenv(env_var);

    if (arg_path)
    {
        path = arg_path;
    }
    else if (env_path)
    {
        path = env_path;
    }
    else
    {
        path = default_path;
    }

    // add the dir where "hyperdex" resides
    path += ":" + std::string(po6::pathname(orig_argv0).dirname().get());

    // add the existing PATH
    char* old_path = getenv("PATH");

    if (old_path)
    {
        path += ":" + std::string(old_path);
    }

    // set the path
    if (setenv("PATH", path.c_str(), 1) < 0)
    {
        std::cerr << "could not set path: " << strerror(errno) << std::endl;
        return EXIT_FAILURE;
    }

    // dispatch the command
    if (strcmp(ap.args()[0], "help") == 0)
    {
        if (ap.args_sz() > 1)
        {
            std::string exec(prefix);
            exec += ap.args()[1];
            std::vector<const char*> args;
            args.push_back("man");
            args.push_back(exec.c_str());
            args.push_back(NULL);

            if (execvp(args[0], const_cast<char* const*>(&args.front())) < 0)
            {
                std::cerr << "failed to exec \"man\" to show help: " << strerror(errno) << std::endl;
                return EXIT_FAILURE;
            }

            abort();
        }
        else
        {
            return subcommands::help(cmd, &ap, commands, commands_sz);
        }
    }

    for (size_t i = 0; i < commands_sz; ++i)
    {
        subcommand* s = commands + i;

        if (strcmp(s->name, ap.args()[0]) == 0)
        {
            std::string exec(prefix);
            exec += s->name;
            std::vector<const char*> args;
            args.push_back(exec.c_str());

            for (size_t j = 1; j < ap.args_sz(); ++j)
            {
                args.push_back(ap.args()[j]);
            }

            args.push_back(NULL);

            if (execvp(args[0], const_cast<char* const*>(&args.front())) < 0)
            {
                std::cerr << "failed to exec " << s->name << ": " << strerror(errno) << std::endl;
                std::cerr << "PATH=" << path << std::endl;
                return EXIT_FAILURE;
            }

            abort();
        }
    }

    std::cerr << "\"" << ap.args()[0] << "\" is not a " << name
              << " command.  See \"" << cmd << " --help\"\n" << std::endl;
    return subcommands::help(cmd, &ap, commands, commands_sz);
}

} // namespace e

#endif // e_subcommand_h_
