#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main() {
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error al crear el socket." << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // Puerto del servidor
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Dirección IP del servidor

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error al conectar al servidor." << std::endl;
        return 1;
    }

    std::ifstream file("wallpaper.jpg", std::ios::binary);
    if (!file) {
        std::cerr << "Error al abrir el archivo." << std::endl;
        return 1;
    }

    // Obtener el tamaño del archivo
    struct stat file_stat;
    if (stat("wallpaper.jpg", &file_stat) == -1) {
        std::cerr << "Error al obtener el tamaño del archivo." << std::endl;
        return 1;
    }

    // Establecer el tamaño del búfer
    const int buffer_size = file_stat.st_size;
    unsigned char* buffer = new unsigned char[buffer_size];

    file.read(reinterpret_cast<char*>(buffer), buffer_size);
    int bytesRead = file.gcount();

    send(clientSocket, buffer, bytesRead, 0);

    file.close();

    close(clientSocket);

    delete[] buffer;

    std::cout << "Archivo enviado al servidor." << std::endl;

    return 0;
}
