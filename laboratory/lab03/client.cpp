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
    
    std::getline(std::cin, nickname);

    std::string notificationData = "N" + completeByteSize(nickname.size(), 2) + nickname;

    // Enviar el nickname al servidor
    
    send(SocketFD, notificationData.c_str(), notificationData.size(), 0);

    return nickname;
}

void getListUsers()
{

    //size of list
    char size_list[4];
    int nBytes;

    nBytes = recv(SocketFD, size_list, 3, 0);

    size_list[nBytes] = '\0';

    int sizeList = atoi(size_list);

    // list 
    char list[sizeList + 1];

    nBytes = recv(SocketFD, list, sizeList, 0);

    list[nBytes] = '\0';

    const char delimiter[] = ","; 

    std::string listaNombres;

    // Usar strtok para dividir la cadena en tokens
    char *token = std::strtok(list, delimiter);

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

    char size_source[3];

    int nBytes;

    // source client data
    nBytes = recv(SocketFD, size_source, 2, 0);

    size_source[nBytes] = '\0';

    int sizeSource = atoi(size_source);

    char source[sizeSource + 1];

    nBytes = recv(SocketFD, source, sizeSource, 0);

    source[nBytes] = '\0';

    // message of client

    char size_msg[4]; 

    nBytes = recv(SocketFD, size_msg, 3, 0);

    size_msg[nBytes] = '\0';

    int sizeMsg = atoi(size_msg);

    char message[sizeMsg + 1];

    nBytes = recv(SocketFD, message, sizeMsg, 0);

    message[nBytes] = '\0';

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

        if(buffer[0] == 'L')
            getListUsers();
        else if(buffer[0] == 'M')
            obtainingMessage();
    }
}

// request to server

void reqListName()
{
    char buffer[4] = "L00";

    send(SocketFD, buffer, 3, 0);
}

void quitServer()
{
    char buffer[4] = "Q00";

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
            payload = "W"+completeByteSize(content.size(), 3) + content; 
        }
        else{
            payload = "M"+completeByteSize(destination.size(), 2) + destination +  completeByteSize(content.size(), 3) + content; 
        }

        std::cout << "payload :" << payload<< std::endl;
        send(SocketFD, payload.c_str(), payload.size(), 0);
    }
}

int main(int argc, char *argv[]) {

    if(argc != 3)
     {
        perror("[!] Debes de pasar la 'direccion ip' del puerto y el 'numero del puerto' como parametros");
        perror("[!] Ejemplo: ./client 127.0.0.1 54001");
    }

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

    std::cout << "list --------------> list users in server\n";
    std::cout << "all, message ------> send message to all users in server\n";
    std::cout << "nickname, message -> send message to specific user\n";
    std::cout << "quit --------------> exit of service\n";

    std::cout << "Input the Nickname: ";

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
