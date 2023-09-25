#include <iostream>
#include <fstream>
#include <vector>

int main(int argc, char **argv) {
    std::cout << argv[1] << std::endl;
    std::ifstream inputFile(argv[1], std::ios::binary);
    if (!inputFile) {
        std::cerr << "Error al abrir el archivo de entrada." << std::endl;
        return 1;
    }

    // Obtener el tamaño del archivo
    inputFile.seekg(0, std::ios::end);
    std::streampos fileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    if (fileSize <= 0) {
        std::cerr << "El archivo está vacío o no se pudo determinar su tamaño." << std::endl;
        return 1;
    }

    // Establecer el tamaño del búfer
    std::vector<unsigned char> buffer(fileSize);

    std::cout << "s " <<buffer.size() << std::endl;
    std::cout << "s " <<fileSize << std::endl;

    // Leer el archivo en el búfer
    inputFile.read(reinterpret_cast<char*>(&buffer[0]), fileSize);

    inputFile.close();

    std::string ofile = argv[1];
    ofile = "a" + ofile;
    std::ofstream outputFile(ofile, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Error al abrir el archivo de salida." << std::endl;
        return 1;
    }

    // Escribir el búfer en el archivo de salida
    outputFile.write(reinterpret_cast<char*>(&buffer[0]), fileSize);
    outputFile.close();

    std::cout << "Archivo copiado exitosamente." << std::endl;

    return 0;
}