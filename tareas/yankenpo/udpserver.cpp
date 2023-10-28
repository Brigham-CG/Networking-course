#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <functional>

#include "server_libraries/options_protocol.cpp"
#include "server_libraries/yankenpo.cpp"

#include <thread>
#include <map>

socklen_t addr_len; // optional

std::map<std::string, struct sockaddr_in> nicknameToIPPort;

void handle_new_clients(int socketFD)
{   
    char buffer[1025];

    std::string message ;

    message.resize(1025);

    int nBytes;
    struct sockaddr_in client_addr;

    while(true)
    {
        nBytes = recvfrom(socketFD, buffer, 1024,
        MSG_WAITALL,
        (struct sockaddr *)&client_addr, &addr_len);

        buffer[1024] = '\0';

        message = buffer;

        if (message[0] == 'N')
        {
            std::thread(initNotification, client_addr, std::ref(nicknameToIPPort), message).detach();
        }
        else if (message[0] == 'L')
        {
            std::thread(sendListUsers, socketFD, client_addr, std::ref(nicknameToIPPort), message).detach();
        }
        else if (message[0] == 'M')
        {
            std::thread(forwardMessage, socketFD, client_addr, std::ref(nicknameToIPPort), message).detach();
        }
        else if (message[0] == 'W')
        {
            std::thread(forwardDiffusionMessage, socketFD, client_addr, std::ref(nicknameToIPPort), message).detach();
        }
        else if (message[0] == 'Q')
        {
            std::thread(closeSession, socketFD, client_addr, std::ref(nicknameToIPPort), message).detach();
        }
        // yankenpo game
        else if (message[0] == 'Y')
        {
            std::thread(createYankenpoParty, socketFD, client_addr, std::ref(nicknameToIPPort), message).detach();
        }
        else if (message[0] == 'G')
        {
            std::thread(sendListGames, socketFD, client_addr, std::ref(nicknameToIPPort), message).detach();
        }
        else if (message[0] == 'J')
        {
            std::thread(joinToParty, socketFD, client_addr, std::ref(nicknameToIPPort), message).detach();
        }
        else if (message[0] == 'P')
        {
            std::thread(receivePlay, socketFD, client_addr, std::ref(nicknameToIPPort), message).detach();
        }
    }
}

void stablish_initial_socket_config(struct sockaddr_in &protocols, std::string port)
{
   protocols.sin_family = AF_INET; // familia de protocolos ip 
   protocols.sin_port = htons(atoi(port.c_str()));
   protocols.sin_addr.s_addr = INADDR_ANY;
   bzero(&(protocols.sin_zero),8);
}

int main(int argc, char *argv[])
{

    if(argc != 2)
    {
        std::cout << "[!] Debes de pasar el 'numero del puerto' como parametro\n";
        std::cout << "[!] Ejemplo: ./server 54001\n";
        exit(1);
    }

    std::string serverPort = argv[1]; 

    int socketFD;

    struct sockaddr_in server_addr;

    if ((socketFD = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        std::cout << "[!] Error to create socket\n";
        exit(1);
    }

    stablish_initial_socket_config(server_addr, serverPort);

    if (bind(socketFD,(struct sockaddr *)&server_addr,
        sizeof(struct sockaddr)) == -1)
    {
        std::cout << "[!] Error to binding\n";
        exit(1);
    }

    addr_len = sizeof(struct sockaddr);
		
	std::cout << "[+] Server started\n";

	handle_new_clients(socketFD);

    close(socketFD);

    return 0;
}