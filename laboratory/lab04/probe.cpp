#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

int main() {
    auto tiempo_actual = std::chrono::system_clock::now();

    std::ifstream archivo_lectura("mario.png", std::ios::binary);
    if (!archivo_lectura) {
        std::cerr << "No se pudo abrir el archivo de entrada." << std::endl;
        return 1;
    }

    // Lee todo el contenido del archivo de entrada en un array de caracteres
    archivo_lectura.seekg(0, std::ios::end);
    std::streampos tamano = archivo_lectura.tellg();
    archivo_lectura.seekg(0, std::ios::beg);

    if (tamano > 0) {
        // char* datos = new char[tamano];
        char datos[(int)tamano + 1];
        archivo_lectura.read(datos, tamano);

        std::cout << tamano << std::endl;
        std::cout << datos << std::endl;

        archivo_lectura.close();    

        // Ahora 'datos' contiene los datos del archivo de entrada

        // Supongamos que deseas guardar los datos en un nuevo archivo
        std::ofstream archivo_escritura("mario2.png", std::ios::binary);

        if (!archivo_escritura) {
            std::cerr << "No se pudo abrir el archivo de salida." << std::endl;
            // delete[] datos; // Liberar la memoria asignada para datos
            return 1;
        }

        int i = 0;

        std::string nombre = datos;

        std::cout << "datos: " << nombre << " s: " <<nombre.size() << std::endl;

        // Escribe los datos del array en el archivo de salida
        archivo_escritura.write(nombre.c_str(), nombre.size());

        archivo_escritura.close();

        std::cout << "Datos guardados en el archivo de salida." << std::endl;

        // Liberar la memoria asignada para datos
        // delete[] datos;
    }

    return 0;
}