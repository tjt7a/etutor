package ai.lucida.openephyra;

// Open Ephyra packages
import info.ephyra.OpenEphyra;
import info.ephyra.search.Result;
import info.ephyra.io.MsgPrinter;

// Java packages
import java.util.List;
import java.io.File;
import java.util.ArrayList;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.io.UnsupportedEncodingException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import static io.grpc.stub.ClientCalls.asyncUnaryCall;
import static io.grpc.stub.ClientCalls.asyncServerStreamingCall;
import static io.grpc.stub.ClientCalls.asyncClientStreamingCall;
import static io.grpc.stub.ClientCalls.asyncBidiStreamingCall;
import static io.grpc.stub.ClientCalls.blockingUnaryCall;
import static io.grpc.stub.ClientCalls.blockingServerStreamingCall;
import static io.grpc.stub.ClientCalls.futureUnaryCall;
import static io.grpc.MethodDescriptor.generateFullMethodName;
import static io.grpc.stub.ServerCalls.asyncUnaryCall;
import static io.grpc.stub.ServerCalls.asyncServerStreamingCall;
import static io.grpc.stub.ServerCalls.asyncClientStreamingCall;
import static io.grpc.stub.ServerCalls.asyncBidiStreamingCall;
import static io.grpc.stub.ServerCalls.asyncUnimplementedUnaryCall;
import static io.grpc.stub.ServerCalls.asyncUnimplementedStreamingCall;

import io.grpc.stub.StreamObserver;
import com.google.protobuf.Empty;
import com.google.protobuf.ByteString;

import ai.lucida.grpc.LucidaServiceGrpc;
import ai.lucida.grpc.Request;
import ai.lucida.grpc.Response;
import ai.lucida.grpc.QuerySpec;
import ai.lucida.grpc.QueryInput;
import ai.lucida.grpc.ServiceConnector;
import ai.lucida.grpc.ServiceNames;

/** Implementation of the question-answer interface defined
 * in the question-answer thrift file. A client request to any
 * method defined in the thrift file is handled by the
 * corresponding method here.
 */
public class QAServiceHandler extends LucidaServiceGrpc.LucidaServiceImplBase {
	private static final Logger logger = LoggerFactory.getLogger(QAServiceHandler.class);

	/** An object that lets the question-answer wrapper use
	 * the end-to-end OpenEphyra framework.
	 */
	private OpenEphyra oe;

	/**
	 * Default answer.
	 */
	private String default_answer;

	/**
	 * Since OpenEphyra is not thread-safe, we enforce the use to be single-threaded.
	 */
	private Lock infer_lock;

	/** Constructs the handler and initializes its OpenEphyra
	 * object.
	 */
	public QAServiceHandler() {
		String dir = "";
		MsgPrinter.enableStatusMsgs(true);
		MsgPrinter.enableErrorMsgs(true);
		// Initialize OE pipeline.
		oe = new OpenEphyra(dir);
		default_answer = "Factoid not found in knowledge base.";
		infer_lock = new ReentrantLock();
		// Create db directory.
		if (!new File("db").exists()) {
			new File("db").mkdir();
		}
	}

	@Override
	/** {@inheritDoc} */
	public void create(Request request, StreamObserver<Empty> responseObserver) {
		MsgPrinter.printStatusMsg("@@@@@ Create; User: " + request.getLUCID());
		responseObserver.onNext(Empty.newBuilder().build());
		responseObserver.onCompleted();
	}

	@Override
	/** {@inheritDoc} */
	public void learn(Request request, StreamObserver<Empty> responseObserver) {
		MsgPrinter.printStatusMsg("@@@@@ Learn; User: " + request.getLUCID());
		try {
			KnowledgeBase kb = KnowledgeBase.getKnowledgeBase(request.getLUCID());
			kb.addKnowledge(request.getSpec());

			responseObserver.onNext(Empty.newBuilder().build());
			responseObserver.onCompleted();

		} catch (Exception e) {
			//e.printStackTrace();
			responseObserver.onError(e);
		}
	}

