package ai.lucida.openephyra;

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

import ai.lucida.openephyra.QAServiceHandler;

import com.google.protobuf.ByteString;

import static org.junit.Assert.*;
import org.junit.Before;
import org.junit.Test; 

/*
 * A testing Client that sends a single query to OpenEphyra Server and prints the results.
 */
public class QADaemonTest {
	/*
	 * Creates a QueryInput.
	 */
	private static final QueryInput createQueryInput(
			final String type,
			final String data,
			final String tag) throws UnsupportedEncodingException {
		return QueryInput.newBuilder()
				.setType(type)
				.addAllData(Collections.singletonList(ByteString.copyFrom(data, "UTF-8")))
				.addAllTags(Collections.singletonList(tag))
				.build();
	}

	/*
	 * Creates a QuerySpec.
	 */
	private static final QuerySpec createQuerySpec(
			String name,
			List<QueryInput> query_input_list) {
		return QuerySpec.newBuilder()
				.setName(name)
				.addAllContent(query_input_list)
				.build();
	}

	@Test
	public void testClientServer() {
		// Collect the port number.
		int port = 8083;

		// User.
		String LUCID = "Clinc";
		ServiceAcceptor server = null;
		try {
			// Knowledge.
			QueryInput knowledge_text = createQueryInput("text", "Clinc is created by Jason and Lingjia.", "1234567");
			QueryInput knowledge_url = createQueryInput("url", "https://en.wikipedia.org/wiki/Apple_Inc.", "abcdefg");
			ArrayList<QueryInput> knowledge = new ArrayList<QueryInput>() {{
				add(knowledge_text);
				add(knowledge_url);
			}};
			// Unlearn.
			QueryInput knowledge_unlearn_input = createQueryInput("unlearn", "", "abcdefg");
			QuerySpec knowledge_unlearn_spec = createQuerySpec(
					"unlearn knowledge",
					new ArrayList<QueryInput>() {{
						add(knowledge_unlearn_input);
					}});

			// Query.
			QueryInput query_input = createQueryInput("text", "Who created Clinc?", "");
			ArrayList<QueryInput> query = new ArrayList<QueryInput>() {{
				add(query_input);
			}};

			server = new ServiceAcceptor(port, new QAServiceHandler());
			server.start();
			System.out.println("QA at port " + port);

			ServiceConnector client = new ServiceConnector("localhost", port);
			Request request;

			client.create(LUCID);

			// Learn knowledge
			System.out.println("///// Learn knowledge /////");
			request = client.buildLearnRequest(LUCID, knowledge);
			client.learn(request);

			System.out.println("///// Infer: /////");
			request = client.buildInferRequest(LUCID, "text", "Who created Clinc");
			// Print the question
			System.out.println(request.getSpec().getContent(0).getData(0).toString("UTF-8"));
			String answer = client.infer(request);
			// Print the answer.
			System.out.println(answer);
			assertTrue(answer == "Clinc is created by Jason and Lingjia.");

			// Unlearn and ask again.
			System.out.println("///// Unlearn knowledge /////");
			request = client.buildRequest(LUCID, knowledge_unlearn_spec);
			System.out.println(request.getSpec().getContent(0).getTags(0));
			client.learn(request);

			System.out.println("///// Infer: /////");
			request = client.buildInferRequest(LUCID, "text", "Who created Clinc");
			System.out.println(request.getSpec().getContent(0).getData(0).toString("UTF-8"));
			answer = client.infer(request);
			// Print the answer.
			System.out.println("///// Answer: /////");
			assertTrue(answer == "");
			System.out.println(answer);

			client.shutdown(10);
		} catch (Exception e) {
			fail(e.getMessage());
		}

		if (server != null)
			server.stop();
	}
}
