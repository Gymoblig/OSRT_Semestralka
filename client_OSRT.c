#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h> // Signály na spracovanie

#define PORT 7777
#define CAS 10
// Štruktúra pre súradnice bodu
typedef struct {
    float X;
    float Y;
} Bod;

pthread_mutex_t mutex;

// Globálny flag pre alarm
volatile sig_atomic_t alarm_triggered = 0;

// Funkcia na obsluhu alarmu
void alarmik(int sig) {
    alarm_triggered = 1; // Alarm je aktivovaný
    printf("\n→ ZADAJ TÚ SÚRADNICU: ");
    fflush(stdout);
    alarm(CAS);
}

// Funkcia na obsluhu Ctrl+C
void signalik(int sig) {
    system("pkill xterm"); // Zatvoriť terminál, keď je Ctrl+C stlačené
    exit(0);
}

// Funkcia na načítanie float hodnoty
void nacitajFloat(const char* prompt, float* value) {
    char input[128];

    while (1) {
        alarm(CAS); // Nastaviť alarm na 10 sekúnd
        printf("%s", prompt);
        fflush(stdout); // Uistiť sa, že prompt sa zobrazí

        // Čítanie vstupu
        if (fgets(input, sizeof(input), stdin) != NULL) {
            alarm_triggered = 0; // Reset alarmu

            // Pokúsiť sa previesť na float
            if (sscanf(input, "%f", value) == 1) {
                break; // Validný vstup
            } else {
                printf("Neplatný vstup! Skús to znova.\n");
                fflush(stdout);
            }
        } else {
            printf("Chyba pri načítaní vstupu.\n");
        }
    }

    alarm(0); // Zrušiť alarm
}

// Funkcia na odoslanie súradníc bodu 1
void posliNaServerBod1(Bod bod) {
    int sock_desc = socket(AF_INET, SOCK_STREAM, 0); // Vytvorenie socketu
    if (sock_desc == -1) {
        printf("Nemôžem vytvoriť socket pre Bod 1!\n");
        exit(1);
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);

    if (connect(sock_desc, (struct sockaddr*)&server, sizeof(server)) != 0) {
        printf("Nemôžem sa pripojiť k serveru pre Bod 1!\n");
        close(sock_desc);
        exit(1);
    }

    // Odošle súradnice bodu
    float data[2] = {bod.X, bod.Y}; 
    send(sock_desc, data, sizeof(data), 0);
    sleep(1);
    close(sock_desc); // Zatvorenie socketu
}

// Funkcia na odoslanie súradníc bodu 2
void posliNaServerBod2(Bod bod) {
    int sock_desc = socket(AF_INET, SOCK_STREAM, 0); // Vytvorenie socketu
    if (sock_desc == -1) {
        printf("Nemôžem vytvoriť socket pre Bod 2!\n");
        exit(1);
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);

    if (connect(sock_desc, (struct sockaddr*)&server, sizeof(server)) != 0) {
        printf("Nemôžem sa pripojiť k serveru pre Bod 2!\n");
        close(sock_desc);
        exit(1);
    }

    // Odošle súradnice bodu
    float data[2] = {bod.X, bod.Y}; 
    send(sock_desc, data, sizeof(data), 0);
    sleep(1);
    close(sock_desc); // Zatvorenie socketu
}

int main() {
    int pipefd1[2], pipefd2[2];

    signal(SIGINT, signalik); // Spracovanie Ctrl+C
    signal(SIGALRM, alarmik);  // Spracovanie alarmu

    // Vytvorenie pipes
    if (pipe(pipefd1) == -1 || pipe(pipefd2) == -1) {
        perror("Chyba pri vytváraní pipe");
        exit(1);
    }

    Bod bod1, bod2;

    // Vytvorenie procesov na načítanie súradníc
    pid_t pid1 = fork();
    if (pid1 == 0) { // Proces pre Bod 1
        nacitajFloat("Zadaj X pre bod 1: ", &bod1.X);
        nacitajFloat("Zadaj Y pre bod 1: ", &bod1.Y);

        // Zápis do pipe
        close(pipefd1[0]); // Zavrieť čítaciu časť
        write(pipefd1[1], &bod1, sizeof(Bod));
        close(pipefd1[1]); // Zavrieť zápisovú časť

        exit(0);
    }
    wait(NULL); // Čakanei na koniec procesu

    pid_t pid2 = fork();
    if (pid2 == 0) { // Proces pre Bod 2
        nacitajFloat("Zadaj X pre bod 2: ", &bod2.X);
        nacitajFloat("Zadaj Y pre bod 2: ", &bod2.Y);

        // Zápis do pipe
        close(pipefd2[0]); // Zavrieť čítaciu časť
        write(pipefd2[1], &bod2, sizeof(Bod));
        close(pipefd2[1]); // Zavrieť zápisovú časť

        exit(0);
    }

    wait(NULL); // Čakanie na koniec procesu

    // Rodič číta hodnoty z pipes
    close(pipefd1[1]); // Zavrieť zápisovú časť pre Bod 1
    read(pipefd1[0], &bod1, sizeof(Bod));
    close(pipefd1[0]); // Zavrieť čítaciu časť pre Bod 1

    close(pipefd2[1]); // Zavrieť zápisovú časť pre Bod 2
    read(pipefd2[0], &bod2, sizeof(Bod));
    close(pipefd2[0]); // Zavrieť čítaciu časť pre Bod 2

    // Poslanie dát na server
    pid_t sendpid1 = fork();
    if (sendpid1 == 0) {
        posliNaServerBod1(bod1); // Odošle Bod 1
        exit(0);
    }
    wait(NULL);

    pid_t sendpid2 = fork();
    if (sendpid2 == 0) {
        posliNaServerBod2(bod2); // Odošle Bod 2
        exit(0);
    }

    wait(NULL); // Čakanie na koniec procesu

    // Pripojenie k serveru na prijatie vzdialenosti
    int sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1) {
        printf("Nemôžem vytvoriť socket na prijímanie vzdialenosti!\n");
        exit(1);
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);

    if (connect(sock_desc, (struct sockaddr*)&server, sizeof(server)) != 0) {
        printf("Nemôžem sa pripojiť k serveru na prijímanie vzdialenosti!\n");
        close(sock_desc);
        exit(1);
    }

    // Prijatie vzdialenosti
    float vzdialenost;
    recv(sock_desc, &vzdialenost, sizeof(vzdialenost), 0);
    printf("Vzdialenosť medzi bodmi je: %.2f\n", vzdialenost);

    close(sock_desc); // Zatvorenie socketu
    printf("Pre ukončenie stlač CTRL+C\n");
    getchar();
    return 0;
}
