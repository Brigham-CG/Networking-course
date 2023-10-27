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

bool game_created = false;
bool in_game = false;

// ######### SEND MESSAGES ##########

void createYankenpoGame(int socketFD, struct sockaddr_in server_protocols, std::string nickname)
{

    std::string message = "Y" + completeByteSize(nickname.size(), 2) + nickname;
    
    message.resize(1024, '\0');
    
    sendto(socketFD, message.c_str(), message.size(), 
        0,
        (struct sockaddr *)&server_protocols, sizeof(struct sockaddr));
    game_created = true;

    std::cout << "[+] Yankenpo - game created!" << std::endl;
    std::cout << "[+] waiting one user to START!" << std::endl;
}

void viewListParties(int  socketFD, struct sockaddr_in server_protocols, std::string nickname)
{
    std::string message = "G" + completeByteSize(nickname.size(), 2) + nickname + "00";

    message.resize(1024, '\0');

    sendto(socketFD, message.c_str(), message.size(), 
        0,
        (struct sockaddr *)&server_protocols, sizeof(struct sockaddr));
}

void joinYankenpoParty(std::string command, int  socketFD, struct sockaddr_in server_protocols, std::string nickname)
{

    std::string id_party = command;
    
    std::string message = "J" + completeByteSize(nickname.size(), 2) + nickname + completeByteSize(id_party.size(), 2) + id_party;

    message.resize(1024, '\0');

    sendto(socketFD, message.c_str(), message.size(), 
        0,
        (struct sockaddr *)&server_protocols, sizeof(struct sockaddr));
    
}

void sendYankenpoPlay(std::string command, int  socketFD, struct sockaddr_in server_protocols, std::string nickname)
{

    char play;

    if(command == "piedra")
    {
        play = '1';
    }
    else if(command == "papel")
    {
        play = '2';
    }
    else if(command == "tijera")
    {
        play = '3';
    }
    else
    {   
        std::cout << " \n[!] Bad play, try again\n";
        return;
    }

    std::string message = "P" + completeByteSize(nickname.size(), 2) + nickname + play;

    message.resize(1024, '\0');

    sendto(socketFD, message.c_str(), message.size(), 
        0,
        (struct sockaddr *)&server_protocols, sizeof(struct sockaddr));
}

// ######### END SEND MESSAGES ##########

// ######### RECIEVE MESSAGES ##########

void getListParties(std::string message)
{

    int init = 1;

    std::string size_list = message.substr(init, 2);
    int sizeList = stoi(size_list);

    init += 2;


    std::string list;

    for(int i = 0 ; i < sizeList; i++)
    {
        std::string size_party_id = message.substr(init, 2);
        int sizePartyId = stoi(size_party_id);
        init += sizePartyId;

        std::string party_id = message.substr(init, sizePartyId);

        list += "*" + party_id + "\n";
    }

    std::cout << "\nList of users\n";
    std::cout << list;
}

void startingPlay(std::string message)
{
    int init = 1;

    std::string size_opponent = message.substr(init, 2);
    int sizeOpponent = stoi(size_opponent);
    init += 2;

    std::string opponent_nickname =  message.substr(init, sizeOpponent);

    in_game = true;

    std::cout << "\n [+] " + opponent_nickname + ", Your opponent is READY\n";
    std::cout << " [+] Send your play (piedra | papel | tijera)\n";
}

void getResult(std::string nickname, std::string message)
{
    int init = 1;

    std::string size_opponent = message.substr(init, 2);
    int sizeOpponent = stoi(size_opponent);
    init += 2;

    std::string winner_nickname =  message.substr(init, sizeOpponent);

    if(winner_nickname == nickname)
        std::cout << "\n [+] you are the winner\n";
    else
    {
        std::cout << "\n [-] You are the looser!\n";
    }
    game_created = false;
    in_game = false;
}

// ######### END RECIEVE MESSAGES ##########
