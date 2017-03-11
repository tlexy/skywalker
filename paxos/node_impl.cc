// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/node_impl.h"

#include <memory>

#include "paxos/node_util.h"
#include "proto/paxos.pb.h"
#include "skywalker/logging.h"

namespace skywalker {

NodeImpl::NodeImpl(const Options& options)
    : options_(options),
      node_id_(MakeNodeId(options.ipport)),
      network_(node_id_) {
}

NodeImpl::~NodeImpl() {
  for (auto& g : groups_) {
    delete g.second;
  }
}

bool NodeImpl::StartWorking() {
  bool ret = true;
  for (uint32_t i = 0; i < options_.group_size; ++i) {
    Group* group = new Group(i, node_id_, options_, &network_);
    ret = group->Start();
    if (ret) {
      groups_[i] = group;
    } else {
      return ret;
    }
  }
  SWLog(DEBUG, "Node::Start - Group Start Successfully!\n");

  network_.StartServer(
      std::bind(&NodeImpl::OnReceiveMessage, this, std::placeholders::_1));
  SWLog(DEBUG, "Node::Start - Network StartServer Successfully!\n");

  for (auto& g : groups_) {
    g.second->SyncMembership();
    if (options_.use_master_) {
      g.second->SyncMaster();
    }
  }

  return ret;
}

bool NodeImpl::Propose(uint32_t group_id,
                       const std::string& value,
                       MachineContext* context,
                       const ProposeCompleteCallback& cb) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->OnPropose(value, context, cb);
}

void NodeImpl::OnReceiveMessage(const Slice& s) {
  std::shared_ptr<Content> c(new Content());
  c->ParseFromArray(s.data(), static_cast<int>(s.size()));

  std::map<uint32_t, Group*>::iterator it = groups_.find(c->group_id());
  if (it != groups_.end()) {
    it->second->OnReceiveContent(c);
  } else {
    SWLog(ERROR, "NodeImpl::OnReceiveMessage - "
          "group_id=%" PRIu32" is wrong!\n", c->group_id());
  }
}

bool NodeImpl::AddMember(uint32_t group_id, const IpPort& i,
                         const MembershipCompleteCallback& cb) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->AddMember(i, cb);
}

bool NodeImpl::RemoveMember(uint32_t group_id, const IpPort& i,
                            const MembershipCompleteCallback& cb) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->RemoveMember(i, cb);
}

bool NodeImpl::ReplaceMember(uint32_t group_id,
                             const IpPort& new_i, const IpPort& old_i,
                             const MembershipCompleteCallback& cb) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->ReplaceMember(new_i, old_i, cb);
}

void NodeImpl::GetMembership(uint32_t group_id,
                             std::vector<IpPort>* result) const {
  std::map<uint32_t, Group*>::const_iterator it = groups_.find(group_id);
  assert(it != groups_.end());
  it->second->GetMembership(result);
}

void NodeImpl::AddMachine(StateMachine* machine) {
  for (auto& g : groups_) {
    g.second->AddMachine(machine);
  }
}

void NodeImpl::RemoveMachine(StateMachine* machine) {
  for (auto& g : groups_) {
    g.second->RemoveMachine(machine);
  }
}

void NodeImpl::AddMachine(uint32_t group_id, StateMachine* machine) {
  assert(groups_.find(group_id) != groups_.end());
  groups_[group_id]->AddMachine(machine);
}

void NodeImpl::RemoveMachine(uint32_t group_id, StateMachine* machine) {
  assert(groups_.find(group_id) != groups_.end());
  groups_[group_id]->RemoveMachine(machine);
}

void NodeImpl::SetMasterLeaseTime(uint64_t micros) {
  for (auto& g : groups_) {
    g.second->SetMasterLeaseTime(micros);
  }
}

void NodeImpl::SetMasterLeaseTime(uint32_t group_id, uint64_t micros) {
  assert(groups_.find(group_id) != groups_.end());
  groups_[group_id]->SetMasterLeaseTime(micros);
}

bool NodeImpl::GetMaster(uint32_t group_id,
                         IpPort* i, uint64_t* version) const {
  std::map<uint32_t, Group*>::const_iterator it = groups_.find(group_id);
  assert(it != groups_.end());
  return it->second->GetMaster(i, version);
}

bool NodeImpl::IsMaster(uint32_t group_id) const {
  std::map<uint32_t, Group*>::const_iterator it = groups_.find(group_id);
  assert(it != groups_.end());
  return it->second->IsMaster();
}

void NodeImpl::RetireMaster(uint32_t group_id) {
  assert(groups_.find(group_id) != groups_.end());
  groups_[group_id]->RetireMaster();
}

bool Node::Start(const Options& options, Node** nodeptr) {
  *nodeptr = nullptr;
  NodeImpl* impl = new NodeImpl(options);
  bool ret = impl->StartWorking();
  if (ret) {
    *nodeptr = impl;
  } else {
    delete impl;
  }
  return ret;
}

}  // namespace skywalker
