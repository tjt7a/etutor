package lucida.test;

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

import ai.lucida.qa.QAServiceHandler;

import com.google.protobuf.ByteString;

/** 
 * A testing Client that sends a single query to OpenEphyra Server and prints the results.
 */
public class QAClient {
	/**
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

	/**
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

	public static void main(String [] args) {
		// Collect the port number.
		int port = 8083;

		if (args.length >= 1) {
			port = Integer.parseInt(args[0]);
		}

		try {
			// User.
			String LUCID = "Clinc";

			// Knowledge.
			final QueryInput knowledge_text = createQueryInput( 
					"text",
					"Clinc is created by Jason and Lingjia.",
					"1234567");
			final QueryInput knowledge_url = createQueryInput(
					"url",
					"https://en.wikipedia.org/wiki/Apple_Inc.",
					"abcdefg");	
			final ArrayList<QueryInput> knowledge = new ArrayList<QueryInput>() {{
						add(knowledge_text);
						add(knowledge_url);
					}};
			// Unlearn.
			final QueryInput knowledge_unlearn_input = createQueryInput(
					"unlearn",
					"",
					"abcdefg");
			final QuerySpec knowledge_unlearn_spec = createQuerySpec(
					"unlearn knowledge",
					new ArrayList<QueryInput>() {{
						add(knowledge_unlearn_input);
					}});

			// Query.
			final QueryInput query_input = createQueryInput(
					"text",
					"Who created Clinc?",
					"");
			final ArrayList<QueryInput> query = new ArrayList<QueryInput>() {{
						add(query_input);
					}};
			/*
			final QuerySpec query = createQuerySpec(
					"query",
					new ArrayList<QueryInput>() {{
						add(query_input);
					}});
			*/

			ServiceAcceptor server = new ServiceAcceptor(port, new QAServiceHandler());
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
			System.out.println("///// Answer: /////");
			System.out.println(answer);

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
			System.out.println(answer);

			client.shutdown(10);
			server.stop();

		} catch (Exception x) {
			x.printStackTrace();
		}
	}
}
