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
#ifndef SERVICE_ACCEPTOR_H_62678E0B_8CC9_49E4_BB77_70E6E3ED515C
#define SERVICE_ACCEPTOR_H_62678E0B_8CC9_49E4_BB77_70E6E3ED515C

#include <future>
#include <memory>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include <grpc++/alarm.h>
#include "generated/lucida_service.grpc.pb.h"
#include "generated/lucida_service.pb.h"
#include "call.h"


namespace lucida {

/// Lucida service
///
class AsyncServiceAcceptor {
protected:
	enum State { INIT, STARTED, SHUTDOWN, STOPPED, ERROR };
	std::unique_ptr<AsyncServiceHandler> service_;
	std::unique_ptr<::grpc::ServerCompletionQueue> cq_;
	std::unique_ptr<::grpc::Server> server_;
	std::unique_ptr<::grpc::Alarm> shutdownAlarm_;
	State state_;
	std::mutex mu_;
	std::string serviceName_;
	std::promise<void> shutdownPromise_;
	std::future<void> shutdownFuture_;

private:
	void HandleRpcs();
public:
	/// Create a service adaptor. 
	///
	/// @param[in]  service The service used to handle requests.
	/// @param[in  name     The service name used in logs.
	/// @remarks Takes ownership of the service.
	AsyncServiceAcceptor(AsyncServiceHandler* service, const std::string& name);
	~AsyncServiceAcceptor();

	/// Start serving requests on hostAndPort.
	///
	/// @param[in]  hostAndPort     The hostname, or ipv4 address, and port.
	/// @param[in]  workerThreads   Sets the number of worker threads to handle
	///             requests. Zero for default.t
	/// @return     True if successful.
	/// @remarks    If successful, returns after shutdown completes.
	bool Start(const std::string& hostAndPort, unsigned workerThreads=0);

	/// Start serving requests on hostAndPort.
	///
	/// @param[in]  builder The server builder.
	/// @param[in]  workerThreads   Sets the number of worker threads to handle
	///             requests. Zero for default.t
	/// @return     True if successful.
	/// @remarks    If successful, returns after shutdown completes.
	bool Start(grpc::ServerBuilder& builder, unsigned workerThreads=0);

	/// Initiate shutdown. Typically called in a signal handler.
	/// @remarks Threadsafe
	void Shutdown();
	
	/// Blocks the calling thread until shutdown has completed.
	/// @param[in]  maxWaitTimeInSeconds A timeout.
	/// @return     True if the shutdown completed, false on timeout.
	bool BlockUntilShutdown(unsigned maxWaitTimeInSeconds=0);
};


/// Lucida service
///
class ServiceAcceptor {
protected:
	enum State { INIT, STARTED, SHUTDOWN, STOPPED, ERROR };
	State state_;
	std::mutex mu_;
	std::unique_ptr<LucidaService::Service> service_;
	std::unique_ptr<::grpc::Server> server_;
	std::string serviceName_;
	std::promise<void> shutdownPromise_;
	std::future<void> shutdownFuture_;

public:
	/// Create a service adaptor. 
	///
	/// @param[in] service The service used to handle requests.
	/// @param[in  name     The service name used in logs.
	/// @remarks Takes ownership of the service.
	ServiceAcceptor(LucidaService::Service* service, const std::string& name);
	~ServiceAcceptor();

	/// Start serving requests on hostAndPort.
	///
	/// @param[in]  hostAndPort     The hostname, or ipv4 address, and port.
	/// @return     True if successful.
	/// @remarks    If successful, returns after shutdown completes.
	bool Start(const std::string& hostAndPort);

	/// Start serving requests on hostAndPort.
	///
	/// @param[in]  builder The server builder.
	/// @return     True if successful.
	/// @remarks    If successful, returns after shutdown completes.
	bool Start(grpc::ServerBuilder& builder);

	/// Initiate shutdown. Typically called in a signal handler.
	/// @remarks Threadsafe
	void Shutdown();
	
	/// Blocks the calling thread until shutdown has completed.
	/// @param[in]  maxWaitTimeInSeconds A timeout.
	/// @return     True if the shutdown completed, false on timeout.
	bool BlockUntilShutdown(unsigned maxWaitTimeInSeconds=0);
};

}		// namespace lucida
#endif	// defined(SERVICE_ACCEPTOR_H_62678E0B_8CC9_49E4_BB77_70E6E3ED515C)
