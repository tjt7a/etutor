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

#include <lucida/request_builder.h>
#include <lucida/service_names.h>
#include <chrono>

namespace lucida {

RequestBuilder::RequestBuilder(): arena_(new ::google::protobuf::Arena()),
	reset_promise_(), barrier_(reset_promise_.get_future()) {
}

// For synchronzing reset - call before running thread and save in thread.
// Each thread calls WaitReset() to coordinate a reset.
std::pair<std::shared_ptr<std::promise<void>>, std::shared_future<void>> RequestBuilder::AddResetWaiter() {
	std::shared_ptr<std::promise<void>> p(new std::promise<void>());
	waiters_.push_back(p);
	return std::make_pair(p, barrier_);
}

// Call this in threads not synchronizing the reset
bool RequestBuilder::WaitReset(std::pair<std::shared_ptr<std::promise<void>>, std::shared_future<void>>& pair, unsigned timeoutInSecs) {
	pair.first->set_value();
	if (!pair.second.valid())
		throw std::future_error(std::future_errc::no_state);
	if (0 == timeoutInSecs) {
		pair.second.wait();
		return true;
	}
	return std::future_status::ready == pair.second.wait_for(std::chrono::seconds(timeoutInSecs));
}

// Call this in the thread synchronizing the reset
void RequestBuilder::SyncReset(unsigned timeoutInSecs) {
	if (waiters_.empty()) {
		arena_->Reset();
		return;
	}

	if (timeoutInSecs) {
		// FIXME: Handle this case
		for (auto it = waiters_.begin(); it != waiters_.end(); ++it)
			(*it)->get_future().wait();
	} else {
		// FIXME: timeout is for total time.
		auto timestamp = std::chrono::system_clock::now();
		for (auto it = waiters_.begin(); it != waiters_.end() && timeoutInSecs; ++it) {
			(*it)->get_future().wait();
		}
	}
	arena_->Reset();
	barrier_.wait();
}

Request* RequestBuilder::PrepareLearnRequest(const std::string& id) {
	Request* request = New<Request>();
	request->set_lucid(id);
	QuerySpec* spec = New<QuerySpec>();
	spec->set_name(ServiceNames::learnCommandName);
	request->set_allocated_spec(spec);
	return request;
}

Request* RequestBuilder::PrepareInferRequest(const std::string& id) {
	Request* request = New<Request>();
	request->set_lucid(id);
	QuerySpec* spec = New<QuerySpec>();
	spec->set_name(ServiceNames::inferCommandName);
	request->set_allocated_spec(spec);
	return request;
}

Request* RequestBuilder::BuildCreateRequest(const std::string& id) {
	Request* request = New<Request>();
	request->set_lucid(id);
	QuerySpec* spec = New<QuerySpec>();
	spec->set_name(ServiceNames::createCommandName);
	request->set_allocated_spec(spec);
	return request;
}

} // namespace lucida

