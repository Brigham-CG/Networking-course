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
#include <string>
#include <iostream>

int SocketFD;

void ReceiveMessages() {

    char buffer[256];
    int n;

    while (1) {
        n = recv(SocketFD, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            printf("[+] Disconnected from server.\n");
            break;
        }
        
        buffer[n] = '\0';

        std::cout <<buffer <<  std::endl;
    }
}

int main(int argc, char *argv[]) {
    struct sockaddr_in stSockAddr;

    if ((SocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[!] Error to create socket");
        exit(1);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(atoi(argv[2])); // El puerto se pasa como segundo argumento
    stSockAddr.sin_addr.s_addr = inet_addr(argv[1]); // La dirección se pasa como primer argumento

    if (connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)) == -1) {
        perror("[!] Error connecting to server");
        close(SocketFD);
        exit(1);
    }

    char buffer[256];
    int n;

    // Recibir el mensaje para establecer el nickname
    n = recv(SocketFD, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
        perror("[!] Error receiving nickname message");
        close(SocketFD);
        exit(1);
    }
    buffer[n] = '\0';
    printf("%s", buffer);

    // Leer el nickname ingresado por el usuario
    std::string nickname;
    std::getline(std::cin, nickname);

    // Enviar el nickname al servidor
    send(SocketFD, nickname.c_str(), nickname.length(), 0);

    // Iniciar un hilo para recibir mensajes del servidor
    std::thread(ReceiveMessages).detach();

    // Leer mensajes del usuario y enviarlos al servidor
    while (1) {
        std::string message;
        std::getline(std::cin, message);

        // Enviar el mensaje al servidor
        send(SocketFD, message.c_str(), message.length(), 0);
    }

    close(SocketFD);

    return 0;
}
