#pragma once

#include <lucida/service_acceptor.h>

namespace lucida { namespace test {

class TestAsyncHandler : public AsyncServiceHandler {
public:
	TestAsyncHandler();
private:
	void OnCreate(TypedCall<Request, ::google::protobuf::Empty>* call) override;
	void OnLearn(TypedCall<Request, ::google::protobuf::Empty>* call) override;
	void OnInfer(TypedCall<Request, Response>* call) override;
};

class TestSyncHandler : public LucidaService::Service {
public:
	TestSyncHandler();
private:
	::grpc::Status infer(::grpc::ServerContext* context, const ::lucida::Request* request, ::lucida::Response* response) override;
};
} } // namespace lucida::test

