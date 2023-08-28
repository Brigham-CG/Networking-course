#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]) {
    struct sockaddr_in stSockAddr;
    struct sockaddr_in cli_addr;
    int client;
    int SocketFD;
    char buffer[256];
    int n;

    if ((SocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[!] Error to create socket");
        exit(1);
    }

    if (setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1) {
        perror("[!] Failed to configure socket");
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

        int ConnectFD = accept(SocketFD, (struct sockaddr *)&cli_addr, &client);
        
        while (1) {
            n = recv(ConnectFD, buffer, sizeof(buffer) - 1, 0);
            if (n < 0) {
                perror("[!] ERROR reading from socket");
                break;  // Salir del bucle interno en caso de error
            }
            if (n == 0) {
                printf("[+] Client disconnected.\n");
                break;  // Salir del bucle interno si el cliente se desconecta
            }
            buffer[n] = '\0';  // Asegurarse de terminar el buffer

            if (strcmp(buffer, "END") == 0) {
                printf("[+] Client requested to end the connection.\n");
                break;  // Salir del bucle interno si el cliente envía 'END'
            }

            printf("client: '%s'\n", buffer);

            n = send(ConnectFD, "I got your message", 18, 0);
        }

        close(ConnectFD);
    }

    close(SocketFD);

    return 0;
}
