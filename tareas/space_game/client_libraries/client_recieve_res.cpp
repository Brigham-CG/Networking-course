#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>

socklen_t addr_len = sizeof(struct sockaddr);

void getListUsers(std::string message) {
    int init = 1;

    // Tamaño de la lista
    std::string size_list = message.substr(init, 3);
    int sizeList = std::stoi(size_list);
    init += 3;

    // Lista
    std::string list = message.substr(init, sizeList);

    // Dividir la lista en tokens utilizando stringstream
    std::istringstream ss(list);
    std::string token;
    std::vector<std::string> tokens;

    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }

    // Construir la lista de nombres
    std::string listaNombres;

    for (const std::string& nombre : tokens) {
        listaNombres += " * " + nombre + "\n";
    }

    std::cout << "\nList of users\n";
    std::cout << listaNombres;
}

void obtainingMessage(std::string message)
{
    int init = 1;

    // source client data

    std::string size_source = message.substr(init, 2);
    
    int sizeSource = stoi(size_source);
    init += 2;

    std::string source = message.substr(init, sizeSource);

    init += sizeSource;

    // message of client

    std::string size_msg = message.substr(init, 3);

    int sizeMsg = stoi(size_msg);

    init += 3;

    std::string content = message.substr(init, sizeMsg);

    std::cout << "\n" << source << " : "<< content << std::endl;
}
