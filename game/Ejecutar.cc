#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <map>
#include <algorithm>

using namespace std;

void printUsage() {
    cout << "Uso: ./Ejecutar s1 s2 s3 s4 -s int1" << endl;
    cout << "s1, s2, s3, s4: cadenas de texto (jugadores)." << endl;
    cout << "-s: bandera para indicar el entero que sigue." << endl;
    cout << "int1: número entero." << endl;
}

vector<int> extractScores(const string& scoreLine) {
    vector<int> scores;
    istringstream iss(scoreLine);
    string token;
    while (iss >> token) {
        if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
            scores.push_back(stoi(token));
        }
    }
    return scores;
}

string getLastScoreLine(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        return "";
    }

    string line;
    string lastScoreLine;
    while (getline(file, line)) {
        if (line.find("score") != string::npos) {
            lastScoreLine = line;
        }
    }

    file.close();
    return lastScoreLine;
}

int main(int argc, char* argv[]) {
    if (argc != 7 || string(argv[5]) != "-s") {
        printUsage();
        return 1;
    }

    // Capturar argumentos en un vector
    vector<string> players = {argv[1], argv[2], argv[3], argv[4]};
    int number;

    try {
        number = stoi(argv[6]);
    } catch (...) {
        cerr << "Error: El argumento del número entero no es válido." << endl;
        return 1;
    }

    // Mapas para almacenar sumas de scores y número de partidas para cada índice de jugador
    map<int, int> totalScores;
    map<int, int> gamesPlayed;

    // Inicializar mapas para 4 jugadores
    for (int i = 0; i < players.size(); ++i) {
        totalScores[i] = 0;
        gamesPlayed[i] = 0;
    }

    // Crear todas las permutaciones posibles de los índices
    vector<int> playerIndices = {0, 1, 2, 3};
    vector<vector<int>> permutations;
    do {
        permutations.push_back(playerIndices);
    } while (next_permutation(playerIndices.begin(), playerIndices.end()));

    // Ejecutar el comando para cada permutación
    for (const auto& perm : permutations) {
        pid_t pid = fork(); // Crear un proceso hijo

        if (pid < 0) {
            cerr << "Error: No se pudo crear un proceso hijo." << endl;
            return 1;
        } else if (pid == 0) {
            // Proceso hijo
            string command = "./Game";
            vector<char*> args;

            // Agregar los argumentos al comando según la permutación actual
            args.push_back(&command[0]);
            for (int idx : perm) {
                args.push_back(const_cast<char*>(players[idx].c_str()));
            }
            args.push_back(const_cast<char*>("-s"));
            args.push_back(const_cast<char*>(to_string(number).c_str()));
            args.push_back(nullptr); // El último argumento debe ser nullptr

            // Redirigir la entrada y salida
            freopen("default.cnf", "r", stdin);
            freopen("default.res", "w", stdout);

            // Ejecutar el comando
            execvp(command.c_str(), args.data());

            // Si execvp falla
            cerr << "Error: No se pudo ejecutar el comando." << endl;
            exit(1);
        } else {
            // Proceso padre
            int status;
            waitpid(pid, &status, 0); // Esperar a que el hijo termine

            if (WIFEXITED(status)) {
                string scoreLine = getLastScoreLine("default.res");
                if (!scoreLine.empty()) {
                    vector<int> scores = extractScores(scoreLine);
                    if (scores.size() == players.size()) {
                        for (size_t j = 0; j < scores.size(); ++j) {
                            totalScores[perm[j]] += scores[j];
                            gamesPlayed[perm[j]]++;
                        }
                    } else {
                        cerr << "Error: El número de scores no coincide con el número de jugadores." << endl;
                    }
                } else {
                    cerr << "No se encontró ninguna línea con 'score' en default.res." << endl;
                }
            } else {
                cerr << "El proceso hijo terminó anormalmente." << endl;
            }
        }
    }

    // Calcular y mostrar las medias
    cout << "Promedios de scores por jugador:" << endl;
    for (int i = 0; i < players.size(); ++i) {
        double average = (gamesPlayed[i] > 0) ? static_cast<double>(totalScores[i]) / gamesPlayed[i] : 0.0;
        cout << players[i] << ": " << average << endl;
    }

    return 0;
}
