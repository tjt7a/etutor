/*
 * Copyright 2016 (c). All rights reserved.
 * Author: Paul Glendenning
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 *    * Neither the name of the author, nor the names of other
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef SERVICE_CONNECTOR_H_C20388E5_21FE_4195_86F9_ED8E5772041A
#define SERVICE_CONNECTOR_H_C20388E5_21FE_4195_86F9_ED8E5772041A
#include <cassert>
#include <future>
#include <thread>
#include <atomic>
#include <grpc++/grpc++.h>
#include "generated/lucida_service.grpc.pb.h"
#include "generated/lucida_service.pb.h"
#include "refcount.h"

namespace lucida {
class AsyncServiceConnector;

/// Generic untyped gRPC call
class RpcCall: public RefCounted {
	friend class AsyncServiceConnector;
protected:
	RpcCall(): ok_(false) {}
	/// True if no errors
	bool ok_;
	/// Storage for the status of the RPC upon completion.
	::grpc::Status status_;
	/// For waiting
	std::future<void> fut_;
	std::promise<void> promise_;
public:
	virtual ~RpcCall() {}
	const ::grpc::Status& GetStatus() const { return status_; }
	bool IsOK() const { return ok_ && status_.ok(); }
	bool Wait(unsigned timeoutInSeconds=0) const;
	std::future<void>& GetFuture() { return fut_; }
	/// Get the response
	virtual bool Get(::google::protobuf::Empty*& p) { p=nullptr; return false; } 
	virtual bool Get(Response*& p) { p=nullptr; return false; } 
};


class AsyncServiceConnector {
private:
	template <class ResponseType>
	class TypedRpcCall: public RpcCall {
		friend class AsyncServiceConnector;
	private:
		std::unique_ptr<::grpc::ClientAsyncResponseReader<ResponseType>> rpc_;
		void Finish() { 
			rpc_->Finish(&response_, &status_, dynamic_cast<RpcCall*>(this)); 
			fut_ = std::move(promise_.get_future());
		}
	public:
		TypedRpcCall(std::unique_ptr<::grpc::ClientAsyncResponseReader<ResponseType>>&& rpc):
			rpc_(std::move(rpc)) {}
		ResponseType response_;
		bool Get(ResponseType*& p) override {
			p = &response_;
			return true;
		} 
	};

	std::shared_ptr<::grpc::CompletionQueue> cq_;
	::grpc::ClientContext context_;
	std::shared_ptr<::grpc::Channel> channel_;
	std::unique_ptr<LucidaService::Stub> stub_;
	std::thread cqThread_;
	std::atomic<unsigned> errorCount_;
	std::atomic<bool> runningAsync_;
public:
	AsyncServiceConnector(const char* hostAndPort);
	AsyncServiceConnector(std::shared_ptr<::grpc::Channel> channel);
	~AsyncServiceConnector();

	void Start();
	void Shutdown();
	unsigned GetErrorCount() const { return errorCount_.load(); }

	/// @{ 
	/// Async interface. The caller can choose to ignore the returned value.
	/// @param[in] request	The request data.
	/// @param[in] context	Context for the client. It could be used to convey 
	///                     extra information to the server and/or tweak certain
	///                     RPC behaviors.
	/// @return The rpc call.
	std::shared_ptr<RpcCall> learnAsync(const Request& request, ::grpc::ClientContext* context=nullptr);
	std::shared_ptr<RpcCall> createAsync(const Request& request, ::grpc::ClientContext* context=nullptr);
	std::shared_ptr<RpcCall> inferAsync(const Request& request, ::grpc::ClientContext* context=nullptr);
	/// @}

	/// @{ 
	/// Blocking interface.
	/// @param[in] request	The request data.
	/// @param[in] context	Context for the client. It could be used to convey 
	///                     extra information to the server and/or tweak certain
	///                     RPC behaviors.
	/// @return The status of the completed rpc call.
	::grpc::Status learn(const Request& request, ::grpc::ClientContext* context=nullptr);
	::grpc::Status create(const Request& request, ::grpc::ClientContext* context=nullptr);
	::grpc::Status infer(const Request& request, Response& response, ::grpc::ClientContext* context=nullptr);
	/// @}
};

inline ::grpc::Status AsyncServiceConnector::learn(const Request& request, ::grpc::ClientContext* context) {
	::google::protobuf::Empty e;
	return stub_->learn((context == nullptr)? &context_: context, request, &e);
}
inline ::grpc::Status AsyncServiceConnector::create(const Request& request, ::grpc::ClientContext* context) {
	::google::protobuf::Empty e;
	return stub_->create((context == nullptr)? &context_: context, request, &e);
}
inline ::grpc::Status AsyncServiceConnector::infer(const Request& request, Response& response, ::grpc::ClientContext* context) {
	return stub_->infer((context == nullptr)? &context_: context, request, &response);
}
inline bool RpcCall::Wait(unsigned timeoutInSeconds) const {
	if (0 == timeoutInSeconds) {
		fut_.wait();
		return true;
	}
	return std::future_status::ready == fut_.wait_for(std::chrono::seconds(timeoutInSeconds));
}

}		// namespace lucida
#endif	// SERVICE_CONNECTOR_H_C20388E5_21FE_4195_86F9_ED8E5772041A
