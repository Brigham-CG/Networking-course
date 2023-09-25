#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error al crear el socket del servidor." << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345); // Puerto del servidor
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error al enlazar el socket." << std::endl;
        return 1;
    }

    if (listen(serverSocket, 1) == -1) {
        std::cerr << "Error al escuchar en el socket." << std::endl;
        return 1;
    }

    std::cout << "Esperando conexiones entrantes..." << std::endl;

    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == -1) {
        std::cerr << "Error al aceptar la conexión del cliente." << std::endl;
        return 1;
    }

    std::ofstream outputFile("wallpaper.jpg", std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error al abrir el archivo de salida en el servidor." << std::endl;
        return 1;
    }

    const int buffer_size = 1024;
    unsigned char buffer[buffer_size];
    int bytesRead = 0;

    while ((bytesRead = recv(clientSocket, buffer, buffer_size, 0)) > 0) {
        outputFile.write(reinterpret_cast<char*>(buffer), bytesRead);
    }

    outputFile.close();
    close(clientSocket);
    close(serverSocket);

    std::cout << "Archivo recibido y guardado en el servidor." << std::endl;

    return 0;
}
