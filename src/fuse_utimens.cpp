/*
  Copyright (c) 2016, Antonio SJ Musumeci <trapexit@spawn.link>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "config.hpp"
#include "errno.hpp"
#include "fs_lutimens.hpp"
#include "fs_path.hpp"
#include "rv.hpp"
#include "ugid.hpp"

#include <fuse.h>

#include <string>
#include <vector>

#include <fcntl.h>

using std::string;
using std::vector;

namespace l
{
  static
  int
  utimens_loop_core(const string   &basepath_,
                    const char     *fusepath_,
                    const timespec  ts_[2],
                    const int       error_)
  {
    int rv;
    string fullpath;

    fullpath = fs::path::make(basepath_,fusepath_);

    rv = fs::lutimens(fullpath,ts_);

    return error::calc(rv,error_,errno);
  }

  static
  int
  utimens_loop(const vector<string> &basepaths_,
               const char           *fusepath_,
               const timespec        ts_[2])
  {
    int error;

    error = -1;
    for(size_t i = 0, ei = basepaths_.size(); i != ei; i++)
      {
        error = l::utimens_loop_core(basepaths_[i],fusepath_,ts_,error);
      }

    return -error;
  }

  static
  int
  utimens(Policy::Func::Action  actionFunc_,
          const Branches       &branches_,
          const char           *fusepath_,
          const timespec        ts_[2])
  {
    int rv;
    vector<string> basepaths;

    rv = actionFunc_(branches_,fusepath_,&basepaths);
    if(rv == -1)
      return -errno;

    return l::utimens_loop(basepaths,fusepath_,ts_);
  }
}

namespace FUSE
{
  int
  utimens(const char     *fusepath_,
          const timespec  ts_[2])
  {
    const fuse_context *fc     = fuse_get_context();
    const Config       &config = Config::ro();
    const ugid::Set     ugid(fc->uid,fc->gid);

    return l::utimens(config.func.utimens.policy,
                      config.branches,
                      fusepath_,
                      ts_);
  }
}
