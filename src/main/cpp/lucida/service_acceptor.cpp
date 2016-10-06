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
#include <lucida/service_acceptor.h>
#include <glog/logging.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

namespace lucida {

AsyncServiceAcceptor::AsyncServiceAcceptor(AsyncServiceHandler* service, const std::string& name):
	service_(service), state_(INIT), serviceName_(name),
	shutdownPromise_(), shutdownFuture_(shutdownPromise_.get_future())  {
}


AsyncServiceAcceptor::~AsyncServiceAcceptor() {
	Shutdown();
	BlockUntilShutdown();
}


bool AsyncServiceAcceptor::Start(const std::string& hostAndPort, unsigned threads) {
	{
		std::lock_guard<std::mutex> guard(mu_);
		if (state_ != INIT) return false;
		state_ = STARTED;
	}
	ServerBuilder builder;

	builder.AddListeningPort(hostAndPort, grpc::InsecureServerCredentials())
		.RegisterService(service_.get());
	cq_ = builder.AddCompletionQueue();
	server_ = builder.BuildAndStart();
	if (server_.get() == nullptr) {
		LOG(ERROR) << "AsyncServiceAcceptor: failed to start";
		std::lock_guard<std::mutex> guard(mu_);
		state_ = ERROR;
		return false;
	}
	LOG(INFO) << "AsyncServiceAcceptor: server started->listening on " << hostAndPort;
	HandleRpcs();
	return true;
}


bool AsyncServiceAcceptor::Start(grpc::ServerBuilder& builder, unsigned threads) {
	{
		std::lock_guard<std::mutex> guard(mu_);
		if (state_ != INIT) return false;
		state_ = STARTED;
	}
	builder.RegisterService(service_.get());
	cq_ = builder.AddCompletionQueue();
	server_ = builder.BuildAndStart();
	if (server_.get() == nullptr) {
		LOG(ERROR) << "AsyncServiceAcceptor: failed to start";
		std::lock_guard<std::mutex> guard(mu_);
		state_ = ERROR;
		return false;
	}
	LOG(INFO) << "AsyncServiceAcceptor: server started";
	HandleRpcs();
	return true;
}


void AsyncServiceAcceptor::Shutdown() {
	// Borrowed from tensorflow source
	bool did_shutdown = false;
	{
		std::lock_guard<std::mutex> guard(mu_);
		if (state_ == STARTED) {
			LOG(INFO) << "AsyncServiceAcceptor: initiating shutdown";
			state_ = SHUTDOWN;
			did_shutdown = true;
		}
	}
	if (did_shutdown) {
		// This enqueues a special event (with a null tag) that causes the completion
		// queue to be shut down on the polling thread.
		::grpc::Alarm* a = new ::grpc::Alarm(cq_.get(), gpr_now(GPR_CLOCK_MONOTONIC), nullptr);
		shutdownAlarm_.reset(a);
	}
}


// This can be run in multiple threads if needed.
void AsyncServiceAcceptor::HandleRpcs() {
	// Spawn a new CallData instance to serve new clients.
	bool shutdown = false;	
	bool ok = true;
	void* tag;  // uniquely identifies a request.

#ifdef DEBUG
	LOG(INFO) << "AsyncServiceAcceptor: enqueueing listeners";
#endif
	(new TypedCall<Request, ::google::protobuf::Empty>(service_.get(), 
		&AsyncServiceHandler::Requestcreate, &AsyncServiceHandler::CreateCallback, cq_.get()))->Proceed(true);
	(new TypedCall<Request, ::google::protobuf::Empty>(service_.get(),
		&AsyncServiceHandler::Requestlearn, &AsyncServiceHandler::LearnCallback, cq_.get()))->Proceed(true);
	(new TypedCall<Request, Response>(service_.get(),
		&AsyncServiceHandler::Requestinfer, &AsyncServiceHandler::InferCallback, cq_.get()))->Proceed(true);
#ifdef DEBUG
	LOG(INFO) << "AsyncServiceAcceptor: listeners ready";
#endif

	// Block waiting to read the next event from the completion queue. The
	// event is uniquely identified by its tag, which in this case is the
	// memory address of a TypedCall instance.
	// The return value of Next should always be checked. This return value
	// tells us whether there is any kind of event or cq_ is shutting down.
	while (cq_->Next(&tag, &ok)) {
#ifdef DEBUG
		LOG(INFO) << "AsyncServiceAcceptor: got tag<" << tag << ">";
#endif
		//assert(ok);
		if (tag == nullptr) {
			if (!shutdown) {
				LOG(INFO) << "AsyncServiceAcceptor: shutdown alarm received";
				// Shutdown requested
				server_->Shutdown();
				// Always shutdown the completion queue after the server.
				cq_->Shutdown();
				shutdown = true;
			}
			continue;
		}
		// If not shutting down continue to listen
		if (ok) {
			if (!shutdown && static_cast<UntypedCall*>(tag)->GetStatus() == UntypedCall::PROCESS)
				static_cast<UntypedCall*>(tag)->CreateListener()->Proceed(true);
			static_cast<UntypedCall*>(tag)->Proceed(ok);            
		} else {
#ifdef DEBUG
			LOG(INFO) << "AsyncServiceAcceptor: flushing completion queue<" << cq_.get() << ">";
#endif
			delete static_cast<UntypedCall*>(tag); 
		}
	}

	if (!shutdown) {
		server_->Shutdown();
		cq_->Shutdown();
	}
	LOG(INFO) << "AsyncServiceAcceptor: server stopped";    
	{
		std::lock_guard<std::mutex> guard(mu_);
		state_ = STOPPED;        
	}
	shutdownPromise_.set_value();
}


bool AsyncServiceAcceptor::BlockUntilShutdown(unsigned maxWaitTimeInSeconds) {
	{
		std::lock_guard<std::mutex> guard(mu_);
		if (SHUTDOWN != state_ && STARTED != state_)
			return false;
	}
	if (0 == maxWaitTimeInSeconds) {
		shutdownFuture_.wait();
		return true;
	}
	if (std::future_status::ready == shutdownFuture_.wait_for(std::chrono::seconds(maxWaitTimeInSeconds))) {
		std::lock_guard<std::mutex> guard(mu_);
	}
	return true;
}


ServiceAcceptor::ServiceAcceptor(LucidaService::Service* service, const std::string& name):
	service_(service), state_(INIT), serviceName_(name), 
	shutdownPromise_(), shutdownFuture_(shutdownPromise_.get_future()) {
}


ServiceAcceptor::~ServiceAcceptor() {
	Shutdown();
	BlockUntilShutdown();
}


bool ServiceAcceptor::Start(const std::string& hostAndPort) {
	{
		std::lock_guard<std::mutex> guard(mu_);
		if (state_ != INIT) return false;
		state_ = STARTED;
	}
	ServerBuilder builder;

	builder.AddListeningPort(hostAndPort, grpc::InsecureServerCredentials())
		.RegisterService(service_.get());
	server_ = builder.BuildAndStart();
	if (server_.get() == nullptr) {
		LOG(ERROR) << "Sync server failed to start";
		std::lock_guard<std::mutex> guard(mu_);
		state_ = ERROR;
		return false;
	}
	LOG(INFO) << "ServiceAcceptor: server started->listening on " << hostAndPort;
	server_->Wait();
	LOG(INFO) << "ServiceAcceptor: server stopped";
	{
		std::lock_guard<std::mutex> guard(mu_);
		state_ = STOPPED;        
	}
	shutdownPromise_.set_value();
	return true;
}


bool ServiceAcceptor::Start(grpc::ServerBuilder& builder) {
	{
		std::lock_guard<std::mutex> guard(mu_);
		if (state_ != INIT) return false;
		state_ = STARTED;
	}
	builder.RegisterService(service_.get());
	server_ = builder.BuildAndStart();
	if (server_.get() == nullptr) {
		LOG(ERROR) << "ServiceAcceptor: server failed to start";
		std::lock_guard<std::mutex> guard(mu_);
		state_ = ERROR;
		return false;
	}
	LOG(INFO) << "ServiceAcceptor: server started";
	server_->Wait();
	LOG(INFO) << "ServiceAcceptor: server stopped";
	{
		std::lock_guard<std::mutex> guard(mu_);
		state_ = STOPPED;        
	}
	shutdownPromise_.set_value();
	return true;
}


void ServiceAcceptor::Shutdown() {
	// Borrowed from tensorflow source
	bool did_shutdown = false;
	{
		std::lock_guard<std::mutex> guard(mu_);
		if (state_ == STARTED) {
			LOG(INFO) << "ServiceAcceptor: initiating shutdown";
			state_ = SHUTDOWN;
			did_shutdown = true;
		}
	}
	if (did_shutdown) {
		server_->Shutdown();
	}
}


bool ServiceAcceptor::BlockUntilShutdown(unsigned maxWaitTimeInSeconds) {
	{
		std::lock_guard<std::mutex> guard(mu_);
		if (STARTED != state_ && SHUTDOWN != state_)
			return false;
	}
	if (0 == maxWaitTimeInSeconds) {
		shutdownFuture_.wait();
		return true;
	}
	return std::future_status::ready == shutdownFuture_.wait_for(std::chrono::seconds(maxWaitTimeInSeconds));
}


} // namespace lucida
