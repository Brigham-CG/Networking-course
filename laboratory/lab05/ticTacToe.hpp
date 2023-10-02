#include <iostream>

class TicTacToe
{
private:
    char tablero[9];
    char jugadorActual;

public:

    TicTacToe()
    {
        for (int i = 0; i < 9; ++i)
            tablero[i] = '1' + i;

        jugadorActual = 'X';
    }

    void mostrarTablero()
    {
        std::cout << "Tablero de Tic-Tac-Toe:" << std::endl;
        for (int i = 0; i < 9; ++i)
        {
            std::cout << tablero[i];
            if (i % 3 != 2)
            {
                std::cout << '|'; // Separador entre columnas
            }
            if (i == 2 || i == 5)
            {
                std::cout << std::endl << "-----" << std::endl; // Línea horizontal entre filas
            }
        }
        std::cout << std::endl;
    }

    bool hacerMovimiento(int posicion)
    {
        // Verifica si la casilla está disponible
        if (posicion >= 1 && posicion <= 9 && tablero[posicion - 1] == '1' + posicion - 1)
        {
            tablero[posicion - 1] = jugadorActual;
            jugadorActual = (jugadorActual == 'X') ? 'O' : 'X'; // Cambia el jugador
            return true; // Movimiento válido
        }
        return false; // Movimiento inválido
    }

    char verificarEstado()
    {
        // Verifica filas, columnas y diagonales
        for (int i = 0; i < 3; ++i)
        {
            if (tablero[i] == tablero[i + 3] && tablero[i + 3] == tablero[i + 6])
            {
                return tablero[i]; // Hay un ganador en una columna
            }
            if (tablero[3 * i] == tablero[3 * i + 1] && tablero[3 * i + 1] == tablero[3 * i + 2])
            {
                return tablero[3 * i]; // Hay un ganador en una fila
            }
        }

        // Verifica diagonales
        if (tablero[0] == tablero[4] && tablero[4] == tablero[8])
        {
            return tablero[0]; // Diagonal principal
        }
        if (tablero[2] == tablero[4] && tablero[4] == tablero[6])
        {
            return tablero[2]; // Diagonal secundaria
        }

        // Verifica si el juego ha terminado en empate
        bool empate = true;
        for (int i = 0; i < 9; ++i)
        {
            if (tablero[i] != 'X' && tablero[i] != 'O')
            {
                empate = false; // Todavía hay casillas vacías, no es un empate
                break;
            }
        }
        if (empate)
        {
            return 'E'; // El juego termina en empate
        }

        return ' '; // El juego está en curso
    }

    char getJugadorActual() const
    {
        return jugadorActual;
    }

};
