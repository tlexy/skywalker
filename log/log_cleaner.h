// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_LOG_LOG_CLEANER_H_
#define SKYWALKER_LOG_LOG_CLEANER_H_

#include <atomic>

#include "util/thread.h"

namespace skywalker {

class Config;
class LogManager;

class LogCleaner {
 public:
  LogCleaner(Config* config, LogManager* manager);
  ~LogCleaner();

  void Start();

  void StartGC();
  void StopGC();

 private:
  static void* StartGC(void* data);
  void GCLoop();

  Config* config_;
  LogManager* manager_;

  bool exit_;
  std::atomic<bool> stop_;
  Thread thread_;

  // No copying allowed
  LogCleaner(const LogCleaner&);
  void operator=(const LogCleaner&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_LOG_CLEANER_H_