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

std::string completeByteSize(int number, int size)
{
    std::string returnNumber = std::to_string(number);
    

    if(returnNumber.size() < size)
    {
        int n = size - returnNumber.size();

        for (int i = 0; i < n; i++) 
            returnNumber = "0" + returnNumber;
    }

    return returnNumber;
}

std::map<std::string, int> clientNicknames;

std::string initNotification(int clientSocket)
{
    char buffer[255];
    int nBytes = 0;

    nBytes = recv(clientSocket, buffer, 2, 0);

    buffer[nBytes] = '\0';

    int nicknameSize = atoi(buffer);

    std::cout << nicknameSize << std::endl;

    if (nicknameSize < 1 || nicknameSize > 255) {
        close(clientSocket);
        std::cout << "[!] Error in size of"<< std::endl;
        return "nothing";
    }

    nBytes = recv(clientSocket, buffer, nicknameSize, 0);

    buffer[nBytes] = '\0';

    if (nBytes <= 0) {
        close(clientSocket);
        return "nothing";
    }

    std::string nickname = buffer;

    // Asociar el nickname con el descriptor de archivo
    clientNicknames[nickname] = clientSocket;

    std::cout << "[+] Client '" << nickname << "' connected.\n";
    return nickname;
}

void getListUsers(int clientSocket)
{
    char buffer[3];

    recv(clientSocket, buffer, 2, 0);

    std::string nickNamesConcat;

    for(auto& client:clientNicknames)
        nickNamesConcat += client.first + ",";
    
    nickNamesConcat.pop_back();

    std::string data = "l" + completeByteSize(nickNamesConcat.size(), 3) + nickNamesConcat;

    std::cout << "'" << data <<"'\n";

    send(clientSocket, data.c_str(), data.size(), 0);

    std::cout << "[+] List of clinents sended\n";

}

void forwardMessage(int clientSocket, std::string nickName)
{

    char buffer[255];
    int nBytes = 0;

    nBytes = recv(clientSocket, buffer, 2, 0);

    if (nBytes <= 0) {
        std::cout << "[+] Client disconnected.\n";
        return;
    }

    buffer[nBytes] = '\0';
    // obtaining destination data 

    int destNicknameSize = atoi(buffer);

    nBytes = recv(clientSocket, buffer, destNicknameSize, 0);

    buffer[nBytes] = '\0';

    std::string destNickName = buffer;

    // obtaining destination message;
    
    nBytes = recv(clientSocket, buffer, 3, 0);

    buffer[nBytes] = '\0';

    int destMsgSize = atoi(buffer);
    
    nBytes = recv(clientSocket, buffer, destMsgSize, 0);
    
    buffer[nBytes] = '\0';

    std::string message = buffer;

    // forward message

    std::string forwardData;

    forwardData = "w" + completeByteSize(nickName.size(), 2) + nickName + completeByteSize(message.size(), 3) + message;

    send(clientNicknames[destNickName], forwardData.c_str(), forwardData.size(), 0);

    std::cout << "[+] Forward message\n";
}

void closeSession(std::string nickName)
{
    char buffer[3];

    recv(clientNicknames[nickName], buffer, 2, 0);

    auto it = clientNicknames.find(nickName);

    int fd = clientNicknames[nickName];

    clientNicknames.erase(it);

    close(fd);
    std::cout << "[-] Client '"<<nickName<<"'disconnected\n";

}

void HandleClient(int clientSocket) {

    char option[2];
    int nBytes;
    std::string nickName;

    // Recibir el mensaje que contiene el nickname del cliente

    while(true)
    {
        nBytes = recv(clientSocket, option, 1, 0);
        
        option[nBytes] = '\0';

        // std::cout << "option: " << option <<std::endl;

        if (nBytes <= 0) {
            close(clientSocket);
            return;
        }

        if (option[0] == 'n')
        {
            nickName = initNotification(clientSocket);
            if (nickName == "nothing")
                break;
        }
        else if (option[0] == 'l')
            getListUsers(clientSocket);
        else if (option[0] == 'm')
            forwardMessage(clientSocket, nickName);
        else if (option[0] == 'q')
            closeSession(nickName);
    }
    
    // // Cerrar el socket del cliente y eliminarlo del mapa
    close(clientSocket);
    clientNicknames.erase(nickName);
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

    std::cout << "[+] Server started\n";

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