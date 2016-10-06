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
#ifndef CALL_H_910ECA26_7826_48BE_9614_E9738490BE5A
#define CALL_H_910ECA26_7826_48BE_9614_E9738490BE5A

#include <cassert>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include <glog/logging.h>
#include "generated/lucida_service.grpc.pb.h"
#include "generated/lucida_service.pb.h"

namespace lucida {

// Forward reference
template<class U, class V> class TypedCall;
class AsyncServiceAcceptor;

/// Lucida service RPC handler
class AsyncServiceHandler: public LucidaService::AsyncService
{
	friend class AsyncServiceAcceptor;
private:
	virtual void OnCreate(TypedCall<Request, ::google::protobuf::Empty>* call) = 0;
	virtual void OnLearn(TypedCall<Request, ::google::protobuf::Empty>* call) = 0;
	virtual void OnInfer(TypedCall<Request, Response>* call) = 0;

	void CreateCallback(TypedCall<Request, ::google::protobuf::Empty>* call, bool ok);
	void LearnCallback(TypedCall<Request, ::google::protobuf::Empty>* call, bool ok);
	void InferCallback(TypedCall<Request, Response>* call, bool ok);
public:
	AsyncServiceHandler() {}
	virtual ~AsyncServiceHandler() {}
};

// TODO: Log if !ok
inline void AsyncServiceHandler::CreateCallback(TypedCall<Request, ::google::protobuf::Empty>* call, bool ok) {
	if (ok) OnCreate(call);
}

inline void AsyncServiceHandler::LearnCallback(TypedCall<Request, ::google::protobuf::Empty>* call, bool ok) {
	if (ok) OnLearn(call);
}

inline void AsyncServiceHandler::InferCallback(TypedCall<Request, Response>* call, bool ok) {
	if (ok) OnInfer(call);
}


class UntypedCall {
public:
	UntypedCall(): status_(CREATE) {}
	virtual ~UntypedCall() {}
	virtual void Proceed(bool ok) = 0;
	virtual UntypedCall* CreateListener() = 0;
	
	// Let's implement a tiny state machine with the following states.
	enum CallState { CREATE, PROCESS, FINISH };

	CallState GetStatus() const { return status_; }
protected:
	CallState status_;  // The current serving state.
};


template<class RequestType, class ResponseType> 
class TypedCall: public UntypedCall {
public:
	typedef void (AsyncServiceHandler::* ListenFn)
				(::grpc::ServerContext*, RequestType*, ::grpc::ServerAsyncResponseWriter<ResponseType>*, 
				 ::grpc::CompletionQueue*, ::grpc::ServerCompletionQueue*, void*);

	typedef void (AsyncServiceHandler::* HandlerFn)(TypedCall*, bool);

	TypedCall(AsyncServiceHandler* service, ListenFn listen, HandlerFn handler, ::grpc::ServerCompletionQueue* cq): 
		service_(service), cq_(cq), responder_(&ctx_), 
		handler_(handler), listen_(listen) {
	}
	TypedCall(const TypedCall&) = delete;
	TypedCall& operator = (const TypedCall&) = delete;

	/// Call this if you need to change the status code, for example INVALID_ARGUMENT.
	/// If Finish is not called by the AsyncServiceHandleir then it will be called 
	/// by the default processing with ::grpc::Status::OK
	void Finish(const ::grpc::Status& status = ::grpc::Status::OK) {
		if (FINISH != status_) {
#ifdef DEBUG
			LOG(INFO) << "TypedCall: finish tag<" << this << ">";
#endif
			status_ = FINISH;
			LOG_IF(ERROR, !status.ok()) << "TypedCall: gRPC handler reported status-code=" << int(status.error_code()) << " and message=\'" << status.error_message() << "\'";
			responder_.Finish(response_, status, this);
		}
	}

	/// Call this to report an error
	void FinishWithError(const ::grpc::Status& status) {
		if (FINISH != status_) {
#ifdef DEBUG
			LOG(INFO) << "TypedCall: finish with error tag<" << this << ">";
#endif
			status_ = FINISH;
			LOG(ERROR) << "TypedCall: gRPC handler reported status-code=" << int(status.error_code()) << " and message=\'" << status.error_message() << "\'";
			responder_.FinishWithError(status, this);
		}
	}

	void Proceed(bool ok) override {
		if (status_ == CREATE) {
			// Make this instance progress to the PROCESS state.
			status_ = PROCESS;

			// As part of the initial CREATE state, we *request* that the system
			// start processing requests. In this request, "this" acts are
			// the tag uniquely identifying the request (so that different TypedCall
			// instances can serve different requests concurrently), in this case
			// the memory address of this TypedCall instance.
#ifdef DEBUG
			LOG(INFO) << "TypedCall: listen on tag<" << this << ">";
#endif
			(service_->*listen_)(&ctx_, &request_, &responder_, cq_, cq_, (void*)this);
		} else if (status_ == PROCESS) {
			// The actual processing.
			(service_->*handler_)(this, ok); 
			Finish();
		} else {
#ifdef DEBUG
			LOG(INFO) << "TypedCall: delete tag<" << this << ">";
#endif
			assert(status_ == FINISH);
			// Once in the FINISH state, deallocate ourselves (TypedCall).
			delete this;
		}
	}

	UntypedCall* CreateListener() override {
		return new TypedCall(service_, listen_, handler_, cq_);
	}

	// What we get from the client.
	RequestType request_;
	// What we send back to the client.
	ResponseType response_;

private:
	// The means of communication with the gRPC runtime for an asynchronous
	// server.
	AsyncServiceHandler* service_;
	// The producer-consumer queue where for asynchronous server notifications.
	::grpc::ServerCompletionQueue* cq_;
	// Context for the rpc, allowing to tweak aspects of it such as the use
	// of compression, authentication, as well as to send metadata back to the
	// client.
	::grpc::ServerContext ctx_;

	// The means to get back to the client.
	::grpc::ServerAsyncResponseWriter<ResponseType> responder_;

	// Called during create
	ListenFn listen_;
	// Handler for RPC
	HandlerFn handler_;
};

}       // namespace lucida
#endif  // CALL_H_910ECA26_7826_48BE_9614_E9738490BE5A
