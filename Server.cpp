#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <iostream>
#include <cstring>
#include <stack>
#include "socket.h"
#include <csignal>

// Class to handle the sockets for each client connection
class SocketThread: public Thread{
    public:
        bool isConnected;
        int id;
        bool endServer = false; 
        Sync::Socket& socket;

        ~SocketThread(){}
   
        SocketThread(Sync::Socket& socket, int id): socket(socket) {
            this->id = id;
            this->isConnected = true;
        }

        Sync::Socket& GetSocket(){
            return socket;
        }

    virtual long ThreadMain(void) override{
        Sync::ByteArray message;

        // Socket will persist until server terminates or client disconnects
        while(!endServer){

            // Get the message in bytes from the client
            int bytes = socket.Read(message);
            
            // If client sends empty packet (i.e. disconnects), then break the loop
            if (bytes == 0) {
                isConnected = false;

                std::cout<<"Client " << id << " has disconnected."<<std::endl;
                break;
            }

            // Re-check if server has terminated after waiting for client message
            if (!endServer) {
                std::cout << "     Client " << id << ": " << message.ToString()<<std::endl;

                // Modify the message (set to uppercase)
                std::string newMessage = message.ToString();
                for (auto & c: newMessage) c = toupper(c);
                std::cout << "     Sending back message: " << newMessage<<std::endl;

                // Send the modified message back to the client
                socket.Write(newMessage);
            }
        }

        // Close the socket
        socket.Close();
        return 0;
    }
};

// Class to handle the server's main thread
class ServerThread : public Thread{
    public:
        Sync::SocketServer& serSock;
        ServerThread(Sync::SocketServer& serSock) : serSock(serSock){}
        std::stack<SocketThread*> tStack;

        bool endServer = false;
        int clientCount = 0;
        
    // Destructor for server thread that terminates all current socket threads
    ~ServerThread() {
        while(!tStack.empty()){
            SocketThread* finishedThread = tStack.top();

            Sync::Socket& socket = finishedThread->GetSocket();
            socket.Close();

            finishedThread -> endServer = true;
            tStack.pop();
        }
    }

    virtual long ThreadMain() {
        while(!endServer){
            try {
                // Wait for a client to connect
                Sync::Socket* newSocket = new Sync::Socket(serSock.Accept());
                std::cout << "Connection established with Client " << clientCount << "." << std::endl;

                // Create a new socket thread for the client and add it to the stack
                Sync::Socket& socketReference = *newSocket;
                tStack.push(new SocketThread(socketReference, clientCount++));
            
            // In the event of an error, terminate the server
            } catch(...) {
                endServer = true;
            }
        }
    }
};

Sync::SocketServer* pSocketServer = nullptr;

int main(void) {
	
    //Start the sever  
    Sync::SocketServer serSock(3000);
    pSocketServer = &serSock;

    std::cout << "Server has started and is listening on PORT:3000." << std::endl;
    std::cout << "To terminate the server, press the RETURN key." << std::endl;
    std::cout.flush();
    
    // Create the main server thread
    ServerThread serverThread(serSock);

    // Wait for the user to press the RETURN key 
    Sync::FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    std::cin.get();

    // Terminate the server
    serSock.Shutdown();
    std::cout << "Server has terminated gracefully." << std::endl;

    return 0;
}