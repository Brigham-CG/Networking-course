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

#include <ncurses.h>
#define TICKRATE 250
#define WORLD_WIDTH 112
#define WORLD_HEIGHT 26

#include "game.cpp"

bool game_created = false;
bool in_game = false;
std::string idParty = "0";

// ######### SEND MESSAGES ##########

void createSpaceGame(int socketFD, struct sockaddr_in server_protocols, std::string nickname)
{
    std::string message = "C" + completeByteSize(nickname.size(), 2) + nickname;
    
    message.resize(1024, '\0');
    
    sendto(socketFD, message.c_str(), message.size(), 
        0,
        (struct sockaddr *)&server_protocols, sizeof(struct sockaddr));
    game_created = true;

    std::cout << "[+] Space Game - created!" << std::endl;
    //std::cout << "[+] waiting one user to START!" << std::endl;
}

void viewListParties(int  socketFD, struct sockaddr_in server_protocols, std::string nickname)
{
    std::string message = "G" + completeByteSize(nickname.size(), 2) + nickname;

    message.resize(1024, '\0');

    sendto(socketFD, message.c_str(), message.size(), 
        0,
        (struct sockaddr *)&server_protocols, sizeof(struct sockaddr));
}

void joinSpaceParty(std::string command, int  socketFD, struct sockaddr_in server_protocols, std::string nickname)
{

    std::string id_party = command;

    idParty = id_party; // establecer localmente el id
    
    std::string message = "J" + completeByteSize(nickname.size(), 2) + nickname + completeByteSize(stoi(idParty), 2);

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

    std::string party = message.substr(init, 2);
    int sizeParty = stoi(party);

    idParty = party;
}

void startingPlay(std::string message, int socketFD, struct sockaddr_in server_protocols, std::string nickname) // Read
{

    std::cout << "empezando el juego\n";

    int init = 1;

    std::string posx_str = message.substr(init, 3);
    posx = stoi(posx_str);
    
    std::cout << posx_str << std::endl;

    init += 3;

    std::string cant_players_str = message.substr(init, 2);
    int cant_players = stoi(cant_players_str);

    std::cout << cant_players_str << std::endl;
    init += 2;

    for(int i = 0; i < cant_players; i++)
    {
        std::string player_char = message.substr(init, 1);
        caracteresJugadores.push_back(player_char[0]);

        init += 1;

        std::string naveX_str = message.substr(init, 3);
        int navex = stoi(naveX_str);

        init += 3;

        std::string naveY_str = message.substr(init, 3);
        int navey = stoi(naveY_str);
        init += 3;

        posicionesJugadores.push_back({navex, navey});
    }
    

    std::cout << "datos recepcionados\n";

    in_game = true;
    juego(socketFD, server_protocols, nickname, idParty);

    game_created = false;
    in_game = false;
    idParty = "0";

}

// ######### END RECIEVE MESSAGES ##########