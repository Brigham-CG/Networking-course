#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include "../tools/tools.cpp"

std::string sendInitNotification(int socketFD, struct sockaddr_in server_protocols)
{   

    std::cout << "Type your nickname: ";

    std::string nickname;

    std::getline(std::cin, nickname);

    std::string notificationData = "N" + completeByteSize(nickname.size(), 2) + nickname;
    
    notificationData.resize(1024, '\0');

    sendto(socketFD,
        notificationData.c_str(), notificationData.size(),
        0,
        (struct sockaddr *) &server_protocols, sizeof(struct sockaddr)
    );

    return nickname;
}

void reqListName(int socketFD, struct sockaddr_in server_protocols, std::string nickname)
{
    std::string listReq = "L" + completeByteSize(nickname.size(), 2) + nickname;

    listReq.resize(1024, '\0');
    
    sendto(socketFD,
        listReq.c_str(), listReq.size(),
        0,
        (struct sockaddr *) &server_protocols, sizeof(struct sockaddr)
    );
}

void quitServer(int socketFD, struct sockaddr_in server_protocols, std::string nickname)
{
    std::string quitReq = "Q" + completeByteSize(nickname.size(), 2) + nickname;

    quitReq.resize(1024, '\0');

    sendto(socketFD,
        quitReq.c_str(), quitReq.size(),
        0,
        (struct sockaddr *) &server_protocols, sizeof(struct sockaddr)
    );

    std::cout << "[*] Session Closed\n";
}

void sendMessage(std::string message, int socketFD, struct sockaddr_in server_protocols, std::string nickname)
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
            payload = "W"+ completeByteSize(nickname.size(), 2) + nickname + completeByteSize(content.size(), 3) + content; 
        }
        else{
            payload = "M"+ completeByteSize(nickname.size(), 2) + nickname + completeByteSize(destination.size(), 2) + destination +  completeByteSize(content.size(), 3) + content; 
        }

        payload.resize(1024, '\0');
        
        sendto(socketFD,
            payload.c_str(), payload.size(),
            0,
            (struct sockaddr *) &server_protocols, sizeof(struct sockaddr)
        );
    }
}