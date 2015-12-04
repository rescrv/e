// Copyright (c) 2015, Robert Escriva
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

#ifndef e_daemonize_h_
#define e_daemonize_h_

// C
#include <limits.h>

// STL
#include <string>

// po6
#include <po6/path.h>

// e
#include <e/daemon.h>

namespace e
{

inline bool
daemonize(bool background,
          std::string log,
          std::string log_prefix,
          std::string pidfile,
          bool has_pidfile
        )
{
    google::LogToStderr();

    if (background)
    {
        char buf[PATH_MAX];
        char* cwd = getcwd(buf, PATH_MAX);

        if (!cwd)
        {
            LOG(ERROR) << "could not get current working directory";
            return false;
        }

        log = po6::path::join(cwd, log);
        struct stat x;

        if (lstat(log.c_str(), &x) < 0 || !S_ISDIR(x.st_mode))
        {
            LOG(ERROR) << "cannot fork off to the background because "
                       << log << " does not exist or is not writable";
            return false;
        }

        if (!has_pidfile)
        {
            LOG(INFO) << "forking off to the background";
            LOG(INFO) << "you can find the log at " << log << "/" << log_prefix << "YYYYMMDD-HHMMSS.sssss";
            LOG(INFO) << "provide \"--foreground\" on the command-line if you want to run in the foreground";
        }

        google::SetLogSymlink(google::INFO, "");
        google::SetLogSymlink(google::WARNING, "");
        google::SetLogSymlink(google::ERROR, "");
        google::SetLogSymlink(google::FATAL, "");
        log = po6::path::join(log, log_prefix);
        google::SetLogDestination(google::INFO, log.c_str());

        if (::daemon(1, 0) < 0)
        {
            PLOG(ERROR) << "could not daemonize";
            return false;
        }

        if (has_pidfile && !create_pidfile(pidfile))
        {
            PLOG(ERROR) << "could not create pidfile " << pidfile.c_str();
            return false;
        }
    }
    else
    {
        LOG(INFO) << "running in the foreground";
        LOG(INFO) << "no log will be generated; instead, the log messages will print to the terminal";
        LOG(INFO) << "provide \"--daemon\" on the command-line if you want to run in the background";
    }

    return true;
}

} // namespace e

#endif // e_daemonize_h_
