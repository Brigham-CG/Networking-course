#include <iostream>
#include <string>
#include <functional>

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

#endif