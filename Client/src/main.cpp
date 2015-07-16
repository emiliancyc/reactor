#include "Socket.hpp"

#include <iostream>

#include <boost/lexical_cast.hpp>

int main(int argc, char* argv[])
{
	int port = 5050;
	string ip = "127.0.0.1";

	if (argc == 2) {
		try {
			port = lexical_cast<int>(argv[1]);
		} catch (const bad_cast& e) {
			cout << "Invalid port \'" << argv[1] << "\' server will run on port 5050" << endl;
		}
	}

	if (argc == 3) {
		try {
			port = lexical_cast<int>(argv[1]);
		} catch (const bad_cast& e) {
			cout << "Invalid port \'" << argv[1] << "\' server will run on port 5050" << endl;
		}

		ip = argv[2];
	}

	try {
		Socket s;
		s.Connect(ip, port);

		while (1) {
			string msgTo, msgFrom;
			cout << "Type message structure (exit to quit)" << endl;
			cin >> msgTo;
			if (msgTo == "exit") break;
			s.Write(msgTo);
			s.Read(msgFrom);
			cout << "Received: \"" << msgFrom << "\"" << endl;
		}
	} catch (const runtime_error& e) {
		cout << "ERROR: " << e.what() << endl;
	}

	return 0;
}

