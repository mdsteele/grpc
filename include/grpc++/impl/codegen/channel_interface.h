/*
 *
 * Copyright 2016 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef GRPCXX_IMPL_CODEGEN_CHANNEL_INTERFACE_H
#define GRPCXX_IMPL_CODEGEN_CHANNEL_INTERFACE_H

#include <grpc++/impl/codegen/status.h>
#include <grpc++/impl/codegen/time.h>
#include <grpc/impl/codegen/connectivity_state.h>

namespace grpc {
class Call;
class ClientContext;
class RpcMethod;
class CallOpSetInterface;
class CompletionQueue;

template <class R>
class ClientReader;
template <class W>
class ClientWriter;
template <class W, class R>
class ClientReaderWriter;
template <class R>
class ClientAsyncReader;
template <class W>
class ClientAsyncWriter;
template <class W, class R>
class ClientAsyncReaderWriter;
template <class R>
class ClientAsyncResponseReader;

/// Codegen interface for \a grpc::Channel.
class ChannelInterface {
 public:
  virtual ~ChannelInterface() {}
  /// Get the current channel state. If the channel is in IDLE and
  /// \a try_to_connect is set to true, try to connect.
  virtual grpc_connectivity_state GetState(bool try_to_connect) = 0;

  /// Return the \a tag on \a cq when the channel state is changed or \a
  /// deadline expires. \a GetState needs to called to get the current state.
  template <typename T>
  void NotifyOnStateChange(grpc_connectivity_state last_observed, T deadline,
                           CompletionQueue* cq, void* tag) {
    TimePoint<T> deadline_tp(deadline);
    NotifyOnStateChangeImpl(last_observed, deadline_tp.raw_time(), cq, tag);
  }

  /// Blocking wait for channel state change or \a deadline expiration.
  /// \a GetState needs to called to get the current state.
  template <typename T>
  bool WaitForStateChange(grpc_connectivity_state last_observed, T deadline) {
    TimePoint<T> deadline_tp(deadline);
    return WaitForStateChangeImpl(last_observed, deadline_tp.raw_time());
  }

  /// Wait for this channel to be connected
  template <typename T>
  bool WaitForConnected(T deadline) {
    grpc_connectivity_state state;
    while ((state = GetState(true)) != GRPC_CHANNEL_READY) {
      if (!WaitForStateChange(state, deadline)) return false;
    }
    return true;
  }

 protected:
  template <class R>
  friend class ::grpc::ClientReader;
  template <class W>
  friend class ::grpc::ClientWriter;
  template <class W, class R>
  friend class ::grpc::ClientReaderWriter;
  template <class R>
  friend class ::grpc::ClientAsyncReader;
  template <class W>
  friend class ::grpc::ClientAsyncWriter;
  template <class W, class R>
  friend class ::grpc::ClientAsyncReaderWriter;
  template <class R>
  friend class ::grpc::ClientAsyncResponseReader;
  template <class InputMessage, class OutputMessage>
  friend Status BlockingUnaryCall(ChannelInterface* channel,
                                  const RpcMethod& method,
                                  ClientContext* context,
                                  const InputMessage& request,
                                  OutputMessage* result);
  friend class ::grpc::RpcMethod;
  virtual Call CreateCall(const RpcMethod& method, ClientContext* context,
                          CompletionQueue* cq) = 0;
  virtual void PerformOpsOnCall(CallOpSetInterface* ops, Call* call) = 0;
  virtual void* RegisterMethod(const char* method) = 0;
  virtual void NotifyOnStateChangeImpl(grpc_connectivity_state last_observed,
                                       gpr_timespec deadline,
                                       CompletionQueue* cq, void* tag) = 0;
  virtual bool WaitForStateChangeImpl(grpc_connectivity_state last_observed,
                                      gpr_timespec deadline) = 0;
};

}  // namespace grpc

#endif  // GRPCXX_IMPL_CODEGEN_CHANNEL_INTERFACE_H
