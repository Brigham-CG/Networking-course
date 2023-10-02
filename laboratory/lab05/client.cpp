// network
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
#include <string>
#include <iostream>
#include <vector>
#include <cstring>

// timestamp 
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

// file 
#include <fstream>
#include <sys/stat.h>

// hash 
#include <openssl/sha.h>

// TicTacToe
#include "ticTacToe.hpp"

int SocketFD;
std::string nickname;

std::string hash_ts;

char symbol[2];

std::vector<std::string> playerList;

TicTacToe game;

bool in_game = 0;

bool in_turn = 0;

int playerTurn = 0;

bool visualize = 0;


// tools
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

std::string calculateSHA1(unsigned char* data, int size, std::string timestamp) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX shaContext;
    if (!SHA1_Init(&shaContext)) {
        std::cerr << "Error al inicializar el contexto SHA-1." << std::endl;
        return "";
    }
    
    if (!SHA1_Update(&shaContext, data, size)) {
        std::cerr << "Error al actualizar el contexto SHA-1 con datos." << std::endl;
        return "";
    }

    if (!SHA1_Update(&shaContext, timestamp.c_str(), timestamp.size())) {
        std::cerr << "Error al actualizar el contexto SHA-1 con la marca de tiempo." << std::endl;
        return "";
    }

    if (!SHA1_Final(hash, &shaContext)) {
        std::cerr << "Error al finalizar el cálculo del hash SHA-1." << std::endl;
        return "";
    }

    // Convertir el resultado en una cadena hexadecimal
    std::stringstream ss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return ss.str();
}


// response of server

