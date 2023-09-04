#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <thread>
#include <map>
#include <string>
#include <iostream>

std::map<int, std::string> clientNicknames;

void HandleClient(int clientSocket) {
    char buffer[256];
    int n;

    // Pedir al cliente que establezca su nickname
    send(clientSocket, "Please enter your nickname: ", 27, 0);

    n = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        close(clientSocket);
        return;
    }

    buffer[n] = '\0';

    std::string nickname = buffer;

    clientNicknames[clientSocket] = nickname;
    
    std::cout <<"[+] Client "<< nickname <<"connected.\n";

    while (1) {
        n = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            std::cout << "[+] Client " << nickname << " disconnected.\n";
            break;
        }
        buffer[n] = '\0';

        // Parsear el mensaje para extraer el destinatario y el contenido
        std::string message = buffer;
        size_t pos = message.find(",");
        std::cout << "message:" << message << std::endl;
        std::cout <<"n: "<< n << std::endl;

        if (pos != std::string::npos) {
            std::string recipient = message.substr(0, pos);
            std::string content = message.substr(pos + 2); // +2 para omitir la coma y el espacio

            // Buscar el socket del destinatario por su nickname
            

            for (auto& pair : clientNicknames) {
                if (pair.second == recipient) {
                    int recipientSocket = pair.first;
                    send(recipientSocket, (nickname + ": " + content).c_str(), nickname.length() + 2 + content.length() , 0);
                    break;
                }
            }
        }
    }

    // Cerrar el socket del cliente y eliminarlo del mapa
    close(clientSocket);
    clientNicknames.erase(clientSocket);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in stSockAddr;
    socklen_t client;
    int SocketFD;

    if ((SocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[!] Error to create socket");
        exit(1);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(atoi(argv[1]));
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(struct sockaddr)) == -1) {
        perror("[!] Error to binding");
        exit(1);
    }

    if (listen(SocketFD, 1) == -1) {
        perror("[!] Error in listen");
        exit(1);
    }

    while (1) {
        client = sizeof(struct sockaddr_in);

        int ConnectFD = accept(SocketFD, nullptr, &client);
        if (ConnectFD == -1) {
            perror("[!] Error accepting connection");
            continue;
        }

        // Crear un nuevo hilo para manejar al cliente
        std::thread(HandleClient, ConnectFD).detach();
    }

    close(SocketFD);

    return 0;
}
