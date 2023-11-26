#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <thread>

#include <string>
#include <iostream>

#include "client_libraries/client_send_req.cpp"
#include "client_libraries/client_recieve_res.cpp"
#include "client_libraries/space_game.cpp"

// --> Writte
void handle_commands(std::string nickname, int socketFD, struct sockaddr_in server_protocols)
{
   std::cout << "type 'help' to view options\n\n";
   while (true)
   {
      std::string command;

      std::cout << nickname <<" #";

      std::getline(std::cin, command);

      if(command == "help")
      {
         std::cout << "list --------------> list users in server\n";
         std::cout << "all, message ------> send message to all users in server\n";
         std::cout << "nickname, message -> send message to specific user\n";
         std::cout << "quit --------------> exit of service\n\n";
         std::cout << "\n            Space game\n\n";
         std::cout << "space create ---> create a space game\n";
         std::cout << "space list -----> view the list of parties\n";
         std::cout << "space join -----> join to space party\n";
      }
      else if (command == "list")
         reqListName(socketFD, server_protocols, nickname);
      else if (command == "quit")
      {
         quitServer(socketFD, server_protocols, nickname);
         break;
      }
      else if (command == "space create")
         createSpaceGame(socketFD, server_protocols, nickname);
      else if (command == "space list")
         viewListParties(socketFD, server_protocols, nickname);
      else if (command.substr(0, 11) == "space join ")
      {
         if(!game_created)
         {
            joinSpaceParty(command.substr(14, command.size()), socketFD, server_protocols, nickname);
         }
         else{
            std::cout << "\n [!] You has been created a game, you cant join!\n"; 
         }
      }
      else if(command.substr(0, 6) =="play, ")
      {
         if(in_game)
            sendYankenpoPlay(command.substr(6, command.size()), socketFD, server_protocols, nickname);
         else 
            std::cout << " \n [!] The game has not started yet\n";
      }
      else
         sendMessage(command, socketFD, server_protocols, nickname);
   }
}

void ReceiveMessages(std::string nickname, int socketFD, struct sockaddr_in server_protocols) {

   char buffer[1025];
   int nBytes;

   while (true) {
        
      nBytes = recvfrom(socketFD, buffer, 1024,
                        0,
                        (struct sockaddr *)&server_protocols, &addr_len);

      if (nBytes <=0) {
         std::cout << "[+] Disconnected from server.\n" ;
         break;
      }
      buffer[nBytes] = '\0'; 
      
      std::string message = buffer;
      
      if(buffer[0] == 'L')             // --> List_Users
         getListUsers(message);
      else if(buffer[0] == 'M')        // --> Read Message
         obtainingMessage(message);
      else if(buffer[0] == 'Y')        // --> Get Room ID
         getIdParty(message);
      else if(buffer[0] == 'G')        // --> Get List of Parties
         getListParties(message);
      else if(buffer[0] == 'P')        // --> Start Game
         startingPlay(message);
      else if(buffer[0] == 'R')
         getResult(nickname, message);
   }
}

void stablish_initial_socket_config(struct sockaddr_in &server_protocols, std::string IPAddress, std::string port)
{
   server_protocols.sin_family = AF_INET; // familia de protocolos ip 
   server_protocols.sin_port = htons(atoi(port.c_str()));
   server_protocols.sin_addr.s_addr = inet_addr(IPAddress.c_str());
   bzero(&(server_protocols.sin_zero),8);
}

int main(int argc, char *argv[])
{
   if(argc != 3)
   {
      std::cout << "[!] Debes de pasar la 'direccion ip' y el 'numero del puerto' del server como parametros\n";
      std::cout << "[!] Ejemplo: ./client 127.0.0.1 54001\n";
      exit(1);
   }

   std::string serverIPAddress = argv[1];
   std::string serverPort = argv[2]; 

   int socketFD;
   struct sockaddr_in server_protocols;

   std::string send_data;

   if ((socketFD = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
   {
      perror("[!] Error to create socket");
      exit(1);
   }

   stablish_initial_socket_config(server_protocols, serverIPAddress, serverPort);
   
   std::string nickname = sendInitNotification(socketFD, server_protocols);

   // Iniciar un hilo para recibir mensajes del servidor
   std::thread(ReceiveMessages, nickname, socketFD, server_protocols).detach();

   handle_commands(nickname, socketFD, server_protocols);

   close(socketFD);
}