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
    std::map<size_t, std::string> &ipAndPortToNickname,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
    )
{
    std::string size_nick = message.substr(1, 2);

    int sizeNick = stoi(size_nick);

    std::string nickname = message.substr(3, sizeNick);

    size_t index = hashString(inet_ntoa(client_addr.sin_addr) + ntohs(client_addr.sin_port));

    ipAndPortToNickname[index] = nickname;

    nicknameToIPPort[nickname] = client_addr;

    std::cout << "[+] Client '" << nickname << "' connected.\n";
}

void sendListUsers(int socketFD,
    struct sockaddr_in client_addr,
    std::map<size_t, std::string> &ipAndPortToNickname,
    std::string message
    )
{

    std::string nickNamesConcat;

    for(auto& client:ipAndPortToNickname)
        nickNamesConcat += client.second + ",";
    
    nickNamesConcat.pop_back();

    std::string data = "L" + completeByteSize(nickNamesConcat.size(), 3) + nickNamesConcat;

    data.resize(1024, '\0');

    size_t index = hashString(inet_ntoa(client_addr.sin_addr) + ntohs(client_addr.sin_port));

    sendto(socketFD, data.c_str(), data.size(),
        0,
    (struct sockaddr *) &client_addr, sizeof(struct sockaddr));

    std::cout << "[+] List of clients sended\n";
}

void forwardMessage(int socketFD,
    struct sockaddr_in client_addr,
    std::map<size_t, std::string> &ipAndPortToNickname,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
    )
{
    int init = 1;

    // obtaining destination data 
    std::string size_destination = message.substr(init, 2);
    
    init += 2;
    int sizeDest = stoi(size_destination);

    std::string destination = message.substr(init, sizeDest);

    init += sizeDest;
    
    // obtaining destination message;
    
    std::string size_msg = message.substr(init, 3);

    int sizeMsg = stoi(size_msg);

    init += 3;
    
    std::string content = message.substr(init, sizeMsg);

    // forward message

    std::string forwardData;

    struct sockaddr_in destination_addr = nicknameToIPPort[destination];

    size_t index = hashString(inet_ntoa(client_addr.sin_addr) + ntohs(client_addr.sin_port));

    std::string source_nickname = ipAndPortToNickname[index];

    forwardData = "M" + completeByteSize(source_nickname.size(), 2) + source_nickname + completeByteSize(sizeMsg, 3) + content;

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
    std::map<size_t, std::string> &ipAndPortToNickname,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
    )
{
    int init = 1;

    // obtaining destination message
    
    std::string size_msg = message.substr(init, 3);

    init += 3;

    int sizeMsg = stoi(size_msg);
    
    std::string content = message.substr(init, sizeMsg);

    // forward message

    std::string forwardData;

    size_t index = hashString(inet_ntoa(client_addr.sin_addr) + ntohs(client_addr.sin_port));

    std::string nickname = ipAndPortToNickname[index];

    for(auto& destination: ipAndPortToNickname)
    {
        if(destination.first != index)
        {   
            forwardData = "M" + completeByteSize(nickname.size(), 2) + nickname + completeByteSize(sizeMsg, 3) + content;

            std::cout << "Diffusion message:'" << forwardData << "'\n";

            sendto(socketFD, forwardData.c_str(), forwardData.size(),
                0,
                (struct sockaddr *) &nicknameToIPPort[destination.second], sizeof(struct sockaddr)
            );  
        }
    }

    std::cout << "[+] Forward Diffusion message\n";
}


void closeSession(int socketFD, 
    struct sockaddr_in client_addr,
    std::map<size_t, std::string> &ipAndPortToNickname,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
    )
{
    size_t index = hashString(inet_ntoa(client_addr.sin_addr) + ntohs(client_addr.sin_port));

    std::string nickname = ipAndPortToNickname[index];

    auto ipToNick = ipAndPortToNickname.find(index);
    ipAndPortToNickname.erase(ipToNick);

    auto nickToIp = nicknameToIPPort.find(nickname);
    nicknameToIPPort.erase(nickToIp);

    std::cout << "[-] Client '"<< nickname<<"'disconnected\n";
}