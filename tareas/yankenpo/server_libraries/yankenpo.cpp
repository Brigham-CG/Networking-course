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
#include "../tools/tools.cpp"

std::map<int, std::pair<std::string,std::string>> partyList;
std::map<int, std::vector<int>> partyPlays;

// ######### SEND MESSAGES ##########

// ######### END SEND MESSAGES ##########

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

void createYankenpoParty(int socketFD,
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

    partyList[partyId] = {nickname,""};
    partyPlays[partyId] = {};

    std::string party_id = std::to_string(partyId);

    std::string message_create = "Y" + completeByteSize(party_id.size(), 2) + party_id;

    sendto(socketFD, message_create.c_str(), message_create.size(),
        0,
    (struct sockaddr *) &client_addr, sizeof(struct sockaddr));


    std::cout << "[+] Se creo party id: " << partyId << " para " << nickname <<"\n";
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
    std::string size_PlayerTwo = message.substr(init, 2);

    int sizePlayerTwo = stoi(size_PlayerTwo);
    init += 2;

    std::string playerTwo = message.substr(init, sizePlayerTwo);

    init += sizePlayerTwo;

    std::string size_id = message.substr(init, 2); 

    int sizeId = stoi(size_id);
    init += 2;

    std::string id = message.substr(init, sizeId);
    int ID = stoi(id);

    if(partyList.find(ID) == partyList.end())
    {
        std::cout << "[!] Intento de unirse a un sala no creada!" << std::endl;
        return;
    }
   
    if(partyList[ID].second != "")
    {
        std::cout << "[!] Intento de unirse a un sala ya ocupada!" << std::endl;
        return;
    }

    partyList[ID].second = playerTwo;
    std::string playerOne = partyList[ID].first;

    // send start playing message to player one 
    std::string message_start_one = "P" + completeByteSize(playerTwo.size(), 2) + playerTwo;
    sendto(socketFD, message_start_one.c_str(), message_start_one.size(),
        0,
    (struct sockaddr *) &nicknameToIPPort[playerOne], sizeof(struct sockaddr));
    
    // send start playing message to player Two
    std::string message_start_two = "P" + completeByteSize(playerOne.size(), 2) + playerOne;
    sendto(socketFD, message_start_two.c_str(), message_start_two.size(),
        0,
    (struct sockaddr *) &client_addr, sizeof(struct sockaddr));
    
    std::cout <<  "[+] Starting game in party " << ID << " with " << playerOne << " and " << playerTwo << std::endl;
}

int verifyWinner(int id) {

    int jugadaJugador1 = partyPlays[id][0];
    int jugadaJugador2 = partyPlays[id][1];

    if (jugadaJugador1 == jugadaJugador2) {
        // Empate
        return 0;
    } else if (
        (jugadaJugador1 == 1 && jugadaJugador2 == 3) || // Piedra vs. Tijeras
        (jugadaJugador1 == 2 && jugadaJugador2 == 1) || // Papel vs. Piedra
        (jugadaJugador1 == 3 && jugadaJugador2 == 2)    // Tijeras vs. Papel
    ) {
        // Jugador 1 gana
        return 1;
    } else {
        // Jugador 2 gana
        return 2;
    }
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

    std::string size_id = message.substr(init, 2); 

    int sizeId = stoi(size_id);
    init += 2;

    std::string id = message.substr(init, sizeId);
    int ID = stoi(id);
    init += sizeId;    

    std::string play = message.substr(init, 1);
    int Play = stoi(play);

    partyPlays[ID].push_back(Play); 

    std::cout << "[+] Play received: " << play << std::endl;
    

    if(partyPlays[ID].size() == 2)
    {
        std::string message_data = "R";
        int winnerIndex = verifyWinner(ID);
        std::string winnerNickname;
        if (winnerIndex == 1)
        {   winnerNickname = partyList[ID].first;
            message_data += completeByteSize(winnerNickname.size() , 2) + winnerNickname;
        }
        else if(winnerIndex == 2)
        {
            winnerNickname = partyList[ID].second;
            message_data += completeByteSize(winnerNickname.size() , 2) + winnerNickname;
        }
        else
        {
            message_data += "00"; // sin ganador
        }

        std::string playerOne = partyList[ID].first;
        std::string playerTwo = partyList[ID].second;

        sendto(socketFD, message_data.c_str(), message_data.size(),
            0,
        (struct sockaddr *) &nicknameToIPPort[playerOne], sizeof(struct sockaddr));
        
        // send start playing message to player Two
        
        sendto(socketFD, message_data.c_str(), message_data.size(),
            0,
        (struct sockaddr *) &nicknameToIPPort[playerTwo], sizeof(struct sockaddr));

        // eliminar party 

        auto party = partyList.find(ID);
        partyList.erase(party);

        auto playParty = partyPlays.find(ID);
        partyPlays.erase(playParty);
    }
}

// ######### END RECIEVE MESSAGES ##########
