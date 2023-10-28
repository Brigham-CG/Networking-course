#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <vector>
#include <sstream>

#include <string>
#include <iostream>
#include "../tools/tools.cpp"

bool game_created = false;
bool in_game = false;
std::string idParty = "0";

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

    idParty = id_party; // establecer localmente el id
    
    std::string message = "J" + completeByteSize(nickname.size(), 2) + nickname + completeByteSize(id_party.size(), 2) + id_party;

    message.resize(1024, '\0');

    sendto(socketFD, message.c_str(), message.size(), 
        0,
        (struct sockaddr *)&server_protocols, sizeof(struct sockaddr));
    
}

void sendYankenpoPlay(std::string command, int  socketFD, struct sockaddr_in server_protocols, std::string nickname)
{

    std::string play;

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

    std::string message = "P" + completeByteSize(nickname.size(), 2) + nickname + completeByteSize(idParty.size(), 2) + idParty+ play;

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

    std::string list = message.substr(init, sizeList);

    std::istringstream ss(list);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }

    // Construir la lista de nombres
    std::string listParty;

    for (const std::string& nombre : tokens)
        listParty += " * " + nombre + "\n";


    std::cout << "\nList of parties\n";
    std::cout << listParty;
}

void getIdParty(std::string message)
{

    int init = 1;

    std::string size_party = message.substr(init, 2);
    int sizeParty = stoi(size_party);
    init += 2;

    std::string party = message.substr(init, sizeParty);

    idParty = party;
}

void startingPlay(std::string message)
{
    int init = 1;

    std::string size_opponent = message.substr(init, 2);
    int sizeOpponent = stoi(size_opponent);
    init += 2;

    std::string opponent_nickname = message.substr(init, sizeOpponent);

    in_game = true;
    
    std::cout << "\n [+] " + opponent_nickname + ", Your opponent is READY\n";
    std::cout << " [+] Send your play: (piedra | papel | tijera)\n";
}

void getResult(std::string nickname, std::string message)
{
    int init = 1;

    std::string size_winner = message.substr(init, 2);
    int sizeWinner = stoi(size_winner);

    game_created = false;
    in_game = false;
    idParty = "0";

    if(sizeWinner == 0)
    {
        std::cout << "\n [!] Hubo un empate\n";
        return;
    }

    init += 2;

    std::string winner_nickname =  message.substr(init, sizeWinner);

    if(winner_nickname == nickname)
        std::cout << "\n [+] you are the winner\n";
    else
    {
        std::cout << "\n [-] You are the looser!\n";
    }
}

// ######### END RECIEVE MESSAGES ##########