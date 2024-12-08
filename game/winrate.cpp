#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include <random>
#include <sstream>

int readline(int fd, char* buffer, int maxLength) {
    int count = 0;
    int bytesRead;
    char c;
    while (count < maxLength - 1) {
        bytesRead = read(fd, &c, 1);
        if (bytesRead == 0) break;
        buffer[count++] = c;
        if (c == '\n') break;
    }
    buffer[count] = '\0';
    return count;
}

void Usage() {
    static const char msg[] = "Usage: ./winrate <player_name> <number_of_runs> < <config_file>\n";
    write(2, msg, sizeof(msg));
    exit(1);
}

// Absolutely no error handling.
int main(int argc, const char* argv[]) {
    char buffer[256];
    int scores[4];
    int fd[2];
    std::random_device rd;
    int wins = 0;
    if (argc != 3) Usage();
    int n = atoi(argv[2]);
    for (int i = 0; i < n; ++i) {
        int pid = fork();
        if (pid == 0) {
            int seed = rd();
            if (seed < 0) seed = -seed;
            sprintf(buffer, "./Game -s %d %s BigTortilla3 BigTortilla3 BigTortilla3 < default.cnf > default.res", seed, argv[1]);
            execlp("bash", "bash", "-c", buffer, NULL);
            exit(1);
        } else {
            waitpid(-1, NULL, 0);
            FILE* result = fopen("./default.res", "r");
            int lineCount = 0;
            while (fgets(buffer, sizeof(buffer), result)) {
                if (std::string(buffer) == std::string("round 200\n")) break;
                else ++lineCount;
            }
            fgets(buffer, sizeof(buffer), result); // Empty line.
            fgets(buffer, sizeof(buffer), result); // Good line.
            std::string content = buffer;
            std::stringstream line(content);
            std::string trash;
            line >> trash >> scores[0] >> scores[1] >> scores[2] >> scores[3];
            int maxIndex = 0;
            int maxScore = scores[0];
            for (int i = 1; i < 4; ++i) {
                if (scores[i] > maxScore) {
                    maxIndex = i;
                    maxScore = scores[i];
                }
            }
            if (maxIndex == 0) ++wins;
        }
    }
    float winrate = 100.0f*(float)wins/(float)n;
    printf("Your winrate is %f\n", winrate);
}