std::string sendInitNotification()
{
    std::cout << "Input the Nickname: ";

    std::getline(std::cin, nickname);

    std::string notificationData = "N" + completeByteSize(nickname.size(), 2) + nickname;

    // Enviar el nicknabme al servidor
    
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

void obtainingFile()
    
{
    // ##### prepayload #####
    char size_source[3];

    int nBytes;

    // source client data
    nBytes = recv(SocketFD, size_source, 2, 0);

    size_source[nBytes] = '\0';

    int sizeSource = atoi(size_source);

    char source[sizeSource + 1];


    nBytes = recv(SocketFD, source, sizeSource, 0);

    source[nBytes] = '\0';

    // file name of source client

    char size_fileName[6]; 

    nBytes = recv(SocketFD, size_fileName, 5, 0);

    size_fileName[nBytes] = '\0';

    int sizeFilename = atoi(size_fileName);

    char fileName[sizeFilename + 1];

    nBytes = recv(SocketFD, fileName, sizeFilename, 0);

    fileName[nBytes] = '\0';
    
    // file content

    char size_file[16];

    nBytes = recv(SocketFD, size_file, 15, 0);

    size_file[nBytes] = '\0';

    int sizeFile = atoi(size_file);

    // ##### end prepayload #####

    // ##### hash and timestamp #####
    // hash 
    char *hash = new char[41];

    nBytes = recv(SocketFD, hash, 40, 0);

    hash[nBytes] = '\0';

    char *timeStamp = new char[15];

    nBytes = recv(SocketFD, timeStamp, 14, 0);
    
    timeStamp[nBytes] = '\0';

    // ##### end hash and timestamp #####


    // obtaining binary data 
    int bufferSize = 1024;

    unsigned char buffer[bufferSize];

    unsigned char *fileData = new unsigned char [sizeFile];

    int totalRead = 0;

    while (totalRead < sizeFile) {
        nBytes = recv(SocketFD, buffer, bufferSize, 0);
        std::memcpy(fileData + totalRead, buffer, nBytes);
        totalRead += nBytes;
    }


    std::string result = calculateSHA1(fileData, sizeFile, timeStamp);

    std::cout << "\n [*] Recalculated Hash: " << hash << std::endl;

    if(strcmp(hash, result.c_str()) != 0)
    {
        delete [] hash;
        delete [] timeStamp;
        std::cerr << "\n [!] La integridad del archivo esta corrupta" << std::endl;
        return;
    }

    delete [] hash;
    delete [] timeStamp;

    
    // ##### saving file in local #####
    std::ofstream archivo_escritura(fileName, std::ios::binary);

    if (!archivo_escritura) {
        std::cerr << "\n [!] No se puede guardar el archivo de salida" << std::endl;
        return;
    }
    archivo_escritura.write(reinterpret_cast<char*>(fileData), totalRead);

    archivo_escritura.close();  

    delete [] fileData;

    // ##### end saving file in local #####

    // ##### end obtaining file #####

    // ##### confirmation response #####

    std::string payload;

    payload = "R" + completeByteSize(sizeSource, 2) + source + result;

    send(SocketFD, payload.c_str(), payload.size(), 0);

    std::cout << "\n [+] File received!" << std::endl;
    // ##### end confirmation response #####
}

void confirmingReception()
{
    // confirmation obtaining
 
    int nBytes;
    
    char size_source[3];

    nBytes = recv(SocketFD, size_source, 2, 0);

    size_source[nBytes] = '\0';

    int sizeSource = atoi(size_source);

    char source[sizeSource + 1];

    nBytes = recv(SocketFD, source, sizeSource, 0);
    source[nBytes] = '\0';

    char hash[41];

    nBytes = recv(SocketFD, hash, 40, 0);

    hash[nBytes] = '\0';

    std::cout << "\n [*] Hash from destination:" << hash << std::endl;

    if (strcmp(hash_ts.c_str(), hash) != 0)
    {
        std::cout << "\n [!] La integridad del archivo fue corrompida en el envio!\n";
        return;
    }

    std::cout << "\n [+] File sended and received!" << std::endl;
}


void cleaning_game()
{

    in_game = 0;
    in_turn = 0;
    playerList.clear();
    playerTurn = 0;

}

void acceptTicTacToe()
{   
    std::cout << "\n [+] Request for play by the server\n" << std::endl;
    int nBytes = recv(SocketFD, symbol, 1, 0);
    symbol[nBytes] = '\0';

    char state[5] = "BTTT";
    send(SocketFD, state, 4, 0);
}

void startTicTacToe()
{   

    //size of list
    char size_list[3];
    int nBytes;

    nBytes = recv(SocketFD, size_list, 2, 0);

    size_list[nBytes] = '\0';

    int sizeList = atoi(size_list);

    // list 
    char list[sizeList + 1];

    nBytes = recv(SocketFD, list, sizeList, 0);

    list[nBytes] = '\0';

    const char delimiter[] = ","; 

    // Usar strtok para dividir la cadena en tokens
    char *token = std::strtok(list, delimiter);

    while (token != nullptr) {
        
        playerList.push_back(token); // Agregar el token a la lista
        token = std::strtok(nullptr, delimiter); // Obtener el siguiente token
    }

    std::cout << "\n Players\n\n";

    for(int i = 0; i < playerList.size(); i++)
    {    
        std::cout << " * " << playerList[i] << std::endl;
    }

    in_game = 1;

    game.mostrarTablero();

    if(playerList[0] == nickname)
    {
        std::cout << "\n [+] You start!\n"; 
        in_turn = 1;
        playerTurn = 0;
    }
    else{
        std::cout << "\n [+] You've  second turn!\n"; 
        playerTurn = 1;
    }
}

void receivingPlay()
{
    
    int nBytes;
    char pos_extract[2];

    nBytes = recv(SocketFD, pos_extract, 1, 0);

    pos_extract[nBytes] = '\0';

    int position = atoi(pos_extract);

    game.hacerMovimiento(position);
    game.mostrarTablero();
    in_turn = 1;

    if(game.verificarEstado() != ' ')
    {
        std::cout << "\n [*] you lose!";
        cleaning_game();
    }

    std::cout << " \n[+] It's your turn!\n" << std::endl;
}

void receivingView()
{
    int nBytes;
    char pos_extract[2];

    nBytes = recv(SocketFD, pos_extract, 1, 0);

    pos_extract[nBytes] = '\0';

    int position = atoi(pos_extract);

    game.hacerMovimiento(position);

    game.mostrarTablero();
}

void winnerStatus()
{

    char winnerContent[4];

    int nBytes = recv(SocketFD, winnerContent, 3, 0);

    winnerContent[nBytes] = '\0';

    std::cout << "\n\n";
    std::cout << "                   .-=========-.\n";
    std::cout << "                   \\'-=======-'/\n";
    std::cout << "                   _|    .=.  |_\n";
    std::cout << "                  ((|  {{1}}  |))\n";
    std::cout << "                   \\|   /|\\   |/\n";
    std::cout << "                    \\__ '`' __/\n";
    std::cout << "                      _`) (`_\n";
    std::cout << "                    _/_______\\_\n";
    std::cout << "                   /___________\\\n\n";

    std::cout << "             You are the WINNER!!! \\O_0/\n";
    
    cleaning_game();

}

void loserStatus()
{
    char winnerContent[4];

    int nBytes = recv(SocketFD, winnerContent, 3, 0);

    winnerContent[nBytes] = '\0';

    std::cout << " [!] Your lose!!!\n";
    cleaning_game();
}

void tiedStatus()
{

    char tiedContent[7];

    int nBytes = recv(SocketFD, tiedContent, 6, 0);

    tiedContent[nBytes] = '\0';

    std::cout << " [!] You tieded!!!\n";
    cleaning_game();

}

void information_game()
{
    //size of list
    char size_list[3];
    int nBytes;

    nBytes = recv(SocketFD, size_list, 2, 0);

    size_list[nBytes] = '\0';

    int sizeList = atoi(size_list);

    // list 
    char list[sizeList + 1];

    nBytes = recv(SocketFD, list, sizeList, 0);

    list[nBytes] = '\0';

    const char delimiter[] = ","; 

    // Usar strtok para dividir la cadena en tokens
    char *token = std::strtok(list, delimiter);

    while (token != nullptr) {
        
        playerList.push_back(token); // Agregar el token a la lista
        token = std::strtok(nullptr, delimiter); // Obtener el siguiente token
    }

    std::cout << "\n Players\n\n";

    if(playerList.size() == 1)
    {
        std::cout << "\n [*] Han empatado\n";
    }
    else{
        std::cout << "\n [*] " << playerList[0] << " -> winner!\n";
        std::cout << " [*] " << playerList[1] << " -> loser!\n";
    }
    visualize = 0;
    
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
        else if(buffer[0] == 'F')
            obtainingFile();
        else if(buffer[0] == 'R')
            confirmingReception();
        else if(buffer[0] == 'B')
            acceptTicTacToe();
        else if(buffer[0] == 'T')
            startTicTacToe();
        else if(in_game && !in_turn && buffer[0] == 'P')
            receivingPlay();
        else if(visualize && buffer[0] == 'V')
            receivingView();
        else if(in_game && !in_turn && buffer[0] == 'W')
            winnerStatus();
        else if(in_game && buffer[0] == 'L')
            loserStatus();
        else if(in_game && buffer[0] == 'E')
            tiedStatus();
        else if(visualize && buffer[0] == 'Z')
            information_game();
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
    
    std::string destination = message.substr(0, pos);
    std::string content = message.substr(pos + 2); 

    if(destination == "all")
    {
        payload = "W"+completeByteSize(content.size(), 3) + content; 
    }
    else{
        payload = "M"+completeByteSize(destination.size(), 2) + destination +  completeByteSize(content.size(), 3) + content; 
    }

    // std::cout << "payload :" << payload<< std::endl;
    send(SocketFD, payload.c_str(), payload.size(), 0);
}

void sendFile(std::string command)
{
    // descomposing command
    size_t pos1 = command.find(",");

    if(pos1 == std::string::npos)
        return;

    size_t pos2 = command.substr(pos1+2).find(",");

    if(pos2 == std::string::npos)
        return;

    std::string fileName = command.substr(pos1+2, pos2);

    std::string destination = command.substr(pos1+pos2+4);

    // reading file content
    std::ifstream file(fileName, std::ios::binary);

    if (!file) {
        std::cerr << "\n [!] Error al abrir el archivo." << std::endl;
        return;
    }

    // Obtener el tamaño del archivo
    struct stat file_stat;
    if (stat(fileName.c_str(), &file_stat) == -1) {
        std::cerr << "\n [!] Error al obtener el tamaño del archivo." << std::endl;
        return ;
    }

    // Establecer el tamaño del búfer
    const int fileSize = file_stat.st_size;
    unsigned char*fileData = new unsigned char[fileSize];

    file.read(reinterpret_cast<char*>(fileData), fileSize);

    int bytesRead = file.gcount();

    // making pre-payload 

    std::string prePayload = "F" + 
        completeByteSize(destination.size(), 2) + destination + 
        completeByteSize(fileName.size(), 5) + fileName + 
        completeByteSize(bytesRead, 15);

    send(SocketFD, prePayload.c_str(), prePayload.size(), 0);  

    // time stamp
    auto tiempo_actual = std::chrono::system_clock::now();

    // Convierte el timestamp a un formato de tiempo legible
    std::time_t tiempo_convertido = std::chrono::system_clock::to_time_t(tiempo_actual);

    // Obtiene las partes individuales de la fecha y hora
    std::tm tiempo_descompuesto = *std::localtime(&tiempo_convertido);
    
    std::ostringstream formato;
    formato << std::put_time(&tiempo_descompuesto, "%Y%m%d%H%M%S");

    std::string timeStamp = formato.str();

    hash_ts = calculateSHA1(fileData, bytesRead, timeStamp);

    std::string hash_complete = hash_ts + timeStamp;

    std::cout << "\n [*] Hash of file(hash + ts): " << hash_ts << std::endl;
    
    send(SocketFD, hash_complete.c_str() , hash_complete.size(), 0);

    // send data file

    send(SocketFD, fileData, bytesRead, 0);

    delete [] fileData;
}

void sendPosition(std::string command)
{
    // descomposing command
    size_t pos = command.find(",");

    if(pos == std::string::npos)
        return;

    std::string pos_extract = command.substr(pos + 2, 1);
    
    int position = atoi(pos_extract.c_str());


    pos_extract = "P" + pos_extract;

    if(game.hacerMovimiento(position))
    {
        game.mostrarTablero();
        send(SocketFD, pos_extract.c_str(), pos_extract.size(), 0);
        in_turn = 0;

        std::cout << " \n[+] El turno de " << playerList[playerTurn ? 0:1] << std::endl;
    }
    else
    {
        game.mostrarTablero();
        std::cout << "\n [!] Invalid movement. try again!" << std::endl;
    }
}

void sendReqViewTicTacToeGame()
{
    std::cout << " [+] Request to view\n";
    std::string reqView = "VTTT";

    visualize = 1;
    send(SocketFD, reqView.c_str(), reqView.size(), 0);
}

int main(int argc, char *argv[]) {

    if(argc != 3)
    {
        perror("[!] Debes de pasar la 'direccion ip' del puerto y el 'numero del puerto' como parametros");
        perror("[!] Ejemplo: ./client 127.0.0.1 54001");
        exit(1);
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

    std::cout << "-------------------------COMMANDS-------------------------\n\n\n";
    std::cout << "list              --------------> list users in server\n\n";
    std::cout << "all, message      --------------> send message to all users in server\n\n";
    std::cout << "nickname, message --------------> send message to specific user\n\n";
    std::cout << "file, 'name of file', nickname -> send a file to other user\n\n";
    std::cout << "pos, 'position'    -------------> send to position of Board in TicTacToe game\n\n";
    std::cout << "view               -------------> spectate a TicTacToe game\n\n";
    std::cout << "quit              --------------> exit of service\n\n\n";


    sendInitNotification();

    // Iniciar un hilo para recibir mensajes del servidor
    std::thread(ReceiveMessages).detach();

    // Leer mensajes del usuario y enviarlos al servidor
    while (true) {

        std::string command;

        std::cout << nickname <<" #";

        std::getline(std::cin, command);

        if (command == "list")
            reqListName();
        else if (command == "quit")
        {
            quitServer();
            break;
        }
        else if (command.substr(0,4) == "file" && command.size() > 4)
        {
            sendFile(command);
        }
        else if (command.substr(0,3) == "pos" && command.size() > 3)
        {   
            if(!in_game)
            {
                std::cout << "\n [!] You aren't in game!\n";
                continue;
            }
            if(!in_turn)
            {
                std::cout << "\n [!] Wait it isn't your turn!\n";
                continue;
            }
            sendPosition(command);
        }
        else if (command == "view")
        {
            sendReqViewTicTacToeGame();
        }
        else 
            sendMessage(command);
    }

    close(SocketFD);

    return 0;
}
