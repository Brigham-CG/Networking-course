#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// strings and data
#include <string.h>
#include <thread>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>

// tic tac toe 
#include <random>
#include <algorithm>
#include "ticTacToe.hpp"

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

std::map<std::string, int> clientNicknames;


// ##### TicTacToe recurses #####

bool in_game = 0;
bool visualizer = 0;

short subjectsCount = 0;
char stateGame = ' ';

TicTacToe game;

// vector to storage the players and viewe of TTT
std::vector<std::string> tictactoeSubjects;

// ##### end TicTacToe recurses #####


std::vector<std::string> selectSubjects() {

    // Obtén las claves del mapa
    std::vector<std::string> claves;
    for (const auto& par : clientNicknames) {
        claves.push_back(par.first);
    }

    // Mezcla las claves de manera aleatoria
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(claves.begin(), claves.end(), g);

    // Selecciona las dos primeras claves y almacénalas en un vector
    std::vector<std::string> resultado;
    for (int i = 0; i < 2 && i < claves.size(); ++i) {
        resultado.push_back(claves[i]);
    }

    return resultado;
}

std::string initNotification(int clientSocket)
{
    // size nickname
    char size_nick[3];
    int nBytes = 0;

    nBytes = recv(clientSocket, size_nick, 2, 0);

    size_nick[nBytes] = '\0';

    int sizeNick = atoi(size_nick);

    // nickname

    char nickname[sizeNick + 1];

    nBytes = recv(clientSocket, nickname, sizeNick, 0);

    nickname[nBytes] = '\0';

    // Asociar el nickname con el descriptor de archivo
    clientNicknames[nickname] = clientSocket;

    std::cout << "[+] Client '" << nickname << "' connected.\n";

    return nickname;
}

void getListUsers(int clientSocket)
{
    char buffer[3];

    recv(clientSocket, buffer, 2, 0);

    std::string nickNamesConcat;

    for(auto& client:clientNicknames)
        nickNamesConcat += client.first + ",";
    
    nickNamesConcat.pop_back();

    std::string data = "L" + completeByteSize(nickNamesConcat.size(), 3) + nickNamesConcat;

    std::cout << "sendList:'" << data <<"'\n";

    send(clientSocket, data.c_str(), data.size(), 0);

    std::cout << "[+] List of clients sended\n";
}

void forwardMessage(int clientSocket, std::string nickName)
{

    // obtaining destination data 
    char size_destination[3];
    int nBytes = 0;

    nBytes = recv(clientSocket, size_destination, 2, 0);

    if (nBytes <= 0) {
        std::cout << "[+] Client disconnected.\n";
        return;
    }

    size_destination[nBytes] = '\0';

    int sizeDest = atoi(size_destination);

    char destination[sizeDest + 1];

    nBytes = recv(clientSocket, destination, sizeDest, 0);

    destination[nBytes] = '\0';

    // obtaining destination message;
    
    char size_msg[4];

    nBytes = recv(clientSocket, size_msg, 3, 0);

    size_msg[nBytes] = '\0';

    int sizeMsg = atoi(size_msg);
    
    char message[sizeMsg];

    nBytes = recv(clientSocket, message, sizeMsg, 0);

    message[nBytes] = '\0';

    // forward message

    std::string forwardData;

    forwardData = "M" + completeByteSize(nickName.size(), 2) + nickName + completeByteSize(sizeMsg, 3) + message;

    std::cout << "unicast message:'" << forwardData << "'\n";

    send(clientNicknames[destination], forwardData.c_str(), forwardData.size(), 0);

    std::cout << "[+] Forward message\n";
}

