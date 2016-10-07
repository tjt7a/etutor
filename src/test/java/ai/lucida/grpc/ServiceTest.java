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

package ai.lucida.grpc;

//Java packages
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;
import java.io.UnsupportedEncodingException;

import ai.lucida.grpc.ServiceAcceptor;
import ai.lucida.grpc.ServiceConnector;
import ai.lucida.grpc.Request;
import ai.lucida.grpc.Response;
import ai.lucida.grpc.QuerySpec;
import ai.lucida.grpc.QueryInput;
import ai.lucida.grpc.Request;

import com.google.protobuf.ByteString;
import io.grpc.stub.StreamObserver;

import static org.junit.Assert.*;
import org.junit.Before;
import org.junit.Test; 

public class ServiceTest {
	public class SyncHandler extends LucidaServiceGrpc.LucidaServiceImplBase  {
		@Override
		public void infer(Request request, StreamObserver<Response> responseObserver) {
		}
	}

	@Test
	public void testSyncClientSyncServer() {

		try {
			ServiceAcceptor server = new ServiceAcceptor(9001, new SyncHandler());
			server.stop();
		} catch(Exception e) {
			fail(e.getMessage());
		}
	}

	public void testAsyncClientSyncServer() {
	}

	public void testSyncClientAsyncServer() {
	}

	public void testAsyncClientAsyncServer() {
	}
}
