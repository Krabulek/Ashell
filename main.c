#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#define delimitory " \t\r\n\a"

//kolory
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

int ASHELL_rozpocznij(char **argumenty);
int ASHELL_wykonaj(char **argumenty);

//wbudowane funkcje
int ASHELL_help(char **argumenty);
int ASHELL_cd(char **argumenty);
int ASHELL_echo(char **argumenty);
int ASHELL_pwd(char **argumenty);
int ASHELL_whoami(char **argumenty);
int ASHELL_hostname(char **argumenty);
int ASHELL_exit(char **argumenty);

char host [100];
char path [1024];

char *wbudowane[] = {"help", "cd", "echo", "pwd", "whoami", "hostname", "exit"};

int (*wbudowaneFunkcje[]) (char **) = {&ASHELL_help, &ASHELL_cd, &ASHELL_echo, &ASHELL_pwd, &ASHELL_whoami, &ASHELL_hostname, &ASHELL_exit};

int main(int argc, char **argv) {

    printf("\n============================MICROSHELL=============================\n\n");

    while (1) {
        char *login = getlogin();
        gethostname(host, sizeof(host));
        getcwd(path, sizeof (path));

        printf(RED "%s@%s" RESET, login, host);
        printf(BLUE "[%s] $ " BLUE, path);

        //wczytywanie linii

        char *linia = NULL;
        ssize_t bufor = getline(&linia, &bufor, stdin);
        int indeks = 0;
        char **argumenty = malloc(1024);
        char *argument;

        if (bufor <= 1) {
            fprintf(stderr, "ASHELL: Wpisz komendę.\n");
            continue;
        }

        if (linia[bufor - 1] == '\n') {
            linia[bufor - 1] = 0;
        }

        //rozdzielanie linii

        argument = strtok(linia, delimitory);

        while (argument != NULL) {
            argumenty[indeks] = argument;
            indeks++;
            argument = strtok(NULL, delimitory);
        }

        argumenty[indeks] = NULL;

        ASHELL_wykonaj(argumenty);

        free(linia);
        free(argumenty);
    }

    return EXIT_SUCCESS;
}

int ASHELL_wykonaj(char **argumenty) {
    int i;

    if (argumenty[0] == NULL) {
        printf("ASHELL: Wpisz komendę.\n");
        return 1;
    }

    for (i = 0; i < (int)(sizeof(wbudowane) / sizeof (char *)); i++) {

        if (strcmp(argumenty[0], wbudowane[i]) == 0) {
            return (*wbudowaneFunkcje[i]) (argumenty);
        }
    }

    return ASHELL_rozpocznij(argumenty);
}

int ASHELL_rozpocznij(char **argumenty) {
    int status;
    pid_t pid = fork();

    if (pid == 0) { //DZIECKO

        if (execvp(argumenty[0], argumenty) == -1) {
            fprintf(stderr, "ASHELL: Nie ma takiej komendy.\n");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0){
        perror("ASHELL: Wystąpiły błędy forkowania.\n ");
    }
    else { //RODZIC
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int ASHELL_help(char **argumenty) {
    int i;

    printf("\n\n========================Autor: Anna Cielas=========================\n\n");
    printf("ASHELL: Wpisz nazwę programu (bez lub z argumentami), a następnie nacisnij enter.\n");
    printf("Programy wbudowane w ASHELL:\n");

    for (i = 0; i < (int)(sizeof(wbudowane) / sizeof (char *)); i++) {
        printf("            $ %s\n", wbudowane[i]);
    }

    printf("ASHELL: Aby dowiedzieć się wiecej o działaniu innych programów, użyj komendy man + nazwa_programu.\n");
    return 1;
}

int ASHELL_cd(char **argumenty) {

    if (argumenty[1] == NULL) {
        fprintf(stderr, "ASHELL: Podaj argument komendy cd.\n");
    }
    else {
        if (chdir(argumenty[1]) != 0) {
            fprintf(stderr, "ASHELL: Nie ma takiego katalogu.\n");
        }
    }
    return 1;
}

int ASHELL_echo(char **argumenty){
    int i = 1;
    char *slowo = argumenty[i];

    while (slowo) {
        if (slowo[0] == '$' && (slowo + 1) != NULL) {
            printf("%s ", getenv(slowo + 1));
        } else if (slowo[0] == '$' && (slowo + 1) == NULL) {
            printf("$ ");
        } else {
            printf("%s ", slowo);
        }
        slowo = argumenty[++i];
    }

    if (argumenty[1] != NULL) {
        printf("\n");
    }

    return 1;
}

int ASHELL_pwd(char **argumenty) {
    getcwd(path, sizeof(path));
    printf("Ścieżka katalogu bieżącego: %s\n", path);
    return 1;
}

int ASHELL_whoami(char **argumenty) {
    char *login = getlogin();
    printf("%s\n", login);
    return 1;
}

int ASHELL_hostname(char **argumenty) {
    gethostname(host, sizeof(host));
    printf("%s\n", host);
    return 1;
}

int ASHELL_exit(char **argumenty) {
    exit(EXIT_SUCCESS);
}