void forwardDiffusionMessage(int clientSocket, std::string nickName)
{

    int nBytes = 0;

    // obtaining destination message
    
    char size_msg[4];

    nBytes = recv(clientSocket, size_msg, 3, 0);

    size_msg[nBytes] = '\0';

    int sizeMsg = atoi(size_msg);
    
    char message[sizeMsg];

    nBytes = recv(clientSocket, message, sizeMsg, 0);

    message[nBytes] = '\0';

    // forward message

    std::string forwardData;

    for(auto& destination: clientNicknames)
    {
        if(destination.first != nickName)
        {
            forwardData = "M" + completeByteSize(nickName.size(), 2) + nickName + completeByteSize(sizeMsg, 3) + message;

            std::cout << "Diffusion message:'" << forwardData << "'\n";

            send(destination.second, forwardData.c_str(), forwardData.size(), 0);
        }
    }

    std::cout << "[+] Forward Diffusion message\n";
}

void fileIntegrityResponse(int clientSocket, std::string nickname)
{   

    // ##### forwarding response to source #####
    int nBytes;
    
    char size_destination[3];   

    nBytes = recv(clientSocket, size_destination, 2, 0);

    size_destination[nBytes] = '\0';

    int sizeDestination = atoi(size_destination);

    char destination[sizeDestination + 1];

    nBytes = recv(clientSocket, destination, sizeDestination, 0);

    destination[nBytes] = '\0';

    // obtaining hash
    char newHash[41];

    nBytes = recv(clientSocket, newHash, 40, 0);

    newHash[nBytes] = '\0';

    std::string response = "R" + completeByteSize(nickname.size(), 2) + nickname + newHash;

    std::cout << "[!] Response to source:" << response << std::endl;

    send(clientNicknames[destination], response.c_str(), response.size(), 0);

    // ##### forwarding response to source #####
}

void forwardFile(int clientSocket, std::string nickName)
{

    // ##### first charge #####
    char size_source[3];

    int nBytes;

    // source client data
    nBytes = recv(clientSocket, size_source, 2, 0);

    size_source[nBytes] = '\0';

    int sizeSource = atoi(size_source);

    char destination[sizeSource + 1];

    nBytes = recv(clientSocket, destination, sizeSource, 0);

    destination[nBytes] = '\0';

    std::cout << "[+] Forwarding file to " << destination << std::endl;

    // file name of source client

    char size_fileName[6]; 

    nBytes = recv(clientSocket, size_fileName, 5, 0);

    size_fileName[nBytes] = '\0';

    int sizeFilename = atoi(size_fileName);

    char fileName[sizeFilename + 1];

    nBytes = recv(clientSocket, fileName, sizeFilename, 0);

    fileName[nBytes] = '\0';

    // file content

    char size_file[16];

    nBytes = recv(clientSocket, size_file, 15, 0);

    size_file[nBytes] = '\0';

    int sizeFile = atoi(size_file);

    // ##### end first charge #####

    // ##### receiving hash and timestamp #####
    char hash[41];

    nBytes = recv(clientSocket, hash, 40, 0);

    hash[nBytes] = '\0';

    char timeStamp[15];

    nBytes = recv(clientSocket, timeStamp, 14, 0);

    timeStamp[nBytes] = '\0';

    // ##### end receiving hash and timestamp #####


    // ##### receiving file #####
    int bufferSize = 1024;

    unsigned char buffer[bufferSize];
    
    unsigned char *fileData = new unsigned char [sizeFile];

    int totalRead = 0;

    while (totalRead < sizeFile) {
        nBytes = recv(clientSocket, buffer, bufferSize, 0);
        std::memcpy(fileData + totalRead, buffer, nBytes);
        totalRead += nBytes;
    }

    // ##### end receiving file #####

    // ##### sending response #####
    std::string prePayload;

    prePayload = "F" + 
        completeByteSize(nickName.size(), 2) + nickName +
        size_fileName + fileName +
        size_file;

    std::cout << "[*] First Charge: " << prePayload << std::endl;

    send(clientNicknames[destination], prePayload.c_str(), prePayload.size(), 0);

    // hash
    std::string hash_complete;

    hash_complete += hash;
    hash_complete += timeStamp;

    std::cout << "[*] Last Charge: " << hash_complete << std::endl;
    
    send(clientNicknames[destination], hash_complete.c_str(), hash_complete.size(), 0);

    // data file
    send(clientNicknames[destination], fileData, sizeFile, 0);

    delete [] fileData;

    // ##### sending response #####

    // forward response

    std::cout << "[+] Forward file\n";
}

