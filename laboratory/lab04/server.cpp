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
    // size nickname
    char size_nick[3];
    int nBytes = 0;

    nBytes = recv(clientSocket, size_nick, 2, 0);

    size_nick[nBytes] = '\0';

    int sizeNick = atoi(size_nick);

    // nickname

    char nickname[sizeNick + 1];

    nBytes = recv(clientSocket, nickname, sizeNick, 0);

    nickname[nBytes] = '\0';

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

    std::string data = "L" + completeByteSize(nickNamesConcat.size(), 3) + nickNamesConcat;

    std::cout << "sendList:'" << data <<"'\n";

    send(clientSocket, data.c_str(), data.size(), 0);

    std::cout << "[+] List of clients sended\n";
}

void forwardMessage(int clientSocket, std::string nickName)
{

    // obtaining destination data 
    char size_destination[3];
    int nBytes = 0;

    nBytes = recv(clientSocket, size_destination, 2, 0);

    if (nBytes <= 0) {
        std::cout << "[+] Client disconnected.\n";
        return;
    }

    size_destination[nBytes] = '\0';

    int sizeDest = atoi(size_destination);

    char destination[sizeDest + 1];

    nBytes = recv(clientSocket, destination, sizeDest, 0);

    destination[nBytes] = '\0';

    // obtaining destination message;
    
    char size_msg[4];

    nBytes = recv(clientSocket, size_msg, 3, 0);

    size_msg[nBytes] = '\0';

    int sizeMsg = atoi(size_msg);
    
    char message[sizeMsg];

    nBytes = recv(clientSocket, message, sizeMsg, 0);

    message[nBytes] = '\0';

    // forward message

    std::string forwardData;

    forwardData = "M" + completeByteSize(nickName.size(), 2) + nickName + completeByteSize(sizeMsg, 3) + message;

    std::cout << "unicast message:'" << forwardData << "'\n";

    send(clientNicknames[destination], forwardData.c_str(), forwardData.size(), 0);

    std::cout << "[+] Forward message\n";
}

void forwardDiffusionMessage(int clientSocket, std::string nickName)
{

    int nBytes = 0;

    // obtaining destination message
    
    char size_msg[4];

    nBytes = recv(clientSocket, size_msg, 3, 0);

    size_msg[nBytes] = '\0';

    int sizeMsg = atoi(size_msg);
    
    char message[sizeMsg];

    nBytes = recv(clientSocket, message, sizeMsg, 0);

    message[nBytes] = '\0';

    // forward message

    std::string forwardData;

    for(auto& destination: clientNicknames)
    {
        if(destination.first != nickName)
        {
            forwardData = "M" + completeByteSize(nickName.size(), 2) + nickName + completeByteSize(sizeMsg, 3) + message;

            std::cout << "Diffusion message:'" << forwardData << "'\n";

            send(destination.second, forwardData.c_str(), forwardData.size(), 0);
        }
    }

    std::cout << "[+] Forward Diffusion message\n";
}

void forwardFile(int clientSocket, std::string nickName)
{

    char size_source[3];

    int nBytes;

    // source client data
    nBytes = recv(clientSocket, size_source, 2, 0);

    size_source[nBytes] = '\0';

    int sizeSource = atoi(size_source);

    char destination[sizeSource + 1];

    nBytes = recv(clientSocket, destination, sizeSource, 0);

    destination[nBytes] = '\0';

    std::cout << "[+] Forwarding file to " << destination << std::endl;

    // file name of source client

    char size_fileName[6]; 

    nBytes = recv(clientSocket, size_fileName, 5, 0);

    size_fileName[nBytes] = '\0';

    int sizeFilename = atoi(size_fileName);

    char fileName[sizeFilename + 1];

    nBytes = recv(clientSocket, fileName, sizeFilename, 0);

    fileName[nBytes] = '\0';

    // file content

    char size_file[16];

    nBytes = recv(clientSocket, size_file, 15, 0);

    size_file[nBytes] = '\0';

    int sizeFile = atoi(size_file);

    // end pre-payload 

    unsigned char fileData[sizeFile + 1];

    nBytes = recv(clientSocket, fileData, sizeFile, 0);

    fileData[nBytes] = '\0';

    char hash[41];

    nBytes = recv(clientSocket, hash, 40, 0);

    hash[nBytes] = '\0';

    char timeStamp[15];

    nBytes = recv(clientSocket, timeStamp, 14, 0);

    timeStamp[nBytes] = '\0';

    std::cout << sizeFile << std::endl;

    std::string prePayload;

    prePayload = "F" + 
        completeByteSize(nickName.size(), 2) + nickName +
        size_fileName + fileName +
        size_file;

    std::cout << prePayload << std::endl;

    send(clientNicknames[destination], prePayload.c_str(), prePayload.size(), 0);

    // data file
    send(clientNicknames[destination], fileData, sizeFile, 0);

    // hash
    std::string hash_complete = "";

    hash_complete += hash;
    hash_complete += timeStamp;
    
    std::cout << hash_complete << std::endl;
    send(clientNicknames[destination], hash_complete.c_str(), hash_complete.size(), 0);

    // forward response
    
    // char option[2];

    // nBytes = recv(clientNicknames[destination], option, 1, 0);

    // option[nBytes] = '\0';
    
    // std::cout << "foption: " << option << std::endl;
    
    // char new_s_source[3];   

    // nBytes = recv(clientNicknames[destination], new_s_source, 2, 0);

    // new_s_source[nBytes] = '\0';

    // sizeSource = atoi(new_s_source);

    // std::cout << "s_source: '" << new_s_source << "' " << nBytes<<std::endl;

    // char source[sizeSource + 1];

    // nBytes = recv(clientNicknames[destination], source, sizeSource, 0);

    // source[nBytes] = '\0';

    // std::cout << "source: '" << source << "' " << nBytes<<std::endl;

    // // obtaining hash
    // char newHash[41];

    // nBytes = recv(clientNicknames[destination], newHash, 40, 0);

    // newHash[nBytes] = '\0';

    // std::cout << "hash: '" << newHash << "' " << nBytes<<std::endl;

    // std::string response = "R" + completeByteSize(nickName.size(), 2) + nickName + newHash;

    // std::cout << "response:" << response << std::endl;

    // send(clientSocket,response.c_str(), response.size(), 0);

    std::cout << "[+] Forward file\n";
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

        if (option[0] == 'N')
        {
            nickName = initNotification(clientSocket);
            if (nickName == "nothing")
                break;
        }
        else if (option[0] == 'L')
            getListUsers(clientSocket);
        else if (option[0] == 'M')
            forwardMessage(clientSocket, nickName);
        else if (option[0] == 'W')
            forwardDiffusionMessage(clientSocket, nickName);
        else if (option[0] == 'F')
            forwardFile(clientSocket, nickName);
        else if (option[0] == 'Q')
            closeSession(nickName);
    }
    
    // // Cerrar el socket del cliente y eliminarlo del mapa
    close(clientSocket);
    clientNicknames.erase(nickName);
}

int main(int argc, char *argv[]) {


    if(argc != 2)
    {
        perror("[!] Debes de pasar el 'numero del puerto' como parametro");
        perror("[!] Ejemplo: ./server 54001");
    }

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

    while (true) {
        client = sizeof(struct sockaddr_in);

        int ConnectFD = accept(SocketFD, nullptr, nullptr);
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