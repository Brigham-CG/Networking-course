#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <map>

#include "../tools/tools.cpp"

void initNotification(
    struct sockaddr_in client_addr,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
    )
{
    std::string size_nick = message.substr(1, 2);

    int sizeNick = stoi(size_nick);

    std::string nickname = message.substr(3, sizeNick);

    nicknameToIPPort[nickname] = client_addr;

    std::cout << "[+] Client '" << nickname << "' connected.\n";
}

void sendListUsers(int socketFD,
    struct sockaddr_in client_addr,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
    )
{
    int init = 1;

    std::string size_origin_nickname = message.substr(init, 2);

    int sizeNickname = stoi(size_origin_nickname);
    init += 2;

    std::string origin_nickname = message.substr(init, sizeNickname);

    std::string nickNamesConcat;

    for(auto& client:nicknameToIPPort)
        nickNamesConcat += client.first + ",";
    
    nickNamesConcat.pop_back();

    std::string data = "L" + completeByteSize(nickNamesConcat.size(), 3) + nickNamesConcat;
    
    data.resize(1024, '\0');

    sendto(socketFD, data.c_str(), data.size(),
        0,
    (struct sockaddr *) &client_addr, sizeof(struct sockaddr));

    std::cout << "[+] List of clients sended\n";
}

void forwardMessage(int socketFD,
    struct sockaddr_in client_addr,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
    )
{
    int init = 1;

    std::string size_originNickname = message.substr(init, 2);

    int sizeOriginNickname = stoi(size_originNickname);
    
    init += 2;

    std::string originNickname = message.substr(init, sizeOriginNickname);
    init += sizeOriginNickname;

    // obtaining destination data 
    std::string size_destinationNickname = message.substr(init, 2);
    
    init += 2;
    int sizeDestinationNickname = stoi(size_destinationNickname);

    std::string destinationNickname = message.substr(init, sizeDestinationNickname);

    init += sizeDestinationNickname;
    
    // obtaining destination message;
    
    std::string size_msg = message.substr(init, 3);

    int sizeMsg = stoi(size_msg);

    init += 3;
    
    std::string content = message.substr(init, sizeMsg);

    
    std::string forwardData;

    struct sockaddr_in destination_addr = nicknameToIPPort[destinationNickname];

    forwardData = "M" + completeByteSize(originNickname.size(), 2) + originNickname + completeByteSize(sizeMsg, 3) + content;

    forwardData.resize(1024, '\0');

    std::cout << "unicast message:'" << forwardData << "'\n";

    sendto(socketFD, forwardData.c_str(), forwardData.size(),
        0,
    (struct sockaddr *) &destination_addr, sizeof(struct sockaddr));

    std::cout << "[+] Forward message\n";
}

void forwardDiffusionMessage(
    int socketFD, 
    struct sockaddr_in client_addr,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
    )
{
    int init = 1;

    // obtaining destination message

    std::string size_originNickname = message.substr(init, 2);

    int sizeOriginNickname = stoi(size_originNickname);
    
    init += 2;

    std::string originNickname = message.substr(init, sizeOriginNickname);
    init += sizeOriginNickname;

    std::string size_msg = message.substr(init, 3);

    init += 3;

    int sizeMsg = stoi(size_msg);
    
    std::string content = message.substr(init, sizeMsg);

    // forward message

    std::string forwardMessage;
    

    for(auto& destination: nicknameToIPPort)
    {
        if(destination.first != originNickname)
        {   
            forwardMessage = "M" + completeByteSize(originNickname.size(), 2) + originNickname + completeByteSize(sizeMsg, 3) + content;

            forwardMessage.resize(1024, '\0');

            sendto(socketFD, forwardMessage.c_str(), forwardMessage.size(),
                0,
                (struct sockaddr *) &nicknameToIPPort[destination.first], sizeof(struct sockaddr)
            );  
        }
    }

    std::cout << "[+] Forward Diffusion message\n";
}

void closeSession(int socketFD, 
    struct sockaddr_in client_addr,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
    )
{
    int init = 1;

    std::string size_nickname = message.substr(init, 2);

    int sizeNickname = stoi(size_nickname);
    init += 2;

    std::string nickname = message.substr(init, sizeNickname);

    auto nickToIp = nicknameToIPPort.find(nickname);
    nicknameToIPPort.erase(nickToIp);

    std::cout << "[-] Client '"<< nickname<<"'disconnected\n";
}