	@Override
	/** {@inheritDoc} */
    public void infer(Request request, StreamObserver<Response> responseObserver) {
		MsgPrinter.printStatusMsg("@@@@@ Infer; User: " + request.getLUCID());
		
	    if (request.getSpec().getContentList().isEmpty() || 
                request.getSpec().getContentList().get(0).getDataList().isEmpty()) {
            logger.info("empty content passed to service");
	        throw new IllegalArgumentException();
	    }

		String answer = "";
		try {
			QueryInput queryInput = request.getSpec().getContentList().get(0);
			infer_lock.lock(); // limit concurrency because OE is not thread-safe
			// Set INDRI_INDEX.
			System.setProperty("INDRI_INDEX",
					KnowledgeBase.getKnowledgeBase(request.getLUCID()).getIndriIndex());
			// Only look for the first item in content and data.
			// The rest part of query is ignored.
			answer = askFactoidThrift(request.getLUCID(), queryInput.getDataList().get(0).toString("UTF-8"));
			MsgPrinter.printStatusMsg("Answer: " + answer);
			// Check if it needs to ask ENSEMBLE.
			if (answer.equals(default_answer) && queryInput.getTagsList().size() >= 2 &&
				queryInput.getTagsList().get(2).equals("1")) {

				String host = request.getSpec().getContentList().get(1).getTagsList().get(0);
				int port = Integer.parseInt(request.getSpec().getContentList().get(1).getTagsList().get(1));
				ServiceConnector ensemble = new ServiceConnector(host, port);

				String result = ensemble.infer(Request.newBuilder()
						.setLUCID(request.getLUCID())
						.setSpec(QuerySpec.newBuilder()
							.setName(ServiceNames.inferCommandName)
							.addContent(request.getSpec().getContentList().get(1))
							.build())
						.build());


				responseObserver.onNext(Response.newBuilder()
					.setMsg(result)
					.build());
				responseObserver.onCompleted();
				/*
				QuerySpec ENSEMBLE_spec = new QuerySpec();
				ENSEMBLE_spec.name = "query";
				ENSEMBLE_spec.content = new ArrayList<QueryInput>();
				ENSEMBLE_spec.content.add(query.content.get(1));
				String ENSEMBLE_addr = query.content.get(1).tags.get(0);
				int ENSEMBLE_port = Integer.parseInt(query.content.get(1).tags.get(1));
				TTransport transport = new TSocket(ENSEMBLE_addr, ENSEMBLE_port);
				TProtocol protocol = new TBinaryProtocol(new TFramedTransport(transport));
				LucidaService.Client client = new LucidaService.Client(protocol);
				MsgPrinter.printStatusMsg("Asking ENSEMBLE at " + ENSEMBLE_addr + " "
				+ ENSEMBLE_port);
				answer = client.infer(LUCID, ENSEMBLE_spec);
				*/
			}
			else
			{
				responseObserver.onNext(Response.newBuilder()
					.setMsg(answer)
					.build());
				responseObserver.onCompleted();
			}

        } catch (UnsupportedEncodingException e) {
            logger.info("non UTF-8 encoding passed to service");
            responseObserver.onError(e);
        } catch (Exception e) {
            logger.info("exception caught - {}", e.getMessage());
            responseObserver.onError(e);
		} finally {
			infer_lock.unlock(); // always unlock at the end
		}
	}

	/**
	 * Forwards the client's question to the OpenEphyra object's askFactoid
	 * method and collects the response.
	 * @param LUCID ID of Lucida user
	 * @param question eg. "what is the speed of light?"
	 * @throws Exception 
	 */
	private String askFactoidThrift(String LUCID, String question) throws Exception {
		MsgPrinter.printStatusMsg("askFactoidThrift(): Arg = " + question);
		Result result = oe.askFactoid(question);	
		String answer = default_answer;
		if (result != null) {
			answer = result.getAnswer();
		}
		// Check if Wikipedia Indri repository is pre-configured.
		String wiki_indri_index = System.getenv("wiki_indri_index");
		if (wiki_indri_index == null) {
			return answer;
		}
		// Set INDRI_INDEX.
		System.setProperty("INDRI_INDEX", wiki_indri_index);
		result = oe.askFactoid(question);
		if (result != null) {
			answer += " (from Wikipedia: ";
			answer += result.getAnswer();
			answer += ")";
		}
		return answer;
	}
}
