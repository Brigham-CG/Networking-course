#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <map>
#include <vector>
#include <utility>

#include <string>
#include <iostream>

#include <thread>

#include "../tools/tools.cpp"
#include "game.cpp"

std::map<int, std::vector<std::string>> partyList;
std::map<std::string, int> partyNicknamesToIndex;
std::map<int, std::vector<char>> charactersOfPlayers;
std::map<int, std::vector<std::pair<int,int>>> positions;

// ######### RECIEVE MESSAGES ##########

int selectPartyId()
{
    const int MAX_PARTIES = 99;
    int roomNumber = -1; // Inicializa con un valor no válido
    // Bucle para buscar el siguiente número de sala disponible
    for (int i = 1; i <= MAX_PARTIES; i++) {
        if (partyList.find(i) == partyList.end()) {
            // La sala no está ocupada
            roomNumber = i;
            break;
        }
    }
    return roomNumber;
}

bool verify_players(int partyId)
{
    return !partyList[partyId].empty();
}


void startGame(
    int socketFD,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    int partyId
    )
{
    bool players_in_party = verify_players(partyId);
    int posx = 0;

    while(players_in_party)
    {
        pantalla_move(positions[partyId], posx);

        std::string message_play = "P" +
        completeByteSize(posx, 3) +
        completeByteSize(charactersOfPlayers[partyId].size(), 2);

        for (int i = 0; i < partyList[partyId].size(); i++)
        {
            message_play += charactersOfPlayers[partyId][i] +
                completeByteSize(positions[partyId][i].first, 3) + 
                completeByteSize(positions[partyId][i].second, 3);

            for(int j = 0; j < partyList[partyId].size(); j++)
            {
                if(j != i)
                    message_play += charactersOfPlayers[partyId][j] +
                    completeByteSize(positions[partyId][j].first, 3) + 
                    completeByteSize(positions[partyId][j].second, 3);
            }

            message_play.resize(1024, '\0');

            sendto(socketFD, message_play.c_str(), message_play.size(),
                0,
            (struct sockaddr *) &nicknameToIPPort[partyList[partyId][i]], sizeof(struct sockaddr));
        }

        players_in_party = verify_players(partyId);
    }

    std::cout << "[!] Game with "<< partyId << " is finished\n";
}

void createSpaceParty(int socketFD,
    struct sockaddr_in client_addr,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
)
{
    int init = 1; 
    std::string size_nick = message.substr(init, 2);

    int sizeNick = stoi(size_nick);
    init += 2;

    std::string nickname = message.substr(init, sizeNick);

    int partyId = selectPartyId();

    if (partyId == -1)
    {
        std::cout << "[!] No se pudo crear la sala para: " << nickname << "\n";
        std::cout << "[!] Capacidad llena: " << nickname << "\n";
        return;
    }

    partyList[partyId].push_back(nickname);
    generateCharacter(charactersOfPlayers[partyId]);
    partyNicknamesToIndex[nickname] = 0;
    int naveX = 10;
    int naveY = 13; 
    positions[partyId].push_back({naveX, naveY});

    std::string message_create = "C" + completeByteSize(partyId, 2);

    message_create.resize(1024, '\0');

    sendto(socketFD, message_create.c_str(), message_create.size(),
        0,
    (struct sockaddr *) &client_addr, sizeof(struct sockaddr));

    std::cout << "[+] Se creo party id: " << partyId << " para " << nickname <<"\n";

    std::thread(startGame, socketFD, std::ref(nicknameToIPPort), partyId).detach();

    std::cout << "[+] Juego iniciado\n";
}

void sendListGames(
    int socketFD,
    struct sockaddr_in client_addr,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
)
{
    int init = 1; 
    std::string size_nick = message.substr(init, 2);

    int sizeNick = stoi(size_nick);
    init += 2;

    std::string nickname = message.substr(init, sizeNick);

    std::string partyListConcat; 

    if(partyList.size() != 0)
    {
        for (const auto& pair : partyList)
            partyListConcat = std::to_string(pair.first) + ",";

        partyListConcat.pop_back();
    }


    std::string message_list = "G" + completeByteSize(partyListConcat.size(), 2) + partyListConcat;

    message_list.resize(1024, '\0');

    sendto(socketFD, message_list.c_str(), message_list.size(),
        0,
    (struct sockaddr *) &client_addr, sizeof(struct sockaddr));

    std::cout << "[+] List of Parties sended\n";
}

void joinToParty(int socketFD,
    struct sockaddr_in client_addr,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
    )
{
    int init = 1; 
    std::string size_Player = message.substr(init, 2);

    int sizePlayer = stoi(size_Player);
    init += 2;

    std::string nickname_player = message.substr(init, sizePlayer);

    init += sizePlayer;

    std::string id = message.substr(init, 2); 

    int ID = stoi(id);

    if(partyList.find(ID) == partyList.end())
    {
        std::cout << "[!] Intento de unirse a un sala no creada!" << std::endl;
        return;
    }

    partyNicknamesToIndex[nickname_player] = partyList[ID].size();
    partyList[ID].push_back(nickname_player);
    generateCharacter(charactersOfPlayers[ID]);
    int naveX = 10;
    int naveY = 13; 
    positions[ID].push_back({naveX, naveY});

    std::cout <<  "[+] Starting game in party " << ID << " to " << nickname_player << std::endl;
}

void receivePlay(int socketFD,
    struct sockaddr_in client_addr,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
)
{
    int init = 1; 
    std::string size_nick = message.substr(init, 2);

    int sizeNick = stoi(size_nick);
    init += 2;

    std::string nickname = message.substr(init, sizeNick);
    init += sizeNick;

    std::string party_id = message.substr(init, 2); 

    int partyId = stoi(party_id);
    init += 2;

    std::string pos_x = message.substr(init, 3);
    int posX = stoi(pos_x);
    init += 3;

    std::string pos_y = message.substr(init, 3);
    int posY = stoi(pos_y);


    // buscar jugador
    int index = partyNicknamesToIndex[nickname];

    positions[partyId][index] = {posX, posY};
}

void exitPlayer(
    int socketFD,
    struct sockaddr_in client_addr,
    std::map<std::string, struct sockaddr_in> &nicknameToIPPort,
    std::string message
)
{
    
    int init = 1;
    std::string size_nick = message.substr(init, 2);

    int sizeNick = stoi(size_nick);
    init += 2;

    std::string nickname = message.substr(init, sizeNick);
    init += sizeNick;

    std::string party_id = message.substr(init, 2); 

    int partyId = stoi(party_id);
    init += 2;


    // eliminar jugador del juego

    auto it = partyNicknamesToIndex.find(nickname);
    if (it == partyNicknamesToIndex.end()) {
        return;
    }

    int index = partyNicknamesToIndex[nickname];

    partyList[partyId].erase(partyList[partyId].begin() + index);

    charactersOfPlayers[partyId].erase(charactersOfPlayers[partyId].begin() + index);

    positions[partyId].erase(positions[partyId].begin() + index);

    partyNicknamesToIndex.erase(it);

    std::string message_exit = "R00";

    message_exit.resize(1024,'\0');

    sendto(socketFD, message_exit.c_str(), message_exit.size(),
        0,
    (struct sockaddr *) &client_addr, sizeof(struct sockaddr));


    std::cout << "[-] Jugador " << nickname <<" retirado del juego\n";
}

// ######### END RECIEVE MESSAGES ##########
