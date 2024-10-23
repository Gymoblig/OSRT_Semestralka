# Nastavenie premennej pre názvy súborov
SERVER = Server
CLIENT = Client

# Nastavenie kompilátora
CC = gcc

# Nastavenie praparametrov pre kompiláciu
CFLAGS_SERVER = -lpthread -lm
CFLAGS_CLIENT = 

# Pravidlá pre kompiláciu servera a klienta
all: $(SERVER) $(CLIENT)

$(SERVER): server_OSRT.c
	$(CC) -o $(SERVER) server_OSRT.c $(CFLAGS_SERVER)

$(CLIENT): client_OSRT.c
	$(CC) -o $(CLIENT) client_OSRT.c $(CFLAGS_CLIENT)

# Pravidlo na spustenie servera a klienta
spustit: all
	xterm -hold -fa 'Monospace' -fs 14 -e ./$(SERVER) &  # Unbuffered output for server
	sleep 1;
	xterm -hold -fa 'Monospace' -fs 14 -e ./$(CLIENT) &  # Unbuffered output for client

# Pravidlo na vyčistenie súborov
clean:
	rm -f $(SERVER) $(CLIENT)
