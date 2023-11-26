#include <iostream>
#include <string>
#include <functional>
#include <ctime>
#include <cstdlib>

#ifndef COMPLETO_H
#define COMPLETO_H

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

size_t hashString(const std::string &str) {
    std::hash<std::string> stringHash;
    return stringHash(str);
}

char generateCharacter(std::vector<char>& caracteresUtilizados) {
    const char caracteresPermitidos[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const int longitud = sizeof(caracteresPermitidos) - 1;

    std::srand(std::time(0));

    char caracterAleatorio;

    do {
        int indiceAleatorio = std::rand() % longitud;
        caracterAleatorio = caracteresPermitidos[indiceAleatorio];
    } while (std::find(caracteresUtilizados.begin(), caracteresUtilizados.end(), caracterAleatorio) != caracteresUtilizados.end());

    // Agregar el caracter utilizado al vector
    caracteresUtilizados.push_back(caracterAleatorio);

    return caracterAleatorio;
}

#endif