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
#ifndef REQUEST_BUILDER_H_B8B98AB4_D40C_465D_90CF_ABF5C3C1DFA8
#define REQUEST_BUILDER_H_B8B98AB4_D40C_465D_90CF_ABF5C3C1DFA8

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include <google/protobuf/arena.h>
#include <future>
#include <memory>
#include <string>
#include <list>
#include "generated/lucida_service.grpc.pb.h"
#include "generated/lucida_service.pb.h"


namespace lucida {

class RequestBuilder final
{
private:
	std::unique_ptr<::google::protobuf::Arena> arena_;
	std::promise<void> reset_promise_;
	std::shared_future<void> barrier_;
	std::list<std::shared_ptr<std::promise<void>>> waiters_;

	void CheckIteratorType(std::forward_iterator_tag) { }
	void CheckIteratorType(std::bidirectional_iterator_tag) { }
	void CheckIteratorType(std::random_access_iterator_tag) { }
	void CheckIteratorType(std::input_iterator_tag) { }
	void CheckIteratorPointerTypeIsQueryInput(QueryInput*) {}

public:
	RequestBuilder();
	RequestBuilder(const RequestBuilder&) = delete;
	RequestBuilder& operator = (const RequestBuilder&) = delete;

	// For synchronzing reset - call before running thread and save in thread.
	// Each thread calls WaitReset() to coordinate a reset.
	std::pair<std::shared_ptr<std::promise<void>>, std::shared_future<void>> AddResetWaiter();

	// Call this in threads not synchronizing the reset
	static bool WaitReset(std::pair<std::shared_ptr<std::promise<void>>, std::shared_future<void>>& pair, unsigned timeoutInSecs=0);
  
	// Call this in the thread synchronizing the reset
	void SyncReset(unsigned timeoutInSecs = 0);

	template<class T> T* New() {
		if (arena_.get() != nullptr)
			return ::google::protobuf::Arena::Create<T>(arena_.get());
		return new T();
	}

	/// Prepare a request for learn.
	///
	/// @param id        The LUCID.
	/// @param content   The data to learn from.
	/// @return The request. Deletion is done by the builder.
	Request* PrepareLearnRequest(const std::string& id); 

	/// Prepare a request for infer.
	///
	/// @param id        The LUCID.
	/// @param content   The data to learn from.
	/// @return The request. Deletion is done by the builder.
	Request* PrepareInferRequest(const std::string& id);

	/// Prepare a request for create.
	///
	/// @param id        The LUCID.
	/// @param content   The data to learn from.
	/// @return The request. Deletion is done by the builder.
	Request* BuildCreateRequest(const std::string& id);
};

}       // namespace lucida
#endif  // REQUEST_BUILDER_H_B8B98AB4_D40C_465D_90CF_ABF5C3C1DFA8
