// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "machine/membership_machine.h"
#include "paxos/config.h"
#include "skywalker/logging.h"
#include "proto/paxos.pb.h"
#include "util/mutexlock.h"
#include "paxos/node_util.h"

namespace skywalker {

MembershipMachine::MembershipMachine(const Options& options, Config* config)
    : config_(config),
      db_(config->GetDB()),
      has_sync_membership_(false) {
  membership_.set_version(0);
  for (auto i : options.membership) {
    membership_.add_node_id(MakeNodeId(i));
  }
}

void MembershipMachine::Recover() {
  Membership m;
  int ret = db_->GetMembership(&m);
  if (ret == 0) {
    has_sync_membership_ = true;
    membership_ = m;
  }
  config_->SetMembership(membership_);
}

bool MembershipMachine::Execute(uint32_t group_id, uint64_t instance_id,
                                const std::string& value,
                                MachineContext* /* context */) {
  Membership m;
  if (m.ParseFromString(value)) {
    int ret = db_->SetMembership(m);
    if (ret == 0) {
      m.set_version(instance_id);
      MutexLock lock(&mutex_);
      membership_ = m;
      // copy to config.
      config_->SetMembership(m);
      if (!has_sync_membership_) {
        has_sync_membership_ = true;
      }
      return true;
    } else {
      SWLog(ERROR, "MembershipMachine::Execute - update membership failed.\n");
    }
  } else {
    SWLog(ERROR, "MembershipMachine::Execute - m.ParseFromString failed.\n");
  }
  return false;
}

const Membership& MembershipMachine::GetMembership() const {
  return membership_;
}

void MembershipMachine::GetMembership(std::vector<IpPort>* result) const {
  MutexLock lock(&mutex_);
  for (int i = 0; i < membership_.node_id_size(); ++i) {
    IpPort ip;
    ParseNodeId(membership_.node_id(i), &ip);
    result->push_back(ip);
  }
}

bool MembershipMachine::HasSyncMembership() const {
  MutexLock lock(&mutex_);
  return has_sync_membership_;
}

}  // namespace skywalker
