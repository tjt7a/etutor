package ai.lucida.openephyra;

import ai.lucida.grpc.ServiceAcceptor;
import ai.lucida.qa.QAServiceHandler;

/**
 * Starts the question-answer server and listens for requests.
 */
public class QADaemon {
	/** 
	 * Entry point for question-answer.
	 * @param args the argument list. Provide port numbers
	 * for both sirius and qa.
	 */
	public static void main(String[] args) throws Exception {
		ServiceAcceptor server = new ServiceAcceptor(8084, new QAServiceHandler());
		server.start();
		System.out.println("QA at port 8084");
		server.blockUntilShutdown();
	}
}
