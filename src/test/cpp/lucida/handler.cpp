#include "handler.h" 

namespace lucida { namespace test {

///////////////////////////////////////////////////////////////////////////////
// Async

TestAsyncHandler::TestAsyncHandler() {
}

void TestAsyncHandler::OnCreate(TypedCall<Request, ::google::protobuf::Empty>* call) {
	call->Finish();
}

void TestAsyncHandler::OnLearn(TypedCall<Request, ::google::protobuf::Empty>* call) {
	call->Finish();
}

void TestAsyncHandler::OnInfer(TypedCall<Request, Response>* call) {
	call->response_.set_msg("got infer");
}

///////////////////////////////////////////////////////////////////////////////
// Sync

TestSyncHandler::TestSyncHandler() {
}

::grpc::Status TestSyncHandler::infer(::grpc::ServerContext* context, const ::lucida::Request* request, ::lucida::Response* response) {
	response->set_msg("got infer");
	return ::grpc::Status::OK;
}

} } // namespace lucida::test

