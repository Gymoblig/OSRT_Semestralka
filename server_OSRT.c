#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h> // Na sqrt
#include <pthread.h> // Na pthreads
#include <signal.h> // Na signály

#define PORT 7777 // Port, na ktorom server počúva

// Globálne premenne na body
float bod1[2], bod2[2];

// Mutex na synchronizáciu prístupu
pthread_mutex_t mutex;

float vypocitajVzdialenost(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2)); // Vzdialenosť
}

// Vlákno na prijímanie bodu
void* prijmiBod(void* sock_desc) {
    int temp_sock_desc = *(int*)sock_desc; // Dočasný socket
    float bod[2]; // Pre prijatý bod

    // Prijatie bodu
    recv(temp_sock_desc, bod, sizeof(bod), 0);
    
    pthread_mutex_lock(&mutex); // Zamykanie mutexu

    // Uloženie prijatého bodu
    if (bod1[0] == 0 && bod1[1] == 0) {
        bod1[0] = bod[0];
        bod1[1] = bod[1];
        printf("Prijatý bod 1: X = %.2f, Y = %.2f\n", bod1[0], bod1[1]);
    } else {
        bod2[0] = bod[0];
        bod2[1] = bod[1];
        printf("Prijatý bod 2: X = %.2f, Y = %.2f\n", bod2[0], bod2[1]);
    }

    pthread_mutex_unlock(&mutex); // Odomykanie mutexu
    
    close(temp_sock_desc); // Zatvorenie socketu
    
    printf("→ Vlákno na prijatie bodu dokončilo prácu.\n");
}

int main() {
    // Vytvorenie socketu
    int sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_desc == -1) {
        printf("Nepodarilo sa vytvoriť socket!\n");
        return 0;
    }

    int opt = 1; // Pre opätovné použitie adresy
    if (setsockopt(sock_desc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt zlyhalo");
        close(sock_desc);
        return 0;
    }

    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // Prijíma pripojenia z akéhokoľvek rozhrania
    server.sin_port = htons(PORT); // Prevod portu na sieťový poriadok bajtov

    // Pripojenie socketu
    if (bind(sock_desc, (struct sockaddr*)&server, sizeof(server)) != 0) {
        printf("Nepodarilo sa pripojiť socket!\n");
        close(sock_desc);
        return 0;
    }

    // Počúvanie na socketu
    if (listen(sock_desc, 20) != 0) {
        printf("Nepodarilo sa počúvať na socket!\n");
        close(sock_desc);
        return 0;
    }

    printf("Server je pripravený, čaká na pripojenie...\n");

    struct sockaddr_in client; // Pre klienta
    socklen_t len = sizeof(client);

    pthread_t threads[2]; // Pole pre ID vlákien

    // Akceptovanie pripojení a vytvorenie vlákien
    for (int i = 0; i < 2; i++) {
        int temp_sock_desc = accept(sock_desc, (struct sockaddr*)&client, &len);
        if (temp_sock_desc == -1) {
            printf("Nepodarilo sa akceptovať klienta!\n");
            close(sock_desc);
            return 0;
        }
        pthread_create(&threads[i], NULL, prijmiBod, (void*)&temp_sock_desc); // Vytvorenie vlákna
        printf("→ Vlákno vytvorené na prijatie bodu %d.\n", i + 1);
    }

    // Čakanie na dokončenie vlákien
    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL); // Čakanie na vlákno
        printf("→ Vlákno %d sa úspešne dokončilo.\n", i + 1);
    }

    // Výpočet vzdialenosti
    float vzdialenost = vypocitajVzdialenost(bod1[0], bod1[1], bod2[0], bod2[1]);
    printf("Vzdialenosť medzi bodmi je: %.2f\n", vzdialenost);

    // Akceptovanie pripojenia na odoslanie vzdialenosti
    int temp_sock_desc = accept(sock_desc, (struct sockaddr*)&client, &len);
    if (temp_sock_desc == -1) {
        printf("→ Nepodarilo sa akceptovať klienta pre výsledok vzdialenosti!\n");
        close(sock_desc);
        return 0;
    }
    // Odoslanie vzdialenosti klientovi
    send(temp_sock_desc, &vzdialenost, sizeof(vzdialenost), 0);
    printf("Vzdialenosť odoslaná klientovi.\n");

    // Uzavretie serverového socketu
    close(temp_sock_desc);
    close(sock_desc);
    return 0;
}
