//
//
// Copyright 2015 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//

// MODIFIED by IBM (Rainer Schoenberger)
// original: #include "test/cpp/util/cli_call.h"
#include "cli_call.h"
// END MODIFIED

// MODIFIED by IBM (Anna Riesch)
// original: no #include <chrono> and #include <optional>"
#include <chrono>
#include <optional>
// END MODIFIED

#include <cmath>
#include <iostream>
#include <utility>

#include "absl/log/check.h"

#include <grpc/grpc.h>
#include <grpc/slice.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/support/byte_buffer.h>

// MODIFIED by IBM (Rainer Schoenberger)
// original: #include "src/core/lib/gprpp/crash.h"
// END MODIFIED

namespace grpc {
namespace testing {
namespace {
void* tag(intptr_t t) { return reinterpret_cast<void*>(t); }
}  // namespace

Status CliCall::Call(const std::string& request, std::string* response,
                     IncomingMetadataContainer* server_initial_metadata,
                     IncomingMetadataContainer* server_trailing_metadata) {
  Write(request);
  WritesDone();
  if (!Read(response, server_initial_metadata)) {
    fprintf(stderr, "Failed to read response.\n");
  }
  return Finish(server_trailing_metadata);
}

CliCall::CliCall(const std::shared_ptr<grpc::Channel>& channel,
                 const std::string& method,
                 const OutgoingMetadataContainer& metadata,
                 // MODIFIED by IBM (Anna Riesch)
                 // original: no argument "deadline"
                 std::optional<std::chrono::time_point<std::chrono::system_clock>> deadline)
                 //END MODIFIED
    : stub_(new grpc::GenericStub(channel)) {
  gpr_mu_init(&write_mu_);
  gpr_cv_init(&write_cv_);
  // MODIFIED by IBM (Anna Riesch, Rainer Schoenberger)
  // original: deadline parsed from CLI args
  if (deadline.has_value()) {
    // Set timeout if optional parameter has a value. Otherwise don't set timeout = infinite deadline
    auto deadlineMs = deadline.value();
    ctx_.set_deadline(deadlineMs);
  }
  
  // END MODIDFIED
  if (!metadata.empty()) {
    for (OutgoingMetadataContainer::const_iterator iter = metadata.begin();
         iter != metadata.end(); ++iter) {
      ctx_.AddMetadata(iter->first, iter->second);
    }
  }
  call_ = stub_->PrepareCall(&ctx_, method, &cq_);
  call_->StartCall(tag(1));
  void* got_tag;
  bool ok;
  cq_.Next(&got_tag, &ok);
  CHECK(ok);
}

CliCall::~CliCall() {
  gpr_cv_destroy(&write_cv_);
  gpr_mu_destroy(&write_mu_);
}

void CliCall::Write(const std::string& request) {
  void* got_tag;
  bool ok;

  grpc_slice s = grpc_slice_from_copied_buffer(request.data(), request.size());
  grpc::Slice req_slice(s, grpc::Slice::STEAL_REF);
  grpc::ByteBuffer send_buffer(&req_slice, 1);
  call_->Write(send_buffer, tag(2));
  cq_.Next(&got_tag, &ok);
  CHECK(ok);
}

bool CliCall::Read(std::string* response,
                   IncomingMetadataContainer* server_initial_metadata) {
  void* got_tag;
  bool ok;

  grpc::ByteBuffer recv_buffer;
  call_->Read(&recv_buffer, tag(3));

  if (!cq_.Next(&got_tag, &ok) || !ok) {
    return false;
  }
  std::vector<grpc::Slice> slices;
  CHECK(recv_buffer.Dump(&slices).ok());

  response->clear();
  for (size_t i = 0; i < slices.size(); i++) {
    response->append(reinterpret_cast<const char*>(slices[i].begin()),
                     slices[i].size());
  }
  if (server_initial_metadata) {
    *server_initial_metadata = ctx_.GetServerInitialMetadata();
  }
  return true;
}

void CliCall::WritesDone() {
  void* got_tag;
  bool ok;

  call_->WritesDone(tag(4));
  cq_.Next(&got_tag, &ok);
  CHECK(ok);
}

void CliCall::WriteAndWait(const std::string& request) {
  grpc::Slice req_slice(request);
  grpc::ByteBuffer send_buffer(&req_slice, 1);

  gpr_mu_lock(&write_mu_);
  call_->Write(send_buffer, tag(2));
  write_done_ = false;
  while (!write_done_) {
    gpr_cv_wait(&write_cv_, &write_mu_, gpr_inf_future(GPR_CLOCK_MONOTONIC));
  }
  gpr_mu_unlock(&write_mu_);
}

void CliCall::WritesDoneAndWait() {
  gpr_mu_lock(&write_mu_);
  call_->WritesDone(tag(4));
  write_done_ = false;
  while (!write_done_) {
    gpr_cv_wait(&write_cv_, &write_mu_, gpr_inf_future(GPR_CLOCK_MONOTONIC));
  }
  gpr_mu_unlock(&write_mu_);
}

bool CliCall::ReadAndMaybeNotifyWrite(
    std::string* response, IncomingMetadataContainer* server_initial_metadata) {
  void* got_tag;
  bool ok;
  grpc::ByteBuffer recv_buffer;

  call_->Read(&recv_buffer, tag(3));
  bool cq_result = cq_.Next(&got_tag, &ok);

  while (got_tag != tag(3)) {
    gpr_mu_lock(&write_mu_);
    write_done_ = true;
    gpr_cv_signal(&write_cv_);
    gpr_mu_unlock(&write_mu_);

    cq_result = cq_.Next(&got_tag, &ok);
    if (got_tag == tag(2)) {
      CHECK(ok);
    }
  }

  if (!cq_result || !ok) {
    // If the RPC is ended on the server side, we should still wait for the
    // pending write on the client side to be done.
    if (!ok) {
      gpr_mu_lock(&write_mu_);
      if (!write_done_) {
        cq_.Next(&got_tag, &ok);
        CHECK(got_tag != tag(2));
        write_done_ = true;
        gpr_cv_signal(&write_cv_);
      }
      gpr_mu_unlock(&write_mu_);
    }
    return false;
  }

  std::vector<grpc::Slice> slices;
  CHECK(recv_buffer.Dump(&slices).ok());
  response->clear();
  for (size_t i = 0; i < slices.size(); i++) {
    response->append(reinterpret_cast<const char*>(slices[i].begin()),
                     slices[i].size());
  }
  if (server_initial_metadata) {
    *server_initial_metadata = ctx_.GetServerInitialMetadata();
  }
  return true;
}

Status CliCall::Finish(IncomingMetadataContainer* server_trailing_metadata) {
  void* got_tag;
  bool ok;
  grpc::Status status;

  call_->Finish(&status, tag(5));
  cq_.Next(&got_tag, &ok);
  CHECK(ok);
  if (server_trailing_metadata) {
    *server_trailing_metadata = ctx_.GetServerTrailingMetadata();
  }

  return status;
}

}  // namespace testing
}  // namespace grpc
