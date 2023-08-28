#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    int n;
    char buffer[256];

    if (-1 == SocketFD) {
        perror("[!] Error to create socket");
        exit(1);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(atoi(argv[2]));

    Res = inet_pton(AF_INET, argv[1], &stSockAddr.sin_addr);

    if (0 > Res) {
        perror("[!] Error first parameter is not a valid address family");
        close(SocketFD);
        exit(1);
    } else if (0 == Res) {
        perror("[!] Invalid ip address");
        close(SocketFD);
        exit(1);
    }

    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) {
        perror("[!] Connection failed");
        close(SocketFD);
        exit(1);
    }

    while (1) {

        printf("Enter a message ('END' to close connection): ");
        fgets(buffer, sizeof(buffer), stdin);

        buffer[strcspn(buffer, "\n")] = '\0'; 

        if (strcmp(buffer, "END") == 0) {
            printf("[+] Closing connection.\n");
            send(SocketFD, buffer, strlen(buffer), 0);
            break;
        }

        n = send(SocketFD, buffer, strlen(buffer), 0);

        bzero(buffer, 256);

        n = recv(SocketFD, buffer, sizeof(buffer) - 1, 0);
        if (n < 0) {
            perror("ERROR reading from socket");
            break;
        }
        buffer[n] = '\0';  
        printf("Server: '%s'\n", buffer);
    }

    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);

    return 0;
}
