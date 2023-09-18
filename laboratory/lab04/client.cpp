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

// hash 
#include <openssl/sha.h>


int SocketFD;
std::string nickname;

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

std::string calculateSHA1(const std::string &input) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char *>(input.c_str()), input.length(), hash);

    // Convertir el resultado en una cadena hexadecimal
    std::string resultado;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", hash[i]);
        resultado += hex;
    }

    return resultado;
}


// response of server

std::string sendInitNotification()
{
    std::cout << "Input the Nickname: ";

    std::getline(std::cin, nickname);

    std::string notificationData = "N" + completeByteSize(nickname.size(), 2) + nickname;

    // Enviar el nickname al servidor
    
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

    char fileData[sizeFile + 1];

    nBytes = recv(SocketFD, fileData, sizeFile, 0);

    fileData[nBytes] = '\0';

    // hash camp
    char hash[41];

    nBytes = recv(SocketFD, hash, 40, 0);

    hash[nBytes] = '\0';

    char timeStamp[15];

    nBytes = recv(SocketFD, timeStamp, 14, 0);
    
    timeStamp[nBytes] = '\0';

    std::string hash_tm;

    hash_tm += fileData;
    hash_tm += timeStamp;


    // std::cout << "hash: " << hash_tm << std::endl;
    std::string result = calculateSHA1(hash_tm);

    // std::cout << "hash: '" << hash <<  " hashtm: '" << hash_tm << std::endl;

    if(strcmp(hash, result.c_str()) != 0)
    {
        std::cerr << "\n[!] La integridad del archivo esta corrupta" << std::endl;
        return;
    }

    // saving file
    std::ofstream archivo_escritura(fileName, std::ios::binary);

    if (!archivo_escritura) {
        std::cerr << "\n[!] No se pudo abrir el archivo de salida" << std::endl;
        return;
    }

    archivo_escritura.write(fileData, sizeFile);

    archivo_escritura.close();

    // confirmation send

    // std::string payload;

    // payload = "R" + completeByteSize(sizeSource, 2) + source + result;

    // std::cout << "r send: " << payload << " "<<payload.size() <<std::endl; 

    // send(SocketFD, payload.c_str(), payload.size(), 0);

    // std::cout << "\n [+] File received!" << std::endl;
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
        {
            obtainingFile();
        }
        
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

    // reading file
    std::ifstream archivo_lectura(fileName, std::ios::binary);

    if (!archivo_lectura) {
        std::cerr << "\n[!] No se pudo abrir el archivo de entrada" << std::endl;
        return;
    }

    archivo_lectura.seekg(0, std::ios::end);

    std::streampos fileSize = archivo_lectura.tellg();

    archivo_lectura.seekg(0, std::ios::beg);

    // char fileData[(int) fileSize]; 
    char* fileData = new char[fileSize];

    if (fileSize == 0) {
        std::cerr << "\n[!] Error de lectura" << std::endl;
        return;
    }

    // copy fileData
    archivo_lectura.read(fileData, fileSize);

    // fileData[fileSize] = '\0';

    archivo_lectura.close();

    std::cout << " \nFile readed: " << fileSize << std::endl;

    // time stamp
    auto tiempo_actual = std::chrono::system_clock::now();

    // Convierte el timestamp a un formato de tiempo legible
    std::time_t tiempo_convertido = std::chrono::system_clock::to_time_t(tiempo_actual);

    // Obtiene las partes individuales de la fecha y hora
    std::tm tiempo_descompuesto = *std::localtime(&tiempo_convertido);
    
    std::ostringstream formato;
    formato << std::put_time(&tiempo_descompuesto, "%Y%m%d%H%M%S");

    std::string timeStamp = formato.str();

    std::string hash_ts = calculateSHA1(fileData + timeStamp);

    std::cout << "'"<<fileData <<"'"<< std::endl;

    // making payload
    std::string payload =  "F" 
     + completeByteSize(destination.size(), 2) + destination
     + completeByteSize(fileName.size(), 5) + fileName
     + completeByteSize(fileSize, 15) + fileData
     + hash_ts
     + timeStamp;

    send(SocketFD, payload.c_str(), payload.size(), 0);
    std::cout << "[-] Send\n";
    delete [] fileData;
    // confirmation sended

    // char option[2];
    // int nBytes;

    // nBytes = recv(SocketFD, option, 1, 0);
    // option[2] = '\0';

    // std::cout << "option: " << option << std::endl;

    // if (option[0] != 'R')
    // {
    //     std::cout << "\n[!] No hubo confirmacion del envio, vuelva a intentarlo\n";
    //     return;
    // }
    
    // char size_source[3];

    // nBytes = recv(SocketFD, size_source, 2, 0);

    // size_source[nBytes] = '\0';

    // std::cout << "s_source: " << size_source << std::endl;

    // int sizeSource = atoi(size_source);

    // char source[sizeSource + 1];

    // nBytes = recv(SocketFD, source, sizeSource, 0);
    // source[nBytes] = '\0';

    // std::cout << "source: " << source << std::endl;

    // std::cout << source  << std::endl;
    // if (strcmp(destination.c_str(), source) != 0)
    // {
    //     std::cout << "[+] Los usuarios no concuerdan\n";
    //     return;
    // }

    // char hash[41];

    // nBytes = recv(SocketFD, hash, 40, 0);

    // hash[nBytes] = '\0';

    // std::cout << "hash: " << hash << std::endl;

    // if (strcmp(hash_ts.c_str(), hash) != 0)
    // {
    //     std::cout << "\n[!] La integridad del archivo fue corrompida en el envio!\n";
    //     return;
    // }

    // std::cout << "\n[+] File sended and received!" << std::endl;

}

int main(int argc, char *argv[]) {

    if(argc != 3)
    {
        perror("[!] Debes de pasar la 'direccion ip' del puerto y el 'numero del puerto' como parametros");
        perror("[!] Ejemplo: ./client 127.0.0.1 54001");
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

    std::cout << "-------------------------COMMANDS-------------------------\n";
    std::cout << "list ---------------------------> list users in server\n";
    std::cout << "all, message -------------------> send message to all users in server\n";
    std::cout << "nickname, message --------------> send message to specific user\n";
    std::cout << "file, 'name of file', nickname -> send a file to other user\n";
    std::cout << "quit ---------------------------> exit of service\n\n";


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
        else 
            sendMessage(command);
    }

    close(SocketFD);

    return 0;
}
