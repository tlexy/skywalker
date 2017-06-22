// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_INCLUDE_STATE_MACHINE_H_
#define SKYWALKER_INCLUDE_STATE_MACHINE_H_

#include <stdint.h>
#include <string>

namespace skywalker {

class StateMachine {
 public:
  StateMachine() {}
  virtual ~StateMachine() {}

  // must set the id > 5
  void set_machine_id(int id) { id_ = id; }
  int machine_id() const { return id_; }

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value, void* context = nullptr) = 0;

 private:
  int id_;

  // No copying allowed
  StateMachine(const StateMachine&);
  void operator=(const StateMachine&);
};

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_STATE_MACHINE_H_
