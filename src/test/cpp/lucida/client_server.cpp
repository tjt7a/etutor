#include <sstream>
#include <csignal>
#include "handler.h"
#include <thread>
#include <future>
#include <chrono>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <lucida/request_builder.h>
#include <lucida/service_connector.h>
#include <gtest/gtest.h>
#include "handler.h"

DEFINE_int32(port,
			 9000,
			 "Port for test (default: 9000)");

DEFINE_int32(threads,
			 4,
			 "Number of threads (default: 4)");

using namespace lucida;
namespace lucida { namespace test {


TEST(LucidaTest, SimpleSyncClientSyncServer) {
	// Prep
	std::ostringstream os;
	os << "localhost:"<< FLAGS_port;
	std::shared_ptr<ServiceAcceptor> server(new ServiceAcceptor(new TestSyncHandler(), "testserver"));
	std::string hostandport = os.str();
	// Start receiving RPC's
	std::thread svr_thread( [hostandport, server]() {
		server->Start(hostandport);
	});

	AsyncServiceConnector client(hostandport.c_str());

	Request  req;
	Response resp;
	auto status = client.infer(req, resp);
	//ASSERT_EQ(status, ::grpc::Status::OK);
	EXPECT_EQ(resp.msg(), "got infer");

	// Wait until shutdown
	server->Shutdown();
	EXPECT_TRUE(server->BlockUntilShutdown(5));
	// Can't shutdown twice
	EXPECT_FALSE(server->BlockUntilShutdown(5));
	svr_thread.join();
}


TEST(LucidaTest, SimpleAsyncClientSyncServer) {
	// Prep
	std::ostringstream os;
	os << "localhost:"<< FLAGS_port;
	std::shared_ptr<ServiceAcceptor> server(new ServiceAcceptor(new TestSyncHandler(), "testserver"));
	std::string hostandport = os.str();
	// Start receiving RPC's
	std::thread svr_thread( [hostandport, server]() {
		server->Start(hostandport);
	});

	AsyncServiceConnector client(hostandport.c_str());
	client.Start();

	Request  req;
	Response* resp = nullptr;
	auto rpc = client.inferAsync(req);
	ASSERT_NE(rpc.get(), nullptr);
	EXPECT_TRUE(rpc->Wait(3));
	EXPECT_TRUE(rpc->Get(resp));
	ASSERT_NE(resp, nullptr);
	EXPECT_EQ(resp->msg(), "got infer");

	// Wait until shutdown
	server->Shutdown();
	EXPECT_TRUE(server->BlockUntilShutdown(5));
	// Can't shutdown twice
	EXPECT_FALSE(server->BlockUntilShutdown(5));
	svr_thread.join();
}
  

TEST(LucidaTest, SimpleSyncClientAsyncServer) {
	// Prep
	std::ostringstream os;
	os << "localhost:"<< FLAGS_port;
	std::shared_ptr<AsyncServiceAcceptor> server(new AsyncServiceAcceptor(new TestAsyncHandler(), "testserver"));
	std::string hostandport = os.str();
	// Start receiving RPC's
	std::thread svr_thread( [hostandport, server]() {
		server->Start(hostandport, 1);
	});

	AsyncServiceConnector client(hostandport.c_str());

	Request  req;
	Response resp;
	auto status = client.infer(req, resp);
	//ASSERT_EQ(status, ::grpc::Status::OK);
	EXPECT_EQ(resp.msg(), "got infer");

	// Wait until shutdown
	server->Shutdown();
	EXPECT_TRUE(server->BlockUntilShutdown(5));
	// Can't shutdown twice
	EXPECT_FALSE(server->BlockUntilShutdown(5));
	svr_thread.join();
}


TEST(LucidaTest, SimpleAsyncClientAsyncServer) {
	// Prep
	std::ostringstream os;
	os << "localhost:"<< FLAGS_port;
	std::shared_ptr<AsyncServiceAcceptor> server(new AsyncServiceAcceptor(new TestAsyncHandler(), "testserver"));
	std::string hostandport = os.str();
	// Start receiving RPC's
	
	std::thread svr_thread( [hostandport, server]() {
		server->Start(hostandport, 1);
	});
	
	AsyncServiceConnector client(hostandport.c_str());
	client.Start();

	Request  req;
	Response* resp = nullptr;
	auto rpc = client.inferAsync(req);
	ASSERT_NE(rpc.get(), nullptr);
	EXPECT_TRUE(rpc->Wait(3));
	EXPECT_TRUE(rpc->Get(resp));
	ASSERT_NE(resp, nullptr);
	EXPECT_EQ(resp->msg(), "got infer");

	// Wait until shutdown
	server->Shutdown();
	EXPECT_TRUE(server->BlockUntilShutdown(5));
	svr_thread.join();
	// Can't shutdown twice
	//EXPECT_FALSE(server->BlockUntilShutdown(5));
}

} } // namespace lucida::test


