#include <iostream>
#include <string>
#include <openssl/sha.h>

std::string calcularSHA1(const std::string &input) {
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

int main() {
    std::string texto = "Hola, mundo!";
    std::string sha1 = calcularSHA1(texto);

    std::cout << "SHA-1 de '" << texto << "': " << sha1 << std::endl;

    return 0;
}