void closeSession(std::string nickName)
{
    char buffer[3];

    recv(clientNicknames[nickName], buffer, 2, 0);

    auto it = clientNicknames.find(nickName);

    int fd = clientNicknames[nickName];

    clientNicknames.erase(it);

    close(fd);
    std::cout << "[-] Client '"<<nickName<<"'disconnected\n";

}

void waitingUsersToStart(int clientSocket, std::string nickName)
{
    // receive response

    char init_res[4];
    int sizeInit = 3;

    int nBytes;

    nBytes = recv(clientSocket, init_res, sizeInit, 0);

    init_res[nBytes] = '\0';

    if(init_res[0] != 'T' && init_res[1] != 'T' && init_res[2] != 'T')
    {
        std::cout << "[!] No se obtuvo respuesta de " << nickName << "\n";
        in_game = 0;
        tictactoeSubjects.clear();
        return;
    }
    std::cout << "[+] Response play " << nickName << std::endl;
    subjectsCount++;
}

void waitingViewerToStart(int clientSocket, std::string nickName)
{

    char init_res[4];
    int sizeInit = 3;

    int nBytes;

    nBytes = recv(clientSocket, init_res, sizeInit, 0);

    std::cout << "v: " << init_res << std::endl;

    init_res[nBytes] = '\0';

    if(init_res[0] != 'T' && init_res[1] != 'T' && init_res[2] != 'T')
    {
        std::cout << "[!] Viewer incorrect format.." << nickName << "\n";
        in_game = 0;
        tictactoeSubjects.clear();
        return;
    }   

    std::cout << "[+] Request view from " << nickName << std::endl;

    tictactoeSubjects.push_back(nickName);

    subjectsCount++;

    // possible waiting 
    while(subjectsCount != 3);

    // send start game

    std::string list = tictactoeSubjects[0] + ", " + tictactoeSubjects[1];

    std::string start = "T" + completeByteSize(list.size(), 2) + list;

    send(clientNicknames[tictactoeSubjects[0]], start.c_str(), start.size(), 0); // first user

    send(clientNicknames[tictactoeSubjects[1]], start.c_str(), start.size(), 0); // second user

    std::cout << "[+] Request to start play to players\n";
}

void receivingPosition(int clientSocket, std::string nickName)
{
    game.mostrarTablero();

    int nBytes;
    char pos_extract[2];

    nBytes = recv(clientSocket, pos_extract, 1, 0);

    pos_extract[nBytes] = '\0';

    int position = atoi(pos_extract);

    game.hacerMovimiento(position);
 
    // forward other subjects
    std::string other = tictactoeSubjects[0] == nickName ? tictactoeSubjects[1]: tictactoeSubjects[0];
    std::string pos = "P";

    pos += pos_extract;

    send(clientNicknames[other], pos.c_str(), pos.size(), 0);

     pos[0] = 'V';

    send(clientNicknames[tictactoeSubjects[2]], pos.c_str(), pos.size(), 0);

    char gameState = game.verificarEstado();

    std::string state = "";

    std::cout << "you: " << nickName << " other: " << other << std::endl;

    if(gameState != ' ')
    {
        
        std::string listView;
        std::string list;

        if(gameState == 'X' || gameState == 'O')
        {

            list = nickName + ","+ other;
            listView = "Z" + completeByteSize(list.size(), 2);

            state = "WTTT";
            send(clientSocket, state.c_str(), state.size(), 0);

            state = "LTTT";
            send(clientNicknames[other], state.c_str(), state.size(), 0);

            std::cout << "[+] States sended (W L)\n";
        }
        else if(gameState == 'E')
        {
            state= "Eempate";
            list = "empate";

            send(clientSocket, state.c_str(), state.size(), 0);
            send(clientNicknames[other], state.c_str(), state.size(), 0);
        }   

        listView = "Z" + completeByteSize(list.size(), 2) + list;
        
        send(clientNicknames[tictactoeSubjects[2]], listView.c_str(), listView.size(),0);

        // in_game = 1;
        subjectsCount = 0;
        tictactoeSubjects.clear();
        gameState = ' ';
    }
}

