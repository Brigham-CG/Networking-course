#include <iostream>
#include <string>

#include <ctime>
#include <cstdlib>

char generarCaracterAlfanumerico(std::string& caracteresUtilizados) {
    const char caracteresPermitidos[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const int longitud = sizeof(caracteresPermitidos) - 1;

    std::srand(std::time(0));

    int indiceAleatorio;
    char caracterAleatorio;

    do {
        indiceAleatorio = std::rand() % longitud;
        caracterAleatorio = caracteresPermitidos[indiceAleatorio];
    } while (caracteresUtilizados.find(caracterAleatorio) != std::string::npos);

    // Agregar el caracter utilizado al registro
    caracteresUtilizados += caracterAleatorio;

    return caracterAleatorio;
}

int main() {
    std::string caracteresUtilizados;
    
    for (int i = 0; i < 10; ++i) {
        char caracter = generarCaracterAlfanumerico(caracteresUtilizados);
        std::cout << "Caracter alfanumerico aleatorio: " << caracter << std::endl;
    }

    return 0;
}