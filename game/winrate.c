// Guia de uso rapida:
// Introducir nombre del bot, [nombre de otros bots], numero de rondas
// El resultado sera vuestra tasa de victoria, puntuacion media y fuerza media
// Las puntuaciones de las partidas se guardan en el fichero resultado.txt
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void Error(char* s, int e) {
    perror(s);
    exit(e);
}

// Convierte una cadena tipo: nombre   int   int   int   int
// Guarda los valores de los ints en el vector
void partir_string(char* buff, int v[4]) {
    char *token = strtok(buff, " \t");  // Usamos " \t" para manejar espacios y tabulaciones
    token = strtok(NULL, " \t"); // Saltamos la primera palabra (nombre)
    int i = 0;
    while (token != NULL && i < 4) {
        v[i] = atoi(token);  // Convertir el token en un entero
        token = strtok(NULL, " \t");  // Continuar con el siguiente token
        i++;
    }
}

void parse_scores_and_strength(int scores[4], int strengths[4]) {
    FILE *file = fopen("./default.res", "r");
    if (file == NULL) Error("Error en el open\n", 1);

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file) && (strncmp(buffer, "round 200\n", 11)));
    fgets(buffer, sizeof(buffer), file); // Empty line.
    fgets(buffer, sizeof(buffer), file); // Score
    partir_string(buffer, scores);
    fgets(buffer, sizeof(buffer), file); // Empty line.
    fgets(buffer, sizeof(buffer), file); // Scr_acc
    fgets(buffer, sizeof(buffer), file); // Empty line.
    fgets(buffer, sizeof(buffer), file); // Strength.
    partir_string(buffer, strengths);
    fclose(file);
}

void Usage() {
    static const char msg[] = "Usage: ./winrate <player_name> [other players] <number_of_runs>\n";
    write(2, msg, sizeof(msg));
    exit(1);
}

int win(int scores[4]) {
    int max = 0;
    for (int i = 0; i < 4; ++i) {
        if (scores[i] >= scores[max]) max = i;
    }
    return max;
}

int main(int argc, const char* argv[]) {
    char buff[256];
    float avg_score[4];
    float avg_strength[4];
    int scores[4];
    int strengths[4];
    int wins[4];
    for (int i = 0; i < 4; ++i) wins[i] = 0;
    for (int i = 0; i < 4; ++i) avg_score[i] = 0;
    for (int i = 0; i < 4; ++i) avg_strength[i] = 0;
    if (argc < 3 || argc > 6) Usage();
    int n = atoi(argv[argc - 1]);
    strcpy(buff, "./Game");
    for (int i = 1; i < argc - 1; ++i) sprintf(buff, "%s %s", buff, argv[i]);
    for (int i = 0; i < 6 - argc; ++i) sprintf(buff, "%s Dummy", buff);
    sprintf(buff, "%s < default.cnf 1> default.res 2>/dev/null", buff);
    remove("resultado.txt");
    for (int i = 0; i < n; ++i) {
        int pid = fork();
        if (pid == 0) {
            srandom(getpid());
            int seed = rand();
            if (seed < 0) seed = -seed;
            char buffer[256];
            sprintf(buffer, "%s -s %d", buff, seed);
            int r = execlp("bash", "bash", "-c", buffer, NULL);
            if (r < 0) Error ("Error en el execlp\n", 1);
            exit(1);
        } 
    }
    int r;
    int cont = 1;
    while ((r = waitpid(-1, NULL, WNOHANG)) >= 0) {
        if (r > 0) {
            parse_scores_and_strength(scores, strengths);
            ++wins[win(scores)];
            for (int i = 0; i < 4; ++i) {
                avg_score[i] += scores[i];
                avg_strength[i] += strengths[i];
            }
            FILE *file = fopen("resultado.txt", "a");
            if (file == NULL) Error("Error en el open\n", 1);
            fprintf(file, "Partida numero: %d\n", cont);
            fprintf(file, "Score: %d %d %d %d\n", scores[0], scores[1], scores[2], scores[3]);
            fprintf(file, "Strength: %d %d %d %d\n\n", strengths[0], strengths[1], strengths[2], strengths[3]);
            ++cont;
        }
    }
    for (int i = 1; i < argc - 1; ++i) {
        char win_rate_buff[256];
        sprintf(win_rate_buff, "Win rate %s: %.2f\n", argv[i], 100.0f*wins[i - 1]/(float)n);
        write(1, win_rate_buff, strlen(win_rate_buff));

        avg_strength[i - 1] /= (float)n;
        avg_score[i - 1] /= (float)n;
        char score_buff[256];
        sprintf(score_buff, "Your avg scores is: %.2f, your avg strength is: %.2f\n", avg_score[i - 1], avg_strength[i - 1]);
        write(1, score_buff, strlen(score_buff));
    }
}