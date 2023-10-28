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
#include "client_libraries/yankenpo.cpp"


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
         std::cout << "\n            Yankenpo game\n\n";
         std::cout << "yankenpo create ---> create a yankenpo game\n";
         std::cout << "yankenpo list -----> view the list of parties\n";
         std::cout << "yankenpo join -----> join to yankenpo party\n";
         std::cout << "play, {play} ------> play of yankenpo {piedra | papel | tijera}\n\n";
      }
      else if (command == "list")
         reqListName(socketFD, server_protocols, nickname);
      else if (command == "quit")
      {
         quitServer(socketFD, server_protocols, nickname);
         break;
      }
      else if (command == "yankenpo create")
         createYankenpoGame(socketFD, server_protocols, nickname);
      else if (command == "yankenpo list")
         viewListParties(socketFD, server_protocols, nickname);
      else if (command.substr(0, 14) == "yankenpo join ")
      {
         if(!game_created)
         {
            joinYankenpoParty(command.substr(14, command.size()), socketFD, server_protocols, nickname);
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
      
      if(buffer[0] == 'L')
         getListUsers(message);
      else if(buffer[0] == 'M')
         obtainingMessage(message);
      else if(buffer[0] == 'Y')
         getIdParty(message);
      else if(buffer[0] == 'G')
         getListParties(message);
      else if(buffer[0] == 'P')
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