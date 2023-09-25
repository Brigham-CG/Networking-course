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

    // std::getline(std::cin, nickname);

    nickname = "Brigham";

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

    // char size_source[3];

    // int nBytes;

    // // source client data
    // nBytes = recv(SocketFD, size_source, 2, 0);

    // size_source[nBytes] = '\0';

    // int sizeSource = atoi(size_source);

    // char source[sizeSource + 1];

    // nBytes = recv(SocketFD, source, sizeSource, 0);

    // source[nBytes] = '\0';

    // // file name of source client

    // char size_fileName[6]; 

    // nBytes = recv(SocketFD, size_fileName, 5, 0);

    // size_fileName[nBytes] = '\0';

    // int sizeFilename = atoi(size_fileName);

    // char fileName[sizeFilename + 1];

    // nBytes = recv(SocketFD, fileName, sizeFilename, 0);

    // fileName[nBytes] = '\0';

    // // file content

    // char size_file[16];

    // nBytes = recv(SocketFD, size_file, 15, 0);

    // size_file[nBytes] = '\0';

    // int sizeFile = atoi(size_file);

    // char fileData[sizeFile + 1];

    // nBytes = recv(SocketFD, fileData, sizeFile, 0);

    // fileData[nBytes] = '\0';

    // // hash camp
    // char hash[41];

    // nBytes = recv(SocketFD, hash, 40, 0);

    // hash[nBytes] = '\0';

    // char timeStamp[15];

    // nBytes = recv(SocketFD, timeStamp, 14, 0);
    
    // timeStamp[nBytes] = '\0';

    // std::string hash_tm;

    // hash_tm += fileData;
    // hash_tm += timeStamp;


    // // std::cout << "hash: " << hash_tm << std::endl;
    // std::string result = calculateSHA1(hash_tm);

    // // std::cout << "hash: '" << hash <<  " hashtm: '" << hash_tm << std::endl;

    // if(strcmp(hash, result.c_str()) != 0)
    // {
    //     std::cerr << "\n[!] La integridad del archivo esta corrupta" << std::endl;
    //     return;
    // }

    // // saving file
    // std::ofstream archivo_escritura(fileName, std::ios::binary);

    // if (!archivo_escritura) {
    //     std::cerr << "\n[!] No se pudo abrir el archivo de salida" << std::endl;
    //     return;
    // }

    // archivo_escritura.write(fileData, sizeFile);

    // archivo_escritura.close();

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

    // reading file content
    std::ifstream file(fileName, std::ios::binary);

    if (!file) {
        std::cerr << "Error al abrir el archivo." << std::endl;
        return;
    }

    // Obtener el tamaño del archivo
    struct stat file_stat;
    if (stat(fileName.c_str(), &file_stat) == -1) {
        std::cerr << "Error al obtener el tamaño del archivo." << std::endl;
        return ;
    }

    // Establecer el tamaño del búfer
    const int fileSize = file_stat.st_size;
    unsigned char*fileData = new unsigned char[fileSize];

    file.read(reinterpret_cast<char*>(fileData), fileSize);

    int bytesRead = file.gcount();
    std::cout << " \nFile readed: " << bytesRead << std::endl;

    // time stamp
    auto tiempo_actual = std::chrono::system_clock::now();

    // Convierte el timestamp a un formato de tiempo legible
    std::time_t tiempo_convertido = std::chrono::system_clock::to_time_t(tiempo_actual);

    // Obtiene las partes individuales de la fecha y hora
    std::tm tiempo_descompuesto = *std::localtime(&tiempo_convertido);
    
    std::ostringstream formato;
    formato << std::put_time(&tiempo_descompuesto, "%Y%m%d%H%M%S");

    std::string timeStamp = formato.str();


    std::string hash_ts = calculateSHA1(fileData, bytesRead, timeStamp);

    std::cout << "'"<<fileData <<"'"<< std::endl;

    // making payload   

    int p_size = 1 + 2 + destination.size() + 5 + fileName.size() + 15 + static_cast<int>(fileSize) + 40 + 14 + 1;
    unsigned char payload[p_size];

    payload[0] = 'F';

    int more = 1;

    std::string d_size_s = completeByteSize(destination.size(), 2);

    for(int i = 0; i < 2; i++)
    {
        payload[more + i] = d_size_s[i];
    }

    more += 2;

    for(int i = 0; i < destination.size(); i++)
    {
        payload[more + i] = destination[i];
    }

    more += destination.size();

    std::string fn_size_s = completeByteSize(fileName.size(), 5);

    for(int i = 0; i < 5; i++)
    {
        payload[more + i] = fn_size_s[i];
    }

    more += 5;

    for(int i = 0; i < fileName.size(); i++)
    {
        payload[more + i] = fileName[i];
    }

    more += fileName.size();

    std::string data_size_s = completeByteSize(bytesRead, 15);

    for(int i = 0; i < 15; i++)
    {
        payload[more + i] = data_size_s[i];
    }
    
    more += 15;

    for(int i = 0; i < bytesRead; i++)
    {
        payload[more + i] = fileData[i];
    }

    more += bytesRead;

    for(int i = 0; i < 40; i++)
    {
        payload[more + i] = hash_ts[i];
    }

    std::cout << hash_ts << std::endl;

    more += 40;

    for(int i = 0; i < 14; i++)
    {
        payload[more + i] = timeStamp[i];
    }

    std::cout << timeStamp << std::endl;

    more += 15;
    payload[more] = '\0';

    std::cout << "payload " << "'" << payload << "'" << std::endl; 

    send(SocketFD, payload, p_size, 0);  

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

    sendInitNotification();

    // Iniciar un hilo para recibir mensajes del servidor
    std::thread(ReceiveMessages).detach();

    // std::string init = "init, Brigham";

    std::string command = "file, mario.png, Tom";
    // Leer mensajes del usuario y enviarlos al servidor

    sendFile(command);

    close(SocketFD);

    return 0;
}
