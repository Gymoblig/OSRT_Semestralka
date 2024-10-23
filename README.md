
# Výpočet bodov - OSRT

## Obsah
- [Úvod](#úvod)
- [Funkcionalita](#funkcionalita)
- [Inštalácia](#inštalácia)
- [Spustenie](#spustenie)
- [Ovládanie](#ovládanie)


## Úvod
Tento projekt predstavuje implementáciu servera a klienta, ktorý si vymieňa súradnice bodov a vypočíta vzdialenosť medzi nimi. Používa multithreading na spracovanie prichádzajúcich požiadaviek a zabezpečuje komunikáciu prostredníctvom socketov.

## Funkcionalita
- Server: Počúva na prichádzajúce spojenia, prijíma súradnice bodov od klienta a vypočíta vzdialenosť medzi nimi.
- Klient: Odosiela súradnice bodov na server a prijíma výslednú vzdialenosť.
- Používa sa alarm na zabezpečenie správneho načítania vstupu.
- Implementované sú mutexy na synchronizáciu prístupu k zdieľaným premenným.

## Inštalácia
Ako prvé treba si nainštalovať Linux distribúciu. Počas vypracovania bolo použité WSL Ubuntu z [Microsoft Store](https://apps.microsoft.com/home?hl=en-US&gl=US)

---
Ak toto máme splnené tak môžeme pokračovať nasledovne:
1. **Nainštalujeme GCC (GNU Compiler Collection)**, ak ho ešte nemáme nainštalovaný. Na Ubuntu môžeme spustiť nasledujúce príkazy:
   ```bash
   sudo apt-get update
   sudo apt install gcc
   sudo apt install xterm
   ```

2. **Naklonujeme repozitár** s kódom:
   ```bash
   git clone 
   ```

3. **Prejdeme do adresára projektu**:
   ```bash
   cd názov
   ```

## Spustenie
1. Skontrolujeme a nastavíme premenné pre názvy xterm okien a aj súborov v súbore `Makefile`:
    ```bash
    SERVER = Server
    CLIENT = Client
    ```
2. Skontrolujeme nastavenie kompilátora **GCC** a aj jeho parametrov:
    ```bash
    CC = gcc
    CFLAGS_SERVER = -lpthread -lm
    CFLAGS_CLIENT =
    ``` 
3. Spustíme kompiláciu:
    ```bash
    make
    ```
    alebo môžeme rovno zkompilovať a spustiť server a následne klienta:
    ```bash
    make spustit
    ```
    ---
*Pri použití `make clean` sa vymaže zkompilovaný server a client*


## Ovládanie
- Po spustení pomocou príkazu `make spustit` v okne xterm Client môžeme zadať súradnice.
- Klient odošle súradnice na server, ktorý vypočíta vzdialenosť a zobrazí ju vspäť v xterm klienta.
- Pre zastavenie servera a ukončenie programu stačí stlačiť `CTRL + C`.
