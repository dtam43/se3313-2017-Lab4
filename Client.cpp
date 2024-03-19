#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <cstdlib>
#include <time.h>
#include <string>

int main(void) {
	
	// Establish a connection with the server
	Sync::Socket sock("127.0.0.1",3000);
	std::string input = " ";
	bool terminated = false;

	// After connecting to the client
	if(sock.Open()){

		std::cout << "Client has connected with the server on PORT:3000" << std::endl;
		
		while(input != "done"){
		
			// Prompt the user for input
			std::cout << "Enter a message to send to the server, or type 'done' to disconnect: ";
			std::cin >> input;

			if (input != "done") {
				// Send the message to the server
				sock.Write(Sync::ByteArray(input));
				Sync::ByteArray response;

				// Get the response from the server
				int bytes = sock.Read(response);

				// If the server sends an empty packet (i.e. disconnects), then break the loop
				if (bytes == 0) {
					std::cout<<"The server has disconnected. Terminating..."<<std::endl;
					terminated = true;
					break;
				}

				std::cout<<"     Server response: "<< response.ToString()<<std::endl;
			} 
		}

		// Close the socket if the server hasn't already disconnected
		if (!terminated) {
			sock.Close();
			std::cout << "Successfully disconnected from the server." << std::endl;
		}	
	}

	return 0;
}