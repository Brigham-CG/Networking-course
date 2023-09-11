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
#include <vector>
#include <cstring>

int SocketFD;
std::string nickname;

// response of server

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


std::string sendInitNotification()
{
    char buffer[256];

    std::cout << "list -> list users in server\n";
    std::cout << "all, message -> send message to all users in server\n";
    std::cout << "nickname, message -> send message to specific user\n";
    std::cout << "quit -> exit of service\n";

    std::cout << "Input the Nickname: ";
    
    std::getline(std::cin, nickname);

    std::string notificationData = "n" + completeByteSize(nickname.size(), 2) + nickname;

    // Enviar el nickname al servidor
    
    send(SocketFD, notificationData.c_str(), notificationData.size(), 0);

    return nickname;
}

void getListUsers()
{

    char buffer[256];
    int nBytes;

    nBytes = recv(SocketFD, buffer, 3, 0);

    buffer[nBytes] = '\0';

    int sizeList = atoi(buffer);

    nBytes = recv(SocketFD, buffer, sizeList, 0);

    buffer[nBytes] = '\0';

    const char delimiter[] = ","; 

    std::string listaNombres;

    // Usar strtok para dividir la cadena en tokens
    char *token = std::strtok(buffer, delimiter);

    while (token != nullptr) {
        listaNombres += " * ";
        listaNombres += token; // Agregar el token a la lista
        listaNombres += "\n";
        token = std::strtok(nullptr, delimiter); // Obtener el siguiente token

    }

    std::cout << "\nList of users\n";
    std::cout << listaNombres;
    
}

void obtainingMessage()
{

    char buffer[256];
    int nBytes;

    // source client data
    nBytes = recv(SocketFD, buffer, 2, 0);

    buffer[nBytes] = '\0';

    int size = atoi(buffer);

    nBytes = recv(SocketFD, buffer, size, 0);

    buffer[nBytes] = '\0';

    std::string source = buffer;

    // message of client

    nBytes = recv(SocketFD, buffer, 3, 0);

    buffer[nBytes] = '\0';

    size = atoi(buffer);

    nBytes = recv(SocketFD, buffer, size, 0);

    buffer[nBytes] = '\0';
    std::string message = buffer;

    std::cout << "\n" << source << " : "<< message << std::endl;
}

void obtainingMessageDifusion()
{

    char buffer[256];
    int nBytes;

    // source client data
    nBytes = recv(SocketFD, buffer, 2, 0);

    buffer[nBytes] = '\0';

    int size = atoi(buffer);

    nBytes = recv(SocketFD, buffer, size, 0);

    buffer[nBytes] = '\0';

    std::string source = buffer;

    // message of client

    nBytes = recv(SocketFD, buffer, 3, 0);

    buffer[nBytes] = '\0';

    size = atoi(buffer);

    nBytes = recv(SocketFD, buffer, size, 0);

    buffer[nBytes] = '\0';
    std::string message = buffer;

    std::cout << "\n" << source << " : "<< message << std::endl;
}

void ReceiveMessages() {

    char buffer[2];
    int nBytes;

    while (true) {

        nBytes = recv(SocketFD, buffer, 1, 0);

        if (nBytes <=0) {
            std::cout << "[+] Disconnected from server.\n" ;
            break;
        }
        
        buffer[nBytes] = '\0';

        if(buffer[0] == 'l')
            getListUsers();
        else if(buffer[0] == 'm')
            obtainingMessage();
        else if(buffer[0] == 'w')
            obtainingMessageDifusion();

    }
}

// request to server

void reqListName()
{
    char buffer[4] = "l";

    send(SocketFD, buffer, 3, 0);

}

void quitServer()
{
    char buffer[4] = "q";

    send(SocketFD, buffer, 3, 0);
    std::cout << "[*] Session Closed\n";
}

void sendMessage(std::string message)
{

    std::string payload;
    
    size_t pos = message.find(",");
    
    if (pos == std::string::npos)   
    {
        return;
    } 
    else
    {
        std::string destination = message.substr(0, pos);
        std::string content = message.substr(pos + 2); 


        if(destination == "all")
        {
            payload = "w"+completeByteSize(content.size(), 3) + content; 
        }
        else{
            payload = "m"+completeByteSize(destination.size(), 2) + destination +  completeByteSize(content.size(), 3) + content; 

            // std::cout << "payload :" <<payload<< std::endl;
        }
        send(SocketFD, payload.c_str(), payload.size(), 0);
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

    sendInitNotification();

    // Iniciar un hilo para recibir mensajes del servidor
    std::thread(ReceiveMessages).detach();

    // Leer mensajes del usuario y enviarlos al servidor
    while (true) {

        std::string message;

        std::cout << nickname <<" #";

        std::getline(std::cin, message);


        if (message == "list")
            reqListName();
        else if (message == "quit")
        {
            quitServer();
            break;
        }
        else 
            sendMessage(message);
    }

    close(SocketFD);

    return 0;
}
