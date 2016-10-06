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
#include <lucida/service_connector.h>
#include <grpc++/alarm.h>
#include <glog/logging.h>

using ::google::protobuf::Empty;
using ::grpc::Channel;
using ::grpc::CompletionQueue;
using ::grpc::Status;

namespace lucida {


AsyncServiceConnector::AsyncServiceConnector(const char* hostAndPort):
	channel_(::grpc::CreateChannel(hostAndPort, ::grpc::InsecureChannelCredentials())),
	stub_(LucidaService::NewStub(channel_)), errorCount_(0), runningAsync_(false) {
}


AsyncServiceConnector::AsyncServiceConnector(std::shared_ptr<Channel> channel):
	channel_(channel), stub_(LucidaService::NewStub(channel)), 
	errorCount_(0), runningAsync_(false) {
}

AsyncServiceConnector::~AsyncServiceConnector() {
	Shutdown();
}

void AsyncServiceConnector::Start() {
	// FIXME: should either use atomic load or mutex and 
	if (runningAsync_.exchange(true)) return;
	cq_.reset(new CompletionQueue);
	auto functor = [](std::shared_ptr<CompletionQueue> cq, std::atomic<unsigned>& errs) ->void {
		bool ok;
		void* tag;
#ifdef DEBUG
			LOG(INFO) << "AsyncServiceConnector: worker thread started cq<" << cq << ">";
#endif        
		while (cq->Next(&tag, &ok)) {
#ifdef DEBUG
			LOG(INFO) << "AsyncServiceConnector: got tag<" << tag << ">";
#endif
			if (tag == nullptr) {
#ifdef DEBUG
				LOG(INFO) << "AsyncServiceConnector: shutdown received";
#endif
				// Shutdown requested
				break;
			}
			if (!ok) ++errs;
			static_cast<RpcCall*>(tag)->ok_ = ok;
			static_cast<RpcCall*>(tag)->promise_.set_value();
			static_cast<RpcCall*>(tag)->Unref();
		}
		cq->Shutdown();
	};
	cqThread_ = std::thread(functor, cq_, std::ref(errorCount_));
}


void AsyncServiceConnector::Shutdown() {
	// This enqueues a special event (with a null tag) that causes the completion
	// queue to be shut down on the polling thread.
	if (runningAsync_.load()) {
#ifdef DEBUG
		LOG(INFO) << "AsyncServiceConnector: shutdown initiated.";
#endif
		std::unique_ptr<::grpc::Alarm> a(new ::grpc::Alarm(cq_.get(), gpr_now(GPR_CLOCK_MONOTONIC), nullptr));
		cqThread_.join();
		runningAsync_ = false;
	}
}


std::shared_ptr<RpcCall> AsyncServiceConnector::learnAsync(const Request& request, ::grpc::ClientContext* context) {
	typedef TypedRpcCall<Empty> _RpcCall;
	assert(runningAsync_.load());
	_RpcCall* tag = new _RpcCall(stub_->Asynclearn((context == nullptr)? &context_: context, request, cq_.get()));
	tag->Ref(); // one for worker thread
	tag->Finish();
	return std::shared_ptr<RpcCall>(dynamic_cast<RpcCall*>(tag), RefDeleter<RpcCall>());
}


std::shared_ptr<RpcCall> AsyncServiceConnector::createAsync(const Request& request, ::grpc::ClientContext* context) {
	typedef TypedRpcCall<Empty> _RpcCall;
	assert(runningAsync_.load());
	_RpcCall* tag = new _RpcCall(stub_->Asynccreate((context == nullptr)? &context_: context, request, cq_.get()));
	tag->Ref(); // one for worker thread
	tag->Finish();
	return std::shared_ptr<RpcCall>(dynamic_cast<RpcCall*>(tag), RefDeleter<RpcCall>());
}


std::shared_ptr<RpcCall> AsyncServiceConnector::inferAsync(const Request& request, ::grpc::ClientContext* context) {
	typedef TypedRpcCall<Response> _RpcCall;
	assert(runningAsync_.load());
	_RpcCall* tag = new _RpcCall(stub_->Asyncinfer((context == nullptr)? &context_: context, request, cq_.get())); 
	tag->Ref(); // one for worker thread
	tag->Finish();
	return std::shared_ptr<RpcCall>(dynamic_cast<RpcCall*>(tag), RefDeleter<RpcCall>());
}

} // namespace lucida