void HandleClient(int clientSocket) {

    char option[2];
    int nBytes;
    std::string nickName;

    while(true)
    {
        nBytes = recv(clientSocket, option, 1, 0);
        
        if (nBytes <= 0) {
            close(clientSocket);
            return;
        }

        option[nBytes] = '\0';

        if (option[0] == 'N')
        {
            nickName = initNotification(clientSocket);
            if (nickName == "nothing")
                break;
        }
        else if (option[0] == 'L')
            getListUsers(clientSocket);
        else if (option[0] == 'M')
            forwardMessage(clientSocket, nickName);
        else if (option[0] == 'W')
            forwardDiffusionMessage(clientSocket, nickName);
        else if (option[0] == 'F')
            forwardFile(clientSocket, nickName);
        else if (option[0] == 'R')
            fileIntegrityResponse(clientSocket, nickName);
        else if (in_game && option[0] == 'B')
            waitingUsersToStart(clientSocket, nickName);
        else if (in_game && option[0] == 'V')
            waitingViewerToStart(clientSocket, nickName);
        else if (in_game && option[0] == 'P')
            receivingPosition(clientSocket, nickName);
        else if (option[0] == 'Q')
            closeSession(nickName);
    }
    
    
    // Cerrar el socket del cliente y eliminarlo del mapa
    close(clientSocket);
    clientNicknames.erase(nickName);
}

void ticTacToePreparation()
{   
    std::cout << "[+] Preparing game...\n";

    tictactoeSubjects = selectSubjects();
    in_game = 1;

    // send offers to play
    char character[3] = "XO";

    
    std::cout << "[+] Send play's request to: " << tictactoeSubjects[0] << std::endl;
    std::string init_req = "BX";
    std::cout << "init_req: '" << init_req << "'"<< std::endl;
    send(clientNicknames[tictactoeSubjects[0]], init_req.c_str(), init_req.size(), 0);

    init_req = "BO";
    std::cout << "init_req: '" << init_req << "'"<< std::endl;
    send(clientNicknames[tictactoeSubjects[1]], init_req.c_str(), init_req.size(), 0);
    
    
}

int main(int argc, char *argv[]) {


    if(argc != 2)
    {
        std::cout << "[!] Debes de pasar el 'numero del puerto' como parametro";
        std::cout << "[!] Ejemplo: ./server 54001";
        exit(1);
    }

    struct sockaddr_in stSockAddr;
    socklen_t client;
    int SocketFD;

    if ((SocketFD = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[!] Error to create socket");
        exit(1);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(atoi(argv[1]));
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(struct sockaddr)) == -1) {
        perror("[!] Error to binding");
        exit(1);
    }

    if (listen(SocketFD, 1) == -1) {
        perror("[!] Error in listen");
        exit(1);
    }

    std::cout << "[+] Server started\n";

    while (true) {

        // choosePlayer
        if(!in_game && clientNicknames.size() >= 2)
        {
            ticTacToePreparation();
        }

        client = sizeof(struct sockaddr_in);

        int ConnectFD = accept(SocketFD, nullptr, nullptr);
        if (ConnectFD == -1) {
            perror("[!] Error accepting connection");
            continue;
        }
        // Crear un nuevo hilo para manejar al cliente
        std::thread(HandleClient, ConnectFD).detach();
    }

    close(SocketFD);

    return 0;